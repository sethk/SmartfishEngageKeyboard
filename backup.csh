#!/bin/csh

rsync -avr --delete --exclude '*.swp' --exclude "build" --delete-excluded . mecha:SmartfishEngageKeyboard-backup
