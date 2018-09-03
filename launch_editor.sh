#!/bin/bash

PROJ_DIR=$(dirname -- $(readlink -fn -- "$0"))
SRC_DIR=$PROJ_DIR/Source

if [ -f $PROJ_DIR/Build/godot/bin/godot.x11.tools.64 ]; then
	$PROJ_DIR/Build/godot/bin/godot.x11.tools.64 --editor --path $SRC_DIR
else
	$PROJ_DIR/Build/godot/bin/godot.windows.tools.64 --editor --path $SRC_DIR
fi

