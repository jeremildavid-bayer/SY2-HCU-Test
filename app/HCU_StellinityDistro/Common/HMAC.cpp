#include "HMAC.h"
#include <QStringList>
#include <QMutex>
#include <QFile>

/**
 * The request-data is: METHOD | URL | QUERY | BODY all forced to lower case invariant.
 * The response-data is: STATUS | BODY all forced to lower case invariant.
 * The " | " (space-pipe-space) is a literal delimiter in the data string to be hashed.
*/

#define SECRET_STRING  "a9da2fb5-eda9-4697-b814-32d8e8913a32"
#define HMAC_SEPARATOR_STRING  " | " // space bar space
//#define ENABLE_HMAC_DEBUG


HMAC::HMAC(const QString &serial) : mac(QCryptographicHash::Sha512, (QString(SECRET_STRING) + serial).toUtf8())
{
}

/**
 * The request-data is: METHOD | URL | QUERY | BODY all forced to lower case invariant.
 *
 * @brief ImrHMAC::getRequestHash
 * @param method
 * @param url_path : this is the URL path not the full URL.
 *                   For examp the url path is "/cru/v11/digest" instead of the full url "http://192.168.11.2/cru/v11/digest"
 * @param query
 * @param body
 * @param hmac_ue
 * @return
 */
QString HMAC::getRequestHash(const QString &method, const QString &url_path, const QString &query, const QString &body, const QString &hmac_ue)
{
    /*
     * CRU adaptation! CRU trim the url as follow:
     *  HCU->CRU: /sru/v0/api/versions => v0/api/versions
     *  CRU->HCU: /sru/v11/profile     => v11/profile
     *  HCU->CRU: /cru/v11/digest      => v11/digest
     */
    auto lower_url = url_path.toLower();
    if (lower_url.startsWith("/sru/v") || lower_url.startsWith("/cru/v"))
        lower_url = lower_url.mid(5); // skip 5 chars to start the string with v

    QStringList list;
    list << method.toLower();
    list << lower_url;
    list << query.toLower();
    if (body.length() > 0) //optional body
        list << body.toLower();
    auto data = list.join(HMAC_SEPARATOR_STRING);

    //CRU: var bytes = Encoding.UTF8.GetBytes(SECRET_SALT + data + hmacUE);
    mac.reset(); //start hmac calculation again with the given key on creation.
    mac.addData(data.toUtf8());
    mac.addData(hmac_ue.toUtf8());

#ifdef ENABLE_HMAC_DEBUG
    {//for debuging
        static QMutex mutex;
        QMutexLocker locker(&mutex); //singleton
        QFile file("/IMAX_USER/log/hmac.log");
        if (file.open(QIODevice::WriteOnly | QIODevice::Append| QFile::Text))
        {
            file.write(data.toUtf8());
            file.write("\n");
            file.write(hmac_ue.toUtf8());
            file.write("\n");
            file.write(mac.result().toHex());
            file.write("\n");
            file.close();
        }
    }
#endif // ENABLE_HMAC_DEBUG

    return QString(mac.result().toHex()).toUpper(); // as CRU need upper case
}

/**
 * The response-data is: STATUS | BODY all forced to lower case invariant.
 *
 * @brief ImrHMAC::getResponseHash
 * @param status
 * @param body
 * @param hmac_ue
 * @return HMAC SHA512 hash valuein string
 */
QString HMAC::getResponseHash(const QString& status, const QString &body, const QString &hmac_ue)
{
    QString data  = status.toLower().toUtf8() + HMAC_SEPARATOR_STRING + body.toLower().toUtf8();
    mac.reset(); //start again
    //var bytes = Encoding.UTF8.GetBytes(SECRET_SALT + data + hmacUE);
    mac.addData(data.toUtf8());
    mac.addData(hmac_ue.toUtf8());
    return QString(mac.result().toHex()).toUpper(); // as CRU need upper case
}

/**
 * @brief HMAC::sortedQueryString
 * @param request
 * @return the parameter request matching CRU implementation SortedQueryString()
 * e.g.       from=11&maxdigests=1&minify=true&omitalerts=true&requestid=76%3b143511362%3b0
 */
QString HMAC::sortedQueryString(const QString unsortedQueryString) const
{
    if (unsortedQueryString.isEmpty())
        return QString();

    QStringList param_list = unsortedQueryString.split("&");
    if (param_list.length() <= 1)
        return unsortedQueryString;

    param_list.sort(); //Sorts the list of strings in ascending order

    return param_list.join("&");
}


/**
 * @brief validateHMAC
 * @param rx_hmac_hash: The received hash key
 * @param rx_hmac_at: The received unix epoch in second when the hash key generated.
 * @param calculated_hmac_hash: the calculated hash key
 * @param now_ue: the current HCU system unix epoch in second
 * @return true if the HMAC validation success, return false otherwise.
 */
bool HMAC::validateHMAC(const QString& rx_hmac_hash, const QString& rx_hmac_at, const QString& computed_hmac_hash, qint64 now_ue, bool ignore_time_diff) const
{
    // compare case insenstive because we don't know might do in the future
    if (computed_hmac_hash.compare(rx_hmac_hash, Qt::CaseInsensitive) != 0)
    {
        return false;
    }

    //Check if the HMAC-UE based-10-number is conversion is valid.
    //Still want to vaidate the time string even when ignore_time_diff is true.
    bool ok = false;
    qint64 rx_hmac_ue = rx_hmac_at.toLongLong(&ok, 10);
    if (!ok)
    {
        return false;
    }
    else if (ignore_time_diff)
    {
        return true;
    }
    else
    {
        /*
         * The receiver checks the timespan between their current local clock (nowAt)
         * and the received SY2-HMAC-UE to detect replay attacks.
         * A certain amount of delta is allowed to accommodate potential clock skew.
         */
        auto delta = now_ue - rx_hmac_ue;
        return (qAbs(delta) <= HMAC_MAX_DELTA_TIME_IN_SECONDS);
    }
}
