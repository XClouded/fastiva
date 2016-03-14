symbol_dir=$OUT/symbols
system_dir=$OUT/system
mkdir -p $JPP_OUT_DIR/fastiva && \
mkdir -p $symbol_dir/vendor/lib && \
mkdir -p $symbol_dir/vendor/bin && \
cp $symbol_dir/system/bin/dexopt    		$symbol_dir/vendor/bin/ && \
cp $symbol_dir/system/lib/libdvm.so 		$symbol_dir/vendor/lib/ && \
cp $symbol_dir/system/lib/libjavacore.so   	$symbol_dir/vendor/lib/ && \
cp $symbol_dir/system/lib/libc.so  		$symbol_dir/vendor/lib/ && \
cp $symbol_dir/system/lib/libbcinfo.so  	$symbol_dir/vendor/lib/ && \
cp $symbol_dir/system/lib/libbcc.so   		$symbol_dir/vendor/lib/ && \
cp $symbol_dir/system/lib/libandroid_runtime.so $symbol_dir/vendor/lib/ && \
\
cp $JPP_OUT_DIR/libs/armeabi-v7a/libfastiva.so  $JPP_OUT_DIR/fastiva && \
cp $system_dir/xbin/dexdump          		$JPP_OUT_DIR/fastiva && \
cp $system_dir/bin/dexopt            		$JPP_OUT_DIR/fastiva && \
cp $system_dir/lib/libdvm.so         		$JPP_OUT_DIR/fastiva && \
cp $system_dir/lib/libjavacore.so   		$JPP_OUT_DIR/fastiva && \
cp $system_dir/lib/libc.so  			$JPP_OUT_DIR/fastiva && \
cp $system_dir/lib/libbcinfo.so  		$JPP_OUT_DIR/fastiva && \
cp $system_dir/lib/libbcc.so   			$JPP_OUT_DIR/fastiva && \
cp $system_dir/lib/libandroid_runtime.so   	$JPP_OUT_DIR/fastiva 
