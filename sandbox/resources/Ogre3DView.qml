import QtQuick 2.0
import Ogre 1.0

Rectangle {
    id: ogre

    Camera {
        id: cam1
        objectName: "cam1"
    }

    OgreItem {
        id: ogreitem
        anchors.fill: parent;
        camera: cam1
        ogreEngine: OgreEngine
        backgroundColor: colors.gray

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            property int prevX: -1
            property int prevY: -1

            onPositionChanged: {
                if (pressedButtons & Qt.LeftButton) {
                    if (prevX > -1) {
                        ogreitem.camera.yaw(-(mouse.x - prevX) / 2)
                    }
                    if (prevY > -1) {
                        ogreitem.camera.pitch(-(mouse.y - prevY) / 2)
                    }
                    prevX = mouse.x
                    prevY = mouse.y
                }
                if (pressedButtons & Qt.RightButton) {
                    if (prevY > -1) {
                        var cameraMovement = ogreitem.camera.zoom + (mouse.y - prevY) / 200
                        var zoomLevel = Math.min(6, Math.max(0.1, cameraMovement));
                        ogreitem.camera.zoom = zoomLevel
                    }

                    prevY = mouse.y
                }
            }
            onReleased: {
                if(prevX == -1 && prevY == -1) {
                    // No dragging has happened between press and release.
                    // We interpret this as a click on the 3D view.
                    // Coordinates must be in normalized screen coordinates.
                    var normX = (mouse.x + ogre.x) / ogre.width
                    var normY = (mouse.y + ogre.y) / ogre.height
                    ApplicationWrapper.onOgreViewClicked(normX, normY)
                } else {
                    prevX = -1
                    prevY = -1
                }
            }
        }

        MainWindowControls {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 14
        }

        Ogre3DCameraControls {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 14
            anchors.leftMargin: 14
        }

        Rectangle {
            color: "#80000000"
            width: playbackControls.width + 48
            height: playbackControls.height + 24
            radius: 3
            anchors.centerIn: playbackControls
        }

        Ogre3DViewPlaybackControls {
            id: playbackControls
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: parent.height * 0.10
        }
    }
}

