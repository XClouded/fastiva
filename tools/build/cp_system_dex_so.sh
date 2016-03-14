adb shell mount -o remount,rw /system

for so in $(ls obj/$1/*.dex.so); do 
	so=$(echo $so | tr -d ' \r\n') 
	dst=$(echo $so | sed -e "s@obj/@@")
	dst=$(echo $dst | sed -e "s@apk\@classes.@@")
	echo "adb push $so $dst"
	adb push $so $dst
done

