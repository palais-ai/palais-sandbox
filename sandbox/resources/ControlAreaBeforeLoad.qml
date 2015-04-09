import QtQuick 2.0

Rectangle {
    color: colors.darkGray
    CallToActionButton {
        id: openButton
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 8*2
        text: "New Project"
        onPressed: newProjectDialog.open()
    }

    CallToActionButton {
        anchors.top: openButton.bottom
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 8*2
        text: "Open Project"
        onPressed: openProjectDialog.open()
    }
}
