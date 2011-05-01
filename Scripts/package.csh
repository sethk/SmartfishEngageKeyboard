#!/bin/csh -xe

umask 022
set projpath="$0:h/.."
cd "$projpath"
set dst="/tmp/SmartfishEngageKeyboard.dst"
sudo rm -r "$dst"
sudo xcodebuild -configuration Release -target SmartfishEngageKeyboard \
	install DSTROOT="$dst"
kextutil -nt "$dst/System/Library/Extensions/SmartfishEngageKeyboard.kext"
set version=`defaults read $cwd/Resources/Info CFBundleShortVersionString`
/Developer/usr/bin/packagemaker -v -d Installer/Installer.pmdoc -m -w -k -n $version \
	-o "Installer/Smartfish Engage™ Keyboard Driver $version.pkg"
