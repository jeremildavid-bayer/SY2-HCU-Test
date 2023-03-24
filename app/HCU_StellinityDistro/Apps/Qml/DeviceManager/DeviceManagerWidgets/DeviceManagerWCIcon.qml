import QtQuick 2.12

Item {
    antialiasing: true

    Text {
        horizontalAlignment: Text.horizontalAlignment
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height

        text: {
            if ( (fluidSourceWasteContainer === undefined) ||
                 (fluidSourceWasteContainer.InstalledAt === undefined) ||
                 (fluidSourceWasteContainer.CurrentVolumes === undefined) ||
                 (fluidSourceWasteContainer.CurrentVolumes.length === 0) )
            {
                return ""
            }
            else if (fluidSourceWasteContainer.NeedsReplaced)
            {
                // Full
                return "\ue90f";
            }
            else if (fluidSourceWasteContainer.CurrentVolumes[0] === 0)
            {
                // Low
                return "\ue911";
            }
            else if (fluidSourceWasteContainer.CurrentVolumes[0] === 1)
            {
                // High
                return "\ue910";
            }
            return "";
        }

        color: {
            if ( (fluidSourceWasteContainer !== undefined) &&
                 (fluidSourceWasteContainer.InstalledAt !== undefined) &&
                 (fluidSourceWasteContainer.NeedsReplaced) )
            {
                return colorMap.deviceIconWarningState;
            }
            return colorMap.deviceIconWasteLevel;
        }
    }

    Text {
        // Waste Container Outer
        anchors.top: parent.top
        anchors.left: parent.left
        horizontalAlignment: Text.horizontalAlignment
        text: "\ue90e"
        font.family: fontDeviceIcon.name
        font.pixelSize: parent.height
        color: {
            if ( (fluidSourceWasteContainer === undefined) ||
                 (fluidSourceWasteContainer.InstalledAt === undefined) ||
                 (fluidSourceWasteContainer.CurrentVolumes === undefined) ||
                 (fluidSourceWasteContainer.CurrentVolumes.length === 0) )
            {
                return colorMap.errText;
            }
            return colorMap.deviceIconSelected;
        }
    }
}
