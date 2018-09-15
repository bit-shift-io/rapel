
ECHO "Setting up environment..."
SET PATH=%PATH%;C:\Python27;C:\Python27\Scripts;C:\Program Files\Git
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64


ECHO "Checking out Godot..."
rmdir /s /q ..\Build\godot\modules\bitshift
rmdir /s /q ..\Build\godot\modules\matrix
git-bash godot_build.sh %1

ECHO "Building..."
cd ..\Build\godot
scons vsproj=yes platform=windows
cd ..\..
