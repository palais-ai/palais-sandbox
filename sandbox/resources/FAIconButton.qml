import QtQuick 2.0

Text {
    id: icon

    signal pressed()
    property string unicode: "\uf00e"
    property real iconSize: 12

    font.family: fontAwesome.name
    font.pointSize: iconSize
    text: unicode
    color: colors.dimmedTextColor
    //renderType: Text.NativeRendering
    height: iconSize
    width: iconSize
    clip: true

    states: [
        State {
                name: "HOVER"
                when: iconMouseArea.containsMouse
                PropertyChanges { target: icon; color: colors.slightlyDimmedTextColor }
        },
        State {
                name: "DISABLED"
                when: !icon.enabled
                PropertyChanges { target: icon; color: colors.stronglyDimmedTextColor }
        }
    ]

    MouseArea {
        id: iconMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if(icon.enabled) icon.pressed()
        }
    }
}
