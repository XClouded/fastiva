export JPP_DIR=/sdk/jpp
export JPP_OUT_DIR=~/android/fastiva-nexus5-4.4.4
export AOSP_PATH=~/android/kitkat-4.4.4
export PATH=$PATH:$JPP_DIR/tools:$JPP_DIR/tools/build
cd $AOSP_PATH
. build/envsetup.sh
choosecombo 1 full_grouper 3
# . init-release.sh
