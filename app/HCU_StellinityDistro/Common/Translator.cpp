#include <QFile>
#include "Translator.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"

Translator::Translator(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("Translator", "TRANSLATOR");
    env->qml.engine->rootContext()->setContextProperty("dsTranslator", this);

    setBundle(bundleCdXx, "cd-XX");
}

Translator::~Translator()
{
    delete envLocal;
}

void Translator::setBundle(Bundle &outBundle, QString cultureCode)
{
    outBundle.cultureCode = cultureCode;

    if (outBundle.cultureCode == _L("db-XX"))
    {
        return;
    }

    QString filePath = QString().asprintf("%s/%s/%s", PATH_RESOURCES_PHRASES, outBundle.cultureCode.CSTR(), PATH_RESOURCES_PHRASE_I18N_BUNDLE_FILENAME);
    QFile fileBuf(filePath);

    outBundle.err = "";
    outBundle.mapPhrase.clear();

    if (fileBuf.open(QFile::ReadOnly | QFile::Text))
    {
        QJsonParseError parseErr;
        QString jsonStr = fileBuf.readAll();
        fileBuf.close();
        QJsonDocument document = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);

        if (parseErr.error == QJsonParseError::NoError)
        {
            QVariantMap jsonBundle = document.toVariant().toMap();
            if (jsonBundle.contains(_L("Phrases")))
            {
                QVariantMap jsonPhrases = jsonBundle[_L("Phrases")].toMap();
                QMap<QString, QVariant>::const_iterator i = jsonPhrases.begin();

                while (i != jsonPhrases.end())
                {
                    Phrase phrase;
                    QString phraseKey = i.key();
                    QVariantMap jsonPhrase = i.value().toMap();

                    if (jsonPhrase.contains(_L("Translation")))
                    {
                        phrase.translation = jsonPhrase[_L("Translation")].toString();
                    }
                    else
                    {
                        outBundle.err = QString().asprintf("Failed to find translation, cultureCode=%s, path=%s, phraseKey=%s", outBundle.cultureCode.CSTR(), filePath.CSTR(), phraseKey.CSTR());
                        break;
                    }
                    outBundle.mapPhrase.insert(phraseKey.toLower(), phrase);
                    i++;
                }
            }
            else
            {
                outBundle.err = QString().asprintf("Failed to find phrase bundle file, cultureCode=%s, path=%s", outBundle.cultureCode.CSTR(), filePath.CSTR());
            }
        }
        else
        {
            outBundle.err = QString().asprintf("Failed to parse phrase bundle file, cultureCode=%s, path=%s, err=%s", outBundle.cultureCode.CSTR(), filePath.CSTR(), parseErr.errorString().CSTR());
        }
    }
    else
    {
        outBundle.err = QString().asprintf("Failed to open phrase bundle file, cultureCode=%s, path=%s", outBundle.cultureCode.CSTR(), filePath.CSTR());
    }

    if (outBundle.err == "")
    {
        LOG_INFO("SetInitBundle(): Culture code(%s) is initialised\n", outBundle.cultureCode.CSTR());
    }
    else
    {
        LOG_ERROR("SetInitBundle(): Culture code(%s) is failed to initialised with err(%s)\n", outBundle.cultureCode.CSTR(), outBundle.err.CSTR());
    }

    if (outBundle.cultureCode == _L("cd-XX"))
    {
        // Reload cd-XX bundle
        bundleCdXx = outBundle;
    }
}

QString Translator::translate(QString sourceStr, QString cultureCode)
{
    if (sourceStr == "")
    {
        return sourceStr;
    }

    if ( (sourceStr == _L("RS0")) ||
         (sourceStr == _L("RC1")) ||
         (sourceStr == _L("RC2")) ||
         (sourceStr == _L("BS0")) ||
         (sourceStr == _L("BC1")) ||
         (sourceStr == _L("BC2")) ||
         (sourceStr == _L("ML")) ||
         (sourceStr == _L("WC")) )
    {
        return translate("T_FluidSourceKey_" + sourceStr);
    }

    if (cultureCode == "")
    {
        cultureCode = env->ds.cfgGlobal->get_Settings_General_CultureCode();
    }

    if (cultureCode == _L("db-XX"))
    {
        // db-XX translation returns cd-XX value with all vowels doubled.
        QString output = translate(sourceStr, "cd-XX");
        return translateDBXX(output);
    }

    QString key, err;
    QStringList dataList;
    parse(sourceStr, &key, &dataList, &err);
    QString outStr = sourceStr;

    if (key == "")
    {
        LOG_ERROR("Bad source string format, cultureCode=%s, sourceStr=%s\n", cultureCode.CSTR(), sourceStr.CSTR());
        return outStr;
    }

    if (err != "")
    {
        LOG_ERROR("Failed to parse the sourceString, cultureCode=%s, sourceStr=%s, err=%s\n", cultureCode.CSTR(), sourceStr.CSTR(), err.CSTR());
        return outStr;
    }

    // Get bundle.
    key = key.toLower();
    Bundle *bundle;

    if (curBundle.cultureCode != cultureCode)
    {
        setBundle(curBundle, cultureCode);

        // Get alternative bundle
        if (cultureCode == _L("de-CH"))
        {
            setBundle(curBundle2, "de-DE");
        }
        else if (cultureCode == _L("fr-CA"))
        {
            setBundle(curBundle2, "fr-FR");
        }
        else if (cultureCode == _L("fr-CH"))
        {
            setBundle(curBundle2, "fr-FR");
        }
        else if (cultureCode == _L("fr-BE"))
        {
            setBundle(curBundle2, "fr-FR");
        }
        else if (cultureCode == _L("it-CH"))
        {
            setBundle(curBundle2, "it-IT");
        }
        else if (cultureCode == _L("nl-BE"))
        {
            setBundle(curBundle2, "nl-NL");
        }
        else
        {
            curBundle2 = bundleCdXx;
        }
    }

    if ( (curBundle.err == "") &&
         (curBundle.mapPhrase.contains(key)) )
    {
        // Ok to use curBundle
        bundle = &curBundle;
    }
    else
    {
        if ( (curBundle2.err == "") &&
             (curBundle2.mapPhrase.contains(key)) )
        {
            bundle = &curBundle2;
        }
        else if ( (bundleCdXx.err == "") &&
                  (bundleCdXx.mapPhrase.contains(key)) )
        {
            bundle = &bundleCdXx;
        }
        else
        {
            LOG_ERROR("Phrase not found: cultureCode=%s, sourceStr=%s, key=%s\n", cultureCode.CSTR(), sourceStr.CSTR(), key.CSTR());
            return sourceStr;
        }
        LOG_WARNING("Phrase not found: cultureCode=%s, sourceStr=%s, key=%s. Using Other CultureCode(%s)..\n", cultureCode.CSTR(), sourceStr.CSTR(), key.CSTR(), bundle->cultureCode.CSTR());
    }

    Phrase phrase = bundle->mapPhrase[key];
    if (phrase.translation == "")
    {
        bundle = &bundleCdXx;
        phrase = bundle->mapPhrase[key];
        LOG_WARNING("Translation not found: cultureCode=%s, sourceStr=%s, key=%s. Using %s..\n", cultureCode.CSTR(), sourceStr.CSTR(), key.CSTR(), bundle->cultureCode.CSTR());
    }

    outStr = phrase.translation;

    int dataExpectedCount = outStr.count(QRegExp("\\{[0-9]*\\}"));
    if (dataExpectedCount > dataList.length())
    {
        LOG_WARNING("Bad Data Count: cultureCode=%s, sourceStr=\"%s\", translation=\"%s\"\n", cultureCode.CSTR(), sourceStr.CSTR(), outStr.CSTR());
    }

    for (int dataIdx = 0; dataIdx < dataList.length(); dataIdx++)
    {
        QString strData = dataList[dataIdx];
        QString strDataKey = QString().asprintf("{%d}", dataIdx);

        if (strData.length() == 0)
        {
            // No data
            //LOG_DEBUG("Token data string[%d] is empty: cultureCode=%s, sourceStr=\"%s\", translation=\"%s\"\n", dataIdx, cultureCode.CSTR(), sourceStr.CSTR(), outStr.CSTR());
        }
        else if (isTranslatableData(strData))
        {
            outStr.replace(strDataKey, translate(strData));
        }
        else if (strData[0] == '{')
        {
            // Data is json type. No translation required
        }
        else if (outStr.contains(strDataKey))
        {
            outStr.replace(strDataKey, strData);
        }
        else
        {
            LOG_DEBUG("Failed to find the phrase token[%d], cultureCode=%s, sourceStr=\"%s\"\n", dataIdx, cultureCode.CSTR(), sourceStr.CSTR());
        }
    }

    return outStr;
}

void Translator::parse(QString sourceStr, QString *keyOut, QStringList *dataListOut, QString *errOut)
{
    // Extract key from source string.
    // Expected sourceStr structure is: "<key>;<data>" where
    // key: T_KEYNAME
    // data: string with separator(;)
    // Example: "T_PROGRAMFAILED_InvalidProtocol Name is bad!"

    QString key = "";
    QStringList dataList;
    QString err = "";

    QStringList tokenList = sourceStr.split(";");
    key = tokenList[0];
    if (tokenList.length() > 1)
    {
        tokenList.removeFirst();
        dataList = tokenList;
    }

    if (key.indexOf(_L("T_")) != 0)
    {
        err = "Failed to find key";
    }

    *keyOut = key;
    *dataListOut = dataList;
    *errOut = err;
}

QString Translator::translateDBXX(QString sourceStr)
{
    // This is translation testing purpose
    // Returns all vowels doouubleed. If the requested string has no vowels (like ml/s), it is sourrounded by ~a.

    QString outputStr = sourceStr;
    outputStr.replace("a", "aa");
    outputStr.replace("e", "ee");
    outputStr.replace("i", "ii");
    outputStr.replace("o", "oo");
    outputStr.replace("u", "uu");
    outputStr.replace("A", "AA");
    outputStr.replace("E", "EE");
    outputStr.replace("I", "II");
    outputStr.replace("O", "OO");
    outputStr.replace("U", "UU");

    if (outputStr == sourceStr)
    {
        outputStr = "~" + sourceStr + "~";
    }
    return outputStr;
}

bool Translator::isTranslatableData(const QString &strData)
{
    bool isTranlatable = ( (strData.indexOf("T_") == 0) ||
                           (strData == _L("RS0")) ||
                           (strData == _L("RC1")) ||
                           (strData == _L("RC2")) ||
                           (strData == _L("BS0")) ||
                           (strData == _L("BC1")) ||
                           (strData == _L("BC2")) ||
                           (strData == _L("ML")) ||
                           (strData == _L("WC")) );
    return isTranlatable;
}
