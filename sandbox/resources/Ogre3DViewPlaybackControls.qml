import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

RowLayout {
    spacing: 24

    FAIconButton {
        id: playButton
        unicode: "\uf04b"
        iconSize: 20
        onPressed: play()

        function play() {

        }
    }

    FAIconButton {
        id: cameraModeButton
        unicode: "\uf03d"
        iconSize: 20
        onPressed: selectCameraMode()

        function selectCameraMode() {
            testDialog.open()
        }
    }

    ColumnLayout {
        spacing: 0

        Text {
            text: "speed: " + speedSlider.value.toFixed(1)
            font.family: openSans.name;
            color: colors.dimmedTextColor
            Layout.alignment: Layout.Center
            renderType: Text.NativeRendering
        }

        Slider {
            id: speedSlider
            minimumValue: 0.1
            maximumValue: 5
            stepSize: 0.1
            value: 1
            opacity: 0.8
        }
    }

    FAIconButton {
        id: expandButton
        unicode: "\uf065"
        iconSize: 20
        onPressed: toggleFullscreen()

        property bool isFullscreen: false

        function toggleFullscreen() {
            if(isFullscreen) {
                appWindow.showNormal()
            } else {
                appWindow.showFullScreen()
            }
            isFullscreen = !isFullscreen;
        }
    }
}
