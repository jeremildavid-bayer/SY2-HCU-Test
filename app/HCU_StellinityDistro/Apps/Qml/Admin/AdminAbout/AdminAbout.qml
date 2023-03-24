import QtQuick 2.12
import QtQuick.Controls 2.12
import "../../Startup"
import "../../Widgets"

Item {
    y: -adminPages.y
    width: parent.width
    height: parent.height + actionBarHeight + adminPages.y

    Startup {
        function slotScreenStateChanged()
        {
            visible = (appMain.screenState == "Admin-About");
            reload();
        }
    }

    Component.onCompleted:
    {
        appMain.screenStateChanged.connect(slotScreenStateChanged);
        slotScreenStateChanged();
    }

    Component.onDestruction: {
        appMain.screenStateChanged.disconnect(slotScreenStateChanged);
    }

    function slotScreenStateChanged()
    {
        visible = (appMain.screenState == "Admin-About");
        reload();
    }

    function reload()
    {
        if (!visible)
        {
            return;
        }
    }
}

