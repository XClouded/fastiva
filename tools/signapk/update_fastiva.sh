mount -o remount,rw /system && \
cd /sdcard/ && \
mkdir -p system.bak && \
cp -a /system/framework system.bak/ && \
cp -a /system/app system.bak/ && \
cp -a /system/priv-app system.bak/ && \
chmod 777 /system/framework && \
chmod 777 /system/app && \
chmod 777 /system/priv-app && \
chmod 777 /system/framework/* && \
chmod 777 /system/app/* && \
chmod 777 /system/priv-app/* && \
rm -f /system/framework/*.odex && \
rm -f /system/app/*.odex && \
rm -f /system/priv-app/*.odex  && \
cd fastiva/system/ && \
cp framework/*.jar /system/framework && \
cp app/*.apk /system/app && \
cp priv-app/*.apk /system/priv-app  && \
echo "rebooting to deodexed dalvik" && \
reboot