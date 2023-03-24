import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    property var modalObjects: []
    property double blurOpacity: 1.0
    property var backgroundWidget: appMainWindow

    id: root
    visible: false
    state: "CLOSED"

    states: [
        State { name: "CLOSED" },
        State { name: "OPEN" }
    ]

    transitions: [
        Transition {
            from: "CLOSED"
            to: "OPEN"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        visible = true;
                    }
                }
            }
        },
        Transition {
            from: "OPEN"
            to: "CLOSED"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        // Restore and delete all modal objects
                        while (Object.keys(modalObjects).length > 0)
                        {
                            var firstKey = Object.keys(modalObjects)[0];
                            removeObject(firstKey);
                        }

                        visible = false;
                    }
                }
            }
        }
    ]

    FastBlur {
        anchors.fill: parent
        radius: 40
        smooth: true
        opacity: blurOpacity
        source: backgroundWidget
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
            var keys = [];
            for (var key in modalObjects)
            {
                if (modalObjects[key].ClickOutsideToClose)
                {
                    keys.push(key);
                }
            }

            for (var i = 0; i < keys.length; i++)
            {
                key = keys[i];
                var cbFn = modalObjects[key].ClickedCbFn;
                if (cbFn !== null)
                {
                    cbFn();
                }
                if (modalObjects[key] !== undefined)
                {
                    close(modalObjects[key].Object);
                }
            }

            // MouseArea to get the focus for the application (so Alt+4 can work)
            appMain.requestActivate();
        }
    }

    Component.onCompleted: {
        modalObjects = {};

        var rectTop = backgroundWidget.mapFromItem(parent, 0, 0);
        x = -rectTop.x;
        y = -rectTop.y;
    }

    function open(newModalObjects, clickOutsideToClose, newClickedCb)
    {
        if (newModalObjects !== undefined)
        {
            for (var objIdx = 0; objIdx < newModalObjects.length; objIdx++)
            {
                addObject(newModalObjects[objIdx], clickOutsideToClose, newClickedCb);
            }
        }
        state = "OPEN";
    }

    function close(newModalObjects)
    {
        for (var objIdx = 0; objIdx < newModalObjects.length; objIdx++)
        {
            var modalObject = newModalObjects[objIdx];
            if ( (modalObjects !== undefined) &&
                 (modalObjects[modalObject] !== undefined) )
            {
                // More modal objects are exist: Do not close the background just remove the current modal object
                removeObject(modalObject);
            }
        }

        if (Object.keys(modalObjects).length == 0)
        {
            // No more modal objects, ok to close
            state = "CLOSED";
        }
    }

    function isOpen()
    {
        return state == "OPEN";
    }

    function addObject(modalObject, clickOutsideToClose, newClickedCb)
    {
        if (modalObject === undefined)
        {
            return;
        }

        modalObjects[modalObject] = {
            "Object": modalObject,
            "ParentObject": modalObject.parent,
            "OrigGeometry": { "X": modalObject.x, "Y": modalObject.y, "Z": modalObject.z, "W": modalObject.width, "H": modalObject.height },
            "ClickedCbFn": newClickedCb,
            "ClickOutsideToClose": clickOutsideToClose
        };

        var pos = root.mapFromItem(modalObject, 0, 0);

        modalObject.parent = root;
        modalObject.x = pos.x;
        modalObject.y = pos.y;
        modalObject.width = modalObjects[modalObject].OrigGeometry.W;
        modalObject.height = modalObjects[modalObject].OrigGeometry.H;
    }

    function removeObject(modalObject)
    {
        if (modalObjects === undefined)
        {
            return;
        }

        if (modalObjects[modalObject] === undefined)
        {
            return;
        }

        modalObjects[modalObject].Object.parent = modalObjects[modalObject].ParentObject;
        modalObjects[modalObject].Object.x = modalObjects[modalObject].OrigGeometry.X;
        modalObjects[modalObject].Object.y = modalObjects[modalObject].OrigGeometry.Y;
        modalObjects[modalObject].Object.z = modalObjects[modalObject].OrigGeometry.Z;
        delete modalObjects[modalObject];
    }
}
