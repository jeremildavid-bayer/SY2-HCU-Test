import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property var dicomParamData: dicomTagsData[index]

    height: topOfItem.height + itemHeader.contentHeight + itemContent.contentHeight + bottomOfItem.height
    width: ListView.view.width

    // only visible for first item
    // this is added so that the list fade is still at the top edge but we still show some margin on the top of the list
    Item {
        id: topOfItem
        visible: (index === 0)
        width: parent.width
        height: visible ? frameMargin * 0.5 : 0
    }

    Text {
        id: itemHeader
        anchors.top: topOfItem.bottom
        width: parent.width
        height: contentHeight
        verticalAlignment: Text.AlignTop
        font.pixelSize: listView.height * 0.045
        font.family: fontRobotoLight.name
        wrapMode: Text.Wrap
        color: useDrawerColors ? colorMap.text01 : colorMap.text02
        text: {
            if (dicomParamData.TranslateName)
            {
                return translate("T_DicomTag_" + dicomParamData.Name);
            }
            return dicomParamData.Name
        }
    }

    Text {
        id: itemContent
        anchors.top: itemHeader.bottom
        width: parent.width
        verticalAlignment: Text.AlignTop
        font.pixelSize: listView.height * 0.047
        font.family: fontRobotoBold.name
        color: useDrawerColors ? colorMap.text01 : colorMap.text01
        wrapMode: Text.Wrap
        text: dicomHelper.getDicomFieldValue(dicomParamData)
    }

    // not visible for last item
    Item {
        id: bottomOfItem
        visible: (index < (dicomTagsData.length - 1))
        anchors.top: itemContent.bottom
        width: parent.width
        height: visible ? listView.height * 0.05 : 0
    }
}
