<#
.SYNOPSIS
    Headless build driver for a VC6 (Visual Studio 98) .dsp/.dsw project, using MSDEV's
    batch-build command line instead of opening the IDE UI.

.DESCRIPTION
    Sets INCLUDE/LIB/PATH for the VC98 toolchain plus this project's shared library trees,
    then drives MSDEV.EXE in batch mode (/MAKE /USEENV /OUT) to compile+link the requested
    configuration. Parses the build log for the "0 error(s)" summary line rather than
    trusting MSDEV's process exit code alone (old MSDEV batch builds are not always
    reliable about a non-zero exit code on failure).

.PARAMETER Config
    "Debug" or "Release" (must match a config name in the .dsp, e.g. "menu - Win32 Debug").

.PARAMETER Rebuild
    Force a full rebuild instead of an incremental build.

.PARAMETER Run
    After a successful build, launch the resulting exe briefly as a smoke test (confirms
    it starts and doesn't immediately crash from a missing DLL/dependency), then close it.

.PARAMETER ProjectFile
    Path to the .dsp. Defaults to the repo root's application .dsp (not a static-library
    one). Projects it depends on in the matching .dsw are built first, same config.

.EXAMPLE
    .vscode\vc6-build.ps1 -Config Debug
    .vscode\vc6-build.ps1 -Config Release -Rebuild
    .vscode\vc6-build.ps1 -Config Debug -Run
#>
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Config = 'Debug',

    [switch]$Rebuild,

    [switch]$Run,

    [string]$ProjectFile,

    # Left empty = auto-detected below (KELVIN: D:\Software\VC98 + E:\ libs; Chris-PC: Program Files VC98 + C:\ libs).
    [string]$VC98Root,
    [string]$MsdevPath,
    [string[]]$ExtraIncludeDirs,
    [string[]]$ExtraLibDirs,
    # Modern (VS 2017+) x86 link.exe for the VS Code debug relink; auto-detected via vswhere if empty.
    [string]$ModernLinker,

    [int]$TimeoutSeconds = 600
)

$ErrorActionPreference = 'Stop'

$RepoRoot = Split-Path -Parent $PSScriptRoot

if (-not $ProjectFile) {
    $dsps = @(Get-ChildItem -Path $RepoRoot -Filter '*.dsp')
    if ($dsps.Count -eq 0) { throw "No .dsp file found in $RepoRoot. Pass -ProjectFile explicitly." }
    $app = $dsps | Where-Object { (Get-Content $_.FullName -Raw) -match 'TARGTYPE "Win32 \(x86\) Application"' } | Select-Object -First 1
    $ProjectFile = if ($app) { $app.FullName } else { $dsps[0].FullName }
}
$ProjectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectFile)

# Projects the main one depends on in the .dsw (e.g. a static library it links).
$DepProjects = @()
$dsw = Get-ChildItem -Path (Split-Path $ProjectFile) -Filter '*.dsw' | Select-Object -First 1
if ($dsw) {
    $dswText = Get-Content $dsw.FullName -Raw
    $block = ($dswText -split '#{10,}') | Where-Object { $_ -match "Project: `"$ProjectName`"=" } | Select-Object -First 1
    foreach ($m in [regex]::Matches([string]$block, 'Project_Dep_Name\s+(\S+)')) {
        $depName = $m.Groups[1].Value
        $depLine = [regex]::Match($dswText, "Project: `"$depName`"=\.\\([^ ]+\.dsp)")
        if ($depLine.Success) {
            $DepProjects += (Join-Path (Split-Path $dsw.FullName) $depLine.Groups[1].Value)
        }
    }
}

function Find-First([string[]]$Candidates, [string]$Probe) {
    foreach ($c in $Candidates) { if (Test-Path (Join-Path $c $Probe)) { return $c } }
    return $null
}

if (-not $VC98Root) {
    $VC98Root = Find-First @('D:\Software\VC98',
                             "${env:ProgramFiles(x86)}\Microsoft Visual Studio\VC98",
                             'C:\VC6\VC98') 'Bin\CL.EXE'
    if (-not $VC98Root) { throw "VC98 not found (looked in D:\Software\VC98 and Program Files). Pass -VC98Root." }
}
if (-not $MsdevPath) {
    # Standalone copy keeps MSDEV under VC98; a standard VS6 install keeps it under ..\Common.
    $MsdevPath = @("$VC98Root\MSDev98\Bin\MSDEV.EXE", "$VC98Root\..\Common\MSDev98\Bin\MSDEV.EXE") |
        Where-Object { Test-Path $_ } | Select-Object -First 1
}
if (-not $ExtraIncludeDirs -or -not $ExtraLibDirs) {
    $libRoot = Find-First @('E:\', 'C:\', '\\kelvin\') 'wlib32'
    if (-not $libRoot) { throw "wlib32/winlib/wtlib32 not found on E:\, C:\ or \\kelvin. Pass -ExtraIncludeDirs/-ExtraLibDirs." }
    $libDirs = @('wlib32', 'winlib', 'wtlib32') | ForEach-Object { "$libRoot$_" }
    if (-not $ExtraIncludeDirs) { $ExtraIncludeDirs = $libDirs }
    if (-not $ExtraLibDirs) { $ExtraLibDirs = $libDirs }
}

$Msdev = $MsdevPath
if (-not $Msdev -or -not (Test-Path $Msdev)) { throw "MSDEV.EXE not found under $VC98Root. Pass -MsdevPath." }

# --- Toolchain environment (mirrors VCVARS32.BAT, plus this project's extra include/lib dirs) ---
$env:PATH = "$VC98Root\MSDev98\Bin;$VC98Root\Bin;$VC98Root\..\Common\MSDev98\Bin;$env:PATH"
$env:INCLUDE = @(
    "$VC98Root\Include",
    "$VC98Root\MFC\Include",
    "$VC98Root\ATL\Include",
    "$VC98Root\MFC\Src"
) + $ExtraIncludeDirs -join ';'
$env:LIB = @(
    "$VC98Root\Lib",
    "$VC98Root\MFC\Lib"
) + $ExtraLibDirs -join ';'

function Invoke-MsdevBuild([string]$Dsp) {
    $Name = [System.IO.Path]::GetFileNameWithoutExtension($Dsp)
    $ConfigTarget = "$Name - Win32 $Config"
    $LogFile = Join-Path $RepoRoot $(if ($Name -eq $ProjectName) { "build-$Config.log" } else { "build-$Name-$Config.log" })
    if (Test-Path $LogFile) { Remove-Item $LogFile -Force }

    $MsdevArgs = @("`"$Dsp`"", '/MAKE', "`"$ConfigTarget`"")
    if ($Rebuild) { $MsdevArgs += '/REBUILD' }
    $MsdevArgs += @('/USEENV', '/OUT', "`"$LogFile`"")

    Write-Host "Building '$ConfigTarget' ..." -ForegroundColor Cyan
    Write-Host "  msdev  : $Msdev"
    Write-Host "  project: $Dsp"
    Write-Host "  log    : $LogFile"

    $proc = Start-Process -FilePath $Msdev -ArgumentList $MsdevArgs -PassThru -WindowStyle Hidden
    if (-not $proc.WaitForExit($TimeoutSeconds * 1000)) {
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        throw "MSDEV did not finish within $TimeoutSeconds seconds (a blocking dialog may have appeared) - killed it. Check $LogFile for partial output."
    }

    $logText = if (Test-Path $LogFile) { Get-Content $LogFile -Raw } else { '' }
    Write-Host $logText

    # MSDEV's own exit code is not reliable for batch builds; trust the "N error(s)" summary line.
    $summaryMatch = [regex]::Match($logText, '(\d+)\s+error\(s\),\s*(\d+)\s+warning\(s\)')
    if (-not $summaryMatch.Success) {
        throw "Could not find a build summary line in $LogFile - treat as failed. (MSDEV exit code: $($proc.ExitCode))"
    }
    $errorCount = [int]$summaryMatch.Groups[1].Value
    $warningCount = [int]$summaryMatch.Groups[2].Value

    if ($errorCount -gt 0) {
        throw "Build failed: $errorCount error(s), $warningCount warning(s). See $LogFile"
    }
    Write-Host "Build succeeded ($Name): 0 errors, $warningCount warning(s)." -ForegroundColor Green
}

# F5 must always pick up code changes: stop a running copy that locks the exe, and do a full rebuild
# when a header / resource / project file is newer than the exe (an incremental build can miss those).
$ExePath = Join-Path (Join-Path $RepoRoot $Config) "$ProjectName.exe"
Get-Process -Name $ProjectName -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $ExePath } | ForEach-Object {
    Write-Host "Stopping running $ProjectName.exe (PID $($_.Id)) so it can be rebuilt ..." -ForegroundColor Yellow
    Stop-Process -Id $_.Id -Force
    $_.WaitForExit(5000) | Out-Null
}
if (-not $Rebuild -and (Test-Path $ExePath)) {
    $exeTime = (Get-Item $ExePath).LastWriteTime
    $changed = @(Get-ChildItem -Path "$RepoRoot\*" -Include *.h, *.rc, *.rc2, *.dsp, *.def -File |
        Where-Object { $_.LastWriteTime -gt $exeTime })
    if ($changed.Count -gt 0) {
        Write-Host "Header/resource/project changed ($($changed[0].Name)$(if ($changed.Count -gt 1) { " and $($changed.Count - 1) more" })) - doing a full rebuild." -ForegroundColor Yellow
        $Rebuild = [switch]$true
    }
}

foreach ($dep in $DepProjects) { Invoke-MsdevBuild $dep }
Invoke-MsdevBuild $ProjectFile

$ExePath = Join-Path (Join-Path $RepoRoot $Config) "$ProjectName.exe"
if (-not (Test-Path $ExePath)) {
    throw "Build reported success but $ExePath does not exist."
}

# VS Code's debugger (vsdbg) ignores VC6's NB10 debug record ("Module was built without
# symbols"), so breakpoints never bind. Relink the same VC6 objects/libs with a modern
# linker to get an RSDS record + PDB 7.0. The PDB gets its own name so VC6's menu.pdb
# stays valid for its next incremental link.
function Get-ModernLinker {
    if ($ModernLinker) { if (Test-Path $ModernLinker) { return $ModernLinker } else { return $null } }
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { return $null }
    foreach ($inst in (& $vswhere -all -products * -property installationPath)) {
        $msvc = Get-ChildItem (Join-Path $inst 'VC\Tools\MSVC') -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending
        foreach ($v in $msvc) {
            $l = Join-Path $v.FullName 'bin\Hostx86\x86\link.exe'
            if (Test-Path $l) { return $l }
        }
    }
    return $null
}

function Test-HasRsds([string]$Exe) {
    $bytes = [System.IO.File]::ReadAllBytes($Exe)
    $text = [System.Text.Encoding]::ASCII.GetString($bytes)
    return $text.Contains("RSDS") -and $text.Contains("$ProjectName.vsdbg.pdb")
}

function Invoke-ModernRelink {
    $link = Get-ModernLinker
    if (-not $link) {
        Write-Warning "No modern MSVC x86 link.exe found - breakpoints will not bind in VS Code."
        return
    }
    $dspText = Get-Content $ProjectFile
    $outDir = Join-Path $RepoRoot $Config

    $cfgStart = [array]::FindIndex([string[]]$dspText, [Predicate[string]]{ param($l) $l -match "CFG\)`" == `"$ProjectName - Win32 $Config`"" })
    $linkLine = $dspText[$cfgStart..($dspText.Count - 1)] | Where-Object { $_ -match '^# ADD LINK32 ' } | Select-Object -First 1
    $linkOpts = ($linkLine -replace '^# ADD LINK32 ', '') -replace '/pdbtype:\S+', ''

    $objs = $dspText | Where-Object { $_ -match '^SOURCE=(.+\.(cpp|c))\s*$' } |
        ForEach-Object { [IO.Path]::GetFileNameWithoutExtension($Matches[1].Trim('"')) } |
        Sort-Object -Unique | ForEach-Object { "`"$outDir\$_.obj`"" }
    $resFiles = $dspText | Where-Object { $_ -match '^SOURCE=(.+\.rc)\s*$' } |
        ForEach-Object { "`"$outDir\$([IO.Path]::GetFileNameWithoutExtension($Matches[1].Trim('"'))).res`"" }

    # Libraries added to the project as files (SOURCE=.\x.lib) are linked by VC6 but not named in the LINK32 line.
    $libFiles = $dspText | Where-Object { $_ -match '^SOURCE=(.+\.lib)\s*$' } |
        ForEach-Object { "`"$($Matches[1].Trim('"'))`"" }

    $rsp = Join-Path $outDir "$ProjectName.vsdbg.rsp"
    @($linkOpts, '/incremental:no',
      "/out:`"$ExePath`"", "/pdb:`"$outDir\$ProjectName.vsdbg.pdb`"", "/implib:`"$outDir\$ProjectName.vsdbg.lib`"") +
      $resFiles + $objs + $libFiles | Set-Content -Path $rsp -Encoding ASCII

    Write-Host "Relinking with $link for VS Code debugging ..." -ForegroundColor Cyan
    $prevEap = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    $linkOut = & $link "@$rsp" 2>&1
    $ErrorActionPreference = $prevEap
    $linkOut | Where-Object { $_ -notmatch 'LNK4075|LNK4254|Creating library' } | ForEach-Object { Write-Host $_ }
    if ($LASTEXITCODE -ne 0) { throw "Modern relink failed (exit $LASTEXITCODE)." }
    Write-Host "Relink done: $ProjectName.vsdbg.pdb" -ForegroundColor Green
}

if ($Config -eq 'Debug' -and -not (Test-HasRsds $ExePath)) { Invoke-ModernRelink }
Write-Host "Executable: $ExePath"

if ($Run) {
    Write-Host "Smoke-testing launch ..." -ForegroundColor Cyan
    $runProc = Start-Process -FilePath $ExePath -WorkingDirectory $RepoRoot -PassThru
    Start-Sleep -Seconds 3
    $stillRunning = -not $runProc.HasExited
    if ($stillRunning) {
        Write-Host "Process is running after 3s (PID $($runProc.Id)) - closing it." -ForegroundColor Green
        Stop-Process -Id $runProc.Id -Force -ErrorAction SilentlyContinue
    } else {
        throw "Process exited immediately with code $($runProc.ExitCode) (0x{0:X8}) - likely a missing dependency or startup crash." -f $runProc.ExitCode
    }
}
