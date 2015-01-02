import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    height: childrenRect.height + separator.anchors.topMargin
    color: colors.darkGray

    Row {
        id: row
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.leftMargin: 8
        spacing: 12

        NativeText {
            id: consoleText
            text: ApplicationWrapper.sceneLoaded ? "Console" : "No Scene Is Loaded Yet."
            font.family: openSans.name
            font.capitalization: Font.AllUppercase
            font.pointSize: 12
            font.weight: Font.Light
            color: colors.lightGray
        }

        FAIconButton {
            unicode: "\uf014"
            onPressed: ConsoleModel.clear()
        }

        FAIconButton {
            unicode: "\uf0de"
            onPressed: consoleListView.positionViewAtBeginning()
        }

        FAIconButton {
            unicode: "\uf0dd"
            onPressed: consoleListView.positionViewAtEnd()
        }
    }

    NativeText {
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.right: parent.right
        anchors.rightMargin: 8
        text: ConsoleModel.passedTimeString
        font.family: openSans.name
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
    }

    ListSeparator {
        id: separator
        anchors.top: row.bottom
        anchors.topMargin: 4
        handleColor: colors.black95
        handleSize: 1
    }
}
