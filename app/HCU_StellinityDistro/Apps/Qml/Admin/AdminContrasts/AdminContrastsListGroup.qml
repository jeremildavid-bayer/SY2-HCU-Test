import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../../Widgets"
import "../../Util.js" as Util
import "AdminContrasts.js" as ContrastsUtil

GenericButton {
    property int groupIndex: index
    property var groupData
    property int animationMs: 200
    property var dragStartPos
    property int itemWidth: ListView.view.width * 0.2795
    property var fluidOptions: dsCfgLocal.fluidOptions
    property int maxBottleIcons: 4
    property var fluidSourceBottlePackages2: dsDevice.fluidSourceBottlePackages2
    property var fluidSourceBottlePackages3: dsDevice.fluidSourceBottlePackages3
    property bool sameContrasts: dsDevice.sameContrasts

    id: root
    height: ListView.view.height
    width: rectInsert.width + rectMain.width
    color: "transparent"
    clip: true
    opacity: getOpacity()
    radius: 0

    state: "IDLE"
    states: [
        State { name: "IDLE" },
        State { name: "DRAGGING" },
        State { name: "DRAG_STOPPED" }
    ]

    transitions: [
        Transition {
            from: "DRAGGING"
            to: "DRAG_STOPPED"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: dragObject; properties: 'x'; to: dragDroppedPos.x; duration: animationMs; easing.type: Easing.InOutQuart}
                    NumberAnimation { target: dragObject; properties: 'y'; to: dragDroppedPos.y; duration: animationMs; easing.type: Easing.InOutQuart}
                }

                ScriptAction { script: {
                        onDragObjectDropped();
                    }
                }
            }
        }
    ]

    content: [
        Rectangle {
            id: rectInsert
            width: getInsertAreaWidth()
            height: parent.height
            color: colorMap.buttonShadow
        },
        Item {
            id: rectMain
            anchors.left: rectInsert.right
            width: getMainAreaWidth()
            height: parent.height

            Rectangle {
                id: selectedBorder
                anchors.fill: parent
                color: "transparent"
                border.color: colorMap.navBtnSelectedBorder
                border.width: width * 0.025
                radius: width * 0.025
                visible: (dummyFamilyIndex == -1) && (dummyGroupIndex == -1) && (selectedFamilyIdx == familyIndex) && (selectedGroupIdx == groupIndex)
            }

            Text {
                width: parent.width * 0.15
                height: parent.height * 0.18
                anchors.right: parent.right
                anchors.rightMargin: parent.width * 0.06
                anchors.top: parent.top
                anchors.topMargin: parent.height * 0.05
                font.family: fontIcon.name
                font.pixelSize: height
                text: "\ue92f"
                visible: {
                    if ( (fluidSourceBottlePackages2[0] !== undefined) &&
                         (fluidSourceBottlePackages2[0].Brand === groupData.Brand) &&
                         (fluidSourceBottlePackages2[0].Concentration === groupData.Concentration) )
                    {
                        return true;
                    }
                    else if ( (fluidSourceBottlePackages2[0] !== undefined) &&
                              (fluidSourceBottlePackages2[0].Brand === groupData.Brand) &&
                              (fluidSourceBottlePackages2[0].Concentration === groupData.Concentration) )
                    {
                        return true;
                    }
                    else if ( (fluidSourceBottlePackages3[0] !== undefined) &&
                              (fluidSourceBottlePackages3[0].Brand === groupData.Brand) &&
                              (fluidSourceBottlePackages3[0].Concentration === groupData.Concentration) )
                    {
                        return true;
                    }
                    else if ( (fluidSourceBottlePackages3[0] !== undefined) &&
                              (fluidSourceBottlePackages3[0].Brand === groupData.Brand) &&
                              (fluidSourceBottlePackages3[0].Concentration === groupData.Concentration) )
                    {
                        return true;
                    }
                    return false;
                }

                color: {
                    if ( (fluidSourceBottlePackages2[0] !== undefined) &&
                         (fluidSourceBottlePackages2[0].Brand === groupData.Brand) &&
                         (fluidSourceBottlePackages2[0].Concentration === groupData.Concentration) )
                    {
                        return colorMap.contrast1;
                    }
                    else if ( (fluidSourceBottlePackages2[0] !== undefined) &&
                              (fluidSourceBottlePackages2[0].Brand === groupData.Brand) &&
                              (fluidSourceBottlePackages2[0].Concentration === groupData.Concentration) )
                    {
                        return colorMap.contrast1;
                    }
                    else if ( (fluidSourceBottlePackages3[0] !== undefined) &&
                              (fluidSourceBottlePackages3[0].Brand === groupData.Brand) &&
                              (fluidSourceBottlePackages3[0].Concentration === groupData.Concentration) )
                    {
                        return sameContrasts ? colorMap.contrast1 : colorMap.contrast2;
                    }
                    else if ( (fluidSourceBottlePackages3[0] !== undefined) &&
                              (fluidSourceBottlePackages3[0].Brand === groupData.Brand) &&
                              (fluidSourceBottlePackages3[0].Concentration === groupData.Concentration) )
                    {
                        return sameContrasts ? colorMap.contrast1 : colorMap.contrast2;
                    }
                    return "transparent";
                }
            }

            Repeater {
                property int bottleCount: Math.min(maxBottleIcons, Math.max(1, groupData.FluidPackages.length - 1))

                id: frameIcon
                model: bottleCount

                delegate: Text {
                    id: icon
                    width: parent.width * 0.5
                    height: parent.height * 0.7
                    y: (parent.height * 0.05) - (index * (height * 0.056))
                    x: (parent.width * 0.25) + (index * (width * 0.1)) - ((frameIcon.bottleCount - 1) * parent.width * 0.02)

                    text: getBottleIcon()
                    color: colorMap.contrast1
                    font.pixelSize: height * 0.7
                    font.family: fontIcon.name
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    Text {
                        // 2nd layer
                        anchors.fill: parent
                        font.pixelSize: icon.font.pixelSize * 1.12
                        font.family: icon.font.family
                        horizontalAlignment: icon.horizontalAlignment
                        verticalAlignment: icon.verticalAlignment
                        text: (icon.text == "\ue958") ? "\ue959" : "\ue968"
                        color: colorMap.text01
                    }
                }
            }

            Text {
                id: textLabel
                width: parent.width * 0.9
                height: parent.height * ((groupData.Brand === "<NEW>") ? 0.3 : 0.15)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: textConcentration.top
                text: {
                    if (groupData.Brand === "<NEW>")
                    {
                        return "+";
                    }
                    else if (groupData.Brand === "")
                    {
                        return "--";
                    }
                    return groupData.Brand;
                }
                color: (ContrastsUtil.getErrorFromGroupData(groupData, familyIndex, groupIndex) === "") ? colorMap.text01 : colorMap.errText
                font.pixelSize: parent.height * ((groupData.Brand === "<NEW>") ? 0.2 : 0.12)
                font.family: fontRobotoLight.name
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            Text {
                id: textConcentration
                width: parent.width * 0.9
                height: parent.height * ((groupData.Brand === "<NEW>") ? 0 : 0.15)
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: parent.height * 0.015
                text: getContrastConcentration()
                color: textLabel.color
                font.pixelSize: parent.height * 0.13
                font.family: fontRobotoBold.name
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            WarningIcon {
                width: parent.width * 0.2
                height: width
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: width / 2
                anchors.topMargin: height / 2
                visible: (ContrastsUtil.getErrorFromGroupData(groupData, familyIndex, groupIndex) !== "")
            }
        }
    ]

    onBtnClicked: {
        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        // Auto-correct last selected group
        ContrastsUtil.reorderSelectedGroupFluidPackages();
        contrastsList.signalReload(selectedFamilyIdx, selectedGroupIdx);

        if (groupData.Brand === "<NEW>")
        {
            ContrastsUtil.setReloadReason("ADDED");
        }
        else
        {
            ContrastsUtil.setReloadReason("SELECTED");
        }


        contrastsPage.selectedFamilyIdx = familyIndex;
        contrastsPage.selectedGroupIdx = groupIndex;

        ContrastsUtil.setReloadReason("NONE");
    }

    onBtnReleased: {
        if (root.state === "DRAGGING")
        {
            dragObject.released();
            root.state = "DRAG_STOPPED";
        }
    }

    onBtnPressedLong: (mouse) => {
        if (groupData.Brand === "<NEW>")
        {
            return;
        }

        if (dragObject.isOpen())
        {
            //logDebug("Trying to drag while other option is busy");
            return;
        }

        if (widgetKeyboard.isOpen())
        {
            widgetKeyboard.close(false);
        }

        if (widgetInputPad.isOpen())
        {
            widgetInputPad.close(false);
        }

        // Select target before start dragging
        ContrastsUtil.setReloadReason("SELECTED");
        contrastsPage.selectedFamilyIdx = familyIndex;
        contrastsPage.selectedGroupIdx = groupIndex;

        dragStartPos = { x: mouse.x, y: mouse.y };

        listViewFamily.interactive = false;
        listViewPackage.interactive = false;

        root.grabToImage(function(result) {

            dragObject.open(root, result.url);
            dragObject.userData = Util.copyObject(groupData);

            root.state = "DRAGGING";

            dragStartFamilyIndex = familyIndex;
            dragStartGroupIndex = groupIndex;

            dummyFamilyIndex = familyIndex;
            dummyGroupIndex = groupIndex + 1;
        }, Qt.size(root.width, root.height));
    }

    onBtnPositionChanged: (mouse) => {
        if (root.state === "DRAGGING")
        {
            var newPos = dragObject.parent.mapFromItem(root, mouse.x, mouse.y);
            dragObject.x = newPos.x - dragStartPos.x;
            dragObject.y = newPos.y - dragStartPos.y;
        }
    }

    onVisibleChanged: {
        if (!visible)
        {
            // Make sure last dragging state is reset
            if (root.state !== "IDLE")
            {
                dragObject.close();
                listViewFamily.interactive = true;
                listViewPackage.interactive = true;
                groupData = Util.copyObject(dragObject.userData);
                dummyFamilyIndex = -1;
                dummyGroupIndex = -1;
                root.state = "IDLE";
            }
        }
    }

    Component.onCompleted: {
        listViewPackage.dragStarted.connect(reset);
        listViewFamily.dragStarted.connect(reset);
        contrastsList.signalReload.connect(reload);
        dragObject.xChanged.connect(onDragObjectMoved);
        dragObject.yChanged.connect(onDragObjectMoved);
        dragObject.signalReleased.connect(onDragObjectReleased);
        reload(familyIndex, groupIndex);
    }

    Component.onDestruction: {
        listViewPackage.dragStarted.disconnect(reset);
        listViewFamily.dragStarted.disconnect(reset);
        contrastsList.signalReload.disconnect(reload);
        dragObject.xChanged.disconnect(onDragObjectMoved);
        dragObject.yChanged.disconnect(onDragObjectMoved);
        dragObject.signalReleased.disconnect(onDragObjectReleased);
    }

    function reload(familyIdx, groupIdx)
    {
        if ( (familyIndex === familyIdx) &&
             (groupIndex === groupIdx) )
        {
            groupData = Util.copyObject(contrastFamilies[familyIndex].Groups[groupIndex]);
        }
    }

    function onDragObjectReleased()
    {
        if ( (familyIndex == dummyFamilyIndex) &&
             (groupIndex == dummyGroupIndex) )
        {
            var rootPos = dragObject.parent.mapFromItem(root, 0, 0);
            dragDroppedPos = { x: rootPos.x, y: rootPos.y };
            //logDebug("Index[" + familyIndex + "][" + groupIndex + "]: dragDroppedPos=" + JSON.stringify(dragDroppedPos));
        }
    }

    function onDragObjectDropped()
    {
        dragObject.close();
        listViewFamily.interactive = true;
        listViewPackage.interactive = true;

        var newDummyGroupIdx = dummyGroupIndex;
        if (dragStartFamilyIndex == dummyFamilyIndex)
        {
            // Drop to same row
            if (dummyGroupIndex > dragStartGroupIndex)
            {
                //logDebug("dummyGroupIndex-- = " + dummyGroupIndex);
                newDummyGroupIdx--;
            }
        }

        var newSelectedFamilyIdx = dummyFamilyIndex;

        var familyLayoutChanged = false;

        if ( (dragStartFamilyIndex != dummyFamilyIndex) ||
             (dragStartGroupIndex != newDummyGroupIdx) )
        {
            //logDebug("Index[" + familyIndex + "][" + groupIndex + "]: Moved to Item[" + dummyFamilyIndex + "][" + newDummyGroupIdx + "], dragDroppedPos=" + dragDroppedPos.x + "," + dragDroppedPos.y);

            if ( (contrastsPage.contrastFamilies[familyIndex].Groups.length === 2) &&
                 (contrastsPage.contrastFamilies[dummyFamilyIndex].Groups.length === 1) &&
                 (familyIndex == contrastsPage.contrastFamilies.length - 2) )
            {
                //logDebug("Item is last family and moved down to empty family. No need to update layout");
                newSelectedFamilyIdx = familyIndex;
            }
            else
            {
                if ( (contrastsPage.contrastFamilies[familyIndex].Groups.length === 2) &&
                     (familyIndex < dummyFamilyIndex) )
                {
                    //logDebug("Item is moved down from single family. Selected family index decremented");
                    newSelectedFamilyIdx--;
                }
                familyLayoutChanged = true;
            }
        }

        if (familyLayoutChanged)
        {
            var contrastFamiliesBuf = Util.copyObject(contrastsPage.contrastFamilies);

            // Cut and paste item from previous location
            //logDebug("Moving Family[" + dragStartFamilyIndex + "].Group[" + dragStartGroupIndex + "] -> Family[" + dummyFamilyIndex + "].Group[" + newDummyGroupIdx + "]")
            contrastFamiliesBuf[dragStartFamilyIndex].Groups.splice(dragStartGroupIndex, 1);
            contrastFamiliesBuf[dummyFamilyIndex].Groups.splice(newDummyGroupIdx, 0, dragObject.userData);

            dummyFamilyIndex = -1;
            dummyGroupIndex = -1;
            root.state = "IDLE";
            contrastsPage.selectedFamilyIdx = newSelectedFamilyIdx;
            contrastsPage.selectedGroupIdx = newDummyGroupIdx;

            ContrastsUtil.setReloadReason("MOVED");

            ContrastsUtil.saveFamilies(contrastFamiliesBuf);
        }
        else
        {
            groupData = Util.copyObject(dragObject.userData);
            dummyFamilyIndex = -1;
            dummyGroupIndex = -1;
            root.state = "IDLE";
        }

    }
    function onDragObjectMoved()
    {
        if (!dragObject.isOpen())
        {
            // Drag Object is closed by other
            return;
        }

        if ( (root.state == "DRAGGING") || (root.state == "DRAG_STOPPED") )
        {
            //logDebug("item[" + familyIndex + "][" + groupIndex + "] is dummy");
            return;
        }

        if ( (dummyFamilyIndex == -1) ||
             (dummyGroupIndex == -1) )
        {
            return;
        }

        var rootPos = dragObject.parent.mapFromItem(root, 0, 0);
        var rootRect = { x: rootPos.x,
                         y: rootPos.y,
                         width: root.width,
                         height: root.height };

        var rectIntersection = Util.getIntersection(dragObject, rootRect);

        if (rectIntersection !== null)
        {
            var intersectionArea = rectIntersection.width * rectIntersection.height;
            //logDebug("Item[" + familyIndex + "][" + groupIndex + "]: rect=" + intersectionArea);

            if (intersectionArea > (root.width * root.height) * 0.5)
            {
                dummyFamilyIndex = familyIndex;
                dummyGroupIndex = groupIndex;
            }
        }
    }

    function getOpacity()
    {
        if ( (root.state == "DRAGGING") || (root.state == "DRAG_STOPPED") )
        {
            return 0.2;
        }
        return 1;
    }

    function getBottleIcon()
    {
        for (var key in fluidOptions.KnownBarcodes)
        {
            if ( (fluidOptions.KnownBarcodes[key].Brand.toLowerCase() === groupData.Brand.toLowerCase()) &&
                 (fluidOptions.KnownBarcodes[key].Concentration === groupData.Concentration) )
            {
                return "\ue967";
            }
        }
        return "\ue958";
    }

    function getContrastConcentration()
    {
        if (groupData.Brand === "<NEW>")
        {
            return "";
        }
        else if (groupData.Concentration === 0)
        {
            return "--";
        }
        return groupData.Concentration;
    }

    function getMainAreaWidth()
    {
        if ( (root.state == "DRAGGING") || (root.state == "DRAG_STOPPED") )
        {
            return 0;
        }
        return itemWidth;
    }

    function getInsertAreaWidth()
    {
        if ( (root.state == "DRAGGING") || (root.state == "DRAG_STOPPED") )
        {
            return 0;
        }
        else if ((familyIndex != dummyFamilyIndex) || (groupIndex != dummyGroupIndex))
        {
            return 0;
        }
        return itemWidth;
    }
}
