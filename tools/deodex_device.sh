mkdir -p preload-apps
rm -rf preload-apps/$1
mkdir -p preload-apps/$1

cd preload-apps/$1
for apk in $(adb shell ls /system/$1/*.*); do 
	apk=$(echo $apk | tr -d ' \r\n') 
	echo "==== extracting $apk ====" 
	adb pull $apk
done

echo ""
for odex in $(ls *.odex); do 
	for i in $(echo $odex | tr "/" " "); do
            odexfile=$i
	done
	dexfile=${odexfile%.*}.dex
	apkfile=${odexfile%.*}.apk
	echo "==== $odexfile -> jar ===="
	java -jar baksmali-2.0.3.jar -d . -o $odexfile.out -x $odexfile
	java -jar smali-2.0.3.jar -o $dexfile $odexfile.out
	zip -r -q $apkfile $dexfile
	java -jar signapk/signapk.jar signapk/testkey.x509.pem signapk/testkey.pk8 $apkfile $apkfile
done

cd ../..
