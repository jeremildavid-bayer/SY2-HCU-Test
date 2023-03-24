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

#define LOG_LINE_CHAR_LIMIT                                     1000
#define LOG_MIN_SIZE_BYTES                                      50000
#define LOG_LRG_SIZE_BYTES                                      5000000

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
    Log(QString logPath, QString logPrefix, qint64 logSizeLimit = LOG_MIN_SIZE_BYTES) :
        path(logPath),
        prefix(logPrefix)
    {
        file = NULL;
        path = logPath;
        prefix = logPrefix;
        sizeLimit = logSizeLimit;
    }

    ~Log()
    {
        close();
    }

    void writeUnknown(QString text)
    {
        QStringList textList = text.split("\n");
        for (int textIdx = 0; textIdx < textList.length(); textIdx++)
        {
            QString textItem = textList[textIdx];
            if (textItem == "")
            {
                continue;
            }

            textItem = textItem + "\n";

            if (textItem.contains("INFO : "))
            {
                textItem.replace("INFO : ", "");
                writeInfo(textItem);
            }
            else if (textItem.contains("WARN : "))
            {
                textItem.replace("WARN : ", "");
                writeWarning(textItem);
            }
            else if (textItem.contains("ERROR: "))
            {
                textItem.replace("ERROR: ", "");
                writeError(textItem);
            }
            else if (textItem.contains("DEBUG: "))
            {
                textItem.replace("DEBUG: ", "");
                writeDebug(textItem);
            }
            else
            {
                writeInfo(text);
            }
        }
    }

    void writeDebug(QString text, QString customPrefix = "")
    {
        write("DEBUG: " + ((customPrefix == "") ? prefix : customPrefix) + ": " + text);
    }

    void writeInfo(QString text, QString customPrefix = "")
    {
        write("INFO : " + ((customPrefix == "") ? prefix : customPrefix) + ": " + text);
    }

    void writeWarning(QString text, QString customPrefix = "")
    {
        write("WARN : " + ((customPrefix == "") ? prefix : customPrefix) + ": " + text);
    }

    void writeError(QString text, QString customPrefix = "")
    {

        write("ERROR: " + ((customPrefix == "") ? prefix : customPrefix) + ": " + text);
    }

    void write(QString text)
    {
        mutexWrite.lock();

        if ( (file != NULL)  && (file->isOpen()) )
        {
            text.replace("\r\n", "\\r\\n").replace("\r", "\\r");
            QString logText = QDateTime::currentDateTime().toString("MMdd-hh:mm:ss.zzz ") + text;

            if (logText.length() > LOG_LINE_CHAR_LIMIT)
            {
                logText.truncate(LOG_LINE_CHAR_LIMIT);
            }

            file->write(logText.toStdString().c_str());

            if (file->size() > sizeLimit)
            {
                // File is too big, backup the current one and re-open in blank log
                file->write(QString().asprintf("\n\n\n===================================== LOG END =====================================\n\n\n").toStdString().c_str());
                QString bakLogFileName = file->fileName();
                bakLogFileName.insert(bakLogFileName.indexOf(".log"), ".old");
                QFile bakFile(bakLogFileName);
                bakFile.remove();
                file->rename(bakLogFileName);
                open();
            }
            else
            {
                file->flush();
            }
        }
        mutexWrite.unlock();
    }

    void open()
    {
        close();

        file = new QFile(path);

        if (file->open(QFile::WriteOnly | QFile::Text | QFile::Append))
        {
            // file open ok
        }
    }

    void close()
    {
        if (file != NULL)
        {
            if (file->isOpen())
            {
                file->close();
            }
            delete file;
            file = NULL;
        }
    }

private:
    QFile *file;
    QString path;
    QMutex mutexWrite;

public:
    QString prefix;
    qint64 sizeLimit;
};

#endif
