
if [ "$1" = "-r" ]
then
	echo Restore dalvik files ...
	echo ===============================================
	adb push system/bin/dexopt              			system/bin && \
	adb push system/lib/libdvm.so           			system/lib && \
	adb push system/xbin/dexdump            			system/xbin && \
	adb push system/bin/dalvikvm            			system/bin && \
	adb push system/bin/linker     					system/bin && \
	adb push system/lib/libandroid.so     			system/lib && \
	adb push system/lib/libc.so     					system/lib && \
	adb push system/lib/libbcinfo.so     			system/lib && \
	adb push system/lib/libbcc.so     				system/lib && \
	adb push system/lib/libandroid_runtime.so     	system/lib && \
	adb push system/lib/libjavacore.so     			system/lib && \
	adb push system/lib/libnativehelper.so  			system/lib && \
	echo "Dalvik restored! Now rebooting ..." && \
	adb reboot && \
	sleep 3 && \
	adb logcat
else
	echo Extractin dalvik files ...
	echo ===============================================
	mkdir system
	mkdir system/bin
	mkdir system/xbin
	mkdir system/lib
	
	adb pull system/bin/dexopt              			system/bin && \
	adb pull system/lib/libdvm.so           			system/lib && \
	adb pull system/xbin/dexdump            			system/xbin && \
	adb pull system/bin/dalvikvm            			system/bin && \
	adb pull system/bin/linker     					system/bin && \
	adb pull system/lib/libandroid.so     			system/lib && \
	adb pull system/lib/libc.so     					system/lib && \
	adb pull system/lib/libbcinfo.so     			system/lib && \
	adb pull system/lib/libbcc.so     				system/lib && \
	adb pull system/lib/libandroid_runtime.so     	system/lib && \
	adb pull system/lib/libjavacore.so     			system/lib && \
	adb pull system/lib/libnativehelper.so  			system/lib 
fi
