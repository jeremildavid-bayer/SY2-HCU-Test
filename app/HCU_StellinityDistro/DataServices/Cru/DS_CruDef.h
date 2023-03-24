#ifndef DS_CRU_DEF_H
#define DS_CRU_DEF_H

#include <QElapsedTimer>
#include <QString>
#include <QVariantMap>
#include <QMap>
#include <QUuid>
#include "Common/Environment.h"

class DS_CruDef
{
public:
    // ==============================================
    // Enumerations
    enum CruLicensedFeature
    {
        CRU_LICENSED_FEATURE_INVALID = 0,
        CRU_LICENSED_FEATURE_UNKNOWN,
        CRU_LICENSED_FEATURE_PATIENT,
        CRU_LICENSED_FEATURE_WORKLIST
    };

    enum CruLinkType
    {
        CRU_LINK_TYPE_INVALID = 0,
        CRU_LINK_TYPE_UNKNOWN,
        CRU_LINK_TYPE_WIRED,
        CRU_LINK_TYPE_WIRELESS
    };

    enum CruLinkState
    {
        CRU_LINK_STATE_INVALID = 0,
        CRU_LINK_STATE_UNKNOWN,
        CRU_LINK_STATE_INACTIVE,
        CRU_LINK_STATE_RECOVERING,
        CRU_LINK_STATE_ACTIVE
    };

    enum CruLinkQuality
    {
        CRU_LINK_QUALITY_INVALID = 0,
        CRU_LINK_QUALITY_UNKNOWN,
        CRU_LINK_QUALITY_POOR,
        CRU_LINK_QUALITY_FAIR,
        CRU_LINK_QUALITY_GOOD,
        CRU_LINK_QUALITY_EXCELLENT
    };

    // ==============================================
    // Data Structures

    typedef QList<CruLicensedFeature> CruLicensedFeatures;

    struct CruLinkStatus
    {
        CruLinkType type;
        CruLinkState state;
        CruLinkQuality quality;
        QString signalLevel;

        CruLinkStatus()
        {
            type = DS_CruDef::CRU_LINK_TYPE_UNKNOWN;
            state = DS_CruDef::CRU_LINK_STATE_INACTIVE;
            quality = DS_CruDef::CRU_LINK_QUALITY_UNKNOWN;
            signalLevel = "";
        }

        bool operator==(const CruLinkStatus &arg) const
        {
            return ( (type == arg.type) &&
                     (state == arg.state) &&
                     (quality == arg.quality) &&
                     (signalLevel == arg.signalLevel) );
        }

        bool operator!=(const CruLinkStatus &arg) const
        {
            return !operator==(arg);
        }
    };
};

Q_DECLARE_METATYPE(DS_CruDef::CruLinkStatus);

#endif // DS_CRU_DEF_H
