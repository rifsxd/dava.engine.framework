#!/bin/bash

BUILDTYPE=Release
OUTPUT_DIR=.

xcodebuild -configuration $BUILDTYPE -sdk iphoneos -target libdxt_ios -project NVTTiPhone.xcodeproj
xcodebuild -configuration $BUILDTYPE -sdk iphonesimulator -target libdxt_ios -project NVTTiPhone.xcodeproj

libtool build/$BUILDTYPE-iphoneos/libdxt_ios.a build/$BUILDTYPE-iphonesimulator/libdxt_ios.a -o $OUTPUT_DIR/libdxt_ios.a

rm -rf build