#include <QApplication>
#include <QQmlApplicationEngine>
#include "Launcher.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("HCU Stellinity Distro Launcher");

    Launcher launcher(&app);

    return app.exec();
}

