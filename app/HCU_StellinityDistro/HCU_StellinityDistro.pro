TEMPLATE = app
QT += qml quick serialport charts opengl websockets
CONFIG += c++11

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

APP_NAME = "HCU_StellinityDistro"

# Get BIN_SUFFIX
win32 {
BIN_SUFFIX = win
}

unix {
BIN_SUFFIX = $$system(uname -r)
}

CONFIG(debug, debug|release) {
    BIN_SUFFIX = "$${BIN_SUFFIX}_d"
    CONFIG += qml_debug
}
# Get Target Path
win32 {
    CONFIG(debug, debug|release) {
        TARGET = ../Release/$${APP_NAME}_$${BIN_SUFFIX}
    } else {
        TARGET = $${APP_NAME}_$${BIN_SUFFIX}
    }
}

unix {
    # Remove BIN_SUFFIX because it cause duplication on target since the storage space is too valuable!
    TARGET = ./Release/$${APP_NAME}
}

include(deployment.pri)
include(Apps/Apps.pri)
include(Common/Common.pri)
include(DataServices/DataServices.pri)

SOURCES += \
    main.cpp \

RESOURCES += \
    Resources.qrc \

PATH_TARGET = ../../target
PATH_RESOURCES = $${PATH_TARGET}/bin/resources

OTHER_FILES += \
    ./Plugins/VirtualKeyboard/content/styles/imax/style.qml \
    $$PATH_TARGET/doc/version.inf \
    $$PATH_RESOURCES/DefaultConfig/AlertDescriptions.json \
    $$PATH_RESOURCES/DefaultConfig/DefaultFluidOptions.json \
    $$PATH_RESOURCES/DefaultConfig/DefaultInjectionPlanTemplate.json \
    $$PATH_RESOURCES/DefaultConfig/SampleCruData.json \
    $$PATH_RESOURCES/DefaultConfig/SampleInjectionPlans.json \
    $$PATH_RESOURCES/DefaultConfig/SampleFluidOptions.json \
    $$PATH_RESOURCES/Phrases/cd-XX/i18n-bundle.json \
    $$PATH_RESOURCES/Doc/ImxRestServer.settings \

CONFIG(debug, debug|release) {
    DEFINES -= QT_NO_DEBUG_OUTPUT
} else {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

DEFINES += QT_NO_DEBUG_OUTPUT

#---------------------------------------------------------------------------------------
# FTDI Library Link
#---------------------------------------------------------------------------------------
unix {
    LIBS += -lftdi
}

#---------------------------------------------------------------------------------------
# Copy Virtaul Keyboard Customised Files
#---------------------------------------------------------------------------------------
# With Qt6 the resource cannot be load through qrc:/ but with file://
# so the virtual keyboard style file must live where it is now $$[QT_INSTALL_QML]/QtQuick/VirtualKeyboard/Styles/imax for normal run (on developer machine)
# - 1st copy is for the default QT installation
# - 2nd copy the target where the Qt depedencies will be copied under /home/user/Imaxeon/bin/resources/Lib/Qt
copydata1.commands = $(COPY_DIR) $$PWD/Plugins/VirtualKeyboard/content/styles/imax $$[QT_INSTALL_QML]/QtQuick/VirtualKeyboard/Styles
copydata2.commands += $(COPY_DIR) $$PWD/Plugins/VirtualKeyboard/content/styles/imax  /home/user/Imaxeon/bin/resources/Lib/Qt/qml/QtQuick/VirtualKeyboard/Styles
first.depends = $(first) copydata1 copydata2
export(first.depends)
export(copydata1.commands)
export(copydata2.commands)
QMAKE_EXTRA_TARGETS += first copydata1 copydata2

#---------------------------------------------------------------------------------------
# The following lines import the shared QtWebApp library.
#---------------------------------------------------------------------------------------

PATH_BUILD = /IMAX_USER/Imaxeon-dev/build/

#install prefix for QZXing install setup
PREFIX = /home/user/Imaxeon/bin/resources

CONFIG += depend_includepath

win32 {
   DEFINES += QTWEBAPPLIB_IMPORT
}

# Directories, where the *.h files are stored
PATH_QT_WEBAPP =

INCLUDEPATH += \
    $${PATH_BUILD}/QtWebApp/QtWebApp

# Directory where the release version of the shared library (*.dll or *.so) is stored, and base name of the file.
# always use release version of qtwebapp
CONFIG(release, debug|release) {
    LIBS += -L$${PATH_BUILD}/QtWebApp/release/ -lQtWebApp
}

CONFIG(debug, debug|release) {
    LIBS += -L$${PATH_BUILD}/QtWebApp/release/ -lQtWebApp
}


#---------------------------------------------------------------------------------------
# The following lines allow QZXing to be statically build and link to the app.
# expect the source to be decompressed at /IMAX_USER/build/qzxing_v3.3.0/src/
#---------------------------------------------------------------------------------------
DEFINES += QZXING_QML QML_MULTIMEDIA QZXING_MULTIMEDIA DISABLE_LIBRARY_FEATURES
CONFIG += qzxing_multimedia qzxing_qml
include($${PATH_BUILD}/qzxing_v3.3.0/src/QZXing.pri)


# Remote debugging
target.path = /home/user/Imaxeon/bin
# the deployment.pr do the targ install
# INSTALLS += target
