#ifndef DS_TEST_DEF_H
#define DS_TEST_DEF_H

#include <QString>
#include <QDateTime>
#include <QVariantList>
#include "Common/HwCapabilities.h"
#include "Common/Common.h"
#include "Common/Util.h"

class DS_TestDef
{
public:
    enum TestType
    {
        TEST_TYPE_UNKNOWN = 0,
        TEST_TYPE_PISTON,
        TEST_TYPE_STOPCOCK,
        TEST_TYPE_NETWORK,
        TEST_TYPE_CONTINUOUS_EXAMS
    };

    struct TestStatus
    {
        QString guid;
        TestType type;
        QString stateStr;
        int progress;
        bool isFinished;
        bool userAborted;
        QVariantMap statusMap;

        TestStatus()
        {
            guid = EMPTY_GUID;
            type = TEST_TYPE_UNKNOWN;
            stateStr = "";
            progress = 0;
            isFinished = false;
            userAborted = false;
            statusMap = QVariantMap();
        }

        bool operator==(const TestStatus &arg) const
        {
            bool equal = ( (guid == arg.guid) &&
                           (type == arg.type) &&
                           (stateStr == arg.stateStr) &&
                           (progress == arg.progress) &&
                           (isFinished == arg.isFinished) &&
                           (userAborted == arg.userAborted) &&
                           (statusMap == arg.statusMap) );
            return equal;
        }

        bool operator!=(const TestStatus &arg) const
        {
            return !operator==(arg);
        }
    };
};


Q_DECLARE_METATYPE(DS_TestDef::TestStatus);
#endif // DS_TEST_DEF_H
