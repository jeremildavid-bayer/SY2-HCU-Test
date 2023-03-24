#ifndef TEST_MSG_H
#define TEST_MSG_H

#include <QDateTime>
#include <QJsonDocument>
#include <QVariantMap>
#include "Common/Util.h"

class TestMsg
{
public:
    QVariantMap params;

    TestMsg()
    {
        init();
    }

    ~TestMsg()
    {
    }

    void init()
    {
        params.insert("TestGuid", Util::newGuid());
        params.insert("TestString", "");
        params.insert("TestBool", false);
        params.insert("ThrowProcessingError", false);
        params.insert("TestInt", 0);
        params.insert("TestDouble", 0);
        params.insert("TestDateTime", QDateTime::currentDateTimeUtc());
        params.insert("TestList", QVariantList());
        params.insert("TestDictionary", QVariantMap());
        params.insert("MessageNumber", 0);
        params.insert("RequestAt", QDateTime::currentDateTimeUtc());
        params.insert("ResponseAt", QDateTime::currentDateTimeUtc());
        params.insert("ProcessingMillis", 0);
    }

    QString serialize() const
    {
        QVariantMap paramsTemp = params;

        if (paramsTemp.contains("TestDateTime"))
        {
            // Change QDateTime to .NetTimeStr.
            QDateTime dateTime = QVariant(paramsTemp["TestDateTime"]).toDateTime();
            paramsTemp["TestDateTime"] = Util::qDateTimeToUtcDateTimeStr(dateTime);
        }

        if (paramsTemp.contains("RequestAt"))
        {
            QDateTime dateTime = QVariant(paramsTemp["RequestAt"]).toDateTime();
            paramsTemp["RequestAt"] = Util::qDateTimeToUtcDateTimeStr(dateTime);
        }

        if (paramsTemp.contains("ResponseAt"))
        {
            QDateTime dateTime = QVariant(paramsTemp["ResponseAt"]).toDateTime();
            paramsTemp["ResponseAt"] = Util::qDateTimeToUtcDateTimeStr(dateTime);
        }

        return Util::qVarientToJsonData(paramsTemp);
    }

    void parse(QString json, QJsonParseError &err)
    {
        QJsonDocument document = QJsonDocument::fromJson(json.toUtf8(), &err);
        params = document.toVariant().toMap();

        // Change .NetTimeStr to QDateTime.
        if (params.contains("TestDateTime"))
        {
            QString dateTimeStr = QVariant(params["TestDateTime"]).toString();
            params["TestDateTime"] = Util::utcDateTimeStrToQDateTime(dateTimeStr);
        }

        if (params.contains("RequestAt"))
        {
            QString dateTimeStr = QVariant(params["RequestAt"]).toString();
            params["RequestAt"] = Util::utcDateTimeStrToQDateTime(dateTimeStr);
        }

        if (params.contains("ResponseAt"))
        {
            QString dateTimeStr = QVariant(params["ResponseAt"]).toString();
            params["ResponseAt"] = Util::utcDateTimeStrToQDateTime(dateTimeStr);
        }
    }
};

#endif // TEST_MSG_H
