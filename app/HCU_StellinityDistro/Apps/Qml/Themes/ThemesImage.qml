import QtQuick 2.12

Item {
    property string pathImages:                             dsCapabilities.pathImages
    property string pathTheme:                              pathImages + "/Purity/Purity"

    property string primingSGif:                            pathImages + "/PrimingSaline.gif"
    property string primingC1Gif:                           pathImages + "/PrimingC1.gif"
    property string primingC2Gif:                           pathImages + "/PrimingC2.gif"
    property string startUpLogo:                            pathImages + "/StartUpLogo.svg"
    property string bayerCrossRGB:                          pathImages + "/BayerCrossRGB.svg"

    property string deviceReservoirEmptySaline:             pathImages + "/SchematicReservoirEmptySaline.svg"
    property string deviceReservoirEmptyC1:                 pathImages + "/SchematicReservoirEmptyC1.svg"
    property string deviceReservoirEmptyC2:                 pathImages + "/SchematicReservoirEmptyC2.svg"
    property string deviceMudsLineEmpty:                    pathImages + "/SchematicMLEmpty.svg"

    property string busySpinner:                            pathTheme + "BusySpinner.png"
    property string startUpBanner:                          pathTheme + "StartUpBanner.svg"
    property string startUpWarning:                         pathTheme + "StartUpWarning.svg"
    property string homeAdmin:                              pathTheme + "HomeAdmin.svg"
    property string homeExam1:                              pathTheme + "HomeExam1.svg"
    property string homeExam2:                              pathTheme + "HomeExam2.svg"
    property string homeExam3:                              pathTheme + "HomeExam3.svg"
    property string homeExam4:                              pathTheme + "HomeExam4.svg"
    property string adminMenuContrasts:                     pathTheme + "AdminMenuContrasts.svg"
    property string adminMenuSalines:                       pathTheme + "AdminMenuSalines.svg"
    property string adminMenuSettings:                      pathTheme + "AdminMenuSettings.svg"
    property string adminMenuService:                       pathTheme + "AdminMenuService.svg"
    property string adminMenuAbout:                         pathTheme + "AdminMenuAbout.svg"
    property string adminMenuServiceSettings:               pathTheme + "AdminMenuServiceSettings.svg"
    property string adminMenuServiceTool:                   pathTheme + "AdminMenuServiceTool.svg"
    property string adminMenuCapabilities:                  pathTheme + "AdminMenuServiceCapabilities.svg"
    property string adminMenuHardwareInfo:                  pathTheme + "AdminMenuServiceHardwareInfo.svg"
    property string adminMenuUpgrade:                       pathTheme + "AdminMenuUpgrade.svg"
    property string examStagePatientComplete:               pathTheme + "ExamStagePatientComplete.svg"
    property string examStagePatientIncomplete:             pathTheme + "ExamStagePatientIncomplete.svg"
    property string examStagePatientSelectedGif:            pathTheme + "ExamStagePatientSelected.gif"
    property string examStageLibrary:                       pathTheme + "ExamStageLibrary.svg"
    property string examStageInjection:                     pathTheme + "ExamStageInjection.svg"
    property string examStageSummaryProgress:               pathTheme + "ExamStageSummaryProgress.svg"
    property string examStageSummaryAbort:                  pathTheme + "ExamStageSummaryAbort.svg"
    property string examStageSummaryAbortGif:               pathTheme + "ExamStageSummaryAbort.gif"
    property string examStageSummaryComplete:               pathTheme + "ExamStageSummaryComplete.svg"
    property string examStageSummaryCompleteGif:            pathTheme + "ExamStageSummaryComplete.gif"
    property string examStageSummaryCompleteToProgressGif:  pathTheme + "ExamStageSummaryCompleteToProgress.gif"
    property string examStageSummaryProgressToCompleteGif:  pathTheme + "ExamStageSummaryProgressToComplete.gif"
    property string examPersonalizedProtocol:               pathTheme + "PersonalisedProtocol.svg"
    property string examProtocolKVP:                        pathTheme + "ProtocolKVP.svg"
    property string examProtocolGlobe:                      pathTheme + "Globe.svg"
    property string examProtocolPersonalised:               pathTheme + "ProtocolPersonalised.svg"
    property string examProtocolPersonalisedKVP:            pathTheme + "ProtocolPersonalisedKVP.svg"
    property string preloadProtocol:                        pathTheme + "Preloadprotocol.svg"
    property string reprimeProtocol:                        pathTheme + "Reprimeprotocol.svg"
    property string bayerIcon:                              pathTheme + "BayerIcon.svg"
    property string sudsLength:                             pathTheme + "PatientLineLength.svg"


    function setTheme(themeName)
    {
        pathTheme = pathImages + "/" + themeName + "/" + themeName;
    }
}
