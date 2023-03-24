#ifndef UTIL_H
#define UTIL_H

#include <QThread>
#include <QString>
#include <QDateTime>
#include <QJsonDocument>
#include <QVariant>
#include <QUuid>
#include <QCryptographicHash>
#include <QtMath>
#include "Common/Environment.h"

class Util
{
public:
    static QString curThreadId()
    {
        return QString::number((long long) QThread::currentThreadId(), 16);
    }

    static QString newGuid()
    {
        // Guid format is .NET format
        QString guid = QUuid::createUuid().toString().toLower();
        guid.replace("{", "").replace("}", "");
        guid.replace("-", "");
        guid.insert(8, "-");
        guid.insert(13, "-");
        guid.insert(18, "-");
        guid.insert(23, "-");
        return guid;
    }

    static double roundFloat(double src, int decPt)
    {
        int multiplyConst = qPow(10, decPt);
        src = src * multiplyConst;
        src = qRound(src);
        src = src / multiplyConst;
        return src;
    }

    static QString millisecToDurationStr(qint64 msec)
    {
        qint64 sec = msec / 1000;
        msec = msec % 1000;
        qint64 min = sec / 60;
        sec = sec % 60;
        qint64 hour = min / 60;
        min = min % 60;
        qint64 day = hour / 24;
        hour = hour % 24;

        QString dayStr = "";
        if (day > 0)
        {
            dayStr = QString().asprintf("%lld.", day);
        }

        return QString().asprintf("%s%02lld:%02lld:%02lld.%03lld", dayStr.toStdString().c_str(), hour, min, sec, msec);
    }

    static qint64 durationStrToMillisec(QString srcStr)
    {
        // Format can be hh:mm:ss.zzz or hh:mm:ss
        qint64 ret;
        int day = 0, hour = 0, min = 0, sec = 0, msec = 0;

        QStringList strList = srcStr.split(":");

        if (strList.length() > 2)
        {
            QStringList dayAndHourList = strList[0].split(".");
            if (dayAndHourList.length() > 1)
            {
                day = dayAndHourList[0].toInt();
                hour = dayAndHourList[1].toInt();
            }
            else
            {
                hour = strList[0].toInt();
            }
            min = strList[1].toInt();
            QStringList secAndmsecList = strList[2].split(".");
            sec = secAndmsecList[0].toInt();
            msec = (secAndmsecList.length() > 1) ? secAndmsecList[1].toInt() : 0;
            ret = (msec) + (sec * 1000) + (min * 60 * 1000) + (hour * 60 * 60 * 1000) + (day * 60 * 60 * 1000 * 24);
        }
        else
        {
            srcStr = srcStr.replace("-", "0");
            srcStr = srcStr.replace(" ", "0");
            min = srcStr.mid(0, 2).toInt();
            sec = srcStr.mid(3, 2).toInt();
            ret = (sec + (min * 60)) * 1000;
        }

        return ret;
    }

    static QString qDateTimeToUtcDateTimeStr(QDateTime dateTime)
    {
        // why 3z? Because we want to pad with leading 0 (.NET style)
        QString out = dateTime.toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzzZ");
        return out;
    }

    static QDateTime utcDateTimeStrToQDateTime(QString str)
    {
        // why 1z? Because sometimes the str has no leading z (.NET style)
        QDateTime dateTime;
        dateTime = QDateTime::fromString(str, "yyyy-MM-ddTHH:mm:ss.zZ");
        if (!dateTime.isValid())
        {
            // Try other parsing
            dateTime = QDateTime::fromString(str, "yyyy-MM-ddTHH:mm:ssZ");
            /*
             * if (!dateTime.isValid())
             * {//invalid again => ~zero chance of getting here
             *   It will return invalid date time and will shown as "1970/01/01 00:00:00"
             *   Acceptable? Yes if there is no data corruption this will not be reacable.
             *   Since we have HMAC validation guard agaist corruption earlier.
             * }
             */
        }
        dateTime.setOffsetFromUtc(0);
        return dateTime;
    }

    static QByteArray qVarientToJsonData(QVariant var, bool minify = true)
    {
        QJsonDocument doc = QJsonDocument::fromVariant(var);
        QString jsonStr = doc.toJson().replace("\\\\/", "\\/");

        jsonStr.replace("\"/Date(", "\"\\/Date(").replace(")/\"", ")\\/\"");

        if (minify)
        {
            return jsonStr.toLocal8Bit().simplified();
        }
        else
        {
            return jsonStr.toLocal8Bit();
        }
    }

    static QVariant jsonDataToQVariant(QString jsonStr, QJsonParseError *parseErr = NULL)
    {
        return jsonDataToQVariant(jsonStr.toUtf8(), parseErr);
    }

    static QVariant jsonDataToQVariant(QByteArray jsonData, QJsonParseError *parseErr = NULL)
    {
        QJsonDocument doc = QJsonDocument::fromJson(jsonData, parseErr);
        return doc.toVariant();
    }

    static QString getCruWifiSsid(QString serialNumber)
    {
        return CRU_SSID_PREFIX + serialNumber;
    }

    static QString getCruWifiPassword(QString serialNumber)
    {
        QString key = serialNumber + QString(CRU_SSID_PASSWORD_MD5_SALT);
        QString password = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5).toHex().toUpper();
        return password;
    }

    static QVariantList configMapToSortedList(QVariantMap configMap, QString *errOut)
    {
        QString err;

        QVariantList ret;
        QVariantMap mapSorted;

        if (err == "")
        {
            QMap<QString, QVariant>::const_iterator i;

            // List is sorted: Both group and DisplayIndex are ordered
            i = configMap.cbegin();
            while (i != configMap.cend())
            {
                QVariantMap configData = i.value().toMap();
                QString key = i.key();
                QString group = key.left(key.indexOf('_'));
                QString newKey = group + QString().asprintf("%08d", configData["DisplayIndex"].toInt());
                if (mapSorted.contains(newKey))
                {
                    newKey += key;
                }
                mapSorted.insert(newKey, configData);
                i++;
            }

            // Complete the list
            i = mapSorted.cbegin();
            while (i != mapSorted.cend())
            {
                ret.append(i.value().toMap());
                i++;
            }
        }

        *errOut = err;

        return ret;
    }

    // compare two float numbers and return true if equal
    static bool areFloatVarsEqual(double arg1, double arg2, int digits = 2)
    {
        double compareRange = ::pow(0.1, digits);
        return ::fabs(arg1 - arg2) < compareRange;
    }

    static bool isFloatVarGreaterThan(double arg1, double arg2, int digits = 2)
    {
        double compareRange = ::pow(0.1, digits);
        return (arg1 - arg2) >= compareRange;
    }

    /* tohex() - Conversion from ASCII char to hex integer
     */
    static quint32 tohex(char c)
    {
        c = toupper(c);

        if ((c >= '0') && (c <= '9'))
            return(c - '0');

        return(c - '0' - 7);
    }

    /* ascii to 2 hex digits
     * returns 0 if failure
     */
    static bool asc2hex2(char *bp, unsigned long *ip)
    {
        if (!isxdigit(*bp) || !isxdigit(*(bp+1)))
            return false;

        *ip = (tohex(*bp) << 4) + tohex(*(bp+1));
        return true;
    }

    /* ascii to 4 hex digits
     * returns 0 if failure
     */
    static bool asc2hex4(char *bp, unsigned long *ip)
    {
        unsigned long result;
        unsigned long result2;

        if (!asc2hex2(bp, &result))
            return false;

        if (!asc2hex2(bp+2, &result2))
            return false;

        *ip = (result << 8) + result2;
        return true;
    }

    /* ascii to 6 hex digits
     * returns 0 if failure
     */
    static bool asc2hex6(char *bp, unsigned long *ip)
    {
        unsigned long result;
        unsigned long result2;
        unsigned long result3;

        if (!asc2hex2(bp, &result))
            return false;

        if (!asc2hex2(bp+2, &result2))
            return false;

        if (!asc2hex2(bp+4, &result3))
            return false;

        *ip = ((unsigned long)result << 16) + ((unsigned long)result2 << 8) + result3;
        return true;
    }

    /* ascii to 8 hex digits
     * returns 0 if failure
     */
    static bool asc2hex8(char *bp, unsigned long *ip)
    {
        unsigned long result;
        unsigned long result2;
        unsigned long result3;
        unsigned long result4;

        if (!asc2hex2(bp, &result))
            return false;

        if (!asc2hex2(bp+2, &result2))
            return false;

        if (!asc2hex2(bp+4, &result3))
            return false;

        if (!asc2hex2(bp+6, &result4))
            return false;

        *ip = ((unsigned long)result << 24) + ((unsigned long)result2 << 16) + ((unsigned long)result3 << 8) + result4;

        return true;
    }

    static void putLong(quint8 *d, unsigned long src)
    {
        quint8 *psrc = (quint8 *)&src;
        *d++ = psrc[3];
        *d++ = psrc[2];
        *d++ = psrc[1];
        *d   = psrc[0];
    }

    static bool writeAddress32bit(quint8 *dst, int address)
    {
        *dst++ = 0x00;
        *dst++ = 0x00;
        *dst++ = (quint8) ((address & 0xFF00) >> 8);
        *dst = (quint8) address;
        return true;
    }

    static uint16_t crc16(const QString &srcStr)
    {
        static const unsigned short CRC_CCITT_TABLE[256] =
        {
            0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
            0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
            0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
            0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
            0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
            0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
            0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
            0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
            0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
            0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
            0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
            0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
            0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
            0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
            0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
            0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
            0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
            0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
            0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
            0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
            0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
            0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
            0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
            0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
            0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
            0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
            0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
            0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
            0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
            0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
            0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
            0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
        };

        int crc = 0xffff;

        for (int i = 0; i < srcStr.length() ; i++)
        {
            int c = srcStr.at(i).unicode();
            int j = (c ^ (crc >> 8)) & 0xff;
            crc = CRC_CCITT_TABLE[j] ^ (crc << 8);
        }
        return ((crc ^ 0) & 0xffff);
    }

    /**
     * Extract an integer from a given string with a given start and end character marker.
     * for example
     *     ASSERT(999 == extractIntFromString(&retInt, " Blah:999,", ':', ','));    //default base 10
     *     ASSERT(255 == extractIntFromString(&retInt, " Blah=ff,", '=', ',', 16)); //hexadecimal
     *
     * @brief extractIntFromString
     * @param retValuePtr
     * @param string
     * @param startDelimiter
     * @param endDelimiter
     * @param base: the base number format of the number in the string. The default is base 10.
     * @return
     */
    static bool extractIntFromString(int *retValuePtr, const QString& string, QChar startDelimiter, QChar endDelimiter='\0', int base=10)
    {
        bool ok = false;
        QStringView str = QStringView{string};

        auto startPos = str.indexOf(startDelimiter);
        if (startPos == -1)
            return false;
        startPos +=1; //skip the start delimiter
        if (endDelimiter == '\0')
        {
            *retValuePtr = str.mid(startPos).toInt(&ok, base);
        }
        else
        {
            auto endPos = str.indexOf(endDelimiter, startPos);
            if (endPos == -1)
                return false;
            *retValuePtr = str.mid(startPos, endPos - startPos).toInt(&ok, base);
        }
        return ok;
    }


};

#endif // UTIL_H
