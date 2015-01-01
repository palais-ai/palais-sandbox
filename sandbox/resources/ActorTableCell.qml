import QtQuick 2.0

Rectangle {
    id: actorCell
    signal pressed()
    property color textColor: colors.lightGray
    property int horizontalMargin: 8

    color: colors.darkGray

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

    NativeText {
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -separator.height
        anchors.left: parent.left
        anchors.leftMargin: horizontalMargin
        text: name
        //font.capitalization: Font.AllUppercase
        font.family: openSans.name
        //font.weight: Font.DemiBold
        font.pointSize: 11
        color: textColor
    }

    NativeText {
        id: actorHideButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -separator.height
        anchors.right: arrowRightIndicator.left
        anchors.rightMargin: horizontalMargin
        text: "\uf06e"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: textColor
        state: visible ? '' : 'HIDDEN'

        MouseArea {
            anchors.fill: parent
            onClicked: {
                var actor = Scene.getActor(index)
                if(actorHideButton.state == '') {
                    actor.hide()
                    actorHideButton.state = 'HIDDEN'
                } else {
                    actor.show()
                    actorHideButton.state = ''
                }
            }
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

    NativeText {
        id: arrowRightIndicator
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -separator.height
        anchors.right: parent.right
        anchors.rightMargin: horizontalMargin
        text: "\uf105"
        font.family: fontAwesome.name
        font.pointSize: 12
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
                target: actorCell
                color: colors.darkerGray
            }
        }
    ]
}
