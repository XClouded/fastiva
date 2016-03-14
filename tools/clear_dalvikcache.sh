#!/bin/bash

echo "***** clearing odex files ******"
adb shell su -c "mount -o remount,rw /system; chmod 777 /data/dalvik-cache; rm -f /data/dalvik-cache/*.dex"
adb reboot
sleep 5
adb logcat
