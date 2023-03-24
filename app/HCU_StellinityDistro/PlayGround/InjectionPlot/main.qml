import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.3

Window {
    id: window
    x: 0
    width: 1900
    height: 1200
    visible: true
    objectName: "main"
    color: "lightblue"

    FontLoader {
        source: "qrc:/fonts/fontawesome-webfont.ttf"
    }

    //===================================================
    InjectionPlot {
        objectName: "InjectionPlot"
        anchors.fill: parent
    }



}





