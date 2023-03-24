#ifndef PLAY_GROUND_H
#define PLAY_GROUND_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDebug>
#include <QQmlComponent>

class PlayGround : public QObject
{
    Q_OBJECT
public:
    explicit PlayGround(QObject *parent = 0) : QObject(parent)
    {
        context = new QQmlContext(engine.rootContext());
        component = new QQmlComponent(&engine, QUrl("qrc:/main.qml"));
        qmlRoot = component->create(context);

        if (!component->isReady())
        {
            qWarning() << "Failed to create QML Component. Error=" << component->errorString();
        }
    }

    ~PlayGround()
    {

    }

private:
    QQmlApplicationEngine engine;
    QQmlContext *context;
    QQmlComponent *component;
    QObject *qmlRoot;

private slots:

};

#endif // PLAY_GROUND_H
