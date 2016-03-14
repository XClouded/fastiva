echo Uninstall fastiva files ...
echo "######################################"
adb shell mount -o remount,rw /system
BASEDIR=$(dirname $0) 

adb shell rm -f    /vendor/bin/dexopt
adb shell rm -f    /vendor/bin/dexdump
adb shell rm -f    /vendor/lib/libdvm.so
adb shell rm -f    /vendor/lib/libfastiva.so
adb shell rm -f    /vendor/lib/libc.so
adb shell rm -f    /vendor/lib/libjavacore.so

adb reboot
sleep 5
adb logcat
