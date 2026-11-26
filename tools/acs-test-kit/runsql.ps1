param([string]$File)
# Runs a .sql file batch by batch (split on GO lines) against allcosmos suasa on torry by IP; prints result sets and PRINT/RAISERROR messages.
$line = Get-Content "C:\Users\KELVIN\AppData\Local\Temp\claude\c4df8ff5c1fdb637\5838c554-7a38-4783-8d49-5dd40a412127\scratchpad\acsrun\PROVIDER.SYS" -TotalCount 1
$u = [regex]::Match($line, 'User ID=([^;]*)').Groups[1].Value
$p = [regex]::Match($line, 'Password=([^;]*)').Groups[1].Value
$cn = New-Object System.Data.SqlClient.SqlConnection "Server=192.168.0.10,1433;Database=allcosmos suasa;User ID=$u;Password=$p;Connect Timeout=10"
$cn.add_InfoMessage({ param($s, $e) Write-Output "MSG: $($e.Message)" })
$cn.FireInfoMessageEventOnUserErrors = $true
$cn.Open()
$batches = [regex]::Split((Get-Content $File -Raw), '(?im)^\s*GO\s*$') | Where-Object { $_.Trim() }
$n = 0
foreach ($b in $batches) {
    $n++
    $cmd = $cn.CreateCommand(); $cmd.CommandText = $b; $cmd.CommandTimeout = 120
    $da = New-Object System.Data.SqlClient.SqlDataAdapter $cmd
    $ds = New-Object System.Data.DataSet
    try { [void]$da.Fill($ds) } catch { Write-Output "BATCH $n ERROR: $($_.Exception.InnerException.Message)"; continue }
    foreach ($t in $ds.Tables) { $t | Format-Table -AutoSize | Out-String -Width 250 }
}
$cn.Close()
"$n batches run"
