import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

RowLayout {
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
        onPressed: Scene.setCameraFocus(ProjectManager.selectedActor)
    }

    ColumnLayout {
        spacing: 6

        NativeText {
            text: "speed: " + speedSlider.value.toFixed(1)
            font.family: openSans.name;
            font.pointSize: 11
            color: colors.dimmedTextColor
            Layout.alignment: Layout.Center
        }

        Slider {
            id: speedSlider
            minimumValue: 0.1
            maximumValue: 5
            stepSize: 0.1
            value: 1
            opacity: 1
            onValueChanged: ProjectManager.setSimulationSpeed(speedSlider.value)
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
