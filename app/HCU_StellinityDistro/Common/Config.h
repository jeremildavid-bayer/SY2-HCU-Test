#ifndef CONFIG_H
#define CONFIG_H

#include <QJsonParseError>
#include <QFile>
#include <QVariant>
#include <QMutex>
#include "Common/Common.h"

//====================================================================
// CLASS: Config
// Manages read/write configuration file.
// Configuration file is a JSON type file and has following structure:
// {
//       "Config": {
//          "<CFG NAME>": {
//              "Description": "",
//              "TYPE": "<STRING/BOOL/INT>",
//              "Value": "<VALUE>"
//          }, ... {}
//      }
// }
//====================================================================

class Config
{
public:
    struct ValidRange
    {
        QVariant lowerLimitInclusive;
        QVariant upperLimitInclusive;
        QVariant resolutionIncrement;

        bool operator==(const ValidRange &arg) const
        {
            bool equal = ( (lowerLimitInclusive == arg.lowerLimitInclusive) &&
                           (upperLimitInclusive == arg.upperLimitInclusive) &&
                           (resolutionIncrement == arg.resolutionIncrement) );
            return equal;
        }

        bool operator!=(const ValidRange &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QStringList LegacyKeyNames;

    struct Item
    {
        int displayIndex;
        QVariant value;
        QVariant defaultValue;
        QVariantList validList;
        QString keyName;
        QString units; // e.g. 'cm'
        QString validDataType; // e.g. string, int, double, bool
        ValidRange validRange;
        qint64 changedAtEpochMs;

        bool compareConfigValue(const Item &arg) const
        {
            bool equal = ( (displayIndex == arg.displayIndex) &&
                           (value == arg.value) &&
                           (defaultValue == arg.defaultValue) &&
                           (validList == arg.validList) &&
                           (keyName == arg.keyName) &&
                           (units == arg.units) &&
                           (validDataType == arg.validDataType) &&
                           (validRange == arg.validRange) );
            return equal;
        }

        bool operator==(const Item &arg) const
        {
            bool equal = ( (displayIndex == arg.displayIndex) &&
                           (value == arg.value) &&
                           (defaultValue == arg.defaultValue) &&
                           (validList == arg.validList) &&
                           (keyName == arg.keyName) &&
                           (units == arg.units) &&
                           (validDataType == arg.validDataType) &&
                           (validRange == arg.validRange) &&
                           (changedAtEpochMs == arg.changedAtEpochMs) );
            return equal;
        }

        bool operator!=(const Item &arg) const
        {
            return !operator==(arg);
        }
    };

    explicit Config(QString pathConfig_ = "", EnvGlobal *env_ = NULL, EnvLocal *envLocal_ = NULL);
    ~Config();
    void setPathConfig(QString pathConfig);
    QString getPathConfig();
    bool isExist(QString name);
    bool isCfgOk();
    QJsonParseError parseError();
    bool operator==(const QVariantMap &newCfgMap);
    bool operator!=(const QVariantMap &newCfgMap);

    void get(QString name, Item &dst);
    void get(QString name, bool &dst);
    void get(QString name, int &dst);
    void get(QString name, double &dst);
    void get(QString name, QString &dst);
    void get(QString name, QVariantMap &dst);
    void get(QString name, QVariantList &dst);
    QVariant get(QString name);
    QVariantMap getCfgMap();

    void set(QString keyName, QString val);
    void set(QString keyName, int val);
    void set(QString keyName, double val);
    void set(QString keyName, bool val);
    void set(QString keyName, QVariantMap val);
    void set(QString keyName, QVariantList val);
    void set(Item item);
    void set(QString keyName, Item item);
    void setCfgMap(QVariantMap cfgMap_);

    static bool verifyItemValue(Item &item);

    Item initItem(QString keyName, int displayIndex, QVariant defaultValue, QString units, QString validDataType, QVariantList validList, QVariant lowerLimitInclusive, QVariant upperLimitInclusive, QVariant resolutionIncrement, LegacyKeyNames oldNamesList = LegacyKeyNames());
    void initCfgFile();
    bool readCfgFile();
    void saveCfgFile();

private:
    QString pathConfig;
    EnvGlobal *env;
    EnvLocal *envLocal;
    QVariantMap cfgMap;
    bool cfgOk;
    QJsonParseError parseErr;
    QMutex mutexConfigs;

    void setItem(Item item, bool setChangedAt = true);
};

/**
 * Q_DECLARE_METATYPE
 * Note from qt: https://doc.qt.io/qt-6/qtcore-threads-queuedcustomtype-example.html
 * We will still need to register it with the meta-object system at
 * run-time by calling the qRegisterMetaType() template function before
 * we make any signal-slot connections that use this type. Even though we
 * do not intend to use the type with QVariant in this example,
 * it is good practice to also declare the new type with Q_DECLARE_METATYPE().
 *
 * Note when the signal and slot are from 2 different thread the data is "serialised"/copied between thread.
 */
Q_DECLARE_METATYPE(Config::Item);

#endif // CONFIG_H
