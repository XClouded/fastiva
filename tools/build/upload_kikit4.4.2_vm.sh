echo Uploading fastiva files ...
echo ===============================================
adb shell mount -o remount,rw /system
BASEDIR=$(dirname $0) 

adb shell mkdir -p /vendor
adb shell mkdir -p /vendor/bin
adb shell mkdir -p /vendor/lib

adb shell rm -f /vendor/lib/libfastiva.so
#adb shell cp -n /system/bin/dexopt /vendor/bin/dexopt_org
adb shell rm -f /vendor/lib/libdvm.so
adb shell rm -f /vendor/lib/libnativehelper.so
adb shell rm -f /vendor/lib/libchromium_net.so
#adb shell rm -f /system/lib/libearthmobile.so

fastiva_dir=/work/kitkat4.4.2-org/out/target/product/grouper/system

#adb push ./libs/armeabi-v7a/libfastiva.so     /vendor/lib && \
#adb push $fastiva_dir/lib/libfastiva.so     /vendor/lib && \
adb push $fastiva_dir/lib/libandroid.so     /system/lib && \
adb push $fastiva_dir/lib/libc.so     /system/lib && \
adb push $fastiva_dir/lib/libbcinfo.so     /system/lib && \
adb push $fastiva_dir/lib/libbcc.so     /system/lib && \
adb push $fastiva_dir/lib/libdvm.so           /system/lib && \
adb push $fastiva_dir/lib/libnativehelper.so  /system/lib && \
adb push $BASEDIR/prebuilts/gdbserver         /vendor/bin && \
adb push $fastiva_dir/bin/dexopt              /system/bin && \
adb push $fastiva_dir/xbin/dexdump            /vendor/bin && \
adb push $fastiva_dir/bin/dalvikvm            /vendor/bin 
if [ "$1" = "-c" ]
then
  adb shell chmod 777 /data/dalvik-cache
  adb shell rm -f /system/app/*.odex && \
  adb shell rm -f /system/framework/*.odex && \
  adb shell rm -f /system/priv-app/*.odex && \
  adb shell rm -f /data/dalvik-cache/*.dex 
fi
#adb shell ln -f /data/lib/libfastiva.so /vendor/lib/libfastiva.so
echo "Kitkat 4.4.2 installed! Now rebooting ..." && \
adb reboot
sleep 3
adb logcat
