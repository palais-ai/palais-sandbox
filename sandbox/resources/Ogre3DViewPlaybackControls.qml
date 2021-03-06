import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.2

RowLayout {
    id: playbackRow
    spacing: 24

    FAIconButton {
        id: playButton
        unicode: ApplicationWrapper.scenePlaying ? "\uf04c" : "\uf04b"
        iconSize: 20
        enabled: ApplicationWrapper.sceneLoaded
        onPressed: ApplicationWrapper.onPlayButtonPressed()
    }

    FAIconButton {
        id: cameraModeButton
        unicode: "\uf03d"
        iconSize: 20
        onPressed: ApplicationWrapper.focusOnSelectedActor()
    }

    ColumnLayout {
        spacing: 6

        NativeText {
            id: speedText
            text: "speed: " + speedSlider.value.toFixed(1)
            font.family: openSans.name;
            font.pointSize: 11
            color: colors.dimmedTextColor
            Layout.alignment: Layout.Center
        }

        Slider {
            style: SliderStyle {
                groove: Rectangle {
                    implicitWidth: 70
                    implicitHeight: 8
                    color: colors.dimmedTextColor
                    radius: 8
                }
            }
            id: speedSlider
            minimumValue: 0.1
            maximumValue: 10
            stepSize: 0.1
            value: 1
            opacity: 1
            onValueChanged: ProjectManager.setSimulationSpeed(speedSlider.value)

            Connections {
                target: speedMenuItem
                onTriggered: speedSlider.value = speedSlider.maximumValue
            }
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
                unicode = "\uf065"
            } else {
                appWindow.showFullScreen()
                unicode = "\uf066"
            }
            isFullscreen = !isFullscreen;
        }
    }
}
