#!/bin/bash
 
 
if [ -f "butler" ]; then
    ./butler upgrade
else
    wget https://dl.itch.ovh/butler/linux-amd64/head/butler
    chmod +x butler
fi

./butler -V

./butler push ../Build/LinuxDemo/ bitshift/trains-and-things:linux-demo
./butler push ../Build/LinuxFull/ bitshift/trains-and-things:linux-full

./butler push ../Build/WindowsDemo/ bitshift/trains-and-things:windows-demo
./butler push ../Build/WindowsFull/ bitshift/trains-and-things:windows-full

./butler push ../Build/MacDemo/ bitshift/trains-and-things:mac-demo
./butler push ../Build/MacFull/ bitshift/trains-and-things:mac-full

echo "Upload to itch.io Completed!"
