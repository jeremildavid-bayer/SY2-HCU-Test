import "DeviceManagerWidgets"

DeviceManagerBase {
    property bool displaySourcePackageInfo: true
    property string rootBackgroundColor: "transparent"

    anchors.fill: parent
    DeviceManagerContainerIcon {
        anchors.fill: parent
    }
}
