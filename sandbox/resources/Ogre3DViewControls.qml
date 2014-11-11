import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    id: viewControls

    Rectangle {
        color: "#80000000"
        width: childLayout.width + 12
        height: childLayout.height + 12
        anchors.centerIn: childLayout
        radius: 3
    }

    RowLayout {
        id: childLayout
        spacing: 12

        FAIconButton {
            id: expandButton
            unicode: "\uf060"
            onPressed: viewControls.state == '' ? viewControls.state = "HIDDEN" : viewControls.state = ''
        }

        FAIconButton {
            id: zoomOutButton
            unicode: "\uf010"
            onPressed: ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom - 0.25))
        }

        FAIconButton {
            id: zoomInButton
            unicode: "\uf00e"
            onPressed: ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom + 0.25))
        }
    }

    states: State {
        name: "HIDDEN"
        PropertyChanges { target: controlArea; visible: "false" }
        PropertyChanges { target: expandButton; unicode: "\uf061" }
    }
}
