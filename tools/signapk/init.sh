keytool -genkey -v -keystore fastiva.keystore -alias cert -keyalg RSA -keysize 2048 -validity 10000

jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore fastiva.keystore -storepass fastiva2012 -keypass fastiva2012 -signedjar obj/system/framework/core.jar obj/system/framework/core.jar.zip cert