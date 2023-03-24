#ifndef DS_MWL_DEF_H
#define DS_MWL_DEF_H

#include <QElapsedTimer>
#include <QString>
#include <QVariantMap>
#include <QMap>
#include <QUuid>
#include "Common/Environment.h"

class DS_MwlDef
{
public:
    // ==============================================
    // Enumerations

    // ==============================================
    // Data Structures

    struct DicomField
    {
        QString name;
        QString value;
        QString dicomValueType;
        QString valueType;
        bool translateName;
        bool translateValue;

        bool operator==(const DicomField &arg) const
        {
            return ( (name == arg.name) &&
                     (value == arg.value) &&
                     (dicomValueType == arg.dicomValueType) &&
                     (valueType == arg.valueType) &&
                     (translateName == arg.translateName) &&
                     (translateValue == arg.translateValue) );
        }

        bool operator!=(const DicomField &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QMap<QString, DicomField> DicomFieldMap;
    typedef QList<DicomField> DicomFields;

    struct WorklistEntry
    {
        QString studyInstanceUid;
        bool isAnonymous;
        DicomFieldMap dicomFields;

        WorklistEntry()
        {
            init();
        }

        void init()
        {
            studyInstanceUid = EMPTY_GUID;
            isAnonymous = false;
        }

        bool operator==(const WorklistEntry &arg) const
        {
            return ( (studyInstanceUid == arg.studyInstanceUid) &&
                     (isAnonymous == arg.isAnonymous) &&
                     (dicomFields == arg.dicomFields) );
        }

        bool operator!=(const WorklistEntry &arg) const
        {
            return !operator==(arg);
        }
    };

    typedef QList<WorklistEntry> WorklistEntries;
};

Q_DECLARE_METATYPE(DS_MwlDef::DicomField);
Q_DECLARE_METATYPE(DS_MwlDef::WorklistEntry);
Q_DECLARE_METATYPE(DS_MwlDef::DicomFieldMap);
Q_DECLARE_METATYPE(DS_MwlDef::WorklistEntries);

#endif // DS_MWL_DEF_H
