#ifndef DS_EXAM_DATA_H
#define DS_EXAM_DATA_H

#include "Common/Common.h"
#include "DS_ExamDef.h"
#include "DataServices/Device/DS_DeviceDef.h"
#include "DataServices/DataServicesMacros.h"

class DS_ExamData : public QObject
{
    Q_OBJECT

public:
    explicit DS_ExamData(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_ExamData();

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<DS_ExamDef::ScannerInterlocks>("DS_ExamDef::ScannerInterlocks");
        qRegisterMetaType<DS_ExamDef::InjectionPlan>("DS_ExamDef::InjectionPlan");
        qRegisterMetaType<DS_ExamDef::InjectionPlan>("DS_ExamDef::InjectionStep");
        qRegisterMetaType<DS_ExamDef::FluidVolumes>("DS_ExamDef::FluidVolumes");
        qRegisterMetaType<DS_ExamDef::ExecutedSteps>("DS_ExamDef::ExecutedSteps");
        qRegisterMetaType<DS_ExamDef::InjectionPlanDigests>("DS_ExamDef::InjectionPlanDigests");
        qRegisterMetaType<DS_ExamDef::InjectionPlanTemplateGroups>("DS_ExamDef::InjectionPlanTemplateGroups");
        qRegisterMetaType<DS_ExamDef::ExamProgressState>("DS_ExamDef::ExamProgressState");
        qRegisterMetaType<DS_ExamDef::InjectionRequestProcessStatus>("DS_ExamDef::InjectionRequestProcessStatus");
        qRegisterMetaType<DS_ExamDef::ExamAdvanceInfo>("DS_ExamDef::ExamAdvanceInfo");
        qRegisterMetaType<DS_ExamDef::PulseSalineVolumeLookupRows>("DS_ExamDef::PulseSalineVolumeLookupRows");
    }

    const DS_ExamDef::InjectionStep *getExecutingStep();
    const DS_ExamDef::InjectionPhase *getExecutingPhase();
    const DS_ExamDef::InjectionPlan *getDefaultInjectionPlanTemplate();
    bool isContrastSelectAllowed();
    bool isExamStarted();
    DS_ExamDef::InjectionPlanDigest *getPlanDigestFromTemplateGuid(DS_ExamDef::InjectionPlanTemplateGroups &group, QString templateGuid);

signals:
    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_DATA_CHANGED_SIGNAL(bool, DataLocked)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::ScannerInterlocks, ScannerInterlocks)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::InjectionPlan, InjectionPlan)
    CREATE_DATA_CHANGED_SIGNAL(bool, IsAirCheckNeeded)
    CREATE_DATA_CHANGED_SIGNAL(bool, IsScannerInterlocksCheckNeeded)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::ExecutedSteps, ExecutedSteps)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::InjectionPlan, InjectionPlanPreview)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::InjectionPlanTemplateGroups, InjectionPlanTemplateGroups)
    CREATE_DATA_CHANGED_SIGNAL(QString, ExamGuid)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::ExamProgressState, ExamProgressState)
    CREATE_DATA_CHANGED_SIGNAL(qint64, ExamStartedAtEpochMs)
    CREATE_DATA_CHANGED_SIGNAL(DS_DeviceDef::FluidInfo, SelectedContrast)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::InjectionRequestProcessStatus, InjectionRequestProcessStatus)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::InjectionStep, PreloadedStep1)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::InjectionStep, PreloadedStep2)
    CREATE_DATA_CHANGED_SIGNAL(QString, SUDSLength)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::ExamAdvanceInfo, ExamAdvanceInfo)
    CREATE_DATA_CHANGED_SIGNAL(DS_ExamDef::PulseSalineVolumeLookupRows, PulseSalineVolumeLookupRows)

private:
    // Creates GET/SET Properties
    CREATE_DATA_MEMBERS(bool, DataLocked)
    CREATE_DATA_MEMBERS(DS_ExamDef::ScannerInterlocks, ScannerInterlocks)
    CREATE_DATA_MEMBERS(DS_ExamDef::InjectionPlan, InjectionPlan)
    CREATE_DATA_MEMBERS(bool, IsAirCheckNeeded)
    CREATE_DATA_MEMBERS(bool, IsScannerInterlocksCheckNeeded)
    CREATE_DATA_MEMBERS(DS_ExamDef::ExecutedSteps, ExecutedSteps)
    CREATE_DATA_MEMBERS(DS_ExamDef::InjectionPlan, InjectionPlanPreview)
    CREATE_DATA_MEMBERS(DS_ExamDef::InjectionPlanTemplateGroups, InjectionPlanTemplateGroups)
    CREATE_DATA_MEMBERS(QString, ExamGuid)
    CREATE_DATA_MEMBERS(DS_ExamDef::ExamProgressState, ExamProgressState)
    CREATE_DATA_MEMBERS(qint64, ExamStartedAtEpochMs)
    CREATE_DATA_MEMBERS(DS_DeviceDef::FluidInfo, SelectedContrast)
    CREATE_DATA_MEMBERS(DS_ExamDef::InjectionRequestProcessStatus, InjectionRequestProcessStatus)
    CREATE_DATA_MEMBERS(DS_ExamDef::InjectionStep, PreloadedStep1)
    CREATE_DATA_MEMBERS(DS_ExamDef::InjectionStep, PreloadedStep2)
    CREATE_DATA_MEMBERS(QString, SUDSLength)
    CREATE_DATA_MEMBERS(DS_ExamDef::ExamAdvanceInfo, ExamAdvanceInfo)
    CREATE_DATA_MEMBERS(DS_ExamDef::PulseSalineVolumeLookupRows, PulseSalineVolumeLookupRows)

    EnvGlobal *env;
    EnvLocal *envLocal;

    void initDefaultInjectionPlanTemplate();

private slots:
    void slotAppStarted();

};

#endif // DS_EXAM_DATA_H
