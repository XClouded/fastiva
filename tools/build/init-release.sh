export PRODUCT_DIR=$OUT
export PRODUCT_TYPE=REL
export SYMBOL_DIR=$OUT/symbols
F=$(dirname $0)/00_REL_gdb.setup

#APK_SYMBOL_PATH=
APK_SYMBOL_PATH=$JPP_OUT_DIR/obj/data/app/com.google.android.gms-2.apk.fastiva/obj/local/armeabi-v7a

#echo set solib-search-path $SYMBOL_DIR/system/bin:$SYMBOL_DIR/vendor/lib:$SYMBOL_DIR/system/lib:$JPP_OUT_DIR/obj/local/armeabi-v7a:$APK_SYMBOL_PATH > $F
echo set solib-search-path $SYMBOL_DIR/vendor/bin:$SYMBOL_DIR/vendor/lib:$SYMBOL_DIR/system/bin:$JPP_OUT_DIR/obj/local/armeabi-v7a:$APK_SYMBOL_PATH > $F
#echo set solib-search-path $SYMBOL_DIR/vendor/lib:$JPP_OUT_DIR/obj/local/armeabi-v7a:$APK_SYMBOL_PATH > $F
#echo file $SYMBOL_DIR/system/bin/dalvikvm >> $F
echo file $SYMBOL_DIR/vendor/lib/libdvm.so >> $F
#echo file $JPP_OUT_DIR/obj/local/armeabi-v7a/libfastiva.so >> $F
#file /work/jpp/out/nexus5/obj/system/priv-app/Phonesky.dex.fastiva/obj/local/armeabi-v7a/system@priv-app@Phonesky.apk@classes.dex.so
echo target remote :5039 >> $F
echo info thread >> $F
echo set sleep_on_break=0 >> $F
echo handle SIGUSR2 pass nostop noprint >> $F
echo p gc_start_threadId >> $F
echo p/x gDvm.gcHeapLock >> $F

