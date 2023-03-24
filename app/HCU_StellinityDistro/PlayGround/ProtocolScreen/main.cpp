#include <QGuiApplication>
#include "AppManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppManager appManager(&app);

    return app.exec();
}

