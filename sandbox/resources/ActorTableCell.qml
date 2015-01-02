import QtQuick 2.0
import SceneModel 1.0

Rectangle {
    id: actorCell
    signal pressed()
    property color textColor: colors.lightGray
    property int horizontalMargin: 8

    color: colors.darkGray
    border.color: colors.primaryColor
    border.width: isSelected ? 1 : 0

    onPressed: ActorModel.setSelectedRequested(index, !isSelected)

    Behavior on color {
        ColorAnimation {
            duration: 200
        }
    }

    MouseArea {
        id: cellMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: actorCell.pressed()
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        height: parent.height
        width: 4
        color: colors.primaryColor
        visible: isSelected
    }

    NativeText {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: horizontalMargin
        text: name
        font.family: openSans.name
        font.pointSize: 11
        color: textColor
    }

    NativeText {
        id: actorHideButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: horizontalMargin
        text: "\uf06e"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: textColor
        state: isVisible ? '' : 'HIDDEN'

        MouseArea {
            anchors.fill: parent
            onClicked: ActorModel.setVisibleRequested(index, !isVisible)
        }

        states: [
            State {
                name: 'HIDDEN'
                PropertyChanges {
                    target: actorHideButton
                    text: "\uf070"
                }
                PropertyChanges {
                    target: actorHideButton
                    color: colors.stronglyDimmedTextColor
                }
            }
        ]
    }

    ListSeparator {
        id: separator
        anchors.bottom: parent.bottom
        handleColor: isSelected ? colors.primaryColor : colors.darkerGray
    }

    states: [
        State {
            name: 'HOVER'
            when: cellMouseArea.containsMouse
            PropertyChanges {
                target: actorCell
                color: colors.darkerGray
            }
        }
    ]
}
