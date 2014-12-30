import QtQuick 2.0

Rectangle {
    signal pressed()
    property string text: "PLACEHOLDER"

    id: callToActionButton
    color: colors.primaryColor
    width: 200
    height: 40
    radius: 2

    NativeText {
        id: buttonText
        anchors.centerIn: parent
        color: "white"
        font.family: openSans.name
        font.pointSize: 12
        font.weight: Font.Bold
        font.capitalization: Font.AllUppercase
        text: parent.text

        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }

    Rectangle {
        id: flatShadow
        z: -1
        color: colors.primaryColorDarker
        width: parent.width
        height: parent.height
        radius: parent.radius
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 2

        Behavior on color {
            ColorAnimation { duration: 200 }
        }
    }

    MouseArea {
        id: buttonMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: callToActionButton.pressed()
    }

    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    states: [
        State {
            name: 'HOVER'
            when: buttonMouseArea.containsMouse
            PropertyChanges {
                target: callToActionButton
                color: colors.primaryColorDarker
            }
            PropertyChanges {
                target: buttonText
                color: "#e5e5e5"
            }
            PropertyChanges {
                target: flatShadow
                color: colors.primaryColorDarkest
            }
        }
    ]
}
