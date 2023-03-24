#include <QApplication>
#include "PlayGround.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PlayGround playGround(&app);

    return app.exec();
}
