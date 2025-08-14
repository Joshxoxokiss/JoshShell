# Run as Administrator once after cloning
$dest = "C:\JoshShell"
if (!(Test-Path $dest)) { New-Item -ItemType Directory -Path $dest | Out-Null }
Copy-Item "$PSScriptRoot\*" $dest -Recurse -Force
Write-Host "Installed to $dest\JoshShell.exe"