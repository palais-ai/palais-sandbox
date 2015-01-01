import QtQuick 2.0
import SceneModel 1.0

Rectangle {
    id: actorCell
    signal pressed()
    property color textColor: colors.lightGray
    property int horizontalMargin: 8

    color: colors.darkGray

    onPressed: {
        ActorModel.setSelectedRequested(index, !isSelected);
    }

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
        anchors.right: parent.right
        anchors.rightMargin: horizontalMargin
        text: "\uf06e"
        font.family: fontAwesome.name
        font.pointSize: 12
        color: textColor

        MouseArea {
            anchors.fill: parent
            onClicked: ActorModel.setVisibleRequested(index, !isVisible)
        }

        Connections {
            target: ActorModel
            onModelChanged: {
                if(idx == index) {
                    actorHideButton.state = visible ? '' : 'HIDDEN'
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
