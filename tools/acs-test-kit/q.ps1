param([string]$Sql)
$line = Get-Content "C:\Users\KELVIN\AppData\Local\Temp\claude\c4df8ff5c1fdb637\5838c554-7a38-4783-8d49-5dd40a412127\scratchpad\acsrun\PROVIDER.SYS" -TotalCount 1
$u = [regex]::Match($line, 'User ID=([^;]*)').Groups[1].Value
$p = [regex]::Match($line, 'Password=([^;]*)').Groups[1].Value
$cn = New-Object System.Data.SqlClient.SqlConnection "Server=192.168.0.10,1433;Database=allcosmos suasa;User ID=$u;Password=$p;Connect Timeout=10"
$cn.Open()
$da = New-Object System.Data.SqlClient.SqlDataAdapter $Sql, $cn
$t = New-Object System.Data.DataTable
[void]$da.Fill($t)
$cn.Close()
$t | Format-List | Out-String -Width 300
