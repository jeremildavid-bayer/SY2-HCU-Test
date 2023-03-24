#include "Log.h"

Log::Log(QString logPath, qint64 logSizeLimit) :
    path(logPath)
{
    file = NULL;
    path = logPath;
    sizeLimit = logSizeLimit;
    oldFileLimit = LOG_NUM_BACKUP_FILES;

    for (int i = 0; i < oldFileLimit; i++)
    {
        QString oldFilePath = path;
        oldFilePath = oldFilePath.insert(oldFilePath.indexOf(".log"), QString().asprintf(".old%d", i + 1));
        if (!QFile::exists(oldFilePath))
        {
            break;
        }
        oldFiles.append(new QFile(oldFilePath));
    }
}

Log::~Log()
{
    close();
}

void Log::writeDebug(const QString &text, const QString &customPrefix)
{
    write("DEBUG: ", customPrefix, text);
}

void Log::writeInfo(const QString &text, const QString &customPrefix)
{
    write("INFO : ", customPrefix, text);
}

void Log::writeWarning(const QString &text, const QString &customPrefix)
{
    write("WARN : ", customPrefix, text);
}

void Log::writeError(const QString &text, const QString &customPrefix)
{
    write("ERROR: ", customPrefix, text);
}

void Log::open()
{
    close();

    file = new QFile(path);

    if (file->open(QFile::WriteOnly | QFile::Text | QFile::Append))
    {
        // file open ok
    }
}

void Log::close()
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

void Log::updateOldFiles()
{
    for (int i = oldFiles.count() - 1; i >= 0; i--)
    {
        QString newOldFileName = oldFiles[i]->fileName();
        int oldFileNum = i + 1;
        if (oldFileNum == oldFileLimit)
        {
            oldFiles[i]->remove();
            delete oldFiles[i];
            oldFiles[i] = NULL;
            oldFiles.removeAt(i);
        }
        else
        {
            newOldFileName.replace(newOldFileName.indexOf(".old"), 5, QString().asprintf(".old%d", i + 2));
            oldFiles[i]->rename(newOldFileName);
        }
    }
}

void Log::write(const char *prefix, const QString &customPrefix, const QString &text)
{
    mutexWrite.lock();

    if ( (file != NULL)  && (file->isOpen()) )
    {//avoid string concatenation by writen separate string and no memory truncation
        file->write(QDateTime::currentDateTime().toString("MMdd-hh:mm:ss.zzz ").toStdString().c_str());
        file->write(prefix);
        if (customPrefix.length() > 0)
            file->write(customPrefix.toStdString().c_str());

        QString logText = text;
        if (logText.contains('\r'))
            logText.replace("\r\n", "\\r\\n").replace("\r", "\\r");

        //avoid truncation all together, assume line chars limit excluding prefixes
        auto std_str = logText.toStdString();
        if (std_str.length() > LOG_LINE_CHAR_LIMIT)
        {
            file->write(std_str.c_str(), LOG_LINE_CHAR_LIMIT);
            file->write("\n");
        }
        else
        {
            file->write(std_str.c_str(), std_str.length());
        }

        if ( file->size() > sizeLimit)
        {
            // File is too big, backup the current one and re-open in blank log
            file->write("\n\n\n===================================== LOG END =====================================\n\n\n");

            updateOldFiles();
            QString bakLogFileName = file->fileName();
            bakLogFileName.insert(bakLogFileName.indexOf(".log"), ".old1");
            file->rename(bakLogFileName);
            oldFiles.insert(0, new QFile(bakLogFileName));
            open();
        }
        else
        {
            file->flush();
        }
    }
    mutexWrite.unlock();
}
