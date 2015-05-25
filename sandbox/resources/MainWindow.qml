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
    title: "palais"
    color: colors.gray

    minimumWidth: 1024
    minimumHeight: 660

    property bool isOgreInitialized: false

    menuBar: MenuBar {
        Menu {
            title: "File"
            enabled: isOgreInitialized

            MenuItem {
                text: "New project"
                shortcut: "Ctrl+N"

                onTriggered: newProjectDialog.open()
            }

            MenuItem {
                text: "Open project"
                shortcut: "Ctrl+O"

                onTriggered: openProjectDialog.open()
            }

            MenuItem {
                text: "Reload project"
                shortcut: "Ctrl+L"
                enabled: ApplicationWrapper.sceneLoaded
                onTriggered: ProjectManager.reloadProject()
            }
        }
        Menu {
            title: "Edit"

            MenuItem {
                text: "Play / Pause simulation"
                shortcut: "Ctrl+R"
                enabled: ApplicationWrapper.sceneLoaded
                onTriggered: ApplicationWrapper.onPlayButtonPressed()
            }

            MenuItem {
                id: speedMenuItem
                text: "Set maximum simulation speed"
                shortcut: "Ctrl+E"
                enabled: ApplicationWrapper.sceneLoaded
            }

            MenuItem {
                text: "Unselect actor"
                shortcut: "Ctrl+U"
                enabled: ApplicationWrapper.sceneLoaded && ActorModel.actorSelected
                onTriggered: ProjectManager.unselectActor()
            }
        }
        Menu {
            title: "View"

            MenuItem {
                text: "Save rendering"
                shortcut: "Ctrl+C"
                enabled: ApplicationWrapper.sceneLoaded
                onTriggered: saveRenderingDialog.open()
            }

            MenuItem {
                text: "Zoom in"
                shortcut: "+"
                enabled: ApplicationWrapper.sceneLoaded
                onTriggered: ApplicationWrapper.onZoomIn()
            }

            MenuItem {
                text: "Zoom out"
                shortcut: "-"
                enabled: ApplicationWrapper.sceneLoaded
                onTriggered: ApplicationWrapper.onZoomOut()
            }

            MenuItem {
                text: "Show / Hide Scene Overview"
                shortcut: "Left"
                onTriggered: appWindow.changeControlAreaState()
            }

            MenuItem {
                text: "Show / Hide Knowledge Inspector"
                shortcut: "Right"
                onTriggered: appWindow.changeInspectorState()
            }

            MenuItem {
                text: "Show / Hide Console"
                shortcut: "Down"
                onTriggered: appWindow.changeConsoleState()
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
        property color almostWhite: "#eeffffff"
        property color slightlyDimmedTextColor: "#CCffffff"
        property color stronglyDimmedTextColor: "#40ffffff"
        property color lightGray: '#a7a7a7'
        property color gray: '#4c4c4c'
        property color darkGray: '#3b3b3b'
        property color darkerGray: '#303030'
        property color black90: '#1a1a1a'
        property color black95: '#111111'
        property color black75: '#555555'
        property color transparent: '#00ffffff'
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

        Column {
            id: centerArea
            height: parent.height
            width: parent.width - controlArea.width - inspectorArea.width
            anchors.left: controlArea.right
            anchors.top: parent.top

            Loader {
                id: mainLoader
                height: parent.height - consoleArea.height
                width: parent.width
                focus: true
                source: "loading.qml"
            }

            ConsoleArea {
                id: consoleArea
                z: mainLoader.z - 1
                height: 200
                width: parent.width

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

    Dialog {
        id: newProjectDialog
        objectName: "newProjectDialog"
        title: "Create a new project"
        visible: false
        standardButtons: StandardButton.Ok | StandardButton.Cancel
        width: 420
        height: 165

        Rectangle {
            id: projectForm
            width: 300
            height: parent.height
            clip: false

            FormRow {
                id: nameRow
                anchors.top: parent.top
                key: "Name"
                value: "my_scene"
            }

            FormRow {
                id: logicRow
                anchors.top: nameRow.bottom
                anchors.topMargin: 12
                key: "Main Logic File"
                value: "main.js"
            }

            FormRow {
                id: directoryRow
                anchors.top: logicRow.bottom
                anchors.topMargin: 12
                key: "DIRECTORY"
                value: ""
                editable: false
            }

            CallToActionButton {
                text: "choose"
                height: directoryRow.height - 3
                width: newProjectDialog.width - projectForm.width - 32
                anchors.left: projectForm.right
                anchors.leftMargin: 8
                anchors.verticalCenter: directoryRow.verticalCenter

                onPressed: {
                    newProjectDialog.close()
                    newProjectDirectoryDialog.open()
                }
            }
        }

        signal createdNewProject(string name, string logicFileName, string directory)

        onAccepted: newProjectDialog.createdNewProject(nameRow.value,
                                                       logicRow.value,
                                                       directoryRow.value)
    }

    FileDialog {
        id: newProjectDirectoryDialog
        title: "Please choose a directory to create your project in"
        visible: false
        selectMultiple: false
        selectFolder: true

        onAccepted: {
            newProjectDialog.open()
            directoryRow.value = newProjectDirectoryDialog.fileUrl
        }
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

    FileDialog {
        id: saveRenderingDialog
        objectName: "saveRenderingDialog"
        title: "Please choose a file"
        visible: false
        selectMultiple: false
        selectExisting: false

        signal renderingFileSelected(url fileurl)

        onAccepted: {
            saveRenderingDialog.renderingFileSelected(saveRenderingDialog.fileUrl)
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

