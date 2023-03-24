TEMPLATE = app

QT += qml quick charts

CONFIG += c++11

SOURCES += main.cpp

RESOURCES += qml.qrc

OTHER_FILES += \
    main.qml \
    InjectionPlot.qml \
    ReminderIcon.qml \
    TransitionExample.qml \
    PhaseTransition.qml \
    ScrollBar.qml \
    ListFade.qml \
    Util.js

# Additional import path used to resolve QML modules in Qt Creator's code model
#QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)


HEADERS += \
    AppManager.h

DISTFILES += \
    TransitionLine.qml

