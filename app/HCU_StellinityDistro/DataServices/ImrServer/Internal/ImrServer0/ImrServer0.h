#ifndef IMR_SERVER_0_H
#define IMR_SERVER_0_H

#include "Common/Common.h"
#include "../ImrServerBase.h"
#include "TestMsg.h"


class ImrServer0 : public ImrServerBase
{
    Q_OBJECT

public:
    ImrServer0(QObject *parent, QString baseUrl, EnvGlobal *env, EnvLocal *envLocal);
    ~ImrServer0();
    void ImrService(Params &params) override;

private:
    TestMsg testMsg;
    int currentImrVersion;

    // Request Handler Functions
    void rhApiDescription(Params &params);
    void rhApiVersions(Params &params);
    void rhTestMessage(Params &params);

};

#endif // IMR_SERVER_0_H
