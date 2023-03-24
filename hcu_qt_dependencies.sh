# Expect the QT installation is relative is related $QMAKE as per standard Qt installation setup.
# 
# Copy HCU's QT library dependencies to the specified output folder
# then use the qt.conf::Prefix to point to the specified output directory
# e.g:  qt.conf has the following lines:
# [Paths]
# Prefix= /home/user/Qt5.15.9_gcc_64
#
#
# To work out the dependencies, run the application using strace and analyse the strace result
# e.g. strace -o trace.txt <HCU-app>
#    cat trace.txt | grep openat | cut -d , -f 2 |cut -d \" -f 2 > cut.txt
# 
# Note ldd is not reliable because it does not anlyse the dependencies of the other .so file.
#

[ -z "$QMAKE" ] && export QMAKE=/IMAX_USER/Qt5.15.3/5.15.3/gcc_64/bin/qmake
QMAKE_DIR=`dirname ${QMAKE}`
QT_DIR=`dirname ${QMAKE_DIR}`
if [ ! -d ${QT_DIR} ]; then
    echo "ERROR: can not find QT gcc directory at ${QT_DIR}"  >&2
    echo "=> Need Qt installation and setup QMAKE variable"  >&2    
    exit 1
fi

outdir=/home/user/Imaxeon/bin/resources/Lib/Qt

dup() {
 echo "cp ${QT_DIR}/lib/$1 ."
 cp ${QT_DIR}/lib/$1 .
}

ddir() {
 echo "copy dir $1"
 cp $1 . -r
}

ld_lib_path() {
    # Need for running HCU locally or in a remote SSH terminal
    export LD_LIBRARY_PATH=/home/user/Imaxeon/bin/resources/Lib/Qt/lib:/home/user/Imaxeon/bin/resources/Lib/QtWebApp
    echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH" > /home/user/Imaxeon/bin/ld_lib_path.txt
    echo "export DISPLAY=:0.0" >>  /home/user/Imaxeon/bin/ld_lib_path.txt
}

[ -d ${outdir} ] || mkdir ${outdir}
cd ${outdir}

[ -d lib ] || mkdir lib
cd lib

dup libicudata.so.56
dup libicui18n.so.56
dup libicuuc.so.56
dup libQt5Core.so.5
dup libQt5DBus.so.5
dup libQt5Gui.so.5
dup libQt5MultimediaGstTools.so.5
dup libQt5Multimedia.so.5
dup libQt5MultimediaWidgets.so.5
dup libQt5Network.so.5
dup libQt5OpenGL.so.5
dup libQt5QmlModels.so.5
dup libQt5Qml.so.5
dup libQt5Quick.so.5
dup libQt5SerialPort.so.5
dup libQt5VirtualKeyboard.so.5
dup libQt5WebSockets.so.5
dup libQt5Widgets.so.5
dup libQt5XcbQpa.so.5

#indirect
dup libQt5QmlWorkerScript.so.5
dup libQt5QuickTemplates2.so.5
dup libQt5QuickControls2.so.5
dup libQt5Charts.so.5
dup libQt5MultimediaQuick.so.5
dup libQt5Svg.so.5


cd $outdir
[ -d plugins ] || mkdir plugins
cd plugins
ddir ${QT_DIR}/plugins/imageformats
ddir ${QT_DIR}/plugins/mediaservice
ddir ${QT_DIR}/plugins/platforminputcontexts
ddir ${QT_DIR}/plugins/platforms
ddir ${QT_DIR}/plugins/platformthemes
ddir ${QT_DIR}/plugins/virtualkeyboard
ddir ${QT_DIR}/plugins/xcbglintegrations
# need for qml remote debugging
ddir ${QT_DIR}/plugins/qmltooling

# want all the translation
cd $outdir
ddir ${QT_DIR}/translations

# want all the qml
ddir ${QT_DIR}/qml

####################################################
# Add qt.conf to gcc directory
# QT_DIR= /home/user/Imaxeon/bin/resources/Lib/Qt
echo "[Paths]
Prefix= $outdir" | tee $outdir/qt.conf
cp $outdir/qt.conf /home/user/Imaxeon/bin
####################################################
# Generate LD_LIBRARY_PATH to /home/user/Imaxeon/bin/ld_lib_path.txt
# the startup script can simply source the generated ld_lib_path.txt to run the exe
ld_lib_path

####################################################
# remove unused debug symbols and
cd $outdir
# remove all debug symbol in any directory bellow if there is any
find . -name "*.debug" -exec rm -rf {} \;

rm -rf ./qml/Qt3D
rm -rf ./qml/QtQuick/Scene3D
rm -rf ./qml/QtQuick/tooling
rm -rf ./qml/QtTest
rm -rf ./qml/Particles
rm -rf ./qml/QtQuick3D
rm -rf ./qml/QtDataVisualization
rm -rf ./qml/QtOpcUa
rm -rf ./qml/QtPositioning
rm -rf ./qml/QtRemoteObjects
rm -rf ./qml/QtScxml
rm -rf ./qml/QtSensors
rm -rf ./qml/QtWebChannel
rm -rf ./qml/QtWebEngine
rm -rf ./qml/QtWebSockets
rm -rf ./qml/QtWebView



du -h -d1


