#include "QML_CfgLocal.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Cru/DS_CruAction.h"
#include "DataServices/Exam/DS_ExamData.h"

QML_CfgLocal::QML_CfgLocal(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("QML_CfgLocal", "CFG_LOCAL");
    qmlSrc = env->qml.object->findChild<QObject*>("dsCfgLocal");
    env->qml.engine->rootContext()->setContextProperty("dsCfgLocalCpp", this);

    if (qmlSrc == NULL)
    {
        LOG_ERROR("Failed to assign qmlSrc.\n");
    }

    // Register Data Callbacks
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_ScreenW, this, [=]{ setScreenGeometry(); });
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_ScreenH, this, [=]{ setScreenGeometry(); });
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_ScreenX, this, [=]{ setScreenGeometry(); });
    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_ScreenY, this, [=]{ setScreenGeometry(); });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Sound_NormalAudioLevel, this, [=](const Config::Item &cfg) {
        double val =  cfg.value.toDouble() / DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX;
        qmlSrc->setProperty("audioLevelNormal", val);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Sound_InjectionAudioLevel, this, [=](const Config::Item &cfg) {
        double val = cfg.value.toDouble() / DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX;
        qmlSrc->setProperty("audioLevelInjection", val);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Sound_SudsPrimedAudioLevel, this, [=](const Config::Item &cfg) {
        double val = cfg.value.toDouble() / DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX;
        qmlSrc->setProperty("audioLevelSudsPrimed", val);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Sound_NotificationAudioLevel, this, [=](const Config::Item &cfg) {
        double val = cfg.value.toDouble() / DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX;
        qmlSrc->setProperty("audioLevelNotification", val);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Sound_KeyClicksEnabled, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("audioKeyClicksEnabled", cfg.value);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_PressureOptionValues, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("pressureCfgOptions", cfg.value);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_FluidOptions, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("fluidOptions", cfg.value);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_CurrentUtcOffsetMinutes, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("currentUtcOffsetMinutes", cfg.value.toInt());
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_Display_Theme, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("uiTheme", cfg.value);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Settings_SruLink_ConnectionType, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("cruLinkType", cfg.value);
        qmlSrc->setProperty("cfgNetworkConnectionType", ImrParser::ToImr_ConfigItem(cfg));
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_ServiceCalibration_IADCalibrationMethod, this, [=](const Config::Item &cfg) {
        qmlSrc->setProperty("iadCalibrationMethod", cfg.value);
    });

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged_Hidden_PMLastPerformedAt, this, [=](const Config::Item &cfg) {
        int epochSecsLastPM = cfg.value.toInt();
        if (epochSecsLastPM != 0)
        {
            QDateTime lastPMPerformed = QDateTime::fromSecsSinceEpoch(epochSecsLastPM);

            QDateTime now = QDateTime::currentDateTimeUtc();
            qint64 daysPastLastPM = lastPMPerformed.daysTo(now);
            if (daysPastLastPM > 365)
            {
                QString lastPMPerformedDateString = lastPMPerformed.toString("dd.MM.yyyy");
                qmlSrc->setProperty("lastPMPerformedDateString", lastPMPerformedDateString);
            }
            else
            {
                qmlSrc->setProperty("lastPMPerformedDateString", "");
            }
        }
        else
        {
            qmlSrc->setProperty("lastPMPerformedDateString", "");
        }
    });

    qmlSrc->setProperty("soundVolumeMin", DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MIN);
    qmlSrc->setProperty("soundVolumeMax", DS_CfgLocalDef::SOUND_VOLUME_LEVEL_MAX);

    connect(env->ds.cfgLocal, &DS_CfgLocal::signalConfigChanged, this, [=](){
        QString err;
        QVariantMap map = env->ds.cfgLocal->getConfigs(&err);

        if (err == "")
        {
            QVariantList list = Util::configMapToSortedList(map, &err);
            qmlSrc->setProperty("configTable", list);
        }

        if (err != "")
        {
            LOG_ERROR("Failed to get config list (err=%s)\n", err.CSTR());
        }
    });
}

QML_CfgLocal::~QML_CfgLocal()
{
    delete envLocal;
}

void QML_CfgLocal::setScreenGeometry()
{
    qmlSrc->setProperty("screenW", QVariant(env->ds.cfgLocal->get_Hidden_ScreenW()));
    qmlSrc->setProperty("screenH", QVariant(env->ds.cfgLocal->get_Hidden_ScreenH()));
    qmlSrc->setProperty("screenX", QVariant(env->ds.cfgLocal->get_Hidden_ScreenX()));
    qmlSrc->setProperty("screenY", QVariant(env->ds.cfgLocal->get_Hidden_ScreenY()));
}

void QML_CfgLocal::slotUiThemeChanged(QString uiTheme)
{
    env->ds.cfgLocal->set_Settings_Display_Theme(uiTheme);
}

void QML_CfgLocal::slotFluidOptionsChanged(QVariantMap fluidOptions)
{
    QString err;

    // Convert CPP then IMR again to populate missing fields
    DS_DeviceDef::FluidOptions fluidOptionsCpp = ImrParser::ToCpp_FluidOptions(fluidOptions, &err);
    fluidOptionsCpp.changedAtEpochMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

    if (err != "")
    {
        LOG_ERROR("Bad Fluid Option. Err=%s, FluidOptions=%s\n", err.CSTR(), Util::qVarientToJsonData(fluidOptions).CSTR());
    }

    // Do not set ExpirationDate and LotBatch fields
    for (int familyIdx = 0; familyIdx < fluidOptionsCpp.contrastFamilies.length(); familyIdx++)
    {
        DS_DeviceDef::FluidFamily *family = &fluidOptionsCpp.contrastFamilies[familyIdx];
        for (int packageIdx = 0; packageIdx < family->fluidPackages.length(); packageIdx++)
        {
            family->fluidPackages[packageIdx].expirationDateEpochMs = -1;
            family->fluidPackages[packageIdx].lotBatch = "";
        }
    }

    DS_DeviceDef::FluidOptions curFluidOptions = ImrParser::ToCpp_FluidOptions(env->ds.cfgLocal->get_Hidden_FluidOptions());

    if (fluidOptionsCpp.isSameFluidOptions(curFluidOptions))
    {
        // fluidOptions not changed
        return;
    }

    LOG_INFO("slotFluidOptionsChanged(): New Contrast Families=\n%s\n", Util::qVarientToJsonData(ImrParser::ToImr_FluidFamilies(fluidOptionsCpp.contrastFamilies), false).CSTR());

    QVariantMap newFluidOptions = ImrParser::ToImr_FluidOptions(fluidOptionsCpp);
    env->ds.cfgLocal->set_Hidden_FluidOptions(newFluidOptions);
}

void QML_CfgLocal::slotConfigChanged(QVariantMap configItem)
{
    QString err;
    QVariantMap map = env->ds.cfgLocal->getConfigs(&err);

    if (err == "")
    {
        map.insert(configItem["KeyName"].toString(), configItem);
        env->ds.cfgLocal->setConfigs(map, true, &err);
    }

    if (err != "")
    {
        LOG_ERROR("Failed to set config (err=%s)\n", err.CSTR());
    }
}

// This is in HeatMaintainerEnabled is in global config, but handled here
void QML_CfgLocal::slotHeaterActive(bool active)
{
    env->ds.cfgGlobal->set_Settings_Injector_HeatMaintainerEnabled(active);
}

void QML_CfgLocal::slotSetLastPMReminderAtToNow()
{
    int value = QDateTime::currentSecsSinceEpoch();
    env->ds.cfgLocal->set_Hidden_PMLastReminderAt(value);
}

void QML_CfgLocal::slotSetLastPMPerformedAtToNow()
{
    int value = QDateTime::currentSecsSinceEpoch();
    env->ds.cfgLocal->set_Hidden_PMLastPerformedAt(value);
}

void QML_CfgLocal::slotLoadSampleFluidOptions()
{
    QString dbFileName = QString(PATH_RESOURCES_DEFAULT_CONFIG) + "/SampleFluidOptions.json";
    Config *db = new Config(dbFileName, env, envLocal);
    if (db != NULL && db->isCfgOk())
    {
        QVariant cfgVal = db->get("Hidden_FluidOptions");
        //@todo is this correct?
        env->ds.cfgLocal->set_Hidden_FluidOptions(cfgVal.toMap());
    }
    else
    {
        LOG_ERROR("slotLoadSampleFluidOptions(): Bad Config\n");
    }
    delete db;
}

void QML_CfgLocal::slotLoadSampleInjectionPlans()
{
    QString dbFileName = QString(PATH_RESOURCES_DEFAULT_CONFIG) + "/SampleInjectionPlans.json";
    Config *db = new Config(dbFileName, env, envLocal);
    if (db != NULL && db->isCfgOk())
    {//todo thread-safe?
        QVariant cfgVal = db->get("Hidden_InjectionPlanTemplateGroups");
        Config::Item cfg = env->ds.cfgLocal->getItem_Hidden_InjectionPlanTemplateGroups();
        cfg.value = cfgVal;
        env->ds.cfgLocal->setHidden_InjectionPlanTemplateGroups(cfg);

        DS_ExamDef::InjectionPlanTemplateGroups injectionPlanTemplateGroups = ImrParser::ToCpp_InjectionPlanTemplateGroups(cfgVal.toList(), NULL, true);
        env->ds.examData->setInjectionPlanTemplateGroups(injectionPlanTemplateGroups);
    }
    else
    {
        LOG_ERROR("slotLoadSampleInjectionPlans(): Bad Config\n");
    }
    delete db;
}
