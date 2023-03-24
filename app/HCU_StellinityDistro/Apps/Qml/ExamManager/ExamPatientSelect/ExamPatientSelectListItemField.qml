import QtQuick 2.12
import "../../Widgets"
import "../../Util.js" as Util

Item {
    property var dicomField
    property double colWidthPercent: 0.1
    property int textMargin: height * 0.1
    property int leftMargin: 0
    property int rightMargin: 0

    property string searchStr: examPatientSelect.searchStr
    property bool searchStrFound: false

    width: parent.width * colWidthPercent
    height: parent.height

    Item {
        x: leftMargin
        width: parent.width - x - rightMargin
        height: parent.height

        Text {
            anchors.left: parent.left
            anchors.leftMargin: textMargin
            anchors.right: parent.right
            anchors.rightMargin: textMargin
            font.pixelSize: parent.height * 0.24
            height: parent.height
            font.family: fontRobotoLight.name
            wrapMode: Text.Wrap
            color: colorMap.text01
            verticalAlignment: Text.AlignVCenter
            text: getDicomFieldValueWithSearchStr(dicomField)
        }
    }

    function getDicomFieldValueWithSearchStr(dicomField)
    {
        var dicomFieldVal = dicomHelper.getDicomFieldValue(dicomField);
        var curSearchStrFound = true;

        if (searchStr.length > 0)
        {
            var searchPt = 0;
            curSearchStrFound = false;
            while (1)
            {
                searchPt = dicomFieldVal.toLowerCase().indexOf(searchStr.toLowerCase(), searchPt);
                if (searchPt == -1)
                {
                    break;
                }

                var searchStrMatch = dicomFieldVal.substring(searchPt, searchPt + searchStr.length);

                curSearchStrFound = true;
                var strLeft = dicomFieldVal.substr(0, searchPt);
                var strRight = dicomFieldVal.substr(searchPt + searchStr.length, dicomFieldVal.length - searchPt + searchStr.length);
                searchPt += searchStr.length + "<b></b>".length;
                dicomFieldVal = strLeft + "<b>" + searchStrMatch + "</b>" + strRight;
            }
        }

        searchStrFound = curSearchStrFound;
        return dicomFieldVal;
    }
}
