import QtQuick 2.0

Rectangle {
    id: actorCell
    signal pressed()
    property color textColor: colors.lightGray
    color: colors.darkGray

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: actorCell.pressed()
        onEntered: {
            actorCell.state = 'HOVER'
            Scene.toggleHighlight(true, index);
        }
        onExited: {
            actorCell.state = ''
            Scene.toggleHighlight(false, index);
        }
    }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -2
        anchors.left: parent.left
        anchors.leftMargin: 8
        text: name
        font.family: openSans.name
        font.pointSize: 12
        color: textColor
        //renderType: Text.NativeRendering
    }

    Text {
        id: actorHideButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -2
        anchors.right: arrowRightIndicator.left
        anchors.rightMargin: 8
        text: "\uf06e"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: textColor
        //renderType: Text.NativeRendering

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

    Text {
        id: arrowRightIndicator
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -2
        anchors.right: parent.right
        anchors.rightMargin: 8
        text: "\uf105"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: textColor
        //renderType: Text.NativeRendering
    }

    ListSeparator {
        anchors.bottom: parent.bottom
    }

    states: [
        State {
            name: 'HOVER'
            PropertyChanges {
                target: actorCell
                textColor: "white"
            }
        }
    ]
}
