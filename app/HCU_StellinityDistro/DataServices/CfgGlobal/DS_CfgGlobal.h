#ifndef DS_CONFIG_GLOBAL_H
#define DS_CONFIG_GLOBAL_H

#include <QObject>
#include "DS_CfgGlobalDef.h"
#include "Common/Common.h"
#include "Common/Config.h"
#include "DataServices/DataServicesMacros.h"

class DS_CfgGlobal : public QObject
{
    Q_OBJECT

public:
    explicit DS_CfgGlobal(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~DS_CfgGlobal();
    void verifyConfigs(QVariantMap configs,  QString *err);
    QVariantMap getConfigs(QString *err = NULL);
    void setConfigs(QVariantMap configs, bool setChangedAt, QString *err);

    static void registerDataTypesForThread()
    {
        qRegisterMetaType<Config::Item>("Config::Item");
    }

    bool isAutoEmptyEnabled(SyringeIdx idx);

signals:
    void signalConfigChanged();

    // Define OUTGOING Signals (i.e. WHEN DATA CHANGED)
    CREATE_CONFIG_CHANGED_SIGNAL(Hidden_ProductSoftwareVersion)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_CultureCode)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_DateFormat)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_TimeFormat)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_PressureUnit)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_WeightUnit)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_HeightUnit)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_General_TemperatureUnit)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_SruLink_WirelessCountryCode)
    CREATE_CONFIG_CHANGED_SIGNAL(Settings_Injector_HeatMaintainerEnabled)
    CREATE_CONFIG_CHANGED_SIGNAL(AccessControl_Settings_AccessPasscode)
    CREATE_CONFIG_CHANGED_SIGNAL(AccessControl_Settings_AccessProtected)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Exam_MandatoryFields)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Exam_QuickNotes)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Exam_ExamTimeoutPeriod)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_MUDSUseLifeLimitEnforced)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_MUDSUseLifeLimitHours)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_MUDSUseLifeGraceHours)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_UseLife_SalineMaximumUseDuration)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_Country)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_AutoEmptySalineEnabled)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_AutoEmptyContrast1Enabled)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_AutoEmptyContrast2Enabled)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_MaximumFlowRateReduction)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_PressureLimitSensitivity)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_ExtendedSUDSAvailable)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_DefaultSUDSLength)
    CREATE_CONFIG_CHANGED_SIGNAL(Configuration_Behaviors_ChangeContrastEnabled)
    CREATE_CONFIG_CHANGED_SIGNAL(Service_ServicePasscode)
    CREATE_CONFIG_CHANGED_SIGNAL(Service_ContactServiceTelephone)
    CREATE_CONFIG_CHANGED_SIGNAL(Service_TradeshowModeEnabled)

private:
    // Creates GET/SET Properties
    CREATE_CONFIG_MEMBERS_EX(Hidden_ProductSoftwareVersion, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_CultureCode, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_DateFormat, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_TimeFormat, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_PressureUnit, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_WeightUnit, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_HeightUnit, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_General_TemperatureUnit, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_SruLink_WirelessCountryCode, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Settings_Injector_HeatMaintainerEnabled, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(AccessControl_Settings_AccessPasscode, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(AccessControl_Settings_AccessProtected, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Exam_MandatoryFields, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Exam_QuickNotes, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Exam_ExamTimeoutPeriod, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Configuration_UseLife_MUDSUseLifeLimitEnforced, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Configuration_UseLife_MUDSUseLifeLimitHours, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Configuration_UseLife_MUDSUseLifeGraceHours, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Configuration_UseLife_SalineMaximumUseDuration, int, toInt)

    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_Country, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_AutoEmptySalineEnabled, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_AutoEmptyContrast1Enabled, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_AutoEmptyContrast2Enabled, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_MaximumFlowRateReduction, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_PressureLimitSensitivity, int, toInt)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_ExtendedSUDSAvailable, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_DefaultSUDSLength, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Configuration_Behaviors_ChangeContrastEnabled, bool, toBool)
    CREATE_CONFIG_MEMBERS_EX(Service_ServicePasscode, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Service_ContactServiceTelephone, QString, toString)
    CREATE_CONFIG_MEMBERS_EX(Service_TradeshowModeEnabled, bool, toBool)

    void emitAllConfigChanged();

private:
    Config *db;
    EnvGlobal *env;
    EnvLocal *envLocal;
    bool dbLoadFailed;

    void initConfig();
private slots:
    void slotAppStarted();
    void slotAppInitialised();
};

#endif // DS_CONFIG_GLOBAL_H
