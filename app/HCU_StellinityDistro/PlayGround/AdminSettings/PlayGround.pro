TEMPLATE = app

QT += qml quick charts

CONFIG += c++11

SOURCES += main.cpp

RESOURCES += qml.qrc

OTHER_FILES += \
    main.qml \
    PlayGround.qml \

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)


HEADERS += \
    PlayGround.h

DISTFILES += \
    MySlider.qml


