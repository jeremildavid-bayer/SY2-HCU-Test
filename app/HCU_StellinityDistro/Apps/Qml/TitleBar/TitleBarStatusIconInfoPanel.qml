import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../Widgets"
import "../Util.js" as Util

Rectangle {
    property int animationMs: 350
    property int openDurationMs: 2000

    id: root
    color: colorMap.titleBarStatusIconInfoPanelBackground
    visible: false
    clip: true
    width: parent.width * 0.27
    height: Math.max(titleBarHeight * 1.1, textInfo.contentHeight * 1.1)

    layer.enabled: true
    layer.effect: Glow {
        radius: 17
        samples: 17
        spread: 0.5
        color: colorMap.buttonShadow
        transparentBorder: true
    }

    Text {
        id: textInfo
        anchors.leftMargin: parent.width * 0.05
        anchors.rightMargin: parent.width * 0.05
        verticalAlignment: Text.AlignVCenter
        anchors.fill: parent
        font.family: fontRobotoLight.name
        color: colorMap.text01
        font.pixelSize: titleBarHeight * 0.28
        wrapMode: Text.Wrap
    }

    SequentialAnimation {
        id: animationOpen
        ScriptAction { script: {
                root.opacity = 0;
                root.visible = true;
            }
        }

        // Open animation
        NumberAnimation { target: root; properties: "opacity"; to: 1; duration: animationMs;  }

        // Stay open
        NumberAnimation { duration: openDurationMs }

        // Open animation
        NumberAnimation { target: root; properties: "opacity"; to: 0; duration: animationMs;  }

        ScriptAction { script: {
                root.visible = false;
            }
        }
    }

    function open(strInfo)
    {
        animationOpen.stop();
        textInfo.text = strInfo;
        animationOpen.start();
    }
}
