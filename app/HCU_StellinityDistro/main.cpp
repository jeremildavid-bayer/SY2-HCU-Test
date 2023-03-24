#include <QApplication>
#include <QQmlApplicationEngine>
#include "Apps/AppManager.h"

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QApplication app(argc, argv);
    app.setApplicationName("HCU Stellinity Distro");

    AppManager appManager(&app);

    if (!appManager.initQmlApp())
        return 1; //error has occur

    return app.exec();
}

