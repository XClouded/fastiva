echo Uploading fastiva files ...
echo "######################################"
BASEDIR=$(dirname $0) 
TARGET=/sdcard/fastiva/vendor

#adb shell mount -o remount,rw /system
adb shell mkdir -p /sdcard/fastiva
adb shell mkdir -p $TARGET
adb shell mkdir -p $TARGET/bin
adb shell mkdir -p $TARGET/lib

adb shell rm -f    /sdcard/fastiva/vendor/bin/*
adb shell rm -f    /sdcard/fastiva/vendor/lib/*

adb shell rm -f $TARGET/bin/dexopt
adb shell rm -f $TARGET/lib/libfastiva.so
adb shell rm -f $TARGET/lib/libdvm.so
adb shell rm -f $TARGET/lib/libjavacore.so
adb shell rm -f $TARGET/lib/libc.so
adb shell rm -f $TARGET/lib/libbcinfo.so
adb shell rm -f $TARGET/lib/libbcc.so


fastiva_dir=$OUT/system

adb push $JPP_OUT_DIR/libs/armeabi-v7a/libfastiva.so     $TARGET/lib && \
adb push $fastiva_dir/xbin/dexdump            $TARGET/bin && \
adb push $fastiva_dir/bin/dexopt              $TARGET/bin && \
adb push $fastiva_dir/lib/libdvm.so           $TARGET/lib && \

if [ "$1" = "-d" ]
then
adb push $fastiva_dir/lib/libjavacore.so      			$TARGET/lib && \
adb push $fastiva_dir/lib/libc.so     					$TARGET/lib && \
adb push $fastiva_dir/lib/libbcinfo.so     				$TARGET/lib && \
adb push $fastiva_dir/lib/libbcc.so     					$TARGET/lib && \
adb push $BASEDIR/prebuilts/gdbserver         			$TARGET/bin && \

#adb push $fastiva_dir/lib/libandroid_runtime.so           $TARGET/lib && \

symbol_dir=$OUT/symbols
mkdir -p  $symbol_dir/vendor/lib
mkdir -p  $symbol_dir/vendor/bin

cp $symbol_dir/system/bin/dexopt    			$symbol_dir/vendor/bin/
cp $symbol_dir/system/lib/libdvm.so 			$symbol_dir/vendor/lib/
cp $symbol_dir/system/lib/libjavacore.so   	$symbol_dir/vendor/lib/
cp $symbol_dir/system/lib/libc.so  			$symbol_dir/vendor/lib/
cp $symbol_dir/system/lib/libbcinfo.so  		$symbol_dir/vendor/lib/
cp $symbol_dir/system/lib/libbcc.so   		$symbol_dir/vendor/lib/
fi

echo Uploading fastiva into vendor directory.
adb shell su -c "mount -o remount,rw /system; mkdir -p /vendor/bin; mkdir -p /vendor/lib; chmod 755 vendor/bin/; chmod 755 vendor/lib/; cp -a $TARGET/bin/* /vendor/bin; cp -a $TARGET/lib/* /vendor/lib; " && \
adb shell su -c "chmod 755 /vendor/bin/* && chmod 644 /vendor/lib/*.so" && \
adb shell su -c "chown root:root /vendor/bin/* && chown root:root /vendor/lib/*.so" && \
echo "Fastiva installed! Now rebooting ..." && \
adb reboot && \
sleep 3 && \
adb logcat 
