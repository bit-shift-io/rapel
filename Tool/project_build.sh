#!/bin/bash

echo "------------------------------------------------------"
echo "This will build the project for all platforms and versions"
echo ""
echo "no args = do it all!"
echo ""
echo "-mac build for mac only"
echo "-linux build for linux only"
echo "-windows build for windows only"
echo ""
echo "-data only assemble the data (dont build exes)"
echo "------------------------------------------------------"

PROJ_DIR=$(dirname -- $(readlink -fn -- "$0"))/../
BUILD_DIR=$PROJ_DIR/Build/
BIN_DIR=$BUILD_DIR/bin

PLATFORM_MAC=0
PLATFORM_LINUX=0
PLATFORM_WINDOWS=0

BUILD_EXECUTABLES=1

# parse command line args
while [ "$1" != "" ]; do
  case $1 in
    -data)
      BUILD_EXECUTABLES=0
        ;;
    -mac)
      PLATFORM_MAC=1
        ;;
    -linux)
      PLATFORM_LINUX=1
        ;;
    -windows)
      PLATFORM_WINDOWS=1
        ;;
    -h|-\?|--help)
      echo "Usage: $(basename $0) -linux -data"
      exit
      ;;
    --)              # End of all options.
      shift
      break
      ;;
    -?*)
      echo "WARN: Unknown option (ignored): $1"
      ;;
    *)               # Default case: If no more options then break out of the loop.
      break
  esac
  shift
done

# no platform set means all platforms are enabled
if [[ $PLATFORM_MAC == 0 ]] && [[ $PLATFORM_LINUX == 0 ]] && [[ $PLATFORM_WINDOWS == 0 ]]; then
    PLATFORM_MAC=1
    PLATFORM_LINUX=1
    PLATFORM_WINDOWS=1
fi

echo ""
echo "PLATFORM_MAC: $PLATFORM_MAC"
echo "PLATFORM_LINUX: $PLATFORM_LINUX"
echo "PLATFORM_WINDOWS: $PLATFORM_WINDOWS"
echo ""
echo "BUILD_EXECUTABLES: $BUILD_EXECUTABLES"
echo ""
echo "------------------------------------------------------"

copy_data() {
    DEST_DIR=$1
    IS_DEMO=$2
    PLATFORM=$3
    
    CP_DEST_DIR=$DEST_DIR
    
    if [ $IS_DEMO == "demo" ]; then
        CP_DEST_DIR=$DEST_DIR/tmp
    fi
    
    #echo "dest dir: " + $DEST_DIR;
    
    # if -data is supplied, delete everything except executables as we will not be rebuilding executables
    if [[ $BUILD_EXECUTABLES == 1 ]]; then
        rm -r $DEST_DIR
    else
        find $DEST_DIR -not -name 'TrainsAndThings' -not -name 'TrainsAndThings.exe' -not -name 'TrainsAndThingsEditor' -not -name 'TrainsAndThingsEditor.exe' -delete
    fi
    
    mkdir $DEST_DIR
    mkdir $CP_DEST_DIR
    cp -Lr $PROJ_DIR/Source/. $CP_DEST_DIR

    if [ $IS_DEMO == "demo" ]; then
        find $CP_DEST_DIR/Map -maxdepth 1 -type d -not -name Hawaii -not -name Map -not -name Tutorial -exec rm -f -r {} +
        
        # build the editor so we can package data
        if [ ! -f $PROJ_DIR/Build/godot/bin/godot.x11.tools.64 ]; then
            ./godot_build.sh
        fi
        
        $PROJ_DIR/Build/godot/bin/godot.x11.tools.64 --path $CP_DEST_DIR --export "$PLATFORM" $DEST_DIR/TrainsAndThings.pck
        #$PROJ_DIR/Build/godot/bin/godot.x11.tools.64 --path $CP_DEST_DIR --export $PLATFORM $DEST_DIR/TrainsAndThings.zip
        rm -r $CP_DEST_DIR
    else
        rm $DEST_DIR/export_presets.cfg
        rm $DEST_DIR/log.txt
        rm $DEST_DIR/rss.txt
        rm $DEST_DIR/settings.cfg
    fi
}

start=`date +%s`

echo "COPYING DATA"

if [ $PLATFORM_LINUX == 1 ]; then
    copy_data $PROJ_DIR/Build/LinuxDemo "demo" "Linux/X11"
    copy_data $PROJ_DIR/Build/LinuxFull "full" "Linux/X11"
fi

if [ $PLATFORM_WINDOWS == 1 ]; then    
    copy_data $PROJ_DIR/Build/WindowsDemo "demo" "Windows Desktop"
    copy_data $PROJ_DIR/Build/WindowsFull "full" "Windows Desktop"
fi

if [ $PLATFORM_MAC == 1 ]; then    
    copy_data $PROJ_DIR/Build/MacDemo "demo" "Mac OSX"
    copy_data $PROJ_DIR/Build/MacFull "full" "Mac OSX"
fi

if [[ $BUILD_EXECUTABLES == 1 ]]; then
    echo "BUILDING EXECUTABLES"

    if [ $PLATFORM_LINUX == 1 ]; then
        ./godot_build.sh -linux -release
    fi

    if [ $PLATFORM_WINDOWS == 1 ]; then 
        ./godot_build.sh -windows -release
    fi

    if [ $PLATFORM_MAC == 1 ]; then 
        ./godot_build.sh -mac -release
    fi    
fi

end=`date +%s`
runtime=$((end-start))

echo "------------------------------------------------------"
echo "Build Completed in $(($runtime / 60)) minutes and $(($runtime % 60)) seconds"
echo ""

