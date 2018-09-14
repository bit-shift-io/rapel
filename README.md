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

