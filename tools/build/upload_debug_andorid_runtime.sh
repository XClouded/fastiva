echo Uploading debug android_runtime file
echo "######################################"
BASEDIR=$(dirname $0) 
TARGET=/vendor
fastiva_dir=$JPP_OUT_DIR/fastiva

adb shell mount -o remount,rw /system
adb shell mkdir -p /sdcard/fastiva
adb push $fastiva_dir/libandroid_runtime.so           $TARGET/lib && \

echo "debug android_runtime installed! Now rebooting ..." && \
adb reboot && \
sleep 3 && \
adb logcat 
