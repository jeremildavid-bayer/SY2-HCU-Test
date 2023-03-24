#ifndef DIGEST_H
#define DIGEST_H

#include <QVariantMap>
#include <QVariantList>
#include <QVariant>
#include "Common/Util.h"
#include "Common/Common.h"

class Digest
{
public:
    Digest()
    {
        setEventAt(Util::qDateTimeToUtcDateTimeStr(QDateTime::currentDateTimeUtc()));
    }

    ~Digest()
    {
    }

    int getId() const
    {
        if (params.contains(_L("DigestNumber")))
        {
            return params[_L("DigestNumber")].toInt();
        }
        return -1;
    }

    void setId(int id)
    {
        params.insert(_L("DigestNumber"), id);
    }

    void setStatePath(QString statePath)
    {
        params.insert(_L("StatePath"), statePath);
    }

    QString getStatePath() const
    {
        if (params.contains(_L("StatePath")))
        {
            return params[_L("StatePath")].toString();
        }
        return "";
    }

    void setHeatMaintainer(QVariantMap map)
    {
        params.insert(_L("HeatMaintainer"), map);
    }

    void setCurrentExamProgressState(QString currentExamProgressState)
    {
        params.insert(_L("CurrentExamProgressState"), currentExamProgressState);
    }

    QString getCurrentExamProgressState() const
    {
        if (params.contains(_L("CurrentExamProgressState")))
        {
            return params[_L("CurrentExamProgressState")].toString();
        }

        return "";
    }

    void setCruLinkState(int linkStatusVal)
    {
        params.insert(_L("CruState"), linkStatusVal);
    }

    void setScannerInterlocks(QVariantMap map)
    {
        params.insert(_L("ScannerInterlocks"), map);
    }

    QVariantMap getInjectionPlan() const
    {
        if (params.contains(_L("ProgrammedPlan")))
        {
            return params[_L("ProgrammedPlan")].toMap();
        }

        return QVariantMap();
    }

    void setInjectionPlan(QVariantMap map)
    {
        params.insert(_L("ProgrammedPlan"), map);
    }

    void setCurrentExamGuid(QString currentExamGuid)
    {
        params.insert(_L("CurrentExamGuid"), currentExamGuid);
    }

    QString getCurrentExamGuid() const
    {
        if (params.contains(_L("CurrentExamGuid")))
        {
            return params[_L("CurrentExamGuid")].toString();
        }
        return "";
    }

    void setCurrentExamStartedAt(QString eventAt)
    {
        params.insert(_L("CurrentExamStartedAt"), eventAt);
    }

    QString getCurrentExamStartedAt() const
    {
        if (params.contains(_L("CurrentExamStartedAt")))
        {
            return params[_L("CurrentExamStartedAt")].toString();
        }
        return "";
    }

    void setCurrentPressure(int currentPressure)
    {
        params.insert(_L("CurrentPressure"), currentPressure);
    }

    void setIsAirCheckNeeded(bool isNeeded)
    {
        params.insert(_L("IsAirCheckNeeded"), isNeeded);
    }

    void setEventAt(QString eventAt)
    {
        params.insert(_L("EventAt"), eventAt);
    }

    QString getEventAt() const
    {
        if (params.contains(_L("EventAt")))
        {
            return params[_L("EventAt")].toString();
        }

        return "";
    }

    void setAlertsUpdatedAt(QString alertsUpdatedAt)
    {
        params.insert(_L("AlertsUpdatedAt"), alertsUpdatedAt);
    }

    void setFluidSources(QVariantMap listFluidSources)
    {
        params.insert(_L("FluidSources"), listFluidSources);
    }

    void setFluidSourcesFromPartial(QVariantMap listFluidSources)
    {
        QVariantMap curfluidSources;
        if (params.contains(_L("FluidSources")))
        {
            //Use old fluid Sources
            curfluidSources = params.value(_L("FluidSources")).toMap();
        }
        else
        {
            //Generate empty sources (for first time init)
            curfluidSources.insert(_L("BS0"), QVariant());
            curfluidSources.insert(_L("BC1"), QVariant());
            curfluidSources.insert(_L("BC2"), QVariant());
            curfluidSources.insert(_L("RS0"), QVariant());
            curfluidSources.insert(_L("RC1"), QVariant());
            curfluidSources.insert(_L("RC2"), QVariant());
            curfluidSources.insert(_L("ML"), QVariant());
            curfluidSources.insert(_L("PL"), QVariant());
            curfluidSources.insert(_L("WC"), QVariant());
        }

        QStringList incomingKeys = listFluidSources.keys();
        QStringList fluidSourcesKeys = curfluidSources.keys();

        for (int i = 0; i < incomingKeys.length(); i++)
        {
            if (fluidSourcesKeys.contains(incomingKeys.at(i)))
            {
                curfluidSources.insert(incomingKeys.at(i), listFluidSources.value(incomingKeys.at(i)));
            }
        }

        params.insert(_L("FluidSources"), curfluidSources);
    }

    void setPowerStatus(QVariantMap powerStatus)
    {
        params.insert(_L("PowerStatus"), powerStatus);
    }

    QVariantList getStepProgress() const
    {
        if (params.contains(_L("StepProgress")))
        {
            return params[_L("StepProgress")].toList();
        }

        return QVariantList();
    }

    void setStepProgress(QVariantList executedSteps)
    {
        params.insert(_L("StepProgress"), executedSteps);
    }

    QVariantMap getFluidSources() const
    {
        if (params.contains(_L("FluidSources")))
        {
            return params[_L("FluidSources")].toMap();
        }
        return QVariantMap();
    }

    void setIwconfig(QVariantMap iwconfigParams)
    {
         params.insert(_L("Iwconfig"), iwconfigParams);
    }

    QString serialize() const
    {
        return Util::qVarientToJsonData(params, false);
    }

    QVariantMap getParams() const
    {
        return params;
    }

    void setParams(QVariantMap params_)
    {
        params = params_;
    }

    bool operator==(const Digest &arg) const
    {
        return (params == arg.params);
    }

    bool operator!=(const Digest &arg) const
    {
        return !operator==(arg);
    }

private:
    QVariantMap params;
};

#endif // DIGEST_H
