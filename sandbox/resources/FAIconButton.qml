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
    renderType: Text.NativeRendering

    states: State {
        name: "HOVER"
        PropertyChanges { target: icon; color: colors.slightlyDimmedTextColor }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: icon.pressed()
        onEntered: icon.state = 'HOVER'
        onExited: icon.state = ''
    }
}
