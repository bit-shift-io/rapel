#!/bin/bash

read -p "Username: " username
read -p "Password: " password

cd steamworks_tools

./builder_linux/steamcmd.sh +login $username $password +run_app_build_http ../full_scripts/app_build_878030.vdf +run_app_build_http ../demo_scripts/app_build_878040.vdf +quit

# ./builder_linux/steamcmd.sh +login $username $password +run_app_build_http ../full_scripts/app_build_878030.vdf +quit
