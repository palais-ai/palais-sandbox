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
    color: colors.gray

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
                enabled: ApplicationWrapper.sceneLoaded

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
        property color primaryColor: "#3498db"
        property color primaryColorDarker: "#2980b9"
        property color primaryColorDarkest: "#006096"
        property color primaryColorLighter: "#4dcaff"
        property color secondaryColor: "#467b24"
        property color secondaryColorLighter: "#7cef09"
        property color dimmedTextColor: "#3Dffffff"
        property color slightlyDimmedTextColor: "#CCffffff"
        property color stronglyDimmedTextColor: "#40ffffff"
        property color lightGray: '#a7a7a7'
        property color gray: '#4c4c4c'
        property color darkGray: '#3b3b3b'
        property color darkerGray: '#303030'
        property color black95: '#111111'
        property color black75: '#555555'
    }

    Item {
        id: sizes
        property int sidebarWidth: 210
        property int inspectorWidth: sidebarWidth
        property int sidebarCellHeight: 30
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

    FontLoader {
        id: sourceSans
        source: "qrc:/fonts/source-sans/SourceCodePro-Regular.ttf"
    }


    Item {
        anchors.fill: parent

        ControlArea {
            id: controlArea
            height: parent.height
            width: sizes.sidebarWidth

            Behavior on width {
                NumberAnimation {
                    id: controlWidthAnimation
                    duration: 120
                }
            }

            states: State {
                name: "HIDDEN"
                PropertyChanges {
                    target: controlArea
                    width: 1
                }
            }
        }

        SplitView {
            id: centerArea
            height: parent.height
            width: parent.width - controlArea.width - inspectorArea.width
            anchors.left: controlArea.right
            anchors.top: parent.top
            orientation: Qt.Vertical

            handleDelegate: ListSeparator {
                handleSize: 1
                handleColor: colors.black95
            }

            onResizingChanged: {
                consoleArea.state = ''
            }

            Loader {
                id: mainLoader
                Layout.minimumHeight: 400
                Layout.fillHeight: true
                focus: true
                source: "loading.qml"
            }

            ConsoleArea {
                id: consoleArea
                z: mainLoader.z - 1
                Layout.minimumHeight: 150

                /**
                Behavior on height {
                    NumberAnimation {
                        id: consoleHeightAnimation
                        duration: 120
                    }
                }*/

                states: State {
                    name: "HIDDEN"
                    PropertyChanges {
                        target: consoleArea
                        Layout.minimumHeight: 2
                    }
                    PropertyChanges {
                        target: consoleArea
                        height: 2
                    }
                }
            }
        }

        InspectorArea {
            id: inspectorArea
            height: parent.height
            width: sizes.inspectorWidth
            anchors.left: centerArea.right
            anchors.top: parent.top

            Behavior on width {
                NumberAnimation {
                    id: inspectorWidthAnimation
                    duration: 120
                }
            }

            states: State {
                name: "HIDDEN"
                PropertyChanges {
                    target: inspectorArea
                    width: 1
                }
            }
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

    function changeControlAreaState() {
        if(!controlWidthAnimation.running)
            controlArea.state = controlArea.state === '' ? "HIDDEN" : ''
    }

    function changeConsoleState() {
        //if(!consoleHeightAnimation.running)
        consoleArea.state = consoleArea.state === '' ? "HIDDEN" : ''
    }

    function changeInspectorState() {
        if(!inspectorWidthAnimation.running)
            inspectorArea.state = inspectorArea.state === '' ? "HIDDEN" : ''
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

