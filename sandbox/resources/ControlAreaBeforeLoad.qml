import QtQuick 2.0

Rectangle {
    color: colors.darkGray
    CallToActionButton {
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 8*2
        text: "Load Project"
        onPressed: openProjectDialog.open()
    }
}
