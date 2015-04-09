import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    id: viewControls
    width: childLayout.width
    height: childLayout.height
    color: "transparent"

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
        focus: true

        FAIconButton {
            id: zoomOutButton
            objectName: "zoomOutButton"
            unicode: "\uf010"
            onPressed: performZoom()

            function performZoom() {
                qoItem.camera.zoom = Math.min(6, Math.max(0.1, qoItem.camera.zoom + 0.25))
            }
        }

        FAIconButton {
            id: zoomInButton
            objectName: "zoomInButton"
            unicode: "\uf00e"
            onPressed: performZoom()

            function performZoom() {
                qoItem.camera.zoom = Math.min(6, Math.max(0.1, qoItem.camera.zoom - 0.25))
            }
        }
    }
}
