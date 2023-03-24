import QtQuick 2.12
import "../../../Widgets"
import "../../../PopupManager"

GenericButton {
    property int planIndex: index
    property var planDigest: groupData.PlanDigests[planIndex]
    property var planData: planDigest.Plan
    property var cruLinkStatus: dsCru.cruLinkStatus

    height: rowHeight
    width: ListView.view.width
    color: "transparent"

    content: [
        Rectangle {
            id: mainRect
            height: rowHeight
            x: itemMargin
            width: parent.width - x
            color: "transparent"

            Image {
                id: personalizedImage
                anchors.right: textPlanName.left
                anchors.rightMargin: width * 0.3
                anchors.verticalCenter: parent.verticalCenter
                source:
                {
                    var containsModifier = false;
                    var containsGenerator = false;
                    var imgSource = imageMap.examProtocolPersonalised;
                    for (var idx = 0 ; idx < planData.Steps.length ; idx++)
                    {
                        for (var modifierIdx = 0 ; modifierIdx < planData.Steps[idx].PersonalizationModifiers.length ; modifierIdx++ )
                        {
                            if (planData.Steps[idx].PersonalizationModifiers[modifierIdx].includes("Tube Voltage Based Modifier"))
                            {
                                containsModifier = true;
                                break;
                            }
                        }

                        if (planData.Steps[idx].PersonalizationGenerator !== "")
                        {
                            containsGenerator = true;
                        }
                        if (containsModifier && containsGenerator)
                            break;
                    }

                    if (containsModifier && containsGenerator)
                    {
                        imgSource = imageMap.examProtocolPersonalisedKVP;
                    }
                    else if (containsModifier)
                    {
                        imgSource = imageMap.examProtocolKVP;
                    }
                    return imgSource;
                }
                width: visible ? itemMargin - (itemMargin * 0.1) : 0
                height: width * 0.65
                sourceSize.width: width
                sourceSize.height: height
                visible: (planDigest.State === "Ready") ? planData.IsPersonalized : planDigest.IsPersonalized
            }

            Text {
                id: textPlanName
                anchors.left: parent.left
                anchors.leftMargin: parent.width * 0.03
                height: parent.height
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.08
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignVCenter
                text: (planDigest.State === "Ready") ? planData.Name : planDigest.Name
                color: colorMap.text01
                font.pixelSize: height * 0.37
                font.family: ((groupIndex == selectedGroupIdx) && (index == selectedPlanIdx)) ? fontRobotoBold.name : fontRobotoLight.name
                elide: Text.ElideRight
            }

            Rectangle {
                id: borderBottom
                y: parent.height - bottomBorderWidth
                width: parent.width - (parent.width * 0.015)
                height: bottomBorderWidth
                color: borderColor
                visible: (index < (groupData.PlanDigests.length - 1)) || ((groupIndex >= planTemplateGroups.length - 1) && (index >= groupData.PlanDigests.length - 1))
            }
        },

        Rectangle {
            id: frameSelected
            visible: (groupIndex === selectedGroupIdx) && (index === selectedPlanIdx)
            width: parent.width
            height: parent.height
            color: "transparent"
            border.width : 2
            border.color: colorMap.actionButtonBackground
            radius: 10

            Rectangle {
                color: colorMap.actionButtonBackground
                width: parent.width * 0.03
                height: parent.height
                x: parent.width - (width * 2)
            }

            Rectangle {
                color: colorMap.actionButtonBackground
                width: parent.width * 0.03
                height: parent.height
                x: parent.width - width
                radius: 10
            }
        }
    ]

    onBtnClicked: {
        var spotlightProtocolSelected = (groupData.Name === "T_SpotlightProtocols");
        var isDeselect = (groupIndex == selectedGroupIdx) && (planIndex == selectedPlanIdx);
        var deselectSpotlightProtocol = (isDeselect && spotlightProtocolSelected);

        //If a non-spotlight template or the default template has been selected, ensure the list doesn't re-select a spotlight protocol.
        setDoNotSelectSpotlightProtocol(!spotlightProtocolSelected || deselectSpotlightProtocol);
        setWasButtonPressed(true);

        if (isDeselect)
        {
            logDebug("Default template selected");
            dsExam.slotDefaultPlanTemplateSelected();
        }
        else if (planDigest.State === "BadData")
        {
            logWarning("Bad Template selected: " + JSON.stringify(planDigest));
            popupManager.popupInvalidInjectionSelected.open();
        }
        else if ( (planDigest.IsPersonalized) &&
                  (cruLinkStatus.State !== "Active") )
        {
            logWarning("CRU Link is " + cruLinkStatus.State + " and personalized protocol selected: " + JSON.stringify(planDigest));
            popupManager.popupPersonalizedProtocolUnavailable.open();
        }
        else
        {
            // Select new plan
            logDebug("Template selected: " + planData.Name);
            dsExam.slotPlanPreviewChanged(planData);
        }
    }

    Component.onCompleted: {
        listViewPlanGroup.dragStarted.connect(reset);
    }

    Component.onDestruction: {
        listViewPlanGroup.dragStarted.disconnect(reset);
    }
}
