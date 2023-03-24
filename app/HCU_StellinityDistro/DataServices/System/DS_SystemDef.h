#ifndef DS_SYSTEM_DEF_H
#define DS_SYSTEM_DEF_H

#include <QString>
#include <QVariantList>

class DS_SystemDef
{


public:
    // ==============================================
    // Config Enumerations

    // ==============================================
    // Data Enumerations
    enum StatePath
    {
        STATE_PATH_OFF_UNREACHABLE = 0,
        STATE_PATH_ON_REACHABLE,
        STATE_PATH_STARTUP_UNKNOWN,
        STATE_PATH_IDLE,
        STATE_PATH_READY_ARMED,
        STATE_PATH_EXECUTING, // injecting or pause phase
        STATE_PATH_BUSY_WAITING,
        STATE_PATH_BUSY_FINISHING,
        STATE_PATH_BUSY_HOLDING, // user paused during injection
        STATE_PATH_BUSY_SERVICING,
        STATE_PATH_ERROR,
        STATE_PATH_SERVICING
    };

    // ==============================================
    // Data Structures
    struct NetworkSettingParams
    {
        bool isWifiType;
        bool isDhcpMode;
        QString activeIface;
        QString inactiveIface;
        QString netmask;
        QString localIp;
        int localPort; // Imr REST api port
        QString serverIp;
        QString ssid;
        QString pwd;
        QString countryCode; //  for wifi
        qint64 setupCompletedEpochMs;
        QString routerIp;

        NetworkSettingParams()
        {
            init();
        }

        void init()
        {
            isWifiType = false;
            isDhcpMode = false;
            activeIface = "";
            inactiveIface = "";
            netmask = "";
            localIp = "";
            localPort = 0;
            serverIp = "";
            ssid = "";
            pwd = "";
            countryCode = "";
            setupCompletedEpochMs = -1;
            routerIp = "";
        }

        bool operator==(const NetworkSettingParams &arg) const
        {
            return ( (isWifiType == arg.isWifiType) &&
                     (isDhcpMode == arg.isDhcpMode) &&
                     (activeIface == arg.activeIface) &&
                     (inactiveIface == arg.inactiveIface) &&
                     (netmask == arg.netmask) &&
                     (localIp == arg.localIp) &&
                     (localPort == arg.localPort) &&
                     (serverIp == arg.serverIp) &&
                     (ssid == arg.ssid) &&
                     (pwd == arg.pwd) &&
                     (countryCode == arg.countryCode) &&
                     (setupCompletedEpochMs == arg.setupCompletedEpochMs) &&
                     (routerIp == arg.routerIp) );
        }

        bool operator!=(const NetworkSettingParams &arg) const
        {
            return !operator==(arg);
        }
    };

    struct IwconfigParams
    {
        int samplePeriodSeconds;
        int sampleWindowSeconds;

        QList<double> linkQualities;
        int linkQualityMin;
        int linkQualityMax;
        int linkQualityAvg;

        QList<double> signalLevels;
        int signalLevelMin;
        int signalLevelMax;
        int signalLevelAvg;

        QList<double> noiseLevels;
        int noiseLevelMin;
        int noiseLevelMax;
        int noiseLevelAvg;

        IwconfigParams()
        {
            init();
        }

        void init()
        {
            samplePeriodSeconds = 0;
            sampleWindowSeconds = 0;

            clearLinkQualityData();
            clearSignalLevelData();
            clearNoiseLevelData();
        }

        void clearLinkQualityData()
        {

            linkQualities.clear();
            linkQualityMin = 0;
            linkQualityMax = 0;
            linkQualityAvg = 0;
        }

        void clearSignalLevelData()
        {
            signalLevels.clear();
            signalLevelMin = 0;
            signalLevelMax = 0;
            signalLevelAvg = 0;
        }

        void clearNoiseLevelData()
        {
            noiseLevels.clear();
            noiseLevelMin = 0;
            noiseLevelMax = 0;
            noiseLevelAvg = 0;
        }

        void computeStats(int periodSeconds, int windowSeconds)
        {
            samplePeriodSeconds = periodSeconds;
            sampleWindowSeconds = windowSeconds;

            if (linkQualities.size() > 0)
            {
                std::sort(linkQualities.begin(), linkQualities.end());
                linkQualityMin = linkQualities.first();
                linkQualityMax = linkQualities.last();
                linkQualityAvg = (int) (std::accumulate(linkQualities.begin(), linkQualities.end(), 0) / linkQualities.size());
            }
            else
            {
                clearLinkQualityData();
            }

            if (signalLevels.size() > 0)
            {
                std::sort(signalLevels.begin(), signalLevels.end());
                signalLevelMin = signalLevels.first();
                signalLevelMax = signalLevels.last();
                signalLevelAvg = (int) (std::accumulate(signalLevels.begin(), signalLevels.end(), 0) / signalLevels.size());
            }
            else
            {
                clearSignalLevelData();
            }

            if (noiseLevels.size() > 0)
            {
                std::sort(noiseLevels.begin(), noiseLevels.end());
                noiseLevelMin = noiseLevels.first();
                noiseLevelMax = noiseLevels.last();
                noiseLevelAvg = (int) (std::accumulate(noiseLevels.begin(), noiseLevels.end(), 0) / noiseLevels.size());
            }
            else
            {
               clearNoiseLevelData();
            }
        }

        bool operator==(const IwconfigParams &arg) const
        {
            return ( (samplePeriodSeconds == arg.samplePeriodSeconds) &&
                     (sampleWindowSeconds == arg.sampleWindowSeconds) &&
                     //purposefully do not include linkQualities, as clients only receive the "stats"
                     (linkQualityMin == arg.linkQualityMin) &&
                     (linkQualityMax == arg.linkQualityMax) &&
                     (linkQualityAvg == arg.linkQualityAvg) &&
                     //purposefully do not include signalLevels, as clients only receive the "stats"
                     (signalLevelMin == arg.signalLevelMin) &&
                     (signalLevelMax == arg.signalLevelMax) &&
                     (signalLevelAvg == arg.signalLevelAvg) &&
                     //purposefully do not include noiseLevels, as clients only receive the "stats"
                     (noiseLevelMin == arg.noiseLevelMin) &&
                     (noiseLevelMax == arg.noiseLevelMax) &&
                     (noiseLevelAvg == arg.noiseLevelAvg) );
        }

        bool operator!=(const IwconfigParams &arg) const
        {
            return !operator==(arg);
        }
    };

    struct HcuTemperatureParams
    {
        double cpuTemperatureCelcius;
        int fanSpeed;

        HcuTemperatureParams()
        {
            init();
        }

        void init()
        {
            cpuTemperatureCelcius = 0;
            fanSpeed = 0;
        }

        bool operator==(const HcuTemperatureParams &arg) const
        {
            return ( (cpuTemperatureCelcius == arg.cpuTemperatureCelcius) &&
                     (fanSpeed == arg.fanSpeed) );
        }

        bool operator!=(const HcuTemperatureParams &arg) const
        {
            return !operator==(arg);
        }
    };
};


Q_DECLARE_METATYPE(DS_SystemDef::StatePath);
Q_DECLARE_METATYPE(DS_SystemDef::NetworkSettingParams);
Q_DECLARE_METATYPE(DS_SystemDef::IwconfigParams);
Q_DECLARE_METATYPE(DS_SystemDef::HcuTemperatureParams);
#endif // DS_SYSTEM_DEF_H
