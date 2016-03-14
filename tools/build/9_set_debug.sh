vi fastiva.dbg
adb shell mkdir -p /sdcard/fastiva
adb push fastiva.dbg /sdcard/fastiva/
adb shell su -c "mount -o remount,rw /system; chmod 777 /data/dalvik-cache; cp /sdcard/fastiva/fastiva.dbg /data/dalvik-cache/; chmod 777 /data/dalvik-cache/fastiva.dbg"
echo adb reboot
