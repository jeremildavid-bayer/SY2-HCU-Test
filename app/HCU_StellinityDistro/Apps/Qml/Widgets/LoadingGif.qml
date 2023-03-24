import QtQuick 2.12

Item {
    visible: false
    width: parent.width
    height: parent.height

    Rectangle {
        width: dsCfgLocal.screenW * 0.04
        height: width
        color: colorMap.buttonShadow
        radius: dsCfgLocal.screenW * 0.01
        anchors.centerIn: parent
        opacity: 0.1
    }

    Image {
        id: imgBusySpinner
        width: dsCfgLocal.screenW * 0.03
        height: width
        source: imageMap.busySpinner
        sourceSize.width: width
        sourceSize.height: height
        anchors.centerIn: parent
    }

    RotationAnimation {
        id: animationSpin
        target: imgBusySpinner
        loops: Animation.Infinite
        duration: 1500
        from: 0
        to: 360
    }

    onVisibleChanged: {
        if (visible)
        {
            animationSpin.start();
        }
        else
        {
            animationSpin.stop();
        }
    }

    function show(showIt)
    {
        visible = showIt;
    }
}
