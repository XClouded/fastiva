echo Uploading fastiva files ...
echo ===============================================
adb shell mount -o remount,rw /system

fastiva_dir=/work/kitkat4.4.2-org/out/target/product/grouper/system

adb push $fastiva_dir/lib/libdvm.so           /system/lib && \
adb push $fastiva_dir/bin/dalvikvm            /vendor/bin 
if [ "$1" = "-c" ]
then
  adb shell chmod 777 /data/dalvik-cache
  adb shell rm -f /system/app/*.odex && \
  adb shell rm -f /system/framework/*.odex && \
  adb shell rm -f /system/priv-app/*.odex && \
  adb shell rm -f /data/dalvik-cache/*.dex 
fi
echo "Fastiva installed! Now rebooting ..." && \
adb reboot
sleep 3
adb logcat
