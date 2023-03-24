import QtQuick 2.12
import "../../../Widgets"
import "../../../Util.js" as Util

ListView {
    id: listView
    orientation: ListView.Horizontal
    spacing: actionButtonWidth * 0.02
    enabled: editSourcePackageEnabled
    clip: true
    highlightMoveDuration: 0
    highlightRangeMode: ListView.NoHighlightRange

    model: {
        if ( (bottleVolumeSelectList.length === 1) &&
             (bottleVolumeSelectList[0] === 0) )
        {
            // Remove "*ml" if it is only one in the list
            return [];
        }
        return bottleVolumeSelectList;
    }

    ListFade {}

    onVisibleChanged: {
        selectItem(currentIndex);
    }

    delegate: GenericButton {
        height: ListView.view.height
        width: ListView.view.width * 0.3
        color: colorMap.keypadButton

        border.color: (index === volumeSelectedIndex) ? colorMap.text01 : "transparent"
        border.width: buttonSelectedBorderWidth

        content: [
            Item {
                anchors.fill: parent
                anchors.margins: parent.width * 0.1

                Text {
                    property int volume: bottleVolumeSelectList[index]

                    width: parent.width
                    height: parent.height
                    font.family: (index === volumeSelectedIndex) ? fontRobotoBold.name : fontRobotoLight.name
                    text: ((volume == 0) ? "*" : volume) + translate("T_Units_ml")
                    color: (index === volumeSelectedIndex) ? colorMap.text01 : colorMap.text02
                    font.pixelSize: height * 0.4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }}
        ]
        onBtnClicked: {
            panelMain.volumeSelectedIndex = index;
            if (checkBottleChanged())
            {
                loadBottle();
            }
        }

        Component.onCompleted: {
            listView.dragStarted.connect(reset);
        }

        Component.onDestruction: {
            listView.dragStarted.disconnect(reset);
        }
    }

    function selectItem(itemIdx)
    {
        currentIndex = -1;
        currentIndex = itemIdx;
    }
}

