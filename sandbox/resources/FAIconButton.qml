import QtQuick 2.0

Text {
    id: icon

    signal pressed()
    property string unicode: "\uf00e"
    property real iconSize: 12

    font.family: fontAwesome.name
    font.pointSize: iconSize
    text: unicode
    color: icon.enabled ? colors.dimmedTextColor : colors.stronglyDimmedTextColor
    //renderType: Text.NativeRendering
    height: iconSize
    width: iconSize
    clip: true

    states: [
        State {
                name: "HOVER"
                PropertyChanges { target: icon; color: colors.slightlyDimmedTextColor }
        },
        State {
                name: "DISABLED"
                PropertyChanges { target: icon; color: colors.stronglyDimmedTextColor }
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if(icon.enabled) icon.pressed()
        }
        onEntered: {
            if(icon.enabled) icon.state = 'HOVER'
        }
        onExited: {
            if(icon.enabled) icon.state = ''
        }
    }

    onEnabledChanged: {
        icon.state = icon.enabled ? icon.state = '' : icon.state = 'DISABLED';
    }
}
