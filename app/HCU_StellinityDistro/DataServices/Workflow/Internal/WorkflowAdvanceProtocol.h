#ifndef WORKFLOW_ADVANCE_PROTOCOL_H
#define WORKFLOW_ADVANCE_PROTOCOL_H

#include "Common/Common.h"
#include "Common/ActionBase.h"
#include "DataServices/Workflow/DS_WorkflowDef.h"

class WorkflowAdvanceProtocol : public ActionBase
{
    Q_OBJECT

public:
    explicit WorkflowAdvanceProtocol(QObject *parent = 0, EnvGlobal *env_ = NULL);
    ~WorkflowAdvanceProtocol();

    
private slots:
    void slotAppInitialised();
};

#endif //WORKFLOW_ADVANCE_PROTOCOL_H
