import QtQuick 2.12

QtObject {
    property string shellOutput: ""

    function slotShellCommand(command, args) { return dsDevToolsCpp.slotShellCommand(command, args); }
}
