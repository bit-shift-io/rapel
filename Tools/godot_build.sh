#!/bin/bash

# add use_llvm=yes for clang support (see https://github.com/godotengine/godot/issues/9300#issuecomment-324886489 for more info)

# http://docs.godotengine.org/en/stable/reference/compiling_for_x11.html
echo "------------------------------------------------------"
echo "Add -update to pull latest godot-git first"
echo ""
echo "Add -linux to build (default)"
echo "Add -windows to do a windows build"
echo "Add -mac to do a mac build"
echo "Add -android to do a android build"
echo ""
echo "Add -release as a second arg to build all platform release executables"
echo "------------------------------------------------------"

TOOLS_DIR=$(dirname -- $(readlink -fn -- "$0"))
PROJ_DIR=$TOOLS_DIR/../
BUILD_DIR=$PROJ_DIR/Build/
ENGINE_DIR=$BUILD_DIR/godot
OSXCROSS_DIR=$BUILD_DIR/osxcross
CPU_THREADS=$(getconf _NPROCESSORS_ONLN) # works on linux + mac

export ANDROID_HOME=$HOME/Android/Sdk
export ANDROID_NDK_ROOT=$HOME/Android/Sdk/ndk-bundle
export OSXCROSS_ROOT=$OSXCROSS_DIR

echo $PROJ_DIR
mkdir $BUILD_DIR

# if godot dir doesnt exist, assume never built before
if [ ! -d $ENGINE_DIR ]; then
    #sudo apt-get install git build-essential scons pkg-config libx11-dev libxcursor-dev libxinerama-dev libgl1-mesa-dev libglu-dev libasound2-dev libpulse-dev libfreetype6-dev libssl-dev libudev-dev libxrandr-dev
    #sudo pacman -S --noconfirm libgeotiff
    sudo pacman -S --noconfirm gdb 
    sudo pacman -S --noconfirm scons 
    sudo pacman -S --noconfirm mingw-w64-gcc
    
    # one for android
    sudo yay -S --noconfirm ncurses5-compat-libs
    
    git clone https://github.com/godotengine/godot.git $ENGINE_DIR  
    
    #sudo pacaur -S libtiff - this doesnt' work
    # so, download from http://www.simplesystems.org/libtiff/
    # extract and run:
    # ./configure && make && sudo make install
fi

# http://docs.godotengine.org/en/3.0/development/compiling/compiling_for_osx.html
if [ "$1" == "-mac" ]; then
    if [ ! -d $OSXCROSS_DIR ]; then
        sudo pacman -S --noconfirm cmake 
        sudo pacman -S --noconfirm lib32-libxml2
        sudo pacman -S --noconfirm fuse 
        
        # setup for osx cross compilation
        git clone https://github.com/tpoechtrager/osxcross.git $OSXCROSS_DIR
        cd $OSXCROSS_DIR
        
        echo ""
        echo "Please download xcode dmg from apple site:"
        echo "https://download.developer.apple.com/Developer_Tools/Xcode_7.3.1/Xcode_7.3.1.dmg"
        echo "and place in the Build dir"
        echo ""
        echo "Comment out the line:"
        echo "git reset --hard $DARLING_DMG_REV"
        echo "in:"
        echo "osxtools/tools/gen_sdk_package_darling_dmg.sh"
        echo ""
        echo "Also change the repo to: git clone https://github.com/darlinghq/darling-dmg.git"
        echo ""
        read -p "Press any key when complete to proceed... "
    
        # See Packing the SDK on Linux, Method 1:
        # https://github.com/tpoechtrager/osxcross#packaging-the-sdk
        ./tools/gen_sdk_package_darling_dmg.sh ../Xcode_7.3.1.dmg
        mv MacOSX10.11.sdk.tar.xz tarballs
        ./build.sh
        #OSX_VERSION_MIN=10.11 ./build.sh
        cd $TOOLS_DIR
    fi
    
    export OSXCROSS_ROOT=$OSXCROSS_DIR
fi

if [ "$1" == "-update" ]; then
    git -C $ENGINE_DIR checkout .
    git -C $ENGINE_DIR pull 
    exit 0
fi

# link project modules into the build
ln -s $PROJ_DIR/Godot/modules/bitshift $ENGINE_DIR/modules
ln -s $PROJ_DIR/Godot/modules/matrix $ENGINE_DIR/modules
ln -s $PROJ_DIR/Godot/modules/gddragonbones $ENGINE_DIR/modules

# copy other changes
cp -r $PROJ_DIR/Godot/drivers $ENGINE_DIR
cp -r $PROJ_DIR/Godot/scene $ENGINE_DIR
cp -r $PROJ_DIR/Godot/core $ENGINE_DIR
cp -r $PROJ_DIR/Godot/servers $ENGINE_DIR

# apply patches
cd $ENGINE_DIR
git am < $PROJ_DIR/Godot/patch/0001-16-bit-PNG-support.patch


# Build!
if [ -z "$1" ] || [ "$1" == "-linux" ]; then
    cd $ENGINE_DIR
    scons -j $CPU_THREADS platform=x11 bits=64 builtin_openssl=yes
fi

if [ "$1" == "-windows" ]; then
    cd $ENGINE_DIR
    CPU_THREADS=4
    scons -j $CPU_THREADS platform=windows bits=64 builtin_openssl=yes
fi

if [ "$1" == "-mac" ]; then    
    cd $ENGINE_DIR
    CPU_THREADS=4
    scons -j $CPU_THREADS platform=osx bits=64 builtin_openssl=yes osxcross_sdk=darwin15
fi

if [ "$1" == "-android" ]; then    
    cd $ENGINE_DIR
    CPU_THREADS=4
    scons -j $CPU_THREADS platform=android target=release_debug builtin_openssl=yes
    cd platform/android/java
    pwd
    ./gradlew build
fi

# build export templates - i.e. all binaries for all platforms
if [ "$2" == "-release" ]; then
    cd $ENGINE_DIR
    
    if [ "$1" == "-linux" ]; then
        # linux demo
        scons -j $CPU_THREADS platform=x11 tools=no target=release bits=64 builtin_openssl=yes demo=yes
        cp $ENGINE_DIR/bin/godot.x11.opt.64 $BUILD_DIR/LinuxDemo/TrainsAndThings
        
        # linux full
        scons -j $CPU_THREADS platform=x11 tools=no target=release bits=64 builtin_openssl=yes demo=no
        cp $ENGINE_DIR/bin/godot.x11.opt.64 $BUILD_DIR/LinuxFull/TrainsAndThings

        scons -j $CPU_THREADS platform=x11 tools=yes target=release_debug debug_release=yes bits=64 builtin_openssl=yes demo=no
        cp $ENGINE_DIR/bin/godot.x11.opt.tools.64 $BUILD_DIR/LinuxFull/TrainsAndThingsEditor
	fi

	if [ "$1" == "-windows" ]; then
        # windows demo
        scons -j $CPU_THREADS platform=windows tools=no target=release bits=64 builtin_openssl=yes demo=yes
        cp $ENGINE_DIR/bin/godot.windows.opt.64.exe $BUILD_DIR/WindowsDemo/TrainsAndThings.exe
        x86_64-w64-mingw32-strip $BUILD_DIR/WindowsDemo/TrainsAndThings.exe
        
        # windows full
        scons -j $CPU_THREADS platform=windows tools=no target=release bits=64 builtin_openssl=yes demo=no
        cp $ENGINE_DIR/bin/godot.windows.opt.64.exe $BUILD_DIR/WindowsFull/TrainsAndThings.exe
        x86_64-w64-mingw32-strip $BUILD_DIR/WindowsFull/TrainsAndThings.exe

        scons -j $CPU_THREADS platform=windows tools=yes target=release_debug debug_release=yes bits=64 builtin_openssl=yes demo=no
        cp $ENGINE_DIR/bin/godot.windows.opt.tools.64.exe $BUILD_DIR/WindowsFull/TrainsAndThingsEditor.exe
        x86_64-w64-mingw32-strip $BUILD_DIR/WindowsFull/TrainsAndThingsEditor.exe
    fi
    
    # http://docs.godotengine.org/en/3.0/development/compiling/compiling_for_osx.html
    if [ "$1" == "-mac" ]; then
        # osx demo
        scons -j $CPU_THREADS platform=osx tools=no target=release bits=64 builtin_openssl=yes demo=yes osxcross_sdk=darwin15
        cp $ENGINE_DIR/bin/godot.osx.opt.64 $BUILD_DIR/MacDemo/TrainsAndThings
        x86_64-w64-mingw32-strip $BUILD_DIR/MacDemo/TrainsAndThings
        
        # osx full
        scons -j $CPU_THREADS platform=osx tools=no target=release bits=64 builtin_openssl=yes demo=no osxcross_sdk=darwin15
        cp $ENGINE_DIR/bin/godot.osx.opt.64 $BUILD_DIR/MacFull/TrainsAndThings
        x86_64-w64-mingw32-strip $BUILD_DIR/MacFull/TrainsAndThings

        scons -j $CPU_THREADS platform=osx tools=yes target=release_debug debug_release=yes bits=64 builtin_openssl=yes demo=no osxcross_sdk=darwin15
        cp $ENGINE_DIR/bin/godot.osx.opt.tools.64 $BUILD_DIR/MacFull/TrainsAndThingsEditor
        x86_64-w64-mingw32-strip $BUILD_DIR/MacFull/TrainsAndThingsEditor
    fi
    
    if [ "$1" == "-android" ]; then    
        cd $ENGINE_DIR
        CPU_THREADS=4
        scons -j $CPU_THREADS platform=android target=release builtin_openssl=yes
        cd platform/android/java
        pwd
        ./gradlew build
    fi
fi

echo "Done"

