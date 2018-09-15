To check out on a mchine, in the Projects directory run:
git clone git+ssh://s@192.168.1.2/~/GIT/Rapel.git

To setup for Android debug/export:

install Android Studio
install some form of SDK and then go into the SDK manager and also get the NDK

run the build script with -android


once built, you can export from godot
then run on your phone via:

adb install Rapel.apk 

once this is done you should be able to use the Android icon to deploy inside the godot editor!

More info:
http://docs.godotengine.org/en/3.0/development/compiling/compiling_for_android.html 
http://docs.godotengine.org/en/3.0/getting_started/workflow/export/exporting_for_android.html


Third party tools:
https://github.com/ndee85/coa_tools
https://github.com/sanja-sa/gddragonbones

Blender GLTF plugin
--------------------
Obtained from https://github.com/KhronosGroup/glTF-Blender-Exporter
Download the repo as a zip, then extract and grab out the "glTF-Blender-Exporter/scripts/addons/io_scene_gltf2/"
directory and make it into a zip. This can then be loaded into blender.

In blender you will need to check "Enable experimental glTF export settings" possibly to get this working.


