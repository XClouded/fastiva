BASEDIR=$(dirname $0) 
adb install -r $BASEDIR/prebuilts/JppAgent.apk
adb install -r $BASEDIR/prebuilts/AMark.apk
adb install -r $BASEDIR/prebuilts/CF-Bench_release.apk
adb install -r $BASEDIR/prebuilts/com.androidrocker.taskkiller-1.apk
