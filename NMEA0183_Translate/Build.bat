make clean
make all
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\MakeAppx.exe" pack /m appxmanifest.xml /f myMapping.txt /p NMEA0183client.msix  
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\SignTool.exe" sign /fd SHA256 /a /f .\cert.pfx /p ADMIN .\NMEA0183client.msix