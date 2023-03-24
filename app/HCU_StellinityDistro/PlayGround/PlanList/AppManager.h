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

        QVariantMap mapItem;
        mapItem.insert("titleText", "Title1");
        mapItem.insert("expanded", false);
        mapItem.insert("items", QStringList() << "Item1.1");
        listValue.append(mapItem);

        mapItem.insert("titleText", "Title2");
        mapItem.insert("expanded", false);
        mapItem.insert("items", QStringList() << "Item1.1" << "Item1.2");
        listValue.append(mapItem);

        mapItem.insert("titleText", "Title3");
        mapItem.insert("expanded", false);
        mapItem.insert("items", QStringList() << "Item1.1" << "Item1.2" << "Item1.3");
        listValue.append(mapItem);

        mapItem.insert("titleText", "Title4");
        mapItem.insert("expanded", false);
        mapItem.insert("items", QStringList() << "Item1.1" << "Item1.2" << "Item1.3" << "Item4.4");
        listValue.append(mapItem);


        context->setContextProperty("prop_listValues", listValue);
        component = new QQmlComponent(&engine, QUrl("qrc:/main.qml"));
        object = component->create(context);


        if (!component->isReady())
        {
            qDebug() << "Failed to create QML Component. Error=" << component->errorString();
        }

        QObject *qmlSrc = object->findChild<QObject*>("listMain");
        qDebug() << "qmlSrc=" << qmlSrc;
        connect(qmlSrc, SIGNAL(itemClicked(int, QString)), this, SLOT(slotItemClicked(int,QString)));
        connect(qmlSrc, SIGNAL(itemsExpanded(int, bool)), this, SLOT(slotItemExpanded(int, bool)));

        connect(&tmrPop, &QTimer::timeout, this, [=](){
            QVariantMap mapItem;
            QString title = "Title" + QString().setNum(listValue.length()+1);
            mapItem.insert("titleText", title);
            mapItem.insert("expanded", false);
            mapItem.insert("items", QStringList() << "Item1.1");
            listValue.append(mapItem);
            context->setContextProperty("prop_listValues", listValue);

            if (listValue.length() == 10)
            {
                tmrPop.stop();
            }
        });
        tmrPop.start(100);
    }

    ~AppManager()
    {

    }
private:
    QQmlApplicationEngine engine;
    QQmlContext *context;
    QQmlComponent *component;
    QObject *object;
    QVariantList listValue;
    QTimer tmrPop;

private slots:
    void slotItemClicked(int index, QString item)
    {
        qDebug() << index << "-" << item;
    }

    void slotItemExpanded(int index, bool expanded)
    {
        QVariantMap mapRow = listValue[index].toMap();
        mapRow["expanded"] = expanded;
        listValue[index].setValue(mapRow);

        context->setContextProperty("prop_listValues", listValue);
    }
};

#endif
