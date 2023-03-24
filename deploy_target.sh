#!/bin/bash

#use external $QMAKE if defined
[ -z "$QMAKE" ] && export QMAKE=/IMAX_USER/Qt5.15.3/5.15.3/gcc_64/bin/qmake

if [ ! -f ${QMAKE} ]; then
    echo "ERROR: missing qmake at ${QMAKE}"  >&2
    echo "=> Need Qt installation and setup QMAKE variable"  >&2
    exit 1
fi


PROC=`cat /proc/cpuinfo  | grep processor | wc -l`
MAKE="make -j${PROC}"
GREEN='\033[1;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NO_COLOR='\033[0m'

# Used to ensure platform version is supported
# Only check Major version
PLATFORM_VERSION_FILE=$HOME/Imaxeon-Platform-Version
COMPATIBLE_PLATFORM_VERSION=3
PLATFORM_CHECK=0

usage(){
    echo "Usage: $0 make [debug|release] [all]"
    echo "    Example: $0 make release all"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

echo ""
echo -e "${GREEN}Checking platform version...${NO_COLOR}"
if [ -f $PLATFORM_VERSION_FILE ]; then
    FULL_VERSION=$(head -n 1 $PLATFORM_VERSION_FILE)
    echo -e "${YELLOW}Current platform full version = $FULL_VERSION${NO_COLOR}"
    MAJOR_VERSION=$(echo $FULL_VERSION | cut -d "." -f 1)
    if [ $COMPATIBLE_PLATFORM_VERSION == $MAJOR_VERSION ]; then
        PLATFORM_CHECK=1
    fi
else
    echo -e "${RED}Error: $PLATFORM_VERSION_FILE not found!. Please install correct platform${NO_COLOR}"
    exit 1
fi

if [ $PLATFORM_CHECK -eq 0 ]; then
    echo -e "${RED}Major platform version        = $MAJOR_VERSION${NO_COLOR}"
    echo -e "${RED}Compatible platform version   = $COMPATIBLE_PLATFORM_VERSION${NO_COLOR}"
    echo -e "${RED}Platform is NOT compatible. Please upgrade platform.${NO_COLOR}"
    exit 1
fi

echo ""
echo -e "${GREEN}Platform version is compatible. Continuing build...${NO_COLOR}"
echo ""

DEPLOY_TYPE="copy"
QMAKE_TYPE="debug"
BUILD_TYPE="normal"

if [ $# -gt 0 ]; then
    DEPLOY_TYPE=$1
fi

if [ $# -gt 1 ]; then
    QMAKE_TYPE=$2
fi

if [ $# -gt 2 ]; then
    BUILD_TYPE=$3
fi

KERNEL_NAME=$(uname -r)
BINARY_TAIL="_${KERNEL_NAME}_d"


PATH_ROOT="$(dirname "$0")"
PATH_ROOT=`realpath $PATH_ROOT`
PATH_SRC="${PATH_ROOT}/app"
PATH_TARGET="${PATH_ROOT}/target"
PATH_WEB_UI_SRC="${PATH_ROOT}/bayer-connect/injector-ui"

# QMAKE_ARG_RELEASE="CONFIG+=qtquickcompiler CONFIG+=force_debug_info CONFIG+=separate_debug_info"
QMAKE_ARG_RELEASE="CONFIG+=qtquickcompiler CONFIG+=force_debug_info  CONFIG+=c++11" 
QMAKE_ARG_DEBUG="CONFIG+=debug CONFIG+=qml_debug  CONFIG+=c++11"
QMAKE_ARG=$QMAKE_ARG_DEBUG

if [ "$QMAKE_TYPE" == "release" ]
then
    QMAKE_ARG=$QMAKE_ARG_RELEASE
    BINARY_TAIL="_${KERNEL_NAME}"
fi

BDIR=/IMAX_USER/Imaxeon-dev/build
if [ "$BUILD_TYPE" == "all" ]; then
    # remove the build directory
    [ -d $BDIR ] &&  rm -rf $BDIR
fi

[ -d $BDIR ] ||  mkdir $BDIR
    

my_mkdir() {
  [ -d $1 ] || mkdir $1
}

# $1 is the .pro filename $2 is the extra argument to qmake
build_pro() {
    fullName=$(realpath $1)
    filename=$(basename -- "$1")
	extension="${filename##*.}"
    projectName="${filename%.*}"
    echo ""
    echo -e "${GREEN}Building $fullName at $BDIR/$projectName/$QMAKE_TYPE ${NO_COLOR}"
    my_mkdir $BDIR/$projectName
    my_mkdir $BDIR/$projectName/$QMAKE_TYPE
    cd $BDIR/$projectName/$QMAKE_TYPE
    echo $QMAKE $fullName -spec linux-g++ $QMAKE_ARG $2
    $QMAKE $fullName -spec linux-g++ $QMAKE_ARG  $2 >> $BDIR/build.txt
    echo $MAKE
    $MAKE >> $BDIR/build.txt
    if [[ $2 == PREFIX* ]]; then
        $MAKE install
    fi
}

build_pro_qtwebapp() {
    # QtWebapp must be built in release mode with addtional supressed flag to stop it from output debug messages
    fullName=$(realpath $1)
    filename=$(basename -- "$1")
	extension="${filename##*.}"
    projectName="${filename%.*}"
    
    echo ""
    echo -e "${GREEN}Building $fullName at $BDIR/$projectName/release ${NO_COLOR}"
    my_mkdir $BDIR/$projectName
    my_mkdir $BDIR/$projectName/release
    cd $BDIR/$projectName/release
    echo $QMAKE $fullName -spec linux-g++ $QMAKE_ARG_RELEASE DEFINES+=QT_NO_DEBUG_OUTPUT DEFINES+=QT_NO_INFO_OUTPUT DEFINES+=QT_NO_WARNING_OUTPUT  $2
    $QMAKE $fullName -spec linux-g++ $QMAKE_ARG_RELEASE DEFINES+=QT_NO_DEBUG_OUTPUT DEFINES+=QT_NO_INFO_OUTPUT DEFINES+=QT_NO_WARNING_OUTPUT  $2 > $BDIR/$QMAKE_${projectName}.txt
    echo $MAKE
    $MAKE >> $BDIR/$QMAKE_${projectName}.txt
    if [[ $2 == PREFIX* ]]; then
        $MAKE install
    fi
}



echo "START---------------------------" > $BDIR/build.txt

if [ "$BUILD_TYPE" == "all" ]
then    
    cd $BDIR
    # only unzip if the source is not there
    [ -f $BDIR/qzxing_v3.3.0/src/QZXing.pro ] || unzip $PATH_SRC/HCU_StellinityDistro/Plugins/qzxing_v3.3.0.zip 
    
    # Don't build it because it statically include in the project.
    # build_pro $BDIR/qzxing_v3.3.0/src/QZXing.pro PREFIX=$PATH_TARGET
    # copy the build result -----------------
    #rm -rf $PATH_TARGET/bin/resources/Lib/QZXing
    #mkdir -p $PATH_TARGET/bin/resources/Lib/QZXing
    #rm -rf ~/Imaxeon/bin/resources/Lib/QZXing
    #cp -v libQZXing* $PATH_TARGET/bin/resources/Lib/QZXing/
    #mkdir -p ~/Imaxeon/bin/resources/Lib/QZXing
    # todo need PREFIX=~/Imaxeon/bin/resources/Lib/QZXing
    #cp -rfv $PATH_TARGET/bin/resources/Lib/QZXing ~/Imaxeon/bin/resources/Lib/

    cd $BDIR
    echo $BDIR/QtWebApp/QtWebApp/QtWebApp.pro
    # only unzip if the source is not there
    [ -f $BDIR/QtWebApp/QtWebApp/QtWebApp.pro ] || unzip $PATH_SRC/HCU_StellinityDistro/Plugins/QtWebApp.zip
    # Always build QtWebApp in release
    build_pro_qtwebapp $BDIR/QtWebApp/QtWebApp/QtWebApp.pro PREFIX=$PATH_TARGET
    # Copy QtWebApp library files to target so other application can link during make
    rm -rf   $PATH_TARGET/bin/resources/Lib/QtWebApp
    rm -rf ~/Imaxeon/bin/resources/Lib/QtWebApp
    mkdir -p $PATH_TARGET/bin/resources/Lib/QtWebApp
    cp -v libQtWebApp*.* $PATH_TARGET/bin/resources/Lib/QtWebApp/
    ls -lah $PATH_TARGET/bin/resources/Lib/QtWebApp/
    mkdir -p ~/Imaxeon/bin/resources/Lib/QtWebApp
    cp -rfv $PATH_TARGET/bin/resources/Lib/QtWebApp ~/Imaxeon/bin/resources/Lib/
    
    cd $BDIR
    build_pro $PATH_SRC/HCU_StellinityDistroLauncher/HCU_StellinityDistroLauncher.pro
    # copy the launcher to the target bin directory
    [ -f Release/HCU_StellinityDistroLauncher_${KERNEL_NAME}_d ] && cp Release/HCU_StellinityDistroLauncher_${KERNEL_NAME}_d $PATH_TARGET/bin/HCU_StellinityDistroLauncher
    [ -f Release/HCU_StellinityDistroLauncher_${KERNEL_NAME} ]  && cp Release/HCU_StellinityDistroLauncher_${KERNEL_NAME} $PATH_TARGET/bin/HCU_StellinityDistroLauncher
fi

cd $BDIR
build_pro $PATH_SRC/HCU_StellinityDistro/HCU_StellinityDistro.pro PREFIX=$PATH_TARGET
[ -f Release/HCU_StellinityDistro_${KERNEL_NAME}_d ] && cp Release/HCU_StellinityDistro_${KERNEL_NAME}_d $PATH_TARGET/bin/HCU_StellinityDistro
[ -f Release/HCU_StellinityDistro_${KERNEL_NAME} ]  && cp Release/HCU_StellinityDistro_${KERNEL_NAME} $PATH_TARGET/bin/HCU_StellinityDistro



