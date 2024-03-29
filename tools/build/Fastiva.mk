.SUFFIXES : .odex .dex .jar.dex2 .apk.dex2 .dex.so

all : libfastiva apps

%.jar.dex2 : %.odex
	odex2dex $< $@ odex/system/framework

%.apk.dex2 : %.odex
	odex2dex $< $@ odex/system/framework


%.apk@classes.dex.so : %.jar
	dex2jpp $< 
	cp $<.fastiva/libs/armeabi-v7a/libfandroid.so $@

%.apk@classes.dex.so : %.apk
	dex2jpp $< 
	cp $<.fastiva/libs/armeabi-v7a/libfandroid.so $@

# list device files (path,extension)
define ls_device
	$(filter %.$2, $(shell adb shell su -c ls $1/*.$2))
endef

get_device_bootclasspath_cmd := adb shell echo $$$$BOOTCLASSPATH |  tr -t ':' ' '

define get_device_bootclasspath
	adb shell echo $$$$BOOTCLASSPATH |  tr -t ':' ' '
endef

working_directories: 
	mkdir -p obj/system/framework
	mkdir -p obj/system/app
	mkdir -p obj/system/priv-app
	mkdir -p obj/data/dalvik-cache
	
## No performance gain.
jni/inc : 
	sudo mkdir -p jni/inc
	sudo mount -t tmpfs -o size=2048M tmpfs jni/inc/	

#=====================================================================================#
#  Download odexes
#=====================================================================================#


jar_odexes := $(call ls_device, /system/framework,odex)
apk_odexes := $(call ls_device, /system/app,odex) \
			  $(call ls_device, /system/priv-app,odex)

dowloaded_jar_odexes := $(foreach ODEX, $(jar_odexes), odex$(ODEX))
dowloaded_apk_odexes := $(foreach ODEX, $(apk_odexes), odex$(ODEX))

converted_jar_dexes := $(foreach ODEX, $(dowloaded_jar_odexes), $(ODEX:.odex=.jar.dex2)) 
converted_apk_dexes := $(foreach ODEX, $(dowloaded_apk_odexes), $(ODEX:.odex=.apk.dex2))


odex2dex : $(converted_jar_dexes) $(converted_apk_dexes)

$(converted_apk_dexes): $(dowloaded_apk_odexes) 

$(converted_jar_dexes): $(dowloaded_jar_odexes) 



	
$(dowloaded_jar_odexes): 
	@adb pull $(subst odex/,,$(@:.odex=.jar)) $(@:.odex=.jar) && \
	 adb pull $(subst odex/,,$@) $@

$(dowloaded_apk_odexes): 
	@adb pull $(subst odex/,,$(@:.odex=.apk)) $(@:.odex=.apk) && \
	 adb pull $(subst odex/,,$@) $@


#=====================================================================================#
#  Sign Jar
#=====================================================================================#

converted_framwork_dexes := $(shell ls odex/system/framework/*.dex2) 
converted_app_dexes := $(shell ls odex/system/app/*.dex2) \
					   $(shell ls odex/system/priv-app/*.dex2)

converted_framework_jars := $(foreach DEX, $(converted_framwork_dexes), $(DEX:.dex2=.zip))
converted_app_apks := $(foreach DEX, $(converted_app_dexes), $(DEX:.dex2=.zip))

signed_jar := odex/done
signed_jar : remount_device_system working_directories $(converted_framework_jars) $(converted_app_apks) 
	adb push /work/jpp/tools/signapk/update_fastiva.sh /sdcard/fastiva


remount_device_system:
	adb shell su -c mount -o remount,rw /system
	adb shell mkdir -p /sdcard/fastiva/system/framework
	adb shell mkdir -p /sdcard/fastiva/system/app
	adb shell mkdir -p /sdcard/fastiva/system/priv-app

$(converted_framework_jars) :
	@cp -f $(@:.zip=.dex2) classes.dex && \
	cp -f $(@:.zip=) temp.jar && \
	zip -r -q temp.jar classes.dex && \
	mv temp.jar $(subst odex/,obj/,$(@:.zip=)) && \
	adb push $(subst odex/,obj/,$(@:.zip=)) $(subst odex/,sdcard/fastiva/,$(@:.zip=))

$(converted_app_apks) :
	@cp -f $(@:.zip=.dex2) classes.dex && \
	cp -f $(@:.zip=) temp.jar && \
	zip -r -q temp.jar classes.dex && \
	mv temp.jar $(subst odex/,obj/,$(@:.zip=)) && \
	adb push $(subst odex/,obj/,$(@:.zip=)) $(subst odex/,sdcard/fastiva/,$(@:.zip=))

$(converted_zipfiles) :
	@cp -f $(@:.zip=.dex2) classes.dex && \
	cp -f $(@:.zip=) $(subst odex/,obj/,$@) && \
	zip -r -q $(subst odex/,obj/,$@) classes.dex && \
	sign_jar $(subst odex/,obj/,$@) $(subst odex/,obj/,$(@:.zip=)) && \
	adb push $(subst odex/,obj/,$(@:.zip=)) $(subst odex/,sdcard/,$(@:.zip=))

# $(converted_pc_dex2s)
#	@echo $< $@
	
#=====================================================================================#
#  build libfastiva.so
#=====================================================================================#

libfastiva := libs/armeabi-v7a/libfastiva.so 
JPP_COD_DIR := $(dir $(shell which jpp))cod

#// for full framework files (for Google System Apps)
bootclasspath_jars := $(call ls_device, /system/framework,jar)
#// for only bootclass files
# bootclasspath_jars := $(shell adb shell echo \$$BOOTCLASSPATH |  tr -t ':' ' ')

dowloaded_framework__jars := $(foreach JAR, $(bootclasspath_jars), obj$(JAR))

jpp_framework_jars := $(foreach  JAR, $(bootclasspath_jars), \
							  framework-jars/$(notdir $(basename $(JAR))-dex2jar.jar))

libfastiva : working_directories jni/Android.mk
	ndk-build NDK_DEBUG=0

jni/Android.mk : $(jpp_framework_jars)
	rm -f framework-jars/core-libart*.jar
	jpp  -ignore_unresolved_ref -out jni -ap_jars framework-jars 
	cp $(JPP_COD_DIR)/Application.mk jni/
	cp $(JPP_COD_DIR)/JppFramework.mk jni/Android.mk 

$(jpp_framework_jars) : $(dowloaded_framework__jars)
	dex2jar --force -o $@ obj/system/framework/$(notdir $(@:-dex2jar.jar=.jar))

$(dowloaded_framework__jars): 
	@adb pull $(subst obj,/,$@) $@
	

#=====================================================================================#
#  convert cached apk -> so
#=====================================================================================#
apps : jpp_user_apps jpp_system_apps 


#=====================================================================================#
#  convert pre-compiled system app -> so
#=====================================================================================#
system_app_list = $(call ls_device, /system/app,apk) $(call ls_device, /system/priv-app,apk)
dowloaded_system_jar := $(foreach DEX, $(system_app_list), obj$(DEX))
compiled_system_apps := $(foreach DEX, $(system_app_list), obj$(DEX:.apk=.apk@classes.dex.so))
#$(DEX).fastiva/libs/armeabi-v7a/libfandroid.so)

jpp_system_apps : working_directories $(compiled_system_apps)

$(compiled_system_apps) : $(dowloaded_system_jar)

$(dowloaded_system_jar) : 
	adb pull $(subst obj/,/, $@) $@

	
#=====================================================================================#
#  convert post-compiled apk (system & downloaded app)
#=====================================================================================#
user_app_list := $(call ls_device, /data/app,apk)
dowloaded_user_apps := $(foreach DEX, $(user_app_list), obj$(DEX))
compiled_user_apps := $(foreach DEX, $(user_app_list), obj$(DEX:.apk=.apk@classes.dex.so))

jpp_user_apps : working_directories $(compiled_user_apps)

$(compiled_user_apps) : $(dowloaded_user_apps)

$(dowloaded_user_apps) : 
	adb pull $(subst obj/,/, $@) $@
	

#=====================================================================================#
#  clean all
#=====================================================================================#

clean:
	rm -rf *
	
	
# 1. Rooting Device
# 2. Install SuperSU or Superuser app
# (cf. Nexus Root Tookit v1.8.0)
# adb shell su -c mount -o remount,rw /system
# adb shell su -c chmod 777 /data/dalvik-cache
#
#
#
#
	
