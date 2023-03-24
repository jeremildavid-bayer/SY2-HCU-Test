import QtQuick 2.12

Item {
    // Data
    property var upgradeDigest: { "State": "Unknown" }

    // Function from QML to CPP
    function slotUpdateSelectedPackageInfo(pathPackage) { return dsUpgradeCpp.slotUpdateSelectedPackageInfo(pathPackage); }
    function slotUpgrade() { return dsUpgradeCpp.slotUpgrade(); }
}
