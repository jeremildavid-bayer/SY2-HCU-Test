#ifndef HMAC_H
#define HMAC_H
#include <QString>
#include <QMessageAuthenticationCode>

/**
 * The maximum absolute time difference allowed between HMAC time and system time
 * to be considered as a valid RPC calls.
 */
#define HMAC_MAX_DELTA_TIME_IN_SECONDS 300
/**
 * HTTP header name for HMAC hash
 */
#define HTTP_HMAC_HASH_HEADER "sy2-hmac"
#define HTTP_HMAC_HASH_HEADER_UPPER "SY2-HMAC"
/**
 * HTTP header name for HMAC Unix epoche time stamp.
 */
#define HTTP_HMAC_UE_HEADER "sy2-hmac-ue"
#define HTTP_HMAC_UE_HEADER_UPPER "SY2-HMAC-UE"

/**
 * HTTP response on invalid HMAC - forbiden
 */
#define HMAC_HTTP_STATUS_RESPONSE_ON_ERROR 403  //http forbiden


class HMAC
{
public:
    HMAC(const QString &serial);
    QString getRequestHash(const QString &method, const QString &url, const QString &query, const QString &body, const QString &hmac_ue);
    QString getResponseHash(const QString& status, const QString &body, const QString &hmac_ue);

    bool validateHMAC(const QString& rx_hmac_hash, const QString& rx_hmac_at, const QString& computed_hmac_hash, qint64 now_ue, bool ignore_time_diff=false) const;
    QString sortedQueryString(const QString unsortedQueryString) const;
private:
    QMessageAuthenticationCode mac;
};

#endif // HMAC_H
