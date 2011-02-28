#!/bin/csh

rsync -avr --delete --exclude '*.swp' --delete-excluded . mecha:SmartfishEngageKeyboard-backup
