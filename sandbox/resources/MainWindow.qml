import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: appWindow
    width: 1024
    height: 660
    visible: true
    title: "AI Sandbox"
    color: "black"

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
        property color primaryColor: "#0355a9"
        property color primaryColorLighter: "#4685e5"
        property color secondaryColor: "#467b24"
        property color secondaryColorLighter: "#7cef09"
        property color dimmedTextColor: "#3Dffffff"
        property color slightlyDimmedTextColor: "#CCffffff"
    }

    FontLoader {
        id: openSans
        source: "qrc:/fonts/open-sans/OpenSans-Regular.ttf"
    }

    FontLoader {
        source: "qrc:/fonts/open-sans/OpenSans-Bold.ttf"
    }

    FontLoader {
        source: "qrc:/fonts/open-sans/OpenSans-Semibold.ttf"
    }

    FontLoader {
        id: fontAwesome
        source: "qrc:/fonts/FontAwesome/fontawesome-webfont.ttf"
    }


    Item {
        anchors.fill: parent
        //spacing: 0

        ControlArea {
            id: controlArea
            height: parent.height
            width: 210

            Behavior on width { NumberAnimation {  id:widthAnimation; duration: 180 } }
        }

        Loader {
            id: mainLoader
            height: parent.height
            width: parent.width - controlArea.width
            anchors.left: controlArea.right
            anchors.top: parent.top
            focus: true
            source: "loading.qml"
        }
    }

    Dialog {
        id: testDialog
        visible: false
        title: "Blue sky dialog"
        standardButtons: StandardButton.Save | StandardButton.Cancel
    }

    function onOgreIsReady() {
        mainLoader.source = "Ogre3DView.qml"
    }

    onClosing: Qt.quit()
}

