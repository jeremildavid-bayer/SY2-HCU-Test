#include <QGuiApplication>
#include "SoundPlay.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    SoundPlay soundPlay(&app);

    return app.exec();
}

