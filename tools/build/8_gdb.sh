#adb shell setenforce 0 \&\& gdbserver :5039 --attach $1 &
adb shell su -c gdbserver :5039 --attach $1 &
if [ -f fastiva_gdb.setup ]; 
then
   echo "starting gdb connection.."
else 
   $JPP_DIR/tools/build/m-init_gdb.setup.sh
fi
sleep 2
arm_linux_androideabi_gdb=/sdk/android-ndk-r10d/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gdb
adb forward tcp:5039 tcp:5039; cgdb -d $arm_linux_androideabi_gdb -- --pid=$1 --command=fastiva_gdb.setup 

