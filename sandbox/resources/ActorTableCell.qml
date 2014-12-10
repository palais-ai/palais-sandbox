import QtQuick 2.0

Rectangle {
    id: actorCell

    property color cellColor: colors.primaryColor
    property color cellColorLighter: colors.primaryColorLighter
    signal pressed()

    opacity: 0.8
    gradient: Gradient {
             GradientStop { position: 1.0; color: cellColor }
             GradientStop { position: 0.0; color: cellColorLighter }
    }

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
        anchors.left: parent.left
        anchors.leftMargin: 10
        text: name
        font.family: openSans.name
        font.pointSize: 12
        color: "white"
        renderType: Text.NativeRendering
    }

    Text {
        id: actorHideButton
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: arrowRightIndicator.left
        anchors.rightMargin: 10
        text: "\uf06e"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: "white"
        renderType: Text.NativeRendering

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
        anchors.right: parent.right
        anchors.rightMargin: 10
        text: "\uf105"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: "white"
        renderType: Text.NativeRendering
    }

    states: [
        State {
            name: 'HOVER'
            PropertyChanges {
                target: actorCell
                cellColor: colors.secondaryColor
            }
            PropertyChanges {
                target: actorCell
                cellColorLighter: colors.secondaryColorLighter
            }
        }
    ]
}
