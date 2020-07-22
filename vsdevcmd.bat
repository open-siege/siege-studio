@if not defined _echo echo off
for /f "usebackq delims=" %%i in (`vswhere.exe -prerelease -latest -property installationPath`) do (
  if exist "%%i\Common7\Tools\vsdevcmd.bat" (
    "%%i\Common7\Tools\vsdevcmd.bat" %*
  )
)
