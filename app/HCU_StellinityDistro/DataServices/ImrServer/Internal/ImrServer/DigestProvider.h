#ifndef DIGEST_PROVIDER_H
#define DIGEST_PROVIDER_H

#include <QTimer>
#include "Digest.h"
#include "Common/Common.h"

class DigestProvider : public QObject
{
    Q_OBJECT

public:
    explicit DigestProvider(QObject *parent = NULL, EnvGlobal *env = NULL);
    ~DigestProvider();
    quint32 getLastDigestId();
    int getDigestWrappedCount();
    QVariantList getDigests(int fromId, int sizeLimit = -1, bool includeNotReady = false, bool omitAlerts = true);
    void getDigestDeltaMap(QVariantMap &ret, int digestId);
    void getDigestDeltaMap(QVariantMap &ret, const QVariantMap &map1, const QVariantMap &map2);

private:
    EnvGlobal *env;
    EnvLocal *envLocal;
    quint32 digestId;
    quint32 digestIdToSave;
    quint32 digestIdSaved;
    quint32 digestWrappedCount;
    //QMutex mutexListDigest;
    QMutex mutexSingleton;
    QList<Digest> listDigest;
    bool commitLocked;
    bool lastDigestIsReady;
    QTimer tmrReleaseLastDigest;
    QTimer tmrDigestSave;

    QTimer tmrSpontaneousDigest;
    int spontaneousDigestsAdded = 0;
    int spontaneousDigestsSkipped = 0;
    bool spontaneousDigestOn;

    void addNewDigest(Digest &digest);
    Digest getLastDigest();
    void loadLastDigest();
    void saveDigest(const QVariantMap &digestMap);
    void restoreLastFluidSources(const Digest &digest);
    void restoreLastExam(const Digest &digest);
    void incrementDigestId();


private slots:
    void slotAppInitialised();
};

#endif // DIGEST_PROVIDER_H
