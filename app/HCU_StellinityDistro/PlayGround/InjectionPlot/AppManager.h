#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDebug>
#include <QQmlComponent>

class AppManager : public QObject
{
    Q_OBJECT
public:
    explicit AppManager(QObject *parent = 0) : QObject(parent)
    {
        context = new QQmlContext(engine.rootContext());
        component = new QQmlComponent(&engine, QUrl("qrc:/main.qml"));
        qmlRoot = component->create(context);

        if (!component->isReady())
        {
            qDebug() << "Failed to create QML Component. Error=" << component->errorString();
        }

        qmlInjectionPlot = qmlRoot->findChild<QObject*>("InjectionPlot");

        connect(&tmrTest, &QTimer::timeout, this, [=] {
            static int x = 0;
            qint64 endTimeMs = 3000000;

            if (x == 0)
            {
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotReset");

                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetRange", Q_ARG(QVariant, 0), Q_ARG(QVariant, endTimeMs));

                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetPhaseInfo", Q_ARG(QVariant, 0), Q_ARG(QVariant, 0), Q_ARG(QVariant, "red"), Q_ARG(QVariant, "red"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetPhaseInfo", Q_ARG(QVariant, 1), Q_ARG(QVariant, 10000), Q_ARG(QVariant, "orange"), Q_ARG(QVariant, "orange"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetPhaseInfo", Q_ARG(QVariant, 2), Q_ARG(QVariant, 20000), Q_ARG(QVariant, "yellow"), Q_ARG(QVariant, "green"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetPhaseInfo", Q_ARG(QVariant, 3), Q_ARG(QVariant, 30000), Q_ARG(QVariant, "green"), Q_ARG(QVariant, "blue"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetPhaseInfo", Q_ARG(QVariant, 4), Q_ARG(QVariant, 40000), Q_ARG(QVariant, "blue"), Q_ARG(QVariant, "purple"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetPhaseInfo", Q_ARG(QVariant, 5), Q_ARG(QVariant, 50000), Q_ARG(QVariant, "purple"), Q_ARG(QVariant, "brown"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotSetEndTime", Q_ARG(QVariant, 6), Q_ARG(QVariant, 60000));


                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddPhaseSeries", Q_ARG(QVariant, "red"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 0), Q_ARG(QVariant, 0), Q_ARG(QVariant, 50));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 0), Q_ARG(QVariant, 10000), Q_ARG(QVariant, 50));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddPhaseSeries", Q_ARG(QVariant, "orange"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 1), Q_ARG(QVariant, 10000), Q_ARG(QVariant, 100));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 1), Q_ARG(QVariant, 20000), Q_ARG(QVariant, 100));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddPhaseSeries", Q_ARG(QVariant, "green"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 2), Q_ARG(QVariant, 20000), Q_ARG(QVariant, 150));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 2), Q_ARG(QVariant, 30000), Q_ARG(QVariant, 150));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddPhaseSeries", Q_ARG(QVariant, "blue"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 3), Q_ARG(QVariant, 30000), Q_ARG(QVariant, 200));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 3), Q_ARG(QVariant, 40000), Q_ARG(QVariant, 200));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddPhaseSeries", Q_ARG(QVariant, "purple"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 4), Q_ARG(QVariant, 40000), Q_ARG(QVariant, 250));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 4), Q_ARG(QVariant, 50000), Q_ARG(QVariant, 250));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddPhaseSeries", Q_ARG(QVariant, "brown"));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 5), Q_ARG(QVariant, 50000), Q_ARG(QVariant, 300));
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 5), Q_ARG(QVariant, 60000), Q_ARG(QVariant, 300));
                x = 60000;
            }
            else
            {
                x += 1000000;
            }

            if (x > 0)
            {
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotAddData", Q_ARG(QVariant, 5), Q_ARG(QVariant, x), Q_ARG(QVariant, 1600));
            }

            if (x > endTimeMs)
            {
                QMetaObject::invokeMethod(qmlInjectionPlot, "slotTest");
                tmrTest.stop();
            }
        });
        tmrTest.start(50);

    }

    ~AppManager()
    {
    }

private:
    QQmlApplicationEngine engine;
    QQmlContext *context;
    QQmlComponent *component;
    QObject *qmlInjectionPlot;
    QObject *qmlRoot;
    QTimer tmrTest;

private slots:

};

#endif
