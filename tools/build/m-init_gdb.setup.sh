SYMBOL_DIR=$OUT/symbols
F=$AOSP_PATH/fastiva_gdb.setup


#echo set solib-search-path $SYMBOL_DIR/system/bin:$SYMBOL_DIR/vendor/lib:$SYMBOL_DIR/system/lib:$JPP_OUT_DIR/obj/local/armeabi-v7a:$1 > $F
echo set solib-search-path $SYMBOL_DIR/vendor/bin:$SYMBOL_DIR/vendor/lib:$SYMBOL_DIR/system/bin:$JPP_OUT_DIR/obj/local/armeabi-v7a:$1 > $F

#echo file $SYMBOL_DIR/system/bin/dalvikvm >> $F
echo file $SYMBOL_DIR/vendor/lib/libdvm.so >> $F

#echo file $JPP_OUT_DIR/obj/local/armeabi-v7a/libfastiva.so >> $F

echo target remote :5039 >> $F
echo info thread >> $F
echo set sleep_on_break = 0 >> $F
echo handle SIGUSR2 pass nostop noprint >> $F
echo p gc_start_threadId >> $F
echo p/x gDvm.gcHeapLock >> $F

