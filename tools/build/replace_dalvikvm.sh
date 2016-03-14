echo Uploading fastiva vm ...
echo ===============================================
BASEDIR=$(dirname $0) 
adb shell mount -o remount,rw /system

fastiva_dir=$OUT/system

$BASEDIR/m-copy_output.sh

# libandroid_runtime.so is for debugging
adb push $fastiva_dir/lib/libdvm.so                /vendor/lib && \
adb push $fastiva_dir/bin/dexopt                   /vendor/bin && \

echo "Fastiva vm installed! Now rebooting ..." && \
adb reboot
sleep 3
adb logcat
