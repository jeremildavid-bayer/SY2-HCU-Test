#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <QUuid>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QDebug>
#include <QQmlComponent>
#include <QThread>
#include <QMutex>

class AppManager : public QObject
{
    Q_OBJECT
public:
    explicit AppManager(QObject *parent = 0) : QObject(parent)
    {
        context = new QQmlContext(engine.rootContext());

        initModelOptions();
        initModelSteps();

        component = new QQmlComponent(&engine, QUrl("qrc:/main.qml"));
        object = component->create(context);

        if (!component->isReady())
        {
            qDebug() << "Failed to create QML Component. Error=" << component->errorString();
        }

        qmlSrc = object->findChild<QObject*>("mainScreen");
        connect(qmlSrc, SIGNAL(signalSwapRows(int, int)), this, SLOT(slotSwapRows(int, int)));
        connect(qmlSrc, SIGNAL(signalSwapSteps(int, int)), this, SLOT(slotSwapSteps(int, int)));
        connect(qmlSrc, SIGNAL(signalRemoveDummyPhase()), this, SLOT(slotRemoveDummyPhase()));
        connect(qmlSrc, SIGNAL(signalSetDummyPhase(int, QString)), this, SLOT(slotSetDummyPhase(int, QString)));
        connect(qmlSrc, SIGNAL(signalSetStepIdForPhase(int, QString)), this, SLOT(slotSetStepIdForPhase(int, QString)));
        connect(qmlSrc, SIGNAL(signalInsertPhaseFromDummy(QString)), this, SLOT(slotInsertPhaseFromDummy(QString)));
        connect(qmlSrc, SIGNAL(signalInsertStepFromDummy(QString)), this, SLOT(slotInsertStepFromDummy(QString)));

        QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
    }

    ~AppManager()
    {

    }
private:
    QQmlApplicationEngine engine;
    QQmlContext *context;
    QQmlComponent *component;
    QObject *object;
    QObject *qmlSrc;
    QVariantList listPhaseValues;
    QVariantList listOptValues;
    QTimer tmrPop;

    void initModelOptions()
    {
        QVariantMap map;
        map.insert("titleColor", "#888888");
        map.insert("titleText", "TEST INJECTION");
        map.insert("titleTextColor", "white");
        map.insert("isStepType", true);
        listOptValues.append(map);

        map.insert("titleColor", "#555555");
        map.insert("titleText", "TEST INJECTION");
        map.insert("titleTextColor", "white");
        map.insert("isStepType", true);
        listOptValues.append(map);

        map.insert("titleColor", "blue");
        map.insert("titleText", "Saline");
        map.insert("titleTextColor", "white");
        map.insert("isStepType", false);
        listOptValues.append(map);

        map.insert("titleColor", "green");
        map.insert("titleText", "Ultravist 300");
        map.insert("titleTextColor", "white");
        map.insert("isStepType", false);
        listOptValues.append(map);

        map.insert("titleColor", "purple");
        map.insert("titleText", "Dual Flow");
        map.insert("titleTextColor", "white");
        map.insert("isStepType", false);
        listOptValues.append(map);

        map.insert("titleColor", "#aaaaaa");
        map.insert("titleText", "Delay");
        map.insert("titleTextColor", "white");
        map.insert("isStepType", false);
        listOptValues.append(map);

        context->setContextProperty("prop_listOptValues", listOptValues);
    }

    void initModelSteps()
    {
        QVariantMap mapPhase;

        mapPhase.insert("titleColor", "#ffff0001");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP1");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ffff8802");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP1");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff00ff03");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP2");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff88ff04");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP2");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ffccff05");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP2");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff0000f6");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP3");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff0088f7");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP3");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff00ccf8");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP3");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff00fff9");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP3");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff0000fa");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP3");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff3388fb");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP4");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff33ccfc");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP4");
        listPhaseValues.append(mapPhase);

        mapPhase.insert("titleColor", "#ff33fffd");
        mapPhase.insert("isDummy", false);
        mapPhase.insert("stepId", "STEP4");
        listPhaseValues.append(mapPhase);
    }


    QVariantList getPhaseValues(int stepIndex)
    {
        QVariantList ret;
        int curStepIdx = -1;
        QString lastStepTitle = "";

        for (int i = 0; i < listPhaseValues.length(); i++)
        {
            QString curSortingStepTitle = listPhaseValues[i].toMap()["stepId"].toString();
            if (listPhaseValues[i].toMap()["stepId"] != lastStepTitle)
            {
                curStepIdx++;
                lastStepTitle = curSortingStepTitle;
            }

            if (curStepIdx == stepIndex)
            {
                ret.append(listPhaseValues[i]);
            }
        }

        return ret;
    }

private slots:
    void slotRemoveDummyPhase()
    {
        for (int i = 0; i < listPhaseValues.length(); i++)
        {
            if (listPhaseValues[i].toMap()["isDummy"].toBool())
            {
                listPhaseValues.removeAt(i);
                QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
                break;
            }
        }
    }

    void slotSetDummyPhase(int rowIdx, QString stepId)
    {
        for (int i = 0; i < listPhaseValues.length(); i++)
        {
            if (listPhaseValues[i].toMap()["isDummy"].toBool())
            {
                listPhaseValues.removeAt(i);
                break;
            }
        }

        QVariantMap mapPhase;
        mapPhase.insert("isDummy", true);
        mapPhase.insert("stepId", stepId);
        listPhaseValues.insert(rowIdx, mapPhase);

        QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
    }

    void slotInsertStepFromDummy(QString titleColor)
    {
        for (int i = 0; i < listPhaseValues.length(); i++)
        {
            QVariantMap mapPhase = listPhaseValues[i].toMap();
            if (mapPhase["isDummy"].toBool())
            {
                mapPhase.insert("isDummy", false);
                mapPhase.insert("titleColor", titleColor);
                mapPhase.insert("stepId", QVariant("Step " + QUuid::createUuid().toString()));
                listPhaseValues[i] = mapPhase;
                QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
                break;
            }
        }
    }

    void slotInsertPhaseFromDummy(QString titleColor)
    {
        for (int i = 0; i < listPhaseValues.length(); i++)
        {
            QVariantMap mapPhase = listPhaseValues[i].toMap();
            if (mapPhase["isDummy"].toBool())
            {
                mapPhase.insert("isDummy", false);
                mapPhase.insert("titleColor", titleColor);
                listPhaseValues[i] = mapPhase;
                QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
                break;
            }
        }
    }

    void slotSetStepIdForPhase(int rowIdx, QString stepId)
    {
        QVariantMap mapPhase = listPhaseValues[rowIdx].toMap();
        mapPhase.insert("stepId", stepId);
        listPhaseValues[rowIdx] = mapPhase;

        QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
    }


    void slotSwapSteps(int from, int to)
    {
        QVariantList listStep;

        int i = 0;

        while (1)
        {
            QVariantList phases = getPhaseValues(i++);
            if (phases.length() == 0)
            {
                break;
            }

            listStep.append(QVariant(phases));
        }

        listStep.swap(from, to);


        QVariantList newList;

        for (int i = 0; i < listStep.length(); i++)
        {
            QVariantList phases = listStep[i].toList();
            for (int j = 0; j < phases.length(); j++)
            {
                newList.append(phases[j].toMap());
            }
        }
        listPhaseValues = newList;
        QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
    }

    void slotSwapRows(int from, int to)
    {
        if (listPhaseValues[from].toMap()["stepId"].toString() != listPhaseValues[to].toMap()["stepId"].toString())
        {
            // Swapping phases rows with different step areas.
            QVariantMap mapTo = listPhaseValues[to].toMap();
            QVariantMap mapFrom = listPhaseValues[from].toMap();
            mapFrom["stepId"] = mapTo["stepId"];
            listPhaseValues[from] = mapFrom;
        }


        // swap row
        listPhaseValues.swap(from, to);

        QMetaObject::invokeMethod(qmlSrc, "slotListPhaseValuesChanged", Q_ARG(QVariant, listPhaseValues));
    }
};

#endif
