#ifndef DS_IMR_SERVER_DEF_H
#define DS_IMR_SERVER_DEF_H

#include "Common/Common.h"
#include "Internal/ImrServer/Digest.h"

class DS_ImrServerDef
{
public:
    // ==============================================
    // Enumerations

    // ==============================================
    // Data Structures

    typedef QMap<QString, QElapsedTimer> ImrRequests; // key: api+method, data: msg received at
};

Q_DECLARE_METATYPE(DS_ImrServerDef::ImrRequests);
#endif // DS_IMR_SERVER_DEF_H
