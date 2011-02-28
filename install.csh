#!/bin/csh

sudo xcodebuild -configuration Release -target SmartfishEngageKeyboard install DSTROOT=/
touch /System/Library/Extensions
kill -HUP `ps -ax | awk '{print $1" "$5}' | grep kextd | awk '{print $1}'`
