import QtQuick 2.0

Rectangle {
    height: childrenRect.height + separator.anchors.topMargin
    color: colors.darkGray

    NativeText {
        id: consoleText
        text: ApplicationWrapper.sceneLoaded ? "Console" : "No Scene Is Loaded Yet."
        font.family: openSans.name
        font.capitalization: Font.AllUppercase
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.left: parent.left
        anchors.leftMargin: 8
    }

    NativeText {
        id: timeText
        text: ConsoleModel.passedTimeString
        font.family: openSans.name
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.right: parent.right
        anchors.rightMargin: 8
    }

    ListSeparator {
        id: separator
        anchors.top: consoleText.bottom
        anchors.topMargin: 2
        handleColor: colors.black95
        handleSize: 1
    }
}
