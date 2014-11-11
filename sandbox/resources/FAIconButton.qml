import QtQuick 2.0

Text {
    id: icon

    signal pressed()
    property string unicode: "\uf00e"

    font.family: fontAwesome.name
    font.pointSize: 12
    text: unicode
    color: "#3Dffffff"
    renderType: Text.NativeRendering

    states: State {
        name: "HOVER"
        PropertyChanges { target: icon; color: "#CCffffff"}
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: icon.pressed()
        onEntered: icon.state = 'HOVER'
        onExited: icon.state = ''
    }
}
