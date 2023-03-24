#ifndef DS_CFG_LOCAL_H
#define DS_CFG_LOCAL_H

#include <QObject>
#include "DS_CfgLocalDef.h"
#include "Common/Common.h"
#include "Common/Config.h"
#include "DataServices/DataServicesMacros.h"

class DS_CfgLocal : public QObject
{
    Q_OBJECT

public:
    explicit DS_CfgLocal(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_CfgLocal();
    QVariantMap getConfigs(QString *err = NULL);
    void setConfigs(QVariantMap configs, bool setChangedAt, QString *err);

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<Config::Item>("Config::Item");
    }

signals:
    void signalConfigChanged();

    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_ScreenY)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_ScreenX)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_ScreenW)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_ScreenH)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_HcuIp)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_HcuPort)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_CruIpWired)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_CruIpWireless)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_CruPort)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_CruRouterIp)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_FluidOptions)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_InjectionPlanTemplateGroups)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_DefaultInjectionPlanTemplate)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_PressureOptionValues)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_CurrentUtcOffsetMinutes)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_MudsSodStatus)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_LastExamAdvanceInfo)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastBatteryIdA)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastBatteryIdB)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastPFResetCycleCountA)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BMSPFResetCountA)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BMSLastPFResetCycleCountB)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BMSPFResetCountB)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_PMLastPerformedAt)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_PMLastReminderAt)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_SyringesUsedInLastExam)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_BottleFillCount)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_SruLink_ConnectionType)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Sound_NormalAudioLevel)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Sound_InjectionAudioLevel)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Sound_SudsPrimedAudioLevel)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Sound_NotificationAudioLevel)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Sound_KeyClicksEnabled)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Display_ScreenBrightness)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Display_ScreenOffTimeoutMinutes)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Display_Theme)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Injector_MoodLightColor)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Injector_PMReminderFrequency)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Injector_EmptyContrastFirst)
    CREATE_CONFIG_CHANGED_SIGNAL(ServiceCalibration_IADCalibrationMethod)

private:
    // Creates GET/SET Properties
    CREATE_CONFIG_MEMBERS_EX(Hidden_ScreenY, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_ScreenX, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_ScreenW, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_ScreenH, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_HcuIp, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Hidden_HcuPort, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_CruIpWired, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Hidden_CruIpWireless, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Hidden_CruPort, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_CruRouterIp, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Hidden_FluidOptions, QVariantMap, toMap)
    CREATE_CONFIG_MEMBERS_EX(Hidden_InjectionPlanTemplateGroups, QVariantList, toList)
    CREATE_CONFIG_MEMBERS_EX(Hidden_DefaultInjectionPlanTemplate, QVariantMap, toMap)
    CREATE_CONFIG_MEMBERS(Hidden_PressureOptionValues)
    CREATE_CONFIG_MEMBERS_EX(Hidden_CurrentUtcOffsetMinutes, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_MudsSodStatus, QVariantMap, toMap)  //todo thread-safe?
    CREATE_CONFIG_MEMBERS_EX(Hidden_LastExamAdvanceInfo, QVariantMap, toMap)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BMSLastBatteryIdA, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BMSLastBatteryIdB, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BMSLastPFResetCycleCountA, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BMSPFResetCountA, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BMSLastPFResetCycleCountB, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BMSPFResetCountB, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_PMLastPerformedAt, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_PMLastReminderAt, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Hidden_SyringesUsedInLastExam, QList<QVariant>, toList)
    CREATE_CONFIG_MEMBERS_EX(Hidden_BottleFillCount, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_SruLink_ConnectionType, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_Sound_NormalAudioLevel, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_Sound_InjectionAudioLevel, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_Sound_SudsPrimedAudioLevel, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_Sound_NotificationAudioLevel, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_Sound_KeyClicksEnabled, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Settings_Display_ScreenBrightness, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_Display_ScreenOffTimeoutMinutes, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Settings_Display_Theme, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_Injector_MoodLightColor, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_Injector_PMReminderFrequency, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_Injector_EmptyContrastFirst, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(ServiceCalibration_IADCalibrationMethod, QString, toString)

private:
    Config *db;
    EnvGlobal *env;
    EnvLocal *envLocal;

    void initConfig();
    void emitAllConfigChanged();

private slots:
    void slotAppStarted();
};

#endif // DS_CFG_LOCAL_H
