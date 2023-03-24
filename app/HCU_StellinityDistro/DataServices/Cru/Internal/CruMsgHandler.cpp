#include "CruMsgHandler.h"
#include "Common/Util.h"
#include "Common/ImrParser.h"
#include "DataServices/Exam/DS_ExamData.h"
#include "DataServices/Cru/DS_CruData.h"
#include "DataServices/Mwl/DS_MwlData.h"
#include "DataServices/System/DS_SystemAction.h"
#include "DataServices/CfgGlobal/DS_CfgGlobal.h"
#include "DataServices/CfgLocal/DS_CfgLocal.h"
#include "DataServices/Exam/DS_ExamAction.h"

CruMsgHandler::CruMsgHandler(QObject *parent, EnvGlobal *env_) :
    QObject(parent),
    env(env_)
{
    envLocal = new EnvLocal("DS_Cru-MsgHandler", "CRU_MSG_HANDLER", LOG_LRG_SIZE_BYTES);
}

CruMsgHandler::~CruMsgHandler()
{
    delete envLocal;
}

QString CruMsgHandler::handleMsg(QString url, QString method, QString replyMsg)
{
    QString errStr = "";

    QString replyMsgStr = replyMsg;
    QString anonymizedUrl = url;

    QString buildType = env->ds.systemData->getBuildType();
    bool anonymizeOn = (buildType == BUILD_TYPE_VNV || buildType == BUILD_TYPE_REL);

    if (anonymizeOn)
    {
        if (url.contains(IMR_CRU_URL_DIGEST))
        {
            // Anonymize patient name
            QJsonDocument document = QJsonDocument::fromJson(replyMsg.toUtf8());
            QVariantMap digestMap = document.toVariant().toMap();
            if (digestMap.contains("PatientName"))
            {
                QString patientName = digestMap["PatientName"].toString();
                digestMap.insert("PatientName", ANONYMIZED_STR);
            }

            // Anonymize entire injector exam object
            if (digestMap.contains("CurrentInjectorExam"))
            {
                digestMap.insert("CurrentInjectorExam", QVariant());
            }

            replyMsgStr = Util::qVarientToJsonData(digestMap);
        }
        else if (url.contains(IMR_CRU_URL_WORKLIST))
        {
            QJsonDocument document = QJsonDocument::fromJson(replyMsg.toUtf8());
            QList<QVariant> worklist = document.toVariant().toList();

            // Anonymize entire list of worlist entry objects, but Slog the count
            replyMsgStr = QString().asprintf("%d items", (int)worklist.count());
        }
        else if (url.contains(IMR_CRU_URL_SELECT_WORKLIST_ENTRY))
        {
            // Anonymize entire injector exam object
            replyMsgStr = ANONYMIZED_STR;
        }
        else if (url.contains(IMR_CRU_URL_UPDATE_EXAM_FIELD) || url.contains(IMR_CRU_URL_UPDATE_EXAM_FIELD_PARAMETER))
        {
            // NOTE: when url contains updateExamField or updateExamFieldParameter, we want to anonymize the fields.
            // Most generic methods chosen here is to anonymize entire field.
            // search for the first occurrence of '&' which is at the end of the examGuid
            // example url = http://192.168.11.3:8901/cru/v3/commands/updateExamField?examGuid=6e4da90d-7c05-4689-a5ff-3a91afb2dc49&name=PatientWeight&value=9
            // above will be anonymized to : http://192.168.11.3:8901/cru/v3/commands/updateExamField?examGuid=6e4da90d-7c05-4689-a5ff-3a91afb2dc49&********

            // indexOf will return -1 if not found
            int fieldStart = url.indexOf('&') + 1;
            if (fieldStart != 0)
            {
                anonymizedUrl.replace(fieldStart, url.length(), ANONYMIZED_STR);
            }
        }
    }

    LOG_DEBUG("Message Received. URL:%s, Reply:\n%s\n\n", anonymizedUrl.CSTR(), replyMsgStr.CSTR());

    if (replyMsg == "")
    {
        errStr = "Empty Reply String";
    }
    else if ( (url.contains(IMR_CRU_URL_DIGEST)) && (method == _L("get")) )
    {
        errStr = handleDigest(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_PROFILE)) && (method == _L("get")) )
    {
        errStr = handleProfile(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_FLUIDS)) && (method == _L("get")) )
    {
        errStr = handleFluids(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_INJECT_PLANS)) && (method == _L("get")) )
    {
        errStr = handleInjectionPlans(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_INJECT_PLAN)) && (method == _L("get")) )
    {
        errStr = handleInjectionPlan(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_WORKLIST)) && (method == _L("get")) )
    {
        errStr = handleGetWorklistEntries(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_CONFIGS)) && (method == _L("get")) )
    {
        errStr = handleGetConfigs(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_UPDATE_INJECTION_PARAMTER)) && (method == _L("post")) )
    {
        errStr = handlePostUpdateInjectionParameter(replyMsg);
    }
    else if ( (url.contains(IMR_CRU_URL_SELECT_WORKLIST_ENTRY)) && (method == _L("post")) )
    {
        errStr = handlePostSelectWorklistEntry(replyMsg);
    }
    else
    {
        LOG_INFO("No associated handler for reply: (url=%s, method=%s, replyMsg=%s) received\n",
                    anonymizedUrl.CSTR(),
                    method.CSTR(),
                    replyMsg.CSTR());
    }

    return errStr;
}

QString CruMsgHandler::handleProfile(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap mapRoot = jsonDoc.toVariant().toMap();

        if (mapRoot.contains("LicensedFeatureOptions"))
        {
            DS_CruDef::CruLicensedFeatures licensedFeatureOptions = ImrParser::ToCpp_CruLicensedFeatures(mapRoot["LicensedFeatureOptions"].toList(), &err);

            env->ds.cruData->setLicenseEnabledWorklistSelection(licensedFeatureOptions.contains(DS_CruDef::CRU_LICENSED_FEATURE_WORKLIST));
            env->ds.cruData->setLicenseEnabledPatientStudyContext(licensedFeatureOptions.contains(DS_CruDef::CRU_LICENSED_FEATURE_PATIENT));
        }
        else
        {
            err = "Missing field (LicensedFeatureOptions)";
        }

        if (mapRoot.contains("SerialNumber"))
        {
            env->ds.cruData->setSerialNumber(mapRoot["SerialNumber"].toString());
        }
        else
        {
            err = "Missing field (SerialNumber)";
        }

        if (mapRoot.contains("SoftwareVersion"))
        {
            env->ds.cruData->setSoftwareVersion(mapRoot["SoftwareVersion"].toString());
        }
        else
        {
            err = "Missing field (SoftwareVersion)";
        }
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("Failed to parse Profile (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handleFluids(QString jsonStr)
{
    jsonStr = jsonStr.simplified();

    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {//@todo check
        QVariantMap value = jsonDoc.toVariant().toMap();
        env->ds.cfgLocal->set_Hidden_FluidOptions(value);
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("Failed to parse Fluid DB (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handleInjectionPlans(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantList groupsImr = jsonDoc.toVariant().toList();

        DS_ExamDef::InjectionPlanTemplateGroups prevGroups = env->ds.examData->getInjectionPlanTemplateGroups();
        DS_ExamDef::InjectionPlanTemplateGroups curGroups = ImrParser::ToCpp_InjectionPlanTemplateGroups(groupsImr, &err, true);

        // Remove empty group plans
        for (int groupIdx = 0; groupIdx < curGroups.length(); groupIdx++)
        {
            if (curGroups[groupIdx].name == "")
            {
                for (int planIdx = 0; planIdx < curGroups[groupIdx].planDigests.length(); planIdx++)
                {
                    if (curGroups[groupIdx].planDigests[planIdx].guid != DEFAULT_INJECT_PLAN_TEMPLATE_GUID)
                    {
                        LOG_WARNING("PLANS: Empty group/non-default plan digest received. Plan digest is removed. TemplateGuid=%s\n",
                                    curGroups[groupIdx].planDigests[planIdx].guid.CSTR());
                        curGroups[groupIdx].planDigests.removeAt(planIdx);
                        planIdx--;
                    }
                }

                if (curGroups[groupIdx].planDigests.length() == 0)
                {
                    LOG_WARNING("PLANS: Group data(name=%s) is empty. Group removed.\n", curGroups[groupIdx].name.CSTR());
                    curGroups.removeAt(groupIdx);
                    groupIdx--;
                }
            }
        }

        // Load available plan data
        for (int groupIdx = 0; groupIdx < curGroups.length(); groupIdx++)
        {
            for (int planIdx = 0; planIdx < curGroups[groupIdx].planDigests.length(); planIdx++)
            {
                DS_ExamDef::InjectionPlanDigest *curPlanDigest = &curGroups[groupIdx].planDigests[planIdx];
                const DS_ExamDef::InjectionPlanDigest *prevPlanDigest = env->ds.examData->getPlanDigestFromTemplateGuid(prevGroups, curPlanDigest->guid);

                bool planUpdateRequired = false;

                if (curPlanDigest == NULL)
                {
                    LOG_ERROR("Group[%s]:Plan[%s]: Plan is empty\n", curGroups[groupIdx].name.CSTR(), curPlanDigest->name.CSTR());
                }
                else if (prevPlanDigest == NULL)
                {
                    LOG_DEBUG("Group[%s]:Plan[%s]: Plan is new\n", curGroups[groupIdx].name.CSTR(), curPlanDigest->name.CSTR());
                    planUpdateRequired = true;
                }
                else if (curPlanDigest->histId != prevPlanDigest->histId)
                {
                    LOG_INFO("Group[%s]:Plan[%s]: Plan is out of date(histId %s->%s)\n", curGroups[groupIdx].name.CSTR(), curPlanDigest->name.CSTR(), prevPlanDigest->histId.CSTR(), curPlanDigest->histId.CSTR());
                    planUpdateRequired = true;
                    curPlanDigest->plan = prevPlanDigest->plan; // Store the old protocol for now
                }
                else if (prevPlanDigest->plan.steps.length() == 0)
                {
                    LOG_WARNING("Group[%s]:Plan[%s]: Plan has no step\n", curGroups[groupIdx].name.CSTR(), prevPlanDigest->name.CSTR());
                    planUpdateRequired = true;
                }
                else
                {
                    // Current plan is same as previous plan. Dont' need to update
                    LOG_DEBUG("Group[%s]:Plan[%s]: Plan is not changed\n", curGroups[groupIdx].name.CSTR(), curPlanDigest->name.CSTR());
                    *curPlanDigest = *prevPlanDigest;
                }

                if (planUpdateRequired)
                {
                    // Prepare plan to receive new plan from CRU
                    curPlanDigest->state = DS_ExamDef::INJECTION_PLAN_DIGEST_STATE_INITIALISING;
                    curPlanDigest->plan.guid = EMPTY_GUID;
                    LOG_INFO("Group[%s]:Plan[%s]: Plan (templateGuid=%s) will be updated soon..\n", curGroups[groupIdx].name.CSTR(), curPlanDigest->name.CSTR(), curPlanDigest->guid.CSTR());
                }
            }
        }

        // Check if current preview plan is still ok to be loaded
        QString curPlanPreviewTemplateGuid = env->ds.examData->getInjectionPlanPreview().templateGuid;
        bool curPlanPreviewReady = false;
        for (int groupIdx = 0; groupIdx < curGroups.length(); groupIdx++)
        {
            DS_ExamDef::InjectionPlanDigest *planDigest = curGroups[groupIdx].getPlanDigestFromTemplateGuid(curPlanPreviewTemplateGuid);
            if (planDigest != NULL)
            {
                curPlanPreviewReady = true;
                break;
            }
        }

        if (!curPlanPreviewReady)
        {
            LOG_WARNING("Current Preview Plan is not ready. Selecting the default plan.\n");
            const DS_ExamDef::InjectionPlan *defaultPlan = env->ds.examData->getDefaultInjectionPlanTemplate();
            if (defaultPlan == NULL)
            {
                LOG_ERROR("defaultPlan is not found. Unabled to set to default plan.\n");
            }
            else
            {
                env->ds.examData->setInjectionPlanPreview(*defaultPlan);
            }
        }

        // Finally, update all groups
        env->ds.examData->setInjectionPlanTemplateGroups(curGroups);
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("Failed to parse Injection Plans (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handleInjectionPlan(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap planImr = jsonDoc.toVariant().toMap();

        DS_ExamDef::InjectionPlan plan = ImrParser::ToCpp_InjectionPlan(planImr, &err);
        DS_ExamDef::InjectionPlanTemplateGroups groups = env->ds.examData->getInjectionPlanTemplateGroups();
        DS_ExamDef::InjectionPlanDigest *planDigest = env->ds.examData->getPlanDigestFromTemplateGuid(groups, plan.templateGuid);

        if (planDigest != NULL)
        {
            if (plan.steps.length() == 0)
            {
                // Received plan data has empty steps. Remove planDigest
                for (int groupIdx = 0; groupIdx < groups.length(); groupIdx++)
                {
                    for (int planIdx = 0; planIdx < groups[groupIdx].planDigests.length(); planIdx++)
                    {
                        if (groups[groupIdx].planDigests[planIdx].guid == plan.templateGuid)
                        {
                            LOG_WARNING("PLAN: Plan data with empty steps received. Plan digest is removed. TemplateGuid=%s\n", plan.templateGuid.CSTR());
                            groups[groupIdx].planDigests.removeAt(planIdx);

                            if (groups[groupIdx].planDigests.length() == 0)
                            {
                                LOG_WARNING("PLAN: Group data(name=%s) is empty. Group removed.\n", groups[groupIdx].name.CSTR());
                                groups.removeAt(groupIdx);
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            else
            {
                // Save plan in the planDigest
                LOG_INFO("PLAN: Plan is fully updated (planGuid=%s, templateGuid=%s, planDigest=%s)\n", plan.guid.CSTR(), plan.templateGuid.CSTR(), planDigest->guid.CSTR());
                planDigest->plan = plan;
            }

            for (int i = 0; i < groups.length(); i++)
            {
                for (int j = 0; j < groups[i].planDigests.length(); j++)
                {
                    LOG_DEBUG("PlanDigest[%d]: guid=%s, plan.templateGuid=%s\n", j, groups[i].planDigests[j].guid.CSTR(), groups[i].planDigests[j].plan.templateGuid.CSTR());
                }
            }
            env->ds.examData->setInjectionPlanTemplateGroups(groups);
        }
        else
        {
            err = QString().asprintf("No Plan Template Found (TemplateGuid=%s)", plan.templateGuid.CSTR());
        }
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("PLAN: Failed to parse Injection Plan Data (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handleDigest(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap mapRoot = jsonDoc.toVariant().toMap();

        DS_CruDef::CruLinkStatus cruLinkStatus;

        if (mapRoot.contains("InjectorLinkStatus"))
        {
            cruLinkStatus = ImrParser::ToCpp_CruLinkStatus(mapRoot["InjectorLinkStatus"].toMap(), &err);

            //if (cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_INACTIVE)
            //{//CRu is talking. Need to change the link state to recovering to allow for synchronisation
            //    cruLinkStatus.state = DS_CruDef::CRU_LINK_STATE_RECOVERING;
            //    LOG_INFO("CRU link state change from inactive to recovering in handleDigest\n");
            //}

            env->ds.cruData->setCruLinkStatus(cruLinkStatus);

        }
        else
        {
            err = "Missing field (InjectorLinkStatus)";
            goto bail;
        }

        if (mapRoot.contains("CurrentUtcNow"))
        {
            QString currentUtcNowStr = mapRoot["CurrentUtcNow"].toString();
            QDateTime currentUtcNow = Util::utcDateTimeStrToQDateTime(currentUtcNowStr);
            env->ds.cruData->setCurrentUtcNowEpochSec(currentUtcNow.toSecsSinceEpoch());
        }

        if (mapRoot.contains("CurrentUtcOffsetMinutes"))
        {//todo check -is this cause a performance hit ever minutes because of the update?
            int currentUtcOffsetMinutes = env->ds.cfgLocal->get_Hidden_CurrentUtcOffsetMinutes();
            int root_utc_offset = mapRoot["CurrentUtcOffsetMinutes"].toInt();
            if (currentUtcOffsetMinutes != mapRoot["CurrentUtcOffsetMinutes"])
            {
                LOG_INFO("CurrentUtcOffsetMinutes is changed from %d to %d\n", currentUtcOffsetMinutes, root_utc_offset);
                env->ds.cfgLocal->set_Hidden_CurrentUtcOffsetMinutes(root_utc_offset);
            }
        }
        else
        {
            err = "Missing field (CurrentUtcOffsetMinutes)";
        }

        if (cruLinkStatus.state == DS_CruDef::CRU_LINK_STATE_ACTIVE)
        {
            if (mapRoot.contains("SuiteName"))
            {
                env->ds.mwlData->setSuiteName(mapRoot["SuiteName"].toString());
            }
            else if (err == "")
            {
                err = "Missing field (SuiteName)";
            }

            if (mapRoot.contains("PatientName"))
            {
                env->ds.mwlData->setPatientName(mapRoot["PatientName"].toString());
            }
            else if (err == "")
            {
                err = "Missing field (PatientName)";
            }

            if (mapRoot.contains("StudyDescription"))
            {
                env->ds.mwlData->setStudyDescription(mapRoot["StudyDescription"].toString());
            }
            else if (err == "")
            {
                err = "Missing field (StudyDescription)";
            }

            if (mapRoot.contains("CurrentInjectorExam"))
            {
                if (env->ds.examData->isExamStarted())
                {
                    // Exam is in progress. Update Exam AdvanceInfo
                    QVariantMap curInjectorExam = mapRoot["CurrentInjectorExam"].toMap();
                    DS_ExamDef::ExamAdvanceInfo examAdvanceInfo = ImrParser::ToCpp_ExamAdvanceInfo(curInjectorExam);
                    env->ds.examData->setExamAdvanceInfo(examAdvanceInfo);
                }
            }
            else if (err == "")
            {
                err = "Missing field (CurrentInjectorExam)";
            }
        }
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

bail:
    if (err != "")
    {
        LOG_ERROR("Failed to parse Digest (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handleGetWorklistEntries(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantList list = jsonDoc.toVariant().toList();
        DS_MwlDef::WorklistEntries worklistEntries = ImrParser::ToCpp_WorklistEntries(list, &err);

        if (err != "")
        {
            LOG_ERROR("handleGetWorklistEntries(): Parse Error (src=%s, err=%s)\n", Util::qVarientToJsonData(list).CSTR(), err.CSTR());
        }
        env->ds.mwlData->setWorklistEntries(worklistEntries);
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
        goto bail;
    }

bail:
    if (err != "")
    {
        LOG_ERROR("Failed to parse Worklist (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handleGetConfigs(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap configs = jsonDoc.toVariant().toMap();
        env->ds.cfgGlobal->setConfigs(configs, false, &err);
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("Failed to parse Worklist (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handlePostUpdateInjectionParameter(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap newPlanMap = jsonDoc.toVariant().toMap();
        DS_ExamDef::InjectionPlan newPlan = ImrParser::ToCpp_InjectionPlan(newPlanMap, &err);
        if (err == "")
        {
            env->ds.examData->setInjectionPlan(newPlan);
        }
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("Failed to parse Worklist (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}

QString CruMsgHandler::handlePostSelectWorklistEntry(QString jsonStr)
{
    QJsonParseError parseErr;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseErr);
    QString err = "";

    if (parseErr.error == QJsonParseError::NoError)
    {
        QVariantMap currentInjectorExamMap = jsonDoc.toVariant().toMap();
        DS_ExamDef::ExamAdvanceInfo examAdvanceInfo = ImrParser::ToCpp_ExamAdvanceInfo(currentInjectorExamMap, &err);
        if (err == "")
        {
            env->ds.examData->setExamAdvanceInfo(examAdvanceInfo);

            DS_ExamDef::InjectionRequestProcessStatus injReqProcessStatus = env->ds.examData->getInjectionRequestProcessStatus();
            // only force start exam when Anonymous was selected from HCU's T_ARMFAILED_ExamNotStarted popup.
            // When CRU shows the popup but HCU selects Anonymous from Patient screen, it shouldn't force anything.
            // There can be two T_ARMFAILED_ExamNotStarted errors, "T_ARMFAILED_ExamNotStarted" and "T_ARMFAILED_ExamNotStarted T_StartExamFor_XXX"
            if (injReqProcessStatus.requestedByHcu && (injReqProcessStatus.state.indexOf("T_ARMFAILED_ExamNotStarted") >= 0))
            {
                LOG_INFO("handlePostSelectWorklistEntry(): ARM failed due to exam not started. The exam guid is now set by CRU. Start Exam..\n");

                DataServiceActionStatus examStartedStatus = env->ds.examAction->actExamStart();
                if (examStartedStatus.state != DS_ACTION_STATE_COMPLETED)
                {
                    err = examStartedStatus.err;
                }
            }
        }
    }
    else
    {
        err = "Parse Failed: " + parseErr.errorString();
    }

    if (err != "")
    {
        LOG_ERROR("handlePostSelectWorklistEntry(): Failed to parse currentInjectorExamMap (err=%s, str=%s)\n", err.CSTR(), jsonStr.CSTR());
    }

    return err;
}
