import QtQuick 2.12
import "../../Widgets"

Popup {
    property alias textWidget: textMessage
    property string contentText: ""
    property string dataText: ""

    heightMin: getPopupHeight()

    content: [
        Item {
            anchors.fill: parent
            Flickable {
                id: flickContent
                flickableDirection: Flickable.VerticalFlick
                anchors.fill: parent
                contentHeight: textMessage.contentHeight
                contentWidth: parent.width
                anchors.centerIn: parent
                enabled:
                {
                    if ( heightMin < (dsCfgLocal.screenH * 0.8) )
                    {
                        return false;
                    }
                    else if ( textMessage.contentHeight > (heightMin - contentSurroundingHeight - 10) )
                    {
                        return true;
                    }
                    else return false;
                }

                Text {
                    id: textMessage
                    anchors.top: parent.top
                    anchors.topMargin: (flickContent.height - height) * 0.5
                    anchors.left: parent.left
                    height: flickContent.enabled ? (heightMin - contentSurroundingHeight) : contentHeight
                    width: parent.width
                    text: translationRequired ? translate(contentText) : contentText
                    color: colorMap.blk01
                    font.family: fontRobotoLight.name
                    font.pixelSize: contentfontPixelSize
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }
            }
            ScrollBar {
                flickable: flickContent
                autoHide: true
            }
            ListFade {
                flickable: flickContent
                fadeColor: color
            }
        }
    ]

    onContentTextChanged: {
        if (isOpen())
        {
            logDebug("PopupMessage: [" + titleText + "]: [" + contentText + "]: Content Changed");

            // Excluding PopupAlertBase content update by detecting userDirectionText
            // PopupAlertBase will trigger onContentTextChanged infinitely
            if (typeof userDirectionText === "undefined")
            {
                    logPopupActivities("contentChanged", "Content Updated");
            }
        }
    }

    onOpened: {
        logDebug("PopupMessage: [" + titleText + "]: [" + contentText + "]: Open");
    }

    onClosed: {
        logDebug("PopupMessage: [" + titleText + "]: [" + contentText + "]: Closed");
    }

    function getPopupHeight()
    {
        var newHeight = dsCfgLocal.screenH * 0.44;
        if (textMessage.contentHeight > (newHeight - contentSurroundingHeight))
        {
            newHeight = textMessage.contentHeight + contentSurroundingHeight + 10;
        }
        newHeight = Math.min(newHeight, dsCfgLocal.screenH * 0.8);

        return newHeight;
    }
}
