echo Uploading fastiva files ...
echo "######################################"
BASEDIR=$(dirname $0) 
TARGET=/vendor

adb shell mount -o remount,rw /system
adb shell mkdir -p /sdcard/fastiva
adb shell mkdir -p $TARGET
adb shell mkdir -p $TARGET/bin
adb shell mkdir -p $TARGET/lib

adb shell rm -f $TARGET/lib/libfastiva.so
adb shell rm -f $TARGET/bin/dexopt
adb shell rm -f $TARGET/bin/dexdump
adb shell rm -f $TARGET/lib/libdvm.so

adb shell rm -f $TARGET/lib/libjavacore.so
adb shell rm -f $TARGET/lib/libc.so
adb shell rm -f $TARGET/lib/libbcinfo.so
adb shell rm -f $TARGET/lib/libbcc.so
adb shell rm -f $TARGET/lib/libandroid_runtime.so

$BASEDIR/m-copy_output.sh

fastiva_dir=$JPP_OUT_DIR/fastiva

adb push $fastiva_dir/libfastiva.so     $TARGET/lib && \
adb push $fastiva_dir/dexdump            $TARGET/bin && \
adb push $fastiva_dir/dexopt             $TARGET/bin && \
adb push $fastiva_dir/libdvm.so          $TARGET/lib && \

if [ "$1" = "-d" ]
then
adb push $fastiva_dir/libjavacore.so      			$TARGET/lib && \
adb push $fastiva_dir/libc.so     					$TARGET/lib && \
adb push $fastiva_dir/libbcinfo.so     				$TARGET/lib && \
adb push $fastiva_dir/libbcc.so     					$TARGET/lib && \
adb push $BASEDIR/prebuilts/gdbserver         		$TARGET/bin 
#adb push $fastiva_dir/libandroid_runtime.so           $TARGET/lib 

adb shell chmod 755 $TARGET/bin/gdbserver
fi

echo "Fastiva installed! Now rebooting ..." && \
adb reboot && \
sleep 3 && \
adb logcat 
