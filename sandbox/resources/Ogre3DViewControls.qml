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
            if(event.key ===
               (viewControls.state == '' ? Qt.Key_Left : Qt.Key_Right)) {
                expandButton.changeState()
            }

            if (event.key === Qt.Key_Minus) {
                zoomOutButton.performZoom()
                event.accepted = true
            }

            if (event.key === Qt.Key_Plus) {
                zoomInButton.performZoom()
                event.accepted = true
            }

            if(event.key == Qt.Key_P && ApplicationWrapper.sceneLoaded) {
                ApplicationWrapper.onPlayButtonPressed()
                event.accepted = true
            }
        }

        FAIconButton {
            id: expandButton
            unicode: "\uf060"
            onPressed: changeState()

            function changeState() {
                if(!widthAnimation.running)
                    viewControls.state =  viewControls.state == '' ? "HIDDEN" : ''
            }
        }

        FAIconButton {
            id: zoomOutButton
            unicode: "\uf010"
            onPressed: performZoom()

            function performZoom() {
                ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom - 0.25))
            }
        }

        FAIconButton {
            id: zoomInButton
            unicode: "\uf00e"
            onPressed: performZoom()

            function performZoom() {
                ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom + 0.25))
            }
        }
    }

    states: State {
        name: "HIDDEN"
        PropertyChanges { target: controlArea; width: 0 }
        PropertyChanges { target: expandButton; unicode: "\uf061" }
    }
}
