#ifndef DS_MWL_DATA_H
#define DS_MWL_DATA_H

#include "DS_MwlDef.h"
#include "Common/Common.h"
#include "DataServices/DataServicesMacros.h"

class DS_MwlData : public QObject
{
    Q_OBJECT

public:
    explicit DS_MwlData(QObject *parent = 0, EnvGlobal * = NULL);
    ~DS_MwlData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_MwlDef::DicomFieldMap>("DS_MwlDef::DicomFieldMap");
        qRegisterMetaType<DS_MwlDef::WorklistEntries>("DS_MwlDef::WorklistEntries");
    }

    const DS_MwlDef::WorklistEntry *getWorklistEntry(QString studyUid) const;

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(QString, SuiteName)
    CREATE_DATA_CHANGED_SIGNAL(QString, PatientName)
    CREATE_DATA_CHANGED_SIGNAL(QString, StudyDescription)
    CREATE_DATA_CHANGED_SIGNAL(DS_MwlDef::WorklistEntries, WorklistEntries)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(QString, SuiteName)
    CREATE_DATA_MEMBERS(QString, PatientName)
    CREATE_DATA_MEMBERS(QString, StudyDescription)
    CREATE_DATA_MEMBERS(DS_MwlDef::WorklistEntries, WorklistEntries)

    EnvGlobal *env;
    EnvLocal *envLocal;

private slots:
    void slotAppStarted();
};

#endif // DS_MWL_DATA_H
