HEADERS += \
    $$PWD/AppManager.h \


SOURCES += \
    $$PWD/AppManager.cpp \

RESOURCES +=

OTHER_FILES += \
    $$PWD/Qml/AppManager.qml \
    $$PWD/Qml/Util.js \
    $$PWD/Qml/DS_Exam.qml \
    $$PWD/Qml/DS_Device.qml \
    $$PWD/Qml/DS_Mcu.qml \
    $$PWD/Qml/DS_McuSim.qml \
    $$PWD/Qml/DS_Cru.qml \
    $$PWD/Qml/DS_Mwl.qml \
    $$PWD/Qml/DS_System.qml \
    $$PWD/Qml/DS_Test.qml \
    $$PWD/Qml/DS_Alert.qml \
    $$PWD/Qml/DS_ImrServer.qml \
    $$PWD/Qml/DS_Upgrade.qml \
    $$PWD/Qml/DS_HardwareInfo.qml \
    $$PWD/Qml/DS_CfgLocal.qml \
    $$PWD/Qml/DS_CfgGlobal.qml \
    $$PWD/Qml/DS_Capabilities.qml \
    $$PWD/Qml/DS_Workflow.qml \
    $$PWD/Qml/DS_DevTools.qml \
    $$PWD/Qml/Themes/Themes.qml \
    $$PWD/Qml/Themes/ThemesImage.qml \
    $$PWD/Qml/Themes/ThemesColor.qml \
    $$PWD/Qml/PopupManager/PopupManager.qml \
    $$PWD/Qml/PopupManager/PopupAutoPrimeFailed.qml \
    $$PWD/Qml/PopupManager/PopupUnexpectExamEnded.qml \
    $$PWD/Qml/PopupManager/PopupSudsReinsertedPrimeRequired.qml \
    $$PWD/Qml/PopupManager/PopupPrimeNeededAfterSodFill.qml \
    $$PWD/Qml/PopupManager/PopupBatteryCriticalShutdown.qml \
    $$PWD/Qml/PopupManager/PopupSODError.qml \
    $$PWD/Qml/PopupManager/PopupSudsAirRecovery.qml \
    $$PWD/Qml/PopupManager/PopupSudsAirRecoveryAutoPrime.qml \
    $$PWD/Qml/PopupManager/PopupSyringeAirCheckFailed.qml \
    $$PWD/Qml/PopupManager/PopupSyringeAirRecovery.qml \
    $$PWD/Qml/PopupManager/PopupEndOfDayPurge.qml \
    $$PWD/Qml/PopupManager/PopupInjectionRequestFailed.qml \
    $$PWD/Qml/PopupManager/PopupArmFluidVolumeAdjust.qml \
    $$PWD/Qml/PopupManager/PopupArmCatheterLimitCheck.qml \
    $$PWD/Qml/PopupManager/PopupFluidRemoval.qml \
    $$PWD/Qml/PopupManager/PopupMudsEject.qml \
    $$PWD/Qml/PopupManager/PopupManualPrimeFailed.qml \
    $$PWD/Qml/PopupManager/PopupDoorOpenFailed.qml \
    $$PWD/Qml/PopupManager/PopupBottleFillAbortWaiting.qml \
    $$PWD/Qml/PopupManager/PopupShutdownRestartIgnored.qml \
    $$PWD/Qml/PopupManager/PopupStopcockUnintendedMotionDetected.qml \
    $$PWD/Qml/PopupManager/PopupManualQualifiedDischarge.qml \
    $$PWD/Qml/PopupManager/PopupShippingMode.qml \
    $$PWD/Qml/PopupManager/PopupBatteryTestMode.qml \
    $$PWD/Qml/PopupManager/PopupGenericAlertManager.qml \
    $$PWD/Qml/PopupManager/PopupGenericAlert.qml \
    $$PWD/Qml/PopupManager/PopupAlertBase.qml \
    $$PWD/Qml/PopupManager/PopupHCUSoftwareErrorAlert.qml \
    $$PWD/Qml/PopupManager/PopupHCUSoftwareErrorAlertManager.qml \
    $$PWD/Qml/PopupManager/PopupSafeRenewMuds.qml \
    $$PWD/Qml/PopupManager/PopupPreloadProtocol.qml \
    $$PWD/Qml/PopupManager/PopupRepreloadReprimeRequired.qml \
    $$PWD/Qml/PopupManager/PopupSharingInformation.qml \
    $$PWD/Qml/PopupManager/PopupArmInsufficientVolumeForSteps.qml \
    $$PWD/Qml/Widgets/SoundPlayer.qml \
    $$PWD/Qml/Widgets/InputPad/InputPad.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadGeneric.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadInteger.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadFloat.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadDuration.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadDualRatio.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadText.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadSelectList.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadMultiSelectList.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadDateTime.qml \
    $$PWD/Qml/Widgets/InputPad/InputPadOneToNineButtons.qml \
    $$PWD/Qml/Widgets/ConfigTable/ConfigTable.qml \
    $$PWD/Qml/Widgets/ConfigTable/ConfigTableRow.qml \
    $$PWD/Qml/Widgets/Popup/Popup.qml \
    $$PWD/Qml/Widgets/Popup/PopupIcon.qml \
    $$PWD/Qml/Widgets/Popup/PopupMessage.qml \
    $$PWD/Qml/Widgets/Popup/PopupSlide.qml \
    $$PWD/Qml/Widgets/SlideToUnlock.qml \
    $$PWD/Qml/Widgets/ScrollBar.qml \
    $$PWD/Qml/Widgets/ListFade.qml \
    $$PWD/Qml/Widgets/BlurBackground.qml \
    $$PWD/Qml/Widgets/GenericRoundedRectangle.qml \
    $$PWD/Qml/Widgets/GenericToggleButton.qml \
    $$PWD/Qml/Widgets/GenericComboBoxItem.qml \
    $$PWD/Qml/Widgets/GenericIconButton.qml \
    $$PWD/Qml/Widgets/GenericButton.qml \
    $$PWD/Qml/Widgets/GenericComboBox.qml \
    $$PWD/Qml/Widgets/GenericSlider.qml \
    $$PWD/Qml/Widgets/LabelAndText.qml \
    $$PWD/Qml/Widgets/FileExplorer.qml \
    $$PWD/Qml/Widgets/Drawer.qml \
    $$PWD/Qml/Widgets/ProgressArc.qml \
    $$PWD/Qml/Widgets/DragRectangle.qml \
    $$PWD/Qml/Widgets/LoadingGif.qml \
    $$PWD/Qml/Widgets/MandatoryFieldMark.qml \
    $$PWD/Qml/Widgets/WarningIcon.qml \
    $$PWD/Qml/Widgets/UnprimedSudsWarning.qml \
    $$PWD/Qml/Startup/Startup.qml \
    $$PWD/Qml/TitleBar/TitleBar.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconInfoPanel.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIcons.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconTradeshowMode.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconPowerStatus.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconDateTime.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconMcuLink.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconCruLink.qml \
    $$PWD/Qml/TitleBar/TitleBarStatusIconIsiStatus.qml \
    $$PWD/Qml/TitleBar/TitleBarSystemAlertsPanel.qml \
    $$PWD/Qml/TitleBar/TitleBarSystemAlertItem.qml \
    $$PWD/Qml/InjectionElapsedTime/InjectionElapsedTime.qml \
    $$PWD/Qml/Admin/Admin.qml \
    $$PWD/Qml/Admin/AdminActionBar.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrasts.js \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrasts.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsActionBar.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsList.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsListFamily.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsListGroup.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsMain.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsMainHeader.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsMainPackageList.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsMainPackageListItem.qml \
    $$PWD/Qml/Admin/AdminContrasts/AdminContrastsMainPackageListItemBarcodeItem.qml \
    $$PWD/Qml/Admin/AdminSettings/AdminSettings.qml \
    $$PWD/Qml/Admin/AdminService/AdminService.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceSettings/AdminServiceSettings.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceHardwareInfo/AdminServiceHardwareInfo.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceHardwareInfo/AdminServiceHardwareInfoBoardList.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceHardwareInfo/AdminServiceHardwareInfoBoardListItem.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceCapabilities/AdminServiceCapabilities.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceUpgrade/AdminServiceUpgrade.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceTool.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolActionBar.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolActionBarSensor.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolPistonsStopcocks.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolLedControl.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolSpecialActions.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolCalibration.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolTestTableRow.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolTestTemplate.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolNetworkTest.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolBarcodeReaderTest.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolPistonCycleTest.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolStopcockCycleTest.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolTouchScreenTest.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolPressureCalibration.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolPressureCalibrationPopup.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolAudioTest.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolBMS.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolBMSTableCell.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolBMSConsole.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTool/AdminServiceToolBMSConsoleTableCell.qml \
    $$PWD/Qml/Admin/AdminService/AdminServiceTradeshow/AdminServiceTradeshow.qml \
    $$PWD/Qml/Admin/AdminService/AdminDevTools/AdminDevTools.qml \
    $$PWD/Qml/Admin/AdminAbout/AdminAbout.qml \
    $$PWD/Qml/ExamManager/ExamManager.qml \
    $$PWD/Qml/ExamManager/ExamReminders/ExamReminders.qml \
    $$PWD/Qml/ExamManager/ExamReminders/ExamReminderItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelect.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectSearch.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectListHeader.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectListHeaderSortButton.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectList.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectListItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectListItemField.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectedItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectDicomTagList.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectDicomTagListItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamList.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamLinkOtherAccessionList.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamLinkOtherAccessionListItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItemValue.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItemCatheterType.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItemEgfr/EgfrDrawer.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItemEgfr/EgfrList.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItemEgfr/EgfrListItem.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/ExamPatientSelectEditAdvanceParamListItemEgfr/EgfrValue.qml \
    $$PWD/Qml/ExamManager/ExamPatientSelect/DicomHelper.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelect.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectList/ExamProtocolSelectList.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectList/ExamProtocolSelectListGroup.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectList/ExamProtocolSelectListItem.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectPreview/ExamProtocolSelectPreview.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectPreview/ExamProtocolSelectPreviewParamList.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectPreview/ExamProtocolSelectPreviewParamItem.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectPreview/ExamProtocolSelectPreviewStep.qml \
    $$PWD/Qml/ExamManager/ExamProtocolSelect/ExamProtocolSelectPreview/ExamProtocolSelectPreviewPhase.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEdit.js \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEdit.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditPlan.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditWarnings.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditParamList.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditParamItem.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditStepList.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditStep.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditPhase.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditOption.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditOptionList.qml \
    $$PWD/Qml/ExamManager/ExamProtocolEdit/ExamProtocolEditReview.qml \
    $$PWD/Qml/ExamManager/ExamInjectionMonitor/ExamInjectionMonitor.qml \
    $$PWD/Qml/ExamManager/ExamInjectionMonitor/ExamInjectionMonitorOverview.qml \
    $$PWD/Qml/ExamManager/ExamInjectionMonitor/ExamInjectionMonitorStep.qml \
    $$PWD/Qml/ExamManager/ExamInjectionMonitor/ExamInjectionMonitorPhase.qml \
    $$PWD/Qml/ExamManager/ExamInjectionPlot/ExamInjectionPlot.qml \
    $$PWD/Qml/ExamManager/ExamInjectionPlot/ExamInjectionPlotTransition.qml \
    $$PWD/Qml/ExamManager/ExamInjectionPlot/ExamInjectionPlotReminder.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummary.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryWarnings.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryInjectedList.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryInjectedListItem.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryAdvanceList.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryAdvanceListItem.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryAdvanceListItemNote.qml \
    $$PWD/Qml/ExamManager/ExamSummary/ExamSummaryAdvanceListItemTechId.qml \
    $$PWD/Qml/ExamManager/ExamNavigationBar/ExamNavigationBar.qml \
    $$PWD/Qml/ExamManager/ExamNavigationBar/ExamNavigationBarGroup.qml \
    $$PWD/Qml/ExamManager/ExamNavigationBar/ExamNavigationBarItem.qml \
    $$PWD/Qml/ExamManager/ExamInjectionControlBar/ExamInjectionControlBar.qml \
    $$PWD/Qml/ExamManager/ExamProtocolOverview/ExamProtocolOverview.qml \
    $$PWD/Qml/ExamManager/ExamProtocolOverview/ExamProtocolOverviewInner.qml \
    $$PWD/Qml/Home/Home.qml \
    $$PWD/Qml/Home/HomeActionBar.qml \
    $$PWD/Qml/Home/HomeActionBtnScreenLock.qml \
    $$PWD/Qml/Shutdown/Shutdown.qml \
    $$PWD/Qml/Shutdown/ShutdownActionBar.qml \
    $$PWD/Qml/McuSimulator/McuSimulator.qml \
    $$PWD/Qml/McuSimulator/McuSimulatorSoundPlunger.qml \
    $$PWD/Qml/McuSimulator/McuSimulatorSoundStopcock.qml \
    $$PWD/Qml/DeviceManager/DeviceManager.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerBase.qml \
    $$PWD/Qml/ContinuousExamsTest/ContinuousExamsTest.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanel.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelContainer.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelMuds/DeviceManagerPanelMuds.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelMuds/DeviceManagerPanelMudsMovePistons.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelSuds/DeviceManagerPanelSuds.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottle.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleBarcodeScan.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleGroupList.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleGroupListItem.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleVolumeList.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleParamList.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelBottle/DeviceManagerPanelBottleParamListItem.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerPanel/DeviceManagerPanelWC/DeviceManagerPanelWC.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerContainerIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerContainerMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerMudsIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerMudsMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerWCIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerWCMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerSudsIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerSudsMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerMudsLineIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerMudsLineMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerGenericButton.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerSyringeIcon.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerSyringeMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerBottleMain.qml \
    $$PWD/Qml/DeviceManager/DeviceManagerWidgets/DeviceManagerBottleIcon.qml \
    $$PWD/Qml/DebugTools/Debug_HCUMonitor.qml \
    $$PWD/Qml/CommonElements/AutoEmptyModeEnabledMsg.qml \
    $$PWD/Qml/CommonElements/AutoEmptyEnabledIcon.qml \
    $$PWD/Qml/SystemManager/AdminMode/Settings/SettingsMode.qml \
    $$PWD/Qml/Widgets/Keyboard/Keyboard.qml
