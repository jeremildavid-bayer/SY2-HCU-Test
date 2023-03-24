import QtQuick 2.12

Item {
    property alias themePurity: setPurity
    property alias themeTwilight: setTwilight

    QtObject {
        id: setPurity

        property string mainBackground: "#f8f8f8"
        property string titleBarBackground: "#e6e7e8"
        property string actionBarBackground: "#e6e7e8"
        property string subPanelBackground: "#aaafb4"
        property string comboBoxBackground: "#d1d3d4"
        property string comboBoxActive: "#e4e5e6"
        property string drawerHandleBackground: "#d1d3d4"
        property string elapsedTimeBackground: "#aaafb4"
        property string remindersBackground: "#e0d1d3d4"
        property string systemAlertsPanelBackground: "#d1d3d4"
        property string titleBarStatusIconInfoPanelBackground: "#d1d3d4"

        property string scrollBar: "#a6a7a8"
        property string actionButtonBackground: "#f0d415"
        property string actionButtonText: "#1e272a"
        property string text01: "#1e272a"
        property string text02: "#777d7f"
        property string grid: "#b2b6b7"
        property string warnText: "#ff7f00"
        property string errText: "#ff0000"

        property string consoleBackground: "#ffffff"
        property string keypadButton: "#d1d3d4"
        property string keypadCancelButton: "#f1f2f2"

        property string popupBackground: "#f8f9f9"

        property string startupBackground: "#e6e7e8"
        property string startupWarningPanel: "#d1d3d4"

        property string injectPhaseProgressBackground: "#66ffffff"

        property string injectPlotGrid: "#d2d2d2"
        property string injectPlotMarkerBackground: "#1e272a"
        property string injectPlotMarkerIcon: "#f8f8f8"

        property string injectControlBarBackground: "#f1f2f2"

        property string statusIconText1: "#1e272a" // dark grey
        property string statusIconText2: "#a0a4a6" // light grey
        property string statusIconRed: "#ef4237"
        property string statusIconBatteryOrange: "#f36f2a"
        property string statusIconBatteryRed: "#ed1c24"
        property string statusIconBatteryGray: "#828789"
        property string statusIconBatteryAQD: "#6bc200" // green color

        property string navBtnSelectedBorder: "#1e272a"
        property string navBtnProgressLine: "#8f9395"

        property string editFieldBackground: "#30777d7f"

        property string homeBackground: "#e6e7e8"
        property string homeMenuBackground: "#f8f8f8"

        property string deviceButtonBackground: "#d1d3d4"
        property string deviceIconMissing: "#90b2b6b7"
        property string deviceIconSelected: "#1e272a"
        property string deviceIconWasteLevel: "#8aa4b1"
        property string deviceIconWarningState: "#f36f2a"
        property string deviceIconSudsError: red
        property string devicePlunger: "#ffffff"
        property string deviceToggleGrabber: "#ffffff"
        property string deviceButtonRed: "#70ed1c24"

        property string saline: "#0090c5"
        property string contrast1: "#6bc200"
        property string contrast2: "#9f7ad1"
        property string paused: "#90969f"
        property string white01: "#FEFEFE"
        property string blk01: "#1E272A"
        property string gry01: "#90969F"
        property string blu01: "#8AA4B1"
        property string yellow01: "#FFFF00"
        property string yellow02: "#FFF3B2"
        property string red: "#ed1c24"
        property string orange: "#f36f2a"
        property string buttonShadow: "#c0e6e7e8"
        property string buttonDisabled: "#e0e6e7e8"
    }

    QtObject {
        id: setTwilight

        property string mainBackground: "#242f32"
        property string titleBarBackground: "#1e272a"
        property string actionBarBackground: "#1e272a"
        property string subPanelBackground: "#5a6369"
        property string comboBoxBackground: "#424c4f"
        property string comboBoxActive: "#333e41"
        property string drawerHandleBackground: "#424c4f"
        property string elapsedTimeBackground: "#5a6369"
        property string remindersBackground: "#e0424c4f"
        property string systemAlertsPanelBackground: "#424c4f"
        property string titleBarStatusIconInfoPanelBackground: "#424c4f"

        property string scrollBar: "#626c6f"
        property string actionButtonBackground: "#f0d415"
        property string actionButtonText: "#1e272a"
        property string text01: "#ffffff"
        property string text02: "#90969f"
        property string grid: "#90969f"
        property string warnText: "#ff7f00"
        property string errText: "#ff0000"

        property string consoleBackground: "#000000"
        property string keypadButton: "#424c4f"
        property string keypadCancelButton: "#b2b6b7"

        property string popupBackground: "#c7caca"

        property string startupBackground: "#1e272a"
        property string startupWarningPanel: "#525c5f"

        property string injectPhaseProgressBackground: "#66000000"

        property string injectPlotGrid: "#3a4447"
        property string injectPlotMarkerBackground: "#ffffff"
        property string injectPlotMarkerIcon: "#1e272a"

        property string injectControlBarBackground: "#161c21"

        property string statusIconText1: "#ffffff" // white
        property string statusIconText2: "#6d7274" // light grey
        property string statusIconRed: "#ef4237"
        property string statusIconBatteryOrange: "#f36f2a"
        property string statusIconBatteryRed: "#ed1c24"
        property string statusIconBatteryGray: "#787d7f"
        property string statusIconBatteryAQD: "#6bc200" // green color

        property string navBtnSelectedBorder: "#ffffff"
        property string navBtnProgressLine: "#a7a9ac"

        property string editFieldBackground: "#3090969f"

        property string homeBackground: "#1e272a"
        property string homeMenuBackground: "#242f32"

        property string deviceButtonBackground: "#424c4f"
        property string deviceIconMissing: "#9090969f"
        property string deviceIconSelected: "#ffffff"
        property string deviceIconWasteLevel: "#8aa4b1"
        property string deviceIconWarningState: "#f36f2a"
        property string deviceIconSudsError: red
        property string devicePlunger: "#1e272a"
        property string deviceToggleGrabber: "#8ba4b2"
        property string deviceButtonRed: "#70ed1c24"

        property string saline: "#0090c5"
        property string contrast1: "#6bc200"
        property string contrast2: "#9f7ad1"
        property string paused: "#90969f"
        property string white01: "#FEFEFE"
        property string blk01: "#1E272A"
        property string gry01: "#90969f"
        property string blu01: "#8AA4B1"
        property string yellow01: "#FFFF00"
        property string yellow02: "#FFF3B2"
        property string red: "#ed1c24"
        property string orange: "#f36f2a"
        property string buttonShadow: "#c01e272a"
        property string buttonDisabled: "#e01e272a"
    }
}
