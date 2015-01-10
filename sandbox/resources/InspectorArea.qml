import QtQuick 2.0

Rectangle {
    id: wrapper
    color: colors.darkGray

    Loader {
        id: inspectorLoader
        sourceComponent: Rectangle {
            width: wrapper.width - separator.width
            height: wrapper.height
            color: wrapper.color

            NativeText {
                text: "Knowledge"
                font.family: openSans.name
                font.capitalization: Font.AllUppercase
                font.pointSize: 12
                font.weight: Font.Light
                color: colors.lightGray
                anchors.top: parent.top
                anchors.topMargin: 6
                anchors.left: parent.left
                anchors.leftMargin: 8
            }
        }
    }

    ListSeparator {
        id: separator
        handleSize: 1
        handleColor: colors.black95
        orientation: Qt.Vertical
        anchors.left: parent.left
        anchors.top: parent.top
    }

    Connections {
        target: ApplicationWrapper
        onSceneLoadedChanged: inspectorLoader.source = "InspectorAreaTable.qml"
    }
}
