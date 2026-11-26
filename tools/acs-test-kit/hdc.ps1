. "C:\Users\KELVIN\.claude\skills\wlib32-app-run-on-new-pc\scripts\hidden-desktop\gui-client.ps1"
$HdDir = "C:\Users\KELVIN\AppData\Local\Temp\claude\c4df8ff5c1fdb637\5838c554-7a38-4783-8d49-5dd40a412127\scratchpad\hd"
$Run = "C:\Users\KELVIN\AppData\Local\Temp\claude\c4df8ff5c1fdb637\5838c554-7a38-4783-8d49-5dd40a412127\scratchpad\acsrun"
function Wt([int]$kg) { for ($i = 0; $i -lt 10; $i++) { try { Set-Content "$env:TEMP\indicator-weight.txt" "$kg" -ErrorAction Stop; return } catch { Start-Sleep -Milliseconds 100 } } }
# Top window (after the two untitled ones) as "hwnd<TAB>hung<TAB>title<TAB>controls".
function TopWin { Send 'wins' | Where-Object { ($_ -split "`t")[2] } | Select-Object -First 1 }
function HwndOf([string]$line) { [long](($line -split "`t")[0]) }
$Sp = "C:\Users\KELVIN\AppData\Local\Temp\claude\c4df8ff5c1fdb637\5838c554-7a38-4783-8d49-5dd40a412127\scratchpad"
$Vid = "$Sp\video"
function AppPid { (Get-Process menu -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq "$Run\menu.exe" } | Select-Object -First 1).Id }
function Cap([string]$Caption, [string]$Sub = '') { New-Item -ItemType Directory -Force $Vid | Out-Null; Set-Content "$Vid\caption.txt" $Caption; Set-Content "$Vid\subtitle.txt" $Sub }
function StartRec([string]$Take) {
    New-Item -ItemType Directory -Force $Vid | Out-Null; Remove-Item "$Vid\rec-stop.txt" -ErrorAction SilentlyContinue
    Remove-Item "$Vid\$Take" -Recurse -Force -ErrorAction SilentlyContinue
    $app = AppPid; Set-Content "$Vid\app.pid" $app
    Send "start|powershell.exe|-NoProfile -ExecutionPolicy Bypass -File `"$Sp\acs-recorder.ps1`" -Dir `"$Vid`" -Take $Take -ProviderFile `"$Run\PROVIDER.SYS`" -WeightFile `"$env:TEMP\indicator-weight.txt`"|$Sp" | Out-Null
    Send "use|$app" | Out-Null
}
function StopRec { Set-Content "$Vid\rec-stop.txt" 'stop'; Start-Sleep 2 }
# Starts the app and logs in; returns the main frame hwnd.
function StartAndLogin([string]$User, [string]$Pw) {
    Send "start|$Run\menu.exe||$Run" | Out-Null
    $h = WaitWin 'ALL COSMOS INDUSTRIES SDN BHD$' 90
    Send "settext|$h|2001|$User" | Out-Null; Send "settext|$h|4401|$User" | Out-Null; Send "settext|$h|2003|$Pw" | Out-Null
    Send "click|$h|Login" | Out-Null
    $m = WaitWin '^menu$' 40; if ($m) { Send "click|$m|OK" | Out-Null }
    WaitWin 'Weighing System' 60
}
# Zero the scale, put the lorry on at $Kg, open Weighing Control, choose Receiving/Despatch, enter plate, Confirm Weight.
function WeighScreen([long]$Frame, [string]$Kind, [string]$Plate, [int]$Kg) {
    Wt 0; Start-Sleep 5; Wt $Kg; Start-Sleep 5
    Send "click|$Frame|F5->" | Out-Null
    $h = WaitWin '^Weighing Control' 30; Start-Sleep 3
    Send "click|$h|^$Kind" | Out-Null
    Send "settext|$h|1263|$Plate" | Out-Null; Start-Sleep 1
    Send "click|$h|Confirm Weight" | Out-Null; Start-Sleep 6
    TopWin
}
