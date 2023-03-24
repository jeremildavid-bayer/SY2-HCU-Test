#ifndef SOUND_PLAY_H
#define SOUND_PLAY_H

#include <QUuid>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QQmlComponent>

class SoundPlay : public QObject
{
    Q_OBJECT
public:
    explicit SoundPlay(QObject *parent = 0) : QObject(parent)
    {
        context = new QQmlContext(engine.rootContext());

        component = new QQmlComponent(&engine, QUrl("qrc:/main.qml"));
        object = component->create(context);

        if (!component->isReady())
        {
            qDebug() << "Failed to create QML Component. Error=" << component->errorString();
        }
    }

    ~SoundPlay()
    {

    }
private:
    QQmlApplicationEngine engine;
    QQmlContext *context;
    QQmlComponent *component;
    QObject *object;


private slots:
};

#endif
