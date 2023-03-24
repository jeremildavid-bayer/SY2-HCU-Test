#ifndef LOG_H
#define LOG_H

//====================================================================
// CLASS: Log
// Manages log write. Log file is backed up once line limit is reached.
//====================================================================

#include <QFile>
#include <QTime>
#include <QDate>
#include <QTextStream>
#include <QStringList>
#include <QMutex>
#include "Environment.h"

enum LogLevel
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
};

class Log
{
public:
    qint64 sizeLimit;

    Log(QString logPath, qint64 logSizeLimit = LOG_MIN_SIZE_BYTES);
    ~Log();

    void writeDebug(const QString &text, const QString &customPrefix = "");
    void writeInfo(const QString &text, const QString &customPrefix = "");
    void writeWarning(const QString &text, const QString &customPrefix = "");
    void writeError(const QString &text, const QString &customPrefix = "");
    void open();
    void close();

private:
    QFile *file;
    QList<QFile *> oldFiles;
    int oldLogCount;
    int oldFileLimit;
    QString path;
    QMutex mutexWrite;

    void updateOldFiles();
    void write(const char *prefix, const QString &customPrefix, const QString &text);
};

#endif
