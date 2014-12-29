import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    id: windowControls
    width: backdrop.width
    height: backdrop.height
    color: "transparent"

    Rectangle {
        id: backdrop
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
            id: controlAreaExpander
            unicode: "\uf053"
            color: controlArea.state === '' ? colors.primaryColor : colors.dimmedTextColor
            onPressed: appWindow.changeControlAreaState()
        }

        FAIconButton {
            id: consoleExpander
            unicode: "\uf078"
            color: consoleArea.state === '' ? colors.primaryColor : colors.dimmedTextColor
            onPressed: appWindow.changeConsoleState()
        }

        FAIconButton {
            id: inspectorAreaExpander
            unicode: "\uf054"
            color: inspectorArea.state === '' ? colors.primaryColor : colors.dimmedTextColor
            onPressed: appWindow.changeInspectorState()
        }

    }
}
