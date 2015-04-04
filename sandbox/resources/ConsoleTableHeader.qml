import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    height: 26
    color: colors.darkGray

    ListSeparator {
        anchors.top: parent.top
        handleColor: colors.black95
        handleSize: 1
    }

    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 8
        spacing: 12

        NativeText {
            anchors.verticalCenter: parent.verticalCenter
            id: consoleText
            text: ApplicationWrapper.sceneLoaded ? "Console" : "No Scene Is Loaded Yet."
            font.family: openSans.name
            font.capitalization: Font.AllUppercase
            font.pointSize: 12
            font.weight: Font.Light
            color: colors.lightGray
        }

        FAIconButton {
            anchors.verticalCenter: parent.verticalCenter
            unicode: "\uf014"
            onPressed: ConsoleModel.clear()
        }

        FAIconButton {
            anchors.verticalCenter: parent.verticalCenter
            unicode: "\uf0de"
            onPressed: consoleListView.positionViewAtBeginning()
        }

        FAIconButton {
            anchors.verticalCenter: parent.verticalCenter
            unicode: "\uf0dd"
            onPressed: consoleListView.positionViewAtEnd()
        }
    }

    NativeText {
        anchors.top: parent.top
        anchors.topMargin: 4
        anchors.right: timeText.left
        anchors.rightMargin: 4
        text: ConsoleModel.fpsString + " |"
        font.family: openSans.name
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
    }

    NativeText {
        id: timeText
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
        anchors.bottom: parent.bottom
        handleColor: colors.black95
        handleSize: 1
    }
}
