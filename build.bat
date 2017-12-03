rem @echo off

echo Setting path
if "%OLDPATH%"=="" set OLDPATH=%PATH%
if "%QT32PATH%"=="" set QT32PATH=C:\Qt\qt-5.6.2-x86-msvc2013\5.6\msvc2013
if "%QT64PATH%"=="" set QT64PATH=C:\Qt\qt-5.6.2-x64-msvc2013\5.6\msvc2013_64
if "%VSVARSALLPATH%"=="" set VSVARSALLPATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat

if "%1"=="32" (
    set "PATH=%QT32PATH%\bin;%PATH%"
    call "%VSVARSALLPATH%" x86
    set MSBUILDPLATFORM=Win32
) else if "%1"=="64" (
    set "PATH=%QT64PATH%\bin;%PATH%"
    call "%VSVARSALLPATH%" x64
    set MSBUILDPLATFORM=x64
) else (
    echo Usage: build.bat 32/64
    goto restorepath
)

echo Preparing directory
rmdir /s /q build%1
mkdir build%1
cd build%1

echo Building MeddleGit
qmake ..\src\MeddleGit.pro -config release -tp vc
if not %ERRORLEVEL%==0 goto :eof
msbuild /m MeddleGit.vcxproj /p:Configuration=Release;Platform=%MSBUILDPLATFORM%
if not %ERRORLEVEL%==0 goto :eof

echo Deploying MeddleGit
mkdir MeddleGit%1
move release\MeddleGit.exe MeddleGit%1\MeddleGit.exe
xcopy /s ..\dist%1 MeddleGit%1\
windeployqt MeddleGit%1\MeddleGit.exe
cd ..

:restorepath
echo Restoring path
set PATH=%OLDPATH%
set OLDPATH=
