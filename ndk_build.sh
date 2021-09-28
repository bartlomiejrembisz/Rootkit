#!/bin/bash

export NDK_PROJECT_PATH=.
./android-ndk-r23/ndk-build NDK_APPLICATION_MK=./Application.mk clean
./android-ndk-r23/ndk-build NDK_APPLICATION_MK=./Application.mk
