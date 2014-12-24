import QtQuick 2.0
import QtQuick.Window 2.0
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

    property bool isOgreInitialized: false

    menuBar: MenuBar {
        Menu {
            title: "File"
            enabled: isOgreInitialized

            MenuItem {
                text: "Open project"
                shortcut: "Ctrl+O"

                onTriggered: openProjectDialog.open()
            }

            MenuItem {
                text: "Save"
                shortcut: "Ctrl+S"
            }

            MenuItem {
                text: "Reload project"
                shortcut: "Ctrl+L"

                onTriggered: ProjectManager.reloadProject();
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
        property color stronglyDimmedTextColor: "#40ffffff"
        property color lightGray: '#a7a7a7'
        property color darkGray: '#3b3b3b'
        property color darkerGray: '#303030'
        property color black95: '#111111'
        property color black75: '#555555'
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
        source: "qrc:/fonts/open-sans/OpenSans-Light.ttf"
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

            Behavior on width { NumberAnimation {  id:widthAnimation; duration: 120 } }
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

    MessageDialog {
        id: errorDialog
        title: "Error"
        icon: StandardIcon.Critical
        visible: false
        standardButtons: StandardButton.Ok
    }

    FileDialog {
        id: openProjectDialog
        objectName: "openProjectDialog"
        title: "Please choose a file"
        visible: false
        selectMultiple: false

        signal projectFileSelected(url fileurl)

        onAccepted: {
            openProjectDialog.projectFileSelected(openProjectDialog.fileUrl)
        }
    }

    function onOgreIsReady() {
        mainLoader.source = "Ogre3DView.qml"
        isOgreInitialized = true
    }

    function showErrorMessage(message) {
       errorDialog.text = message
       errorDialog.open()
    }

    onClosing: Qt.quit()
}

