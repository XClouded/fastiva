echo Install fastiva files ...
echo "######################################"
BASEDIR=$(dirname $0) 
TARGET=/sdcard/fastiva/vendor

adb shell mkdir -p /sdcard/fastiva
adb shell su -c mkdir -p /vendor/bin
adb shell su -c mkdir -p /vendor/lib
adb shell mkdir -p $TARGET
adb shell mkdir -p $TARGET/bin
adb shell mkdir -p $TARGET/lib

adb shell rm -f    $TARGET/bin/dexopt
adb shell rm -f    $TARGET/bin/dexdump
adb shell rm -f    $TARGET/lib/libdvm.so
adb shell rm -f    $TARGET/lib/libfastiva.so


fastiva_dir=$OUT/system

adb push $JPP_OUT_DIR/libs/armeabi-v7a/libfastiva.so     $TARGET/lib && \
adb push $fastiva_dir/bin/dexopt              $TARGET/bin && \
adb push $fastiva_dir/xbin/dexdump            $TARGET/bin && \
adb push $fastiva_dir/lib/libdvm.so           $TARGET/lib && \

if [ "$1" = "-d" ]
then
adb push $fastiva_dir/lib/libjavacore.so      			/vendor/lib && \
adb push $fastiva_dir/lib/libc.so     					/vendor/lib && \
adb push $fastiva_dir/lib/libandroid_runtime.so           /vendor/lib && \
adb push $BASEDIR/prebuilts/gdbserver         			/vendor/bin && \

symbol_dir=$OUT/symbols
mkdir -p  $symbol_dir/vendor/lib
mkdir -p  $symbol_dir/vendor/bin

cp $symbol_dir/system/lib/libdvm.so 			$symbol_dir/vendor/lib/
cp $symbol_dir/system/lib/libc.so  			$symbol_dir/vendor/lib/
cp $symbol_dir/system/lib/libjavacore.so   	$symbol_dir/vendor/lib/
cp $symbol_dir/system/bin/dexopt    			$symbol_dir/vendor/bin/
cp $symbol_dir/system/bin/linker			   	$symbol_dir/vendor/linker
fi

adb shell su -c "mount -o remount,rw /system; cp -a $TARGET/bin/* /vendor/bin; cp -a $TARGET/lib/* /vendor/lib; " && \
adb shell su -c "chmod 644 /vendor/lib/*" && \
adb shell su -c "chmod 755 /vendor/bin/*"

echo "Fastiva installed! Now rebooting ..." && \
adb reboot && \
sleep 3 && \
adb logcat 
