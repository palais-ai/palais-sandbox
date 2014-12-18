import QtQuick 2.0

Rectangle {
    width: parent.width
    height: 2

    Rectangle {
        color: colors.black95
        width: parent.width
        anchors.top: parent.top
        height: 1
    }

    Rectangle {
        color: colors.black75
        width: parent.width
        anchors.bottom: parent.bottom
        height:1
    }
}
