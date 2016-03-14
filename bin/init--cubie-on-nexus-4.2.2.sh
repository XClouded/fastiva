export JPP_DIR=/sdk/jpp
export PATH=$PATH:$JPP_DIR/tools:$JPP_DIR/tools/build

export JPP_OUT_DIR=~/android/fastiva-cubie-4.2.2
export AOSP_PATH=~/android/cubie-4.2.2/android

cd $AOSP_PATH
. build/envsetup.sh
choosecombo 1 full_grouper 3
# . init-release.sh

