# Recorder for the AllCosmos API videos. Runs ON the hidden desktop next to the app (started through gui-server).
# Each 1/Fps s: 1920x1080 frame = caption bar (<Dir>\caption.txt, <Dir>\subtitle.txt) + the app's windows (left)
# + a panel (right): scale weight, newest AcsApiRequestLog/AcsApiResponseLog row, flags of the newest test ticket.
# <Dir>\app.pid = app process id; <Dir>\rec-stop.txt = stop. Frames go to <Dir>\<Take>\f00001.jpg ...
param([string]$Dir, [string]$Take = 'frames', [int]$Fps = 5, [string]$ProviderFile, [string]$WeightFile)
Add-Type -AssemblyName System.Drawing
Add-Type -Name W -Namespace AcsRec -MemberDefinition @'
public delegate bool EnumProc(System.IntPtr h, System.IntPtr l);
[System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, System.IntPtr l);
[System.Runtime.InteropServices.DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(System.IntPtr h, out uint p);
[System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool IsWindowVisible(System.IntPtr h);
[System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool IsHungAppWindow(System.IntPtr h);
[System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool GetWindowRect(System.IntPtr h, out RECT r);
[System.Runtime.InteropServices.DllImport("user32.dll")] public static extern bool PrintWindow(System.IntPtr h, System.IntPtr hdc, uint f);
public struct RECT { public int Left, Top, Right, Bottom; }
'@
$frames = Join-Path $Dir $Take; New-Item -ItemType Directory -Force $frames | Out-Null
$startTs = (Get-Date).AddSeconds(-2).ToString('yyyy-MM-dd HH:mm:ss')
$W = 1920; $H = 1080; $capH = 96; $panelW = 600; $appW = $W - $panelW
$jpg = [Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object MimeType -eq 'image/jpeg'
$ep = New-Object Drawing.Imaging.EncoderParameters 1; $ep.Param[0] = New-Object Drawing.Imaging.EncoderParameter ([Drawing.Imaging.Encoder]::Quality), 90L
$line = Get-Content $ProviderFile -TotalCount 1
$cs = "Server=192.168.0.10,1433;Database=allcosmos suasa;User ID=$([regex]::Match($line, 'User ID=([^;]*)').Groups[1].Value);Password=$([regex]::Match($line, 'Password=([^;]*)').Groups[1].Value);Connect Timeout=5"
$font = { param($s, $b) New-Object Drawing.Font('Segoe UI', $s, $(if ($b) { [Drawing.FontStyle]::Bold } else { [Drawing.FontStyle]::Regular }), [Drawing.GraphicsUnit]::Pixel) }
$mono = New-Object Drawing.Font('Consolas', 14, [Drawing.FontStyle]::Regular, [Drawing.GraphicsUnit]::Pixel)
$cBg = [Drawing.Color]::FromArgb(241,244,248); $cBar = [Drawing.Color]::FromArgb(24,35,56); $cText = [Drawing.Color]::FromArgb(30,41,59)
$cMuted = [Drawing.Color]::FromArgb(100,116,139); $cGreen = [Drawing.Color]::FromArgb(22,163,74); $cRed = [Drawing.Color]::FromArgb(220,38,38)
$cBlue = [Drawing.Color]::FromArgb(37,99,235); $cGrey = [Drawing.Color]::FromArgb(148,163,184)

function Read1([string]$f, [string]$d = '') { try { (Get-Content $f -ErrorAction Stop | Select-Object -First 1) } catch { $d } }
function AppWindows([uint32]$appPid) {
    $list = New-Object System.Collections.Generic.List[IntPtr]
    [AcsRec.W]::EnumWindows({ param($h, $l) $p = 0; [void][AcsRec.W]::GetWindowThreadProcessId($h, [ref]$p)
        if ($p -eq $appPid -and [AcsRec.W]::IsWindowVisible($h)) { $list.Add($h) }; $true }, [IntPtr]::Zero) | Out-Null
    $list
}
function Grab([IntPtr]$h) {
    $r = New-Object AcsRec.W+RECT; [void][AcsRec.W]::GetWindowRect($h, [ref]$r)
    $w = $r.Right - $r.Left; $hh = $r.Bottom - $r.Top
    if ($w -lt 120 -or $hh -lt 80 -or [AcsRec.W]::IsHungAppWindow($h)) { return $null }
    $b = New-Object Drawing.Bitmap $w, $hh; $g = [Drawing.Graphics]::FromImage($b); $dc = $g.GetHdc()
    [void][AcsRec.W]::PrintWindow($h, $dc, 2); $g.ReleaseHdc($dc); $g.Dispose()
    [pscustomobject]@{ Bmp = $b; R = $r }
}
# The app's windows composed at their real positions, cropped to the area they cover (the main window is full screen).
function AppImage {
    $appPid = [uint32](Read1 (Join-Path $Dir 'app.pid') '0'); if (-not $appPid) { return $null }
    $shots = @(AppWindows $appPid | ForEach-Object { Grab $_ } | Where-Object { $_ })
    if (-not $shots) { return $null }
    $main = $shots | Sort-Object { ($_.R.Right - $_.R.Left) * ($_.R.Bottom - $_.R.Top) } -Descending | Select-Object -First 1
    $img = New-Object Drawing.Bitmap $main.Bmp.Width, $main.Bmp.Height; $g = [Drawing.Graphics]::FromImage($img)
    $g.DrawImage($main.Bmp, 0, 0)
    $dialogs = @($shots | Where-Object { $_ -ne $main })
    # EnumWindows lists the topmost window first: draw from the bottom up so the active dialog ends on top
    for ($i = $dialogs.Count - 1; $i -ge 0; $i--) {
        $s = $dialogs[$i]
        $g.DrawImage($s.Bmp, $s.R.Left - $main.R.Left, $s.R.Top - $main.R.Top)
        $g.DrawRectangle((New-Object Drawing.Pen $cBlue, 3), $s.R.Left - $main.R.Left, $s.R.Top - $main.R.Top, $s.Bmp.Width - 1, $s.Bmp.Height - 1)
    }
    $g.Dispose(); $shots | ForEach-Object { $_.Bmp.Dispose() }
    if ($dialogs.Count -gt 0) {
        # zoom on the dialogs (plus a margin) so their text stays readable
        $l = ($dialogs | ForEach-Object { $_.R.Left } | Measure-Object -Minimum).Minimum - $main.R.Left - 40
        $t = ($dialogs | ForEach-Object { $_.R.Top } | Measure-Object -Minimum).Minimum - $main.R.Top - 40
        $r = ($dialogs | ForEach-Object { $_.R.Right } | Measure-Object -Maximum).Maximum - $main.R.Left + 40
        $b = ($dialogs | ForEach-Object { $_.R.Bottom } | Measure-Object -Maximum).Maximum - $main.R.Top + 40
        $l = [Math]::Max(0, $l); $t = [Math]::Max(0, $t); $r = [Math]::Min($img.Width, $r); $b = [Math]::Min($img.Height, $b)
        $crop = $img.Clone((New-Object Drawing.Rectangle $l, $t, ($r - $l), ($b - $t)), $img.PixelFormat); $img.Dispose(); return $crop
    }
    $img
}
$script:req = $null; $script:tk = $null; $script:dbAt = [datetime]::MinValue
function Db {
    if (((Get-Date) - $script:dbAt).TotalMilliseconds -lt 700) { return }
    $script:dbAt = Get-Date
    try {
        $cn = New-Object System.Data.SqlClient.SqlConnection $cs; $cn.Open(); $c = $cn.CreateCommand()
        $c.CommandText = "select top 1 q.ENDPOINT, q.TICKET_NO, cast(q.REQUEST_DATA as nvarchar(2000)), convert(varchar(19), q.REQUEST_TS, 120), s.IS_SUCCESS, isnull(s.MESSAGE, '') from AcsApiRequestLog q left join AcsApiResponseLog s on s.REQUEST_PKKEY = q.PKKEY where q.REQUEST_TS >= '$startTs' order by q.PKKEY desc"
        $r = $c.ExecuteReader(); $script:req = $null
        if ($r.Read()) { $script:req = [pscustomobject]@{ Ep = $r.GetString(0); Ticket = $r.GetString(1); Json = $r.GetString(2); Ts = $r.GetString(3); Ok = $(if ($r.IsDBNull(4)) { $null } else { $r.GetBoolean(4) }); Msg = $r.GetString(5) } }
        $r.Close()
        if ($script:req) {
            $t = $script:req.Ticket.Replace("'", "''")
            $c.CommandText = "select top 1 t, isnull(API_REF_NO,''), isnull(API_IN_OK,'-'), isnull(API_OUT_OK,'-') from (select 'FIRST' t, API_REF_NO, API_IN_OK, API_OUT_OK from [first] where TICKET = '$t' union all select 'WB', API_REF_NO, API_IN_OK, API_OUT_OK from wb where TICKET = '$t' union all select 'DESPATCH', API_REF_NO, API_IN_OK, API_OUT_OK from despatch where TICKET = '$t') x order by case t when 'FIRST' then 2 else 1 end"
            $r = $c.ExecuteReader(); $script:tk = $null
            if ($r.Read()) { $script:tk = "{0}  {1}   ref {2}   IN_OK={3}  OUT_OK={4}" -f $r.GetString(0), $script:req.Ticket, $r.GetString(1), $r.GetString(2), $r.GetString(3) }
            $r.Close()
        }
        $cn.Close()
    } catch { }
}
function Box($g, $x, $y, $w, $h, $fill, $border) {
    $g.FillRectangle((New-Object Drawing.SolidBrush $fill), $x, $y, $w, $h)
    if ($border) { $g.DrawRectangle((New-Object Drawing.Pen $border, 2), $x, $y, $w, $h) }
}
function Txt($g, $t, $x, $y, $size, $bold, $color) { $g.DrawString($t, (& $font $size $bold), (New-Object Drawing.SolidBrush $color), [single]$x, [single]$y) }
function Wrap($g, $t, $f, $x, $y, $w, $h, $color) { $g.DrawString($t, $f, (New-Object Drawing.SolidBrush $color), (New-Object Drawing.RectangleF ([single]$x), ([single]$y), ([single]$w), ([single]$h))) }
function Panel($g) {
    $x0 = $appW; $y = $capH; $bw = $panelW - 48
    Box $g $x0 $y $panelW ($H - $capH) ([Drawing.Color]::White) $null
    $g.DrawLine((New-Object Drawing.Pen $cGrey, 2), $x0, $y, $x0, $H)
    $x = $x0 + 24; $y += 18
    $kg = [int](Read1 $WeightFile '0')
    Txt $g 'WEIGHBRIDGE (simulated indicator)' $x $y 15 $true $cMuted; $y += 24
    Txt $g ('{0:N0} kg' -f $kg) $x $y 34 $true $cText; $y += 60
    Db
    Txt $g 'ALLCOSMOS API  -  LAST CALL (AcsApiRequestLog)' $x $y 15 $true $cMuted; $y += 26
    if ($script:req) {
        Box $g $x $y $bw 300 $cBg $cGrey
        Txt $g ("POST /api/{0}" -f $script:req.Ep) ($x + 12) ($y + 8) 22 $true $cBlue
        Txt $g ("sent {0}" -f $script:req.Ts) ($x + 12) ($y + 40) 14 $false $cMuted
        $json = $script:req.Json -replace ',"', ",`n `"" -replace '^\{', "{`n " -replace '\}$', "`n}"
        Wrap $g $json $mono ($x + 12) ($y + 64) ($bw - 24) 230 $cText
        $y += 316
        Txt $g 'WEBSITE REPLY (AcsApiResponseLog)' $x $y 15 $true $cMuted; $y += 26
        $ok = $script:req.Ok; $col = $(if ($ok -eq $true) { $cGreen } elseif ($ok -eq $false) { $cRed } else { $cGrey })
        Box $g $x $y $bw 210 $cBg $col
        Txt $g $(if ($ok -eq $true) { 'isSuccess: true' } elseif ($ok -eq $false) { 'isSuccess: false' } else { 'waiting for reply...' }) ($x + 12) ($y + 8) 22 $true $col
        Wrap $g $script:req.Msg (& $font 16 $false) ($x + 12) ($y + 42) ($bw - 24) 164 $cText
        $y += 228
        if ($script:tk) { Txt $g 'TICKET IN WEIGHBRIDGE DB' $x $y 15 $true $cMuted; $y += 24; Wrap $g $script:tk (& $font 16 $true) $x $y $bw 50 $cText }
    } else { Box $g $x $y $bw 80 $cBg $cGrey; Txt $g 'no API call yet in this clip' ($x + 12) ($y + 26) 18 $false $cMuted }
    Txt $g (Get-Date -Format 'dd/MM/yyyy  HH:mm:ss') $x ($H - 36) 15 $false $cMuted
}

$n = 0; $sw = [Diagnostics.Stopwatch]::StartNew()
while (-not (Test-Path (Join-Path $Dir 'rec-stop.txt'))) {
    $frame = New-Object Drawing.Bitmap $W, $H; $g = [Drawing.Graphics]::FromImage($frame)
    $g.SmoothingMode = 'AntiAlias'; $g.TextRenderingHint = 'ClearTypeGridFit'; $g.InterpolationMode = 'HighQualityBicubic'
    $g.Clear($cBg)
    $g.FillRectangle((New-Object Drawing.SolidBrush $cBar), 0, 0, $W, $capH)
    Txt $g (Read1 (Join-Path $Dir 'caption.txt') 'AllCosmos weighbridge API') 24 12 32 $true ([Drawing.Color]::White)
    Txt $g (Read1 (Join-Path $Dir 'subtitle.txt') '') 26 58 18 $false ([Drawing.Color]::FromArgb(226,232,240))
    $img = AppImage
    if ($img) {
        $s = [Math]::Min(($appW - 20) / $img.Width, ($H - $capH - 20) / $img.Height); $s = [Math]::Min($s, 1.6)
        $dw = [int]($img.Width * $s); $dh = [int]($img.Height * $s)
        $g.DrawImage($img, [int](($appW - $dw) / 2), $capH + [int](($H - $capH - $dh) / 2), $dw, $dh); $img.Dispose()
    }
    Panel $g
    $g.Dispose()
    $due = [int][Math]::Floor($sw.Elapsed.TotalSeconds * $Fps) + 1
    $file = Join-Path $frames ('f{0:D5}.jpg' -f ($n + 1))
    $frame.Save($file, $jpg, $ep); $frame.Dispose(); $n++
    while ($n -lt $due) { Copy-Item $file (Join-Path $frames ('f{0:D5}.jpg' -f ($n + 1))); $n++ }
    $wait = [int](($n / $Fps) * 1000 - $sw.Elapsed.TotalMilliseconds); if ($wait -gt 0) { Start-Sleep -Milliseconds $wait }
}
"$n frames"
