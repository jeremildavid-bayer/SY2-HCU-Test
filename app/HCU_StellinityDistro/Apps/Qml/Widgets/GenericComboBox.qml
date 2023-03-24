import QtQuick 2.12
import QtGraphicalEffects 1.12
import "../Util.js" as Util

Item {
    property int rowHeight: 60
    property int itemRadius : buttonRadius
    property int animationMs: 250
    property string itemValueColor: colorMap.text01
    property double itemIconTextFontPixelSize: itemValueTextFontPixelSize
    property double itemValueTextFontPixelSize: rowHeight * 0.35
    property string itemIconColor: colorMap.text01
    property string itemUnitColor: colorMap.text02
    property double itemUnitTextFontPixelSize: rowHeight * 0.25
    property string itemValueFontFamily: fontRobotoBold.name
    property string itemUnitFontFamily: fontRobotoLight.name
    property string expandSymbolColor: colorMap.text02
    property bool expanded: false
    property string itemColor: colorMap.comboBoxBackground
    property int visibleItemCount: 5
    property int itemSpacing: rowHeight * 0.03
    property string itemSpacingColor: "transparent"
    property int itemListHeight: ((rowHeight + itemSpacing) * Math.min(visibleItemCount, optionList.length - 1))

    property var optionList: []
    property int currentIndex: -1
    property int newIndex: -1

    property bool readOnly: false
    property bool enableSignalCurrentIndexChanged: true

    property bool translateValueText: false

    signal signalOpened()
    signal signalCurrentIndexChanged(int index)

    property bool useSvgIcon: false
    property string svgIconSource: ""
    property int svgIconWidth: width
    property int svgIconHeight: height

    id: root
    state: "COMPRESSED"
    height: expanded ? header.height + items.height : header.height

    GenericButton {
        id: btnHeader
        width: parent.width
        height: rowHeight
        enabled: !readOnly
        disabledColor: "transparent"

        content: [
            GenericComboBoxItem {
                id: header
                type: "HEADER"
                color: readOnly ? "transparent" : ((root.state == "COMPRESSED") ? itemColor : colorMap.comboBoxActive)
                border.width: readOnly ? 2 : 0
                border.color: itemColor

                radius: itemRadius

                iconText: (getCurrentValue() === null) ? "" : getCurrentValue().icon
                iconColor: ((getCurrentValue() !== null) && (getCurrentValue().iconColor !== undefined)) ? getCurrentValue().iconColor : itemIconColor
                valueColor: itemValueColor
                valueText: (getCurrentValue() === null) ? "" : (translateValueText ? translate(getCurrentValue().value) : getCurrentValue().value)
                unitText: (getCurrentValue() === null) ? "" : getCurrentValue().unit
                unitColor: itemUnitColor

                useSvgIcon: root.useSvgIcon
                svgIconSource: root.svgIconSource
                svgIconWidth: root.svgIconWidth
                svgIconHeight: root.svgIconHeight
            }
        ]

        onBtnClicked: {
            if (root.state === "COMPRESSED")
            {
                open();
            }
            else
            {
                close();
            }
        }
    }

    Rectangle {
        id: rectBackground
        x: items.x
        y: items.y
        width: items.width
        height: items.height
        opacity: 0.5
        color: colorMap.buttonShadow
    }

    ListView {
        id: items
        clip: true
        height: expanded ? itemListHeight : 0
        width: parent.width
        anchors.top: btnHeader.bottom
        anchors.topMargin: itemSpacing
        model: optionList
        boundsBehavior: (contentHeight > height) ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds

        ScrollBar {}
        ListFade {
            fadeColor: colorMap.buttonShadow
        }

        delegate: GenericButton {
            id: itemDelegate
            height: (currentIndex === index) ? 0 : (rowHeight + itemSpacing)
            visible: (currentIndex !== index)
            width: ListView.view.width
            color: "transparent"

            content: [
                GenericComboBoxItem {
                    id: item
                    height: rowHeight
                    color: itemColor
                    radius: itemRadius

                    iconText: optionList[index].icon
                    iconColor: (optionList[index].iconColor !== undefined) ? optionList[index].iconColor : root.itemIconColor
                    valueText: (translateValueText ? translate(optionList[index].value) : optionList[index].value)
                    valueColor: itemValueColor
                    unitText: optionList[index].unit
                    unitColor: itemUnitColor

                    useSvgIcon: root.useSvgIcon
                    svgIconSource: root.svgIconSource
                    svgIconWidth: root.svgIconWidth
                    svgIconHeight: root.svgIconHeight
                },
                Rectangle {
                    y: rowHeight
                    color: itemSpacingColor
                    width: parent.width
                    height: itemSpacing
                }
            ]

            onBtnClicked: {
                newIndex = index;
                close();
            }

            Component.onCompleted: {
                items.dragStarted.connect(reset);
            }

            Component.onDestruction: {
                items.dragStarted.disconnect(reset);
            }
        }

        layer.enabled: true
        layer.effect: Glow {
            radius: 17
            samples: 17
            spread: 0.5
            color: colorMap.buttonShadow
            transparentBorder: true
        }
    }

    states: [
        State { name: "COMPRESSED" },
        State { name: "EXPANDED" }
    ]

    transitions: [
        Transition {
            from: "COMPRESSED"
            to: "EXPANDED"
            SequentialAnimation {
                ScriptAction { script: {
                        root.z = 2;
                    }
                }

                ParallelAnimation {
                    NumberAnimation { target: header.stateIconFrame; properties: 'rotation'; to: 180; duration: animationMs }
                    NumberAnimation { target: root; properties: 'height'; to: header.height + items.height; duration: animationMs }
                    NumberAnimation { target: items; properties: 'height'; to: itemListHeight; duration: animationMs }
                }

                ScriptAction { script: {
                        expanded = true;
                    }
                }
            }
        },

        Transition {
            from: "EXPANDED"
            to: "COMPRESSED"

            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation { target: header.stateIconFrame; properties: 'rotation'; to: 0; duration: animationMs }
                    NumberAnimation { target: root; properties: 'height'; to: header.height; duration: animationMs }
                    NumberAnimation { target: items; properties: 'height'; to: 0; duration: animationMs }
                }

                ScriptAction { script: {
                        root.z  = 0;
                        expanded = false;
                        if ( (newIndex != -1) && (currentIndex != newIndex) )
                        {
                            setCurrentIndex(newIndex);
                        }
                    }
                }
            }
        }
    ]

    onVisibleChanged: {
        if (!visible)
        {
            close();
        }
    }

    Component.onCompleted: {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        // Screen changed - close combo box
        close();
    }

    function getCurrentValue()
    {
        if ( (optionList.length > 0) &&
             (currentIndex >= 0) &&
             (currentIndex < optionList.length) )
        {
            return optionList[currentIndex];
        }
        return null;
    }

    function setCurrentValue(newOption)
    {
        var indexFound = false;

        for (var i = 0; i < optionList.length; i++)
        {
            if ( (optionList[i].value === newOption.value) &&
                 (optionList[i].unit === newOption.unit) )
            {
                setCurrentIndex(i);
                indexFound = true;
                break;
            }
        }

        if (!indexFound)
        {
            logDebug("WARNING: combo box doesn't have value(" + JSON.stringify(newOption) + "). Adding the new item..");
            var newOptionList = optionList;
            newOptionList.push(newOption);
            setOptionList(newOptionList);
            setCurrentIndex(optionList.length - 1);
        }
    }

    function setCurrentIndex(newIdx)
    {
        if (currentIndex !== newIdx)
        {
            currentIndex = newIdx;
            if (enableSignalCurrentIndexChanged)
            {
                signalCurrentIndexChanged(currentIndex);
            }
        }
    }

    function setOptionList(newOptionList)
    {
        optionList = newOptionList;
    }

    function open()
    {
        signalOpened();
        root.state = "EXPANDED";
    }

    function close()
    {
        var prevState = root.state;
        root.state = "COMPRESSED";
    }
}

