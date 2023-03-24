#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "Common.h"

class Translator : public QObject
{
    Q_OBJECT

public:
    explicit Translator(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~Translator();

    Q_INVOKABLE QString translate(QString sourceStr, QString cultureCode = "");

private:
    struct Phrase
    {
        QString translation;
    };

    struct Bundle
    {
        QString cultureCode;
        QMap<QString, Phrase> mapPhrase; // key is PhraseString
        QString err;
    };

    EnvGlobal *env;
    EnvLocal *envLocal;
    Bundle bundleCdXx;
    Bundle curBundle;
    Bundle curBundle2;

    void setBundle(Bundle &outBundle, QString cultureCode);
    void parse(QString sourceStr, QString *keyOut, QStringList *dataListOut, QString *errOut);
    QString translateDBXX(QString sourceStr);
    bool isTranslatableData(const QString &strData);
};

#endif // TRANSLATOR_H
