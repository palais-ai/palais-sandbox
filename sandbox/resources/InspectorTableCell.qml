import QtQuick 2.0

Rectangle {
    id: inspectorCell
    signal pressed()
    property color textColor: colors.lightGray
    property int horizontalMargin: 8

    color: colors.darkGray

    function typeNameForObject(value) {
        if(value instanceof Object && value.hasOwnProperty("objectName")) {
            return value.objectName
        }
        return typeof(value)
    }

    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    MouseArea {
        id: cellMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: inspectorCell.pressed()
    }

    NativeText {
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -separator.height
        anchors.left: parent.left
        anchors.leftMargin: horizontalMargin
        text: key
        font.family: openSans.name
        font.pointSize: 11
        color: textColor
    }

    NativeText {
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -separator.height
        anchors.right: parent.right
        anchors.rightMargin: horizontalMargin
        text: typeNameForObject(value)
        font.family: openSans.name
        font.pointSize: 11
        color: textColor
    }

    ListSeparator {
        id: separator
        anchors.bottom: parent.bottom
    }

    states: [
        State {
            name: 'HOVER'
            when: cellMouseArea.containsMouse
            PropertyChanges {
                target: inspectorCell
                color: colors.darkerGray
            }
        }
    ]
}
