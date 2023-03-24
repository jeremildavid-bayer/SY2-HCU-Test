#include "Config.h"
#include "Common.h"
#include "ImrParser.h"

Config::Config(QString pathConfig_, EnvGlobal *env_, EnvLocal *envLocal_) :
        pathConfig(pathConfig_),
        env(env_),
        envLocal(envLocal_)
{
    cfgOk = false;
    parseErr.error = QJsonParseError::NoError;
    readCfgFile();
}

Config::~Config()
{
}

void Config::setPathConfig(QString pathConfig_)
{
    pathConfig = pathConfig_;
}

QString Config::getPathConfig()
{
    return pathConfig;
}

bool Config::isExist(QString name)
{
    return (cfgMap.contains(name));
}

void Config::get(QString name, Item &dst)
{
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap itemMap = cfgMap[name].toMap();
        QString err;
        dst = ImrParser::ToCpp_ConfigItem(itemMap, &err);
    }
    mutexConfigs.unlock();
}

void Config::get(QString name, bool &dst)
{
    mutexConfigs.lock();
    dst = false;
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")].toBool();
    }
    mutexConfigs.unlock();
}

void Config::get(QString name, int &dst)
{
    dst = 0;
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")].toInt();
    }
    mutexConfigs.unlock();
}

void Config::get(QString name, double &dst)
{
    dst = 0;
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")].toDouble();
    }
    mutexConfigs.unlock();
}

void Config::get(QString name, QString &dst)
{
    dst = "";
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")].toString();
    }
    mutexConfigs.unlock();
}

void Config::get(QString name, QVariantMap &dst)
{
    dst = QVariantMap();
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")].toMap();
    }
    mutexConfigs.unlock();
}

void Config::get(QString name, QVariantList &dst)
{
    dst = QVariantList();
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")].toList();
    }
    mutexConfigs.unlock();
}

QVariant Config::get(QString name)
{
    QVariant dst = QVariant();
    mutexConfigs.lock();
    if (cfgMap.contains(name))
    {
        QVariantMap map = cfgMap[name].toMap();
        dst = map[_L("Value")];
    }
    mutexConfigs.unlock();
    return dst;
}

void Config::set(QString keyName, QString val)
{
    Item item;
    get(keyName, item);
    item.keyName = keyName;
    item.validDataType = "string";
    item.value = QVariant(val);
    setItem(item);
}

void Config::set(QString keyName, int val)
{
    Item item;
    get(keyName, item);
    item.keyName = keyName;
    item.validDataType = "int";
    item.value = QVariant(val);
    setItem(item);
}

void Config::set(QString keyName, double val)
{
    Item item;
    get(keyName, item);
    item.keyName = keyName;
    item.validDataType = "double";
    item.value = QVariant(val);
    setItem(item);
}

void Config::set(QString keyName, bool val)
{
    Item item;
    get(keyName, item);
    item.keyName = keyName;
    item.validDataType = "bool";
    item.value = QVariant(val);
    setItem(item);
}

void Config::set(QString keyName, QVariantMap val)
{
    Item item;
    get(keyName, item);
    item.keyName = keyName;
    item.validDataType = "map";
    item.value = val;
    setItem(item);
}

void Config::set(QString keyName, QVariantList val)
{
    Item item;
    get(keyName, item);
    item.keyName = keyName;
    item.validDataType = "list";
    item.value = val;
    setItem(item);
}

void Config::set(Item item)
{
    // Item contains the changedAt time, so don't overwrite it
    setItem(item, false);
}

void Config::set(QString keyName, Item item)
{
    item.keyName = keyName;
    setItem(item);
}

bool Config::isCfgOk()
{
    return cfgOk;
}

QJsonParseError Config::parseError()
{
    return parseErr;
}

bool Config::readCfgFile()
{
    if (pathConfig == "")
    {
        return false;
    }

    cfgOk = false;

    // Acquire info from file and parse data
    mutexConfigs.lock();
    QFile fileBuf(pathConfig);
    QByteArray jsonStr;
    if (fileBuf.open(QFile::ReadWrite | QFile::Text))
    {
        jsonStr = fileBuf.readAll();
        fileBuf.close();
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr, &parseErr);
    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap root = jsonDoc.toVariant().toMap();
        cfgMap = root[_L("Config")].toMap();
        if (cfgMap.size() > 0)
        {
            cfgOk = true;
        }
    }
    mutexConfigs.unlock();

    return cfgOk;
}

void Config::saveCfgFile()
{
    if (pathConfig == "")
    {
        return;
    }

    mutexConfigs.lock();

    QVariantMap root;
    root.insert("Config", cfgMap);

    QByteArray jsonStr = Util::qVarientToJsonData(root, false);
    QFile fileBuf(pathConfig);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write(jsonStr);
        fileBuf.close();
    }

    mutexConfigs.unlock();
}

void Config::initCfgFile()
{
    if (pathConfig == "")
    {
        return;
    }

    QFile fileBuf(pathConfig);
    if (fileBuf.open(QFile::WriteOnly | QFile::Text))
    {
        fileBuf.write("{ \"Config\": {} }\n");
        fileBuf.close();
    }
    readCfgFile();
}

void Config::setCfgMap(QVariantMap cfgMap_)
{
    mutexConfigs.lock();
    cfgMap = cfgMap_;
    mutexConfigs.unlock();
}

QVariantMap Config::getCfgMap()
{
    QVariantMap ret;
    mutexConfigs.lock();
    ret = cfgMap;
    mutexConfigs.unlock();
    return ret;
}

Config::Item Config::initItem(QString keyName,
                              int displayIndex,
                              QVariant defaultValue,
                              QString units,
                              QString validDataType,
                              QVariantList validList,
                              QVariant lowerLimitInclusive,
                              QVariant upperLimitInclusive,
                              QVariant resolutionIncrement,
                              LegacyKeyNames oldNamesList)
{
    Item cfgItem;

    // if keyName doesn't exist, check if anything matches from oldNamesList
    QString matchingKeyName = keyName;
    while (!isExist(matchingKeyName) && !oldNamesList.isEmpty())
    {
        matchingKeyName = oldNamesList.takeLast();
    }

    if (isExist(matchingKeyName))
    {
        Item existItem;
        get(matchingKeyName, existItem);
        cfgItem.changedAtEpochMs = existItem.changedAtEpochMs;
        cfgItem.value = existItem.value;
    }
    else
    {
        cfgItem.changedAtEpochMs = -1;
        cfgItem.value = defaultValue;
    }

    cfgItem.keyName = keyName;
    cfgItem.displayIndex = displayIndex;
    cfgItem.defaultValue = defaultValue;
    cfgItem.units = units;
    cfgItem.validDataType = validDataType;
    cfgItem.validList = validList;
    cfgItem.validRange.lowerLimitInclusive = lowerLimitInclusive;
    cfgItem.validRange.upperLimitInclusive = upperLimitInclusive;
    cfgItem.validRange.resolutionIncrement = resolutionIncrement;

    return cfgItem;
}

void Config::setItem(Item item, bool setChangedAt)
{
    if (!verifyItemValue(item))
    {
        // Item value verification failed. Set to default
        QVariant val = item.value;
        val = item.defaultValue;
        item.value = val;
        if (pathConfig != "")
        {
            LOG_INFO("Config Verification Failed: Cfg[%s]: Set to Default\n", item.keyName.CSTR());
        }
    }

    if (setChangedAt)
    {
        item.changedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    }

    QVariantMap itemMap = ImrParser::ToImr_ConfigItem(item);

    mutexConfigs.lock();
    cfgMap.insert(item.keyName, itemMap);
    mutexConfigs.unlock();

    QString value = Util::qVarientToJsonData(itemMap, false);

    QString type = getenv("BUILD_TYPE");
    bool anonymizeOn = (type == BUILD_TYPE_VNV || type == BUILD_TYPE_REL);

    // Anonymize request string depending on configuration key name
    if ((anonymizeOn) && (item.keyName == "Hidden_LastExamAdvanceInfo"))
    {
        // Anonymize by not including the value
        value = ANONYMIZED_STR;
    }

    if (pathConfig != "")
    {
        LOG_INFO("Config Changed: Cfg[%s] = %s\n", item.keyName.CSTR(), value.CSTR());
    }

    saveCfgFile();
}

bool Config::verifyItemValue(Item &item)
{
    if (item.validList.length() > 0)
    {
        QVariant val = item.value;
        if (!item.validList.contains(val))
        {
            return false;
        }
    }
    else if (item.validDataType == _L("int"))
    {
        int val = item.value.toInt();
        if ( (val < item.validRange.lowerLimitInclusive.toInt()) ||
             (val > item.validRange.upperLimitInclusive.toInt()) )
        {
            return false;
        }
    }
    else if (item.validDataType == _L("double"))
    {
        double val = item.value.toDouble();
        if ( (val < item.validRange.lowerLimitInclusive.toDouble()) ||
             (val > item.validRange.upperLimitInclusive.toDouble()) )
        {
            return false;
        }
    }
    return true;
}


bool Config::operator==(const QVariantMap &newCfgMap)
{
    bool ret = true;
    mutexConfigs.lock();
    QStringList lstNewKeys = newCfgMap.keys();
    QStringList lstOldKeys = cfgMap.keys();

    if (lstNewKeys.length() == lstOldKeys.length())
    {
        for (int i = 0; i < lstOldKeys.length(); i++)
        {
            if (cfgMap.contains(lstNewKeys.at(i)))
            {
                QVariantMap mapItemOld = cfgMap[lstNewKeys.at(i)].toMap();
                QVariantMap mapItemNew = newCfgMap[lstNewKeys.at(i)].toMap();
                QString err;
                Config::Item itemOld = ImrParser::ToCpp_ConfigItem(mapItemOld, &err);
                Config::Item itemNew = ImrParser::ToCpp_ConfigItem(mapItemNew, &err);
                if (itemOld != itemNew)
                {
                    ret = false;
                    break;
                }
            }
        }
    }
    else
    {
        ret = false;
    }

    mutexConfigs.unlock();

    return ret;
}

bool Config::operator!=(const QVariantMap &newCfgMap)
{
    bool ret = false;

    mutexConfigs.lock();
    ret = !operator ==(newCfgMap);
    mutexConfigs.unlock();

    return ret;
}
