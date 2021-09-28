#!/bin/bash

[ ! -d "android-ndk-r23" ] && source fetch_android_dependencies.sh

export NDK_PROJECT_PATH=.
./android-ndk-r23/ndk-build NDK_APPLICATION_MK=./Application.mk clean
./android-ndk-r23/ndk-build NDK_APPLICATION_MK=./Application.mk

