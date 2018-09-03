#!/bin/bash
#
# sudo pip install sphinx sphinx-autobuild recommonmark sphinx_rtd_theme
# 
 
PROJ_DIR=$(dirname -- $(readlink -fn -- "$0"))/../
BUILD_DIR=$PROJ_DIR/Build/
DOCS_DIR=$PROJ_DIR/Docs/

cd $DOCS_DIR
make html

echo "Done"

