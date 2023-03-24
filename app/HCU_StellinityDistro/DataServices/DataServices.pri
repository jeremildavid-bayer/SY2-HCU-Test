        INCLUDEPATH += $$PWD/Mcu/Internal/Simulator

    HEADERS += \
        $$PWD/DataServicesMacros.h \
        \
        $$PWD/Mcu/DS_McuDef.h \
        $$PWD/Mcu/DS_McuData.h \
        $$PWD/Mcu/DS_McuAction.h \
        $$PWD/Mcu/QML_Mcu.h \
        $$PWD/Mcu/QML_McuSim.h \
        $$PWD/Mcu/Internal/McuMonitor.h \
        $$PWD/Mcu/Internal/McuAlarm.h \
        $$PWD/Mcu/Internal/McuAlarmMonitor.h \
        $$PWD/Mcu/Internal/McuMsgHandler.h \
        $$PWD/Mcu/Internal/McuLink.h \
        $$PWD/Mcu/Internal/McuPressureCalibration.h \
        $$PWD/Mcu/Internal/McuHw/McuHw.h \
        $$PWD/Mcu/Internal/McuSim/McuSim.h \
        $$PWD/Mcu/Internal/McuSim/McuSimStopcock.h \
        $$PWD/Mcu/Internal/McuSim/McuSimSyringeGroup.h \
        $$PWD/Mcu/Internal/McuSim/McuSimSyringe.h \
        $$PWD/Mcu/Internal/McuSim/McuSimStateMachine.h \
        \
        $$PWD/Cru/DS_CruDef.h \
        $$PWD/Cru/DS_CruData.h \
        $$PWD/Cru/DS_CruAction.h \
        $$PWD/Cru/QML_Cru.h \
        $$PWD/Cru/Internal/CruMsgHandler.h \
        $$PWD/Cru/Internal/CruLink.h \
        $$PWD/Cru/Internal/CruLinkMsgComm.h \
        \
        $$PWD/Mwl/DS_MwlDef.h \
        $$PWD/Mwl/DS_MwlData.h \
        $$PWD/Mwl/DS_MwlAction.h \
        $$PWD/Mwl/QML_Mwl.h \
        $$PWD/Mwl/Internal/MwlMonitor.h \
        \
        $$PWD/ImrServer/DS_ImrServerDef.h \
        $$PWD/ImrServer/DS_ImrServerData.h \
        $$PWD/ImrServer/DS_ImrServerAction.h \
        $$PWD/ImrServer/QML_ImrServer.h \
        $$PWD/ImrServer/Internal/ImrServerWrapper.h \
        $$PWD/ImrServer/Internal/ImrServerBase.h \
        $$PWD/ImrServer/Internal/ImrServerWebSocket.h \
        $$PWD/ImrServer/Internal/ImrServer0/ImrServer0.h \
        $$PWD/ImrServer/Internal/ImrServer0/TestMsg.h \
        $$PWD/ImrServer/Internal/ImrServer/ImrServer.h \
        $$PWD/ImrServer/Internal/ImrServer/Digest.h \
        $$PWD/ImrServer/Internal/ImrServer/DigestProvider.h \
        \
        $$PWD/Workflow/DS_WorkflowDef.h \
        $$PWD/Workflow/DS_WorkflowData.h \
        $$PWD/Workflow/DS_WorkflowAction.h \
        $$PWD/Workflow/QML_Workflow.h \
        $$PWD/Workflow/Internal/WorkflowMain.h \
        $$PWD/Workflow/Internal/WorkflowSod.h \
        $$PWD/Workflow/Internal/WorkflowMudsEject.h \
        $$PWD/Workflow/Internal/WorkflowSudsAirRecovery.h \
        $$PWD/Workflow/Internal/WorkflowSyringeAirRecovery.h \
        $$PWD/Workflow/Internal/WorkflowAdvanceProtocol.h \
        $$PWD/Workflow/Internal/WorkflowFluidRemoval.h \
        $$PWD/Workflow/Internal/WorkflowEndOfDayPurge.h \
        $$PWD/Workflow/Internal/WorkflowAutoEmpty.h \
        $$PWD/Workflow/Internal/WorkflowManualQualifiedDischarge.h \
        $$PWD/Workflow/Internal/WorkflowAutomaticQualifiedDischarge.h \
        $$PWD/Workflow/Internal/WorkflowShippingMode.h \
        $$PWD/Workflow/Internal/WorkflowPreloadProtocol.h \
        $$PWD/Workflow/Internal/WorkflowBattery.h \
        \
        $$PWD/Device/DS_DeviceDef.h \
        $$PWD/Device/DS_DeviceData.h \
        $$PWD/Device/DS_DeviceAction.h \
        $$PWD/Device/QML_Device.h \
        $$PWD/Device/Internal/DeviceMonitor.h \
        $$PWD/Device/Internal/DeviceBarcodeReader.h \
        $$PWD/Device/Internal/DeviceBottle.h \
        $$PWD/Device/Internal/DeviceLeds.h \
        $$PWD/Device/Internal/DeviceMuds.h \
        $$PWD/Device/Internal/DeviceMudsSod.h \
        $$PWD/Device/Internal/DeviceMudsDisengage.h \
        $$PWD/Device/Internal/DeviceMudsPreload.h \
        $$PWD/Device/Internal/DeviceOutletAirDoor.h \
        $$PWD/Device/Internal/DeviceStopcock.h \
        $$PWD/Device/Internal/DeviceSuds.h \
        $$PWD/Device/Internal/DeviceSyringe.h \
        $$PWD/Device/Internal/DeviceSyringeFill.h \
        $$PWD/Device/Internal/DeviceSyringePrime.h \
        $$PWD/Device/Internal/DeviceSyringeSod.h \
        $$PWD/Device/Internal/DeviceSyringeAirCheck.h \
        $$PWD/Device/Internal/DeviceWasteContainer.h \
        $$PWD/Device/Internal/DeviceHeatMaintainer.h \
        $$PWD/Device/Internal/DeviceDoor.h \
        $$PWD/Device/Internal/DeviceBattery.h \
        \
        $$PWD/Exam/DS_ExamDef.h \
        $$PWD/Exam/DS_ExamData.h \
        $$PWD/Exam/DS_ExamAction.h \
        $$PWD/Exam/QML_Exam.h \
        $$PWD/Exam/Internal/ExamMonitor.h \
        $$PWD/Exam/Internal/ExamInjection.h \
        \
        $$PWD/System/DS_SystemAction.h \
        $$PWD/System/DS_SystemData.h \
        $$PWD/System/DS_SystemDef.h \
        $$PWD/System/QML_System.h \
        $$PWD/System/Internal/SystemMonitor.h \
        $$PWD/System/Internal/SystemMonitorOS.h \
        \
        $$PWD/Alert/DS_AlertDef.h \
        $$PWD/Alert/DS_AlertAction.h \
        $$PWD/Alert/DS_AlertData.h \
        $$PWD/Alert/QML_Alert.h \
        $$PWD/Alert/Internal/AlertMonitor.h\
        \
        $$PWD/Upgrade/DS_UpgradeDef.h \
        $$PWD/Upgrade/DS_UpgradeAction.h \
        $$PWD/Upgrade/DS_UpgradeData.h \
        $$PWD/Upgrade/QML_Upgrade.h \
        $$PWD/Upgrade/Internal/UpgradeSru.h \
        $$PWD/Upgrade/Internal/UpgradeHcu.h \
        $$PWD/Upgrade/Internal/UpgradeHwBase.h \
        $$PWD/Upgrade/Internal/UpgradeHwMcu.h \
        $$PWD/Upgrade/Internal/UpgradeHwStopcock.h \
        \
        $$PWD/Test/DS_TestDef.h \
        $$PWD/Test/DS_TestAction.h \
        $$PWD/Test/DS_TestData.h \
        $$PWD/Test/QML_Test.h \
        $$PWD/Test/Internal/TestHwPiston.h \
        $$PWD/Test/Internal/TestHwStopcock.h \
        $$PWD/Test/Internal/TestNetwork.h \
        $$PWD/Test/Internal/TestContinuousExams.h \
        \
        $$PWD/HardwareInfo/DS_HardwareInfo.h \
        $$PWD/HardwareInfo/DS_HardwareInfoDef.h \
        $$PWD/HardwareInfo/QML_HardwareInfo.h \
        \
        $$PWD/CfgLocal/DS_CfgLocal.h \
        $$PWD/CfgLocal/DS_CfgLocalDef.h \
        $$PWD/CfgLocal/QML_CfgLocal.h \
        \
        $$PWD/CfgGlobal/DS_CfgGlobal.h \
        $$PWD/CfgGlobal/DS_CfgGlobalDef.h \
        $$PWD/CfgGlobal/QML_CfgGlobal.h \
        \
        $$PWD/Capabilities/DS_Capabilities.h \
        $$PWD/Capabilities/DS_CapabilitiesDef.h \
        $$PWD/Capabilities/QML_Capabilities.h \
        \
        $$PWD/DevTools/QML_DevTools.h



    SOURCES += \
        $$PWD/Mcu/DS_McuAction.cpp \
        $$PWD/Mcu/DS_McuData.cpp \
        $$PWD/Mcu/QML_Mcu.cpp \
        $$PWD/Mcu/QML_McuSim.cpp \
        $$PWD/Mcu/Internal/McuMonitor.cpp \
        $$PWD/Mcu/Internal/McuAlarmMonitor.cpp \
        $$PWD/Mcu/Internal/McuMsgHandler.cpp \
        $$PWD/Mcu/Internal/McuLink.cpp \
        $$PWD/Mcu/Internal/McuPressureCalibration.cpp \
        $$PWD/Mcu/Internal/McuHw/McuHw.cpp \
        $$PWD/Mcu/Internal/McuSim/McuSim.cpp \
        $$PWD/Mcu/Internal/McuSim/McuSimStopcock.cpp \
        $$PWD/Mcu/Internal/McuSim/McuSimSyringe.cpp \
        $$PWD/Mcu/Internal/McuSim/McuSimStateMachine.cpp \
        \
        $$PWD/Cru/DS_CruData.cpp \
        $$PWD/Cru/DS_CruAction.cpp \
        $$PWD/Cru/QML_Cru.cpp \
        $$PWD/Cru/Internal/CruMsgHandler.cpp \
        $$PWD/Cru/Internal/CruLink.cpp \
        $$PWD/Cru/Internal/CruLinkMsgComm.cpp \
        \
        $$PWD/Mwl/DS_MwlData.cpp \
        $$PWD/Mwl/DS_MwlAction.cpp \
        $$PWD/Mwl/QML_Mwl.cpp \
        $$PWD/Mwl/Internal/MwlMonitor.cpp \
        \
        $$PWD/ImrServer/DS_ImrServerData.cpp \
        $$PWD/ImrServer/DS_ImrServerAction.cpp \
        $$PWD/ImrServer/QML_ImrServer.cpp \
        $$PWD/ImrServer/Internal/ImrServerWrapper.cpp \
        $$PWD/ImrServer/Internal/ImrServerWebSocket.cpp \
        $$PWD/ImrServer/Internal/ImrServer0/ImrServer0.cpp \
        $$PWD/ImrServer/Internal/ImrServer/ImrServer.cpp \
        $$PWD/ImrServer/Internal/ImrServer/DigestProvider.cpp \
        \
        $$PWD/Workflow/DS_WorkflowData.cpp \
        $$PWD/Workflow/DS_WorkflowAction.cpp \
        $$PWD/Workflow/QML_Workflow.cpp \
        $$PWD/Workflow/Internal/WorkflowMain.cpp \
        $$PWD/Workflow/Internal/WorkflowSod.cpp \
        $$PWD/Workflow/Internal/WorkflowMudsEject.cpp \
        $$PWD/Workflow/Internal/WorkflowSudsAirRecovery.cpp \
        $$PWD/Workflow/Internal/WorkflowSyringeAirRecovery.cpp \
        $$PWD/Workflow/Internal/WorkflowAdvanceProtocol.cpp \
        $$PWD/Workflow/Internal/WorkflowFluidRemoval.cpp \
        $$PWD/Workflow/Internal/WorkflowEndOfDayPurge.cpp \
        $$PWD/Workflow/Internal/WorkflowAutoEmpty.cpp \
        $$PWD/Workflow/Internal/WorkflowManualQualifiedDischarge.cpp \
        $$PWD/Workflow/Internal/WorkflowAutomaticQualifiedDischarge.cpp \
        $$PWD/Workflow/Internal/WorkflowShippingMode.cpp \
        $$PWD/Workflow/Internal/WorkflowPreloadProtocol.cpp \
        $$PWD/Workflow/Internal/WorkflowBattery.cpp \
        \
        $$PWD/Device/DS_DeviceData.cpp \
        $$PWD/Device/DS_DeviceAction.cpp \
        $$PWD/Device/QML_Device.cpp \
        $$PWD/Device/Internal/DeviceMonitor.cpp \
        $$PWD/Device/Internal/DeviceBarcodeReader.cpp \
        $$PWD/Device/Internal/DeviceBottle.cpp \
        $$PWD/Device/Internal/DeviceLeds.cpp \
        $$PWD/Device/Internal/DeviceMuds.cpp \
        $$PWD/Device/Internal/DeviceMudsSod.cpp \
        $$PWD/Device/Internal/DeviceMudsDisengage.cpp \
        $$PWD/Device/Internal/DeviceMudsPreload.cpp \
        $$PWD/Device/Internal/DeviceOutletAirDoor.cpp \
        $$PWD/Device/Internal/DeviceStopcock.cpp \
        $$PWD/Device/Internal/DeviceSuds.cpp \
        $$PWD/Device/Internal/DeviceSyringe.cpp \
        $$PWD/Device/Internal/DeviceSyringeFill.cpp \
        $$PWD/Device/Internal/DeviceSyringePrime.cpp \
        $$PWD/Device/Internal/DeviceSyringeSod.cpp \
        $$PWD/Device/Internal/DeviceSyringeAirCheck.cpp \
        $$PWD/Device/Internal/DeviceWasteContainer.cpp \
        $$PWD/Device/Internal/DeviceHeatMaintainer.cpp \
        $$PWD/Device/Internal/DeviceDoor.cpp \
        $$PWD/Device/Internal/DeviceBattery.cpp \
        \
        $$PWD/Exam/DS_ExamData.cpp \
        $$PWD/Exam/DS_ExamAction.cpp \
        $$PWD/Exam/QML_Exam.cpp \
        $$PWD/Exam/Internal/ExamMonitor.cpp \
        $$PWD/Exam/Internal/ExamInjection.cpp \
        \
        $$PWD/System/DS_SystemAction.cpp \
        $$PWD/System/DS_SystemData.cpp \
        $$PWD/System/QML_System.cpp \
        $$PWD/System/Internal/SystemMonitor.cpp \
        $$PWD/System/Internal/SystemMonitorOS.cpp \
        \
        $$PWD/Alert/DS_AlertAction.cpp \
        $$PWD/Alert/DS_AlertData.cpp \
        $$PWD/Alert/QML_Alert.cpp \
        $$PWD/Alert/Internal/AlertMonitor.cpp\
        \
        $$PWD/Upgrade/DS_UpgradeAction.cpp \
        $$PWD/Upgrade/DS_UpgradeData.cpp \
        $$PWD/Upgrade/QML_Upgrade.cpp \
        $$PWD/Upgrade/Internal/UpgradeSru.cpp \
        $$PWD/Upgrade/Internal/UpgradeHcu.cpp \
        $$PWD/Upgrade/Internal/UpgradeHwBase.cpp \
        $$PWD/Upgrade/Internal/UpgradeHwMcu.cpp \
        $$PWD/Upgrade/Internal/UpgradeHwStopcock.cpp \
        \
        $$PWD/Test/DS_TestAction.cpp \
        $$PWD/Test/DS_TestData.cpp \
        $$PWD/Test/QML_Test.cpp \
        $$PWD/Test/Internal/TestHwPiston.cpp \
        $$PWD/Test/Internal/TestHwStopcock.cpp \
        $$PWD/Test/Internal/TestNetwork.cpp \
        $$PWD/Test/Internal/TestContinuousExams.cpp \
        \
        $$PWD/HardwareInfo/DS_HardwareInfo.cpp \
        $$PWD/HardwareInfo/QML_HardwareInfo.cpp \
        \
        $$PWD/CfgLocal/DS_CfgLocal.cpp \
        $$PWD/CfgLocal/QML_CfgLocal.cpp \
        \
        $$PWD/CfgGlobal/DS_CfgGlobal.cpp \
        $$PWD/CfgGlobal/QML_CfgGlobal.cpp \
        \
        $$PWD/Capabilities/DS_Capabilities.cpp \
        $$PWD/Capabilities/QML_Capabilities.cpp \
        \
        $$PWD/DevTools/QML_DevTools.cpp
