import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: appWindow
    width: 1024
    height: 660
    visible: true
    title: "AI Sandbox"

    minimumWidth: 1024
    minimumHeight: 660

    menuBar: MenuBar {
        Menu {
            title: "File"

            MenuItem {
                text: "Open"
                shortcut: "Ctrl+O"
            }

            MenuItem {
                text: "Save"
                shortcut: "Ctrl+S"
            }
        }
        Menu {
            title: "Edit"

            MenuItem {
                text: "Cut"
                shortcut: "Ctrl+X"
            }

            MenuItem {
                text: "Copy"
                shortcut: "Ctrl+C"
            }

            MenuItem {
                text: "Paste"
                shortcut: "Ctrl+V"
            }
        }
    }

    Item {
        id: colors
        property color primaryColor: "#4affff"
        property color secondaryColor: "#4685e5"
    }

    FontLoader {
        id: openSans
        source: "qrc:/fonts/open-sans/OpenSans-Regular.ttf"
    }

    FontLoader {
        id: fontAwesome
        source: "qrc:/fonts/FontAwesome/fontawesome-webfont.ttf"
    }


    RowLayout {
        anchors.fill: parent
        spacing: 0

        ControlArea {
            id: controlArea
            Layout.fillHeight: true
            width: 215

            Behavior on width { NumberAnimation { duration: 1000 } }
        }

        Loader {
            id: mainLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: "loading.qml"
        }
    }

    function onOgreIsReady() {
        mainLoader.source = "Ogre3DView.qml"
    }

    onClosing: Qt.quit()
}
