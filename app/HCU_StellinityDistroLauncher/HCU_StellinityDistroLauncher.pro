TEMPLATE = app
QT += qml quick widgets
CONFIG += c++11

QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

APP_NAME = "HCU_StellinityDistroLauncher"

# Get BIN_SUFFIX
win32 {
BIN_SUFFIX = win
}

unix {
BIN_SUFFIX = $$system(uname -r)
}

CONFIG(debug, debug|release) {
    BIN_SUFFIX = "$${BIN_SUFFIX}_d"
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
    TARGET = ./Release/$${APP_NAME}_$${BIN_SUFFIX}
}

include(deployment.pri)

HEADERS += \
    Launcher.h \
    Log.h

SOURCES += \
    main.cpp \
    Launcher.cpp

RESOURCES += \
    Resources.qrc \



