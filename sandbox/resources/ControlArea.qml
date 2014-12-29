import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    color: colors.darkGray

    Loader {
        id: controlAreaLoader
        z: 2
        width: parent.width - separator.width
        height: parent.height
        source: "ControlAreaBeforeLoad.qml"
    }

    NumberAnimation {
        id: moveOut
        target: controlAreaLoader
        property: "opacity"
        from: 1
        to: 0
        duration: 300

        onStopped: {
            controlAreaLoader.setSource("ActorTable.qml")
            moveIn.start()
        }
    }

    NumberAnimation {
        id: moveIn
        target: controlAreaLoader
        property: "opacity"
        from: 0
        to: 1
        duration: 300
    }

    ListSeparator {
        id: separator
        handleSize: 1
        handleColor: colors.black95
        orientation: Qt.Vertical
        anchors.right: parent.right
        anchors.top: parent.top
    }

    Connections {
        target: ApplicationWrapper
        onSceneLoadedChanged: moveOut.start()
    }
}
