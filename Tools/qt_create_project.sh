#!/bin/bash


PROJ_DIR=$(readlink -m $(dirname -- $(readlink -fn -- "$0"))/../)
BUILD_DIR=$PROJ_DIR/Build/
BIN_DIR=$BUILD_DIR/bin
ENGINE_DIR=$BUILD_DIR/godot
 
echo $ENGINE_DIR
cp QtProject/* $ENGINE_DIR


# read the qt EnvironmentId:
envIdLine=$(awk -F "=" '/EnvironmentId/ {print $2}' $HOME/.config/QtProject/QtCreator.ini)
envId=$(echo $envIdLine|  cut -c 12-49)
echo "Qt EnvironmentId: $envId"

# read the <value type="QString" key="PE.Profile.Id">{2be68cb0-e621-41af-99fd-31a070a58aab}</value> from $HOME/.config/QtProject/qtcreator/profiles.xml
profileIdLine=$(awk -F ">" '/PE.Profile.Id/ {print $2}' $HOME/.config/QtProject/qtcreator/profiles.xml)
profileId=$(echo $profileIdLine|  cut -c 1-38)
echo "Qt ProfileId: $profileId"

sed -i -e 's|\$PROJ_DIR|'"${PROJ_DIR}"'|g' $ENGINE_DIR/godot.creator.user
sed -i -e 's|\$ENVIRONMENT_ID|'"${envId}"'|g' $ENGINE_DIR/godot.creator.user
sed -i -e 's|\$PROFILE_ID|'"${profileId}"'|g' $ENGINE_DIR/godot.creator.user



cd $ENGINE_DIR

# generate a list of all files
files=`find -L ./ -type f \( -iname \*.h -o -iname \*.cpp -o -iname \*.c -o -iname \*.hpp \)`
fixedFiles=${files//.\//}
echo "$fixedFiles" > "godot.files"

# generate list of dirs
dirs=`find -L . -type d`
fixedDirs=${dirs//.\//}
echo "$fixedDirs" > "godot.includes"

echo "Done"

