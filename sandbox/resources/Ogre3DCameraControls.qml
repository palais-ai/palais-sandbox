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
        focus: true
        Keys.onPressed: {
            switch(event.key) {
            case Qt.Key_Left:
                appWindow.changeControlAreaState()
                break;
            case Qt.Key_Right:
                appWindow.changeInspectorState()
                break;
            case Qt.Key_Down:
                appWindow.changeConsoleState()
                break;
            case Qt.Key_Minus:
                zoomOutButton.performZoom()
                event.accepted = true
                break;
            case Qt.Key_Plus:
                zoomInButton.performZoom()
                event.accepted = true
                break;
            case Qt.Key_P:
                if(ApplicationWrapper.sceneLoaded) {
                    ApplicationWrapper.onPlayButtonPressed()
                    event.accepted = true
                }
                break;
            }
        }

        FAIconButton {
            id: zoomOutButton
            unicode: "\uf010"
            onPressed: performZoom()

            function performZoom() {
                ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom + 0.25))
            }
        }

        FAIconButton {
            id: zoomInButton
            unicode: "\uf00e"
            onPressed: performZoom()

            function performZoom() {
                ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom - 0.25))
            }
        }
    }
}
