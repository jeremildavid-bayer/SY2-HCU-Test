#include <QApplication>
#include "AppManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    AppManager appManager(&app);

    return app.exec();
}
