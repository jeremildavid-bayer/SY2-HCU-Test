import QtQuick 2.12
import "../Widgets"
import "../Widgets/Popup"

PopupMessage {
    property string logo: ""
    property string credits: ""
    property string version: ""
    property string url: ""

    type: "INFO"
    showOkBtn: true
    showCancelBtn: false
    heightMin: ( (flickTCredits.contentHeight > dsCfgLocal.screenH * 0.18) || (urlQRcode.visible) ) ? (dsCfgLocal.screenH * 0.52) : (dsCfgLocal.screenH * 0.44)

    content: [
        Item {
            id: sharingInformationCredits
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            width: parent.width * 0.65
            Flickable {
                id: flickTCredits
                anchors.fill: parent
                flickableDirection: Flickable.VerticalFlick
                contentHeight: creditsTitle.height + textCredits.height + textUrl.height + textVersion.height
                contentWidth: parent.width * 0.95


                Image {
                    id: sharedLogo
                    anchors.left: parent.left
                    height: creditsTitle.height
                    width: height
                    source: logo
                }

                Text {
                    id: creditsTitle
                    anchors.left: sharedLogo.right
                    anchors.leftMargin: width * 0.2
                    anchors.top: parent.top
                    anchors.topMargin: height * ( height / contentHeight - 1 ) * 0.5
                    color: colorMap.blk01
                    text: "Credits"
                    width: contentWidth
                    height: contentHeight * 1.4
                    font.pixelSize: dsCfgLocal.screenH * 0.03
                    font.family: fontRobotoMedium.name
                }

                Text {
                    id: textCredits
                    anchors.top: creditsTitle.bottom
                    width: parent.width
                    height: contentHeight
                    color: colorMap.blk01
                    font.pixelSize: dsCfgLocal.screenH * 0.025
                    font.family: fontRobotoLight.name
                    wrapMode: Text.Wrap
                }
                Text {
                    id: textUrl
                    anchors.top: textCredits.bottom
                    width: parent.width
                    height: contentHeight
                    color: colorMap.blk01
                    font.pixelSize: dsCfgLocal.screenH * 0.02
                    font.family: fontRobotoLight.name
                    wrapMode: Text.Wrap
                }
                Text {
                    id: textVersion
                    anchors.top: textUrl.bottom
                    width: parent.width
                    height: contentHeight
                    color: colorMap.blk01
                    font.pixelSize: dsCfgLocal.screenH * 0.02
                    font.family: fontRobotoLight.name
                    wrapMode: Text.Wrap
                }
            }
            ScrollBar {
                flickable: flickTCredits
                autoHide: false
            }
            ListFade {
                flickable: flickTCredits
                fadeColor: popupSharingInformation.color
            }
        },

        Image {
            id: urlQRcode
            anchors.top: parent.top
            anchors.right: parent.right
            sourceSize.width: parent.width * 0.3
            sourceSize.height: parent.width * 0.3
            visible: (url !== "") && (url !== "No URL information entered")
        }

    ]

    onOpened: {
        getContentText();
    }

    onBtnOkClicked: {
        close();
        textCredits.text = "";
        textUrl.text = "";
        textVersion.text = "";
        logo = "";
        credits = "";
        url = "";
        version = "";
    }

    function getContentText()
    {
        textCredits.text = credits.replace(/, /g, "\n");
        textUrl.text = "\n" + url;
        textVersion.text = "\n" + version;

        urlQRcode.source = Qt.binding(function() { return "image://QZXing/encode/" + "URL:" + url; });
    }
}
