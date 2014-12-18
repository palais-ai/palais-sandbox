import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    id: headerColumn
    property real headerPadding: 16
    height: childrenRect.height
    width: parent.width
    color: "transparent"

    Rectangle {
        id: pad1
        width: parent.width
        height: headerColumn.headerPadding
        color: "transparent"
        anchors.top: parent.top
        anchors.left: parent.left
    }

    Text {
        id: scenarioText
        Layout.alignment: Layout.Left
        text: ApplicationWrapper.sceneLoaded ? "Scenario" : "No Scene Is Loaded Yet."
        font.family: openSans.name
        font.capitalization: Font.AllUppercase
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
        anchors.top: pad1.bottom
        anchors.left: parent.left
        anchors.leftMargin: 8
        //renderType: Text.NativeRendering

        //TextShadow {}
    }

    Text {
        id: scenarioName
        text: Scene.name
        font.family: openSans.name
        font.capitalization: Font.AllUppercase
        font.pointSize: 16
        font.weight: Font.DemiBold
        color: "white"
        maximumLineCount: 1
        elide: Text.ElideRight
        clip: true
        anchors.top: scenarioText.bottom
        anchors.topMargin: -5
        anchors.left: parent.left
        anchors.leftMargin: 8
        //renderType: Text.NativeRendering

        //TextShadow {}
    }

    Rectangle {
        id: pad2
        width: parent.width
        height: ApplicationWrapper.sceneLoaded ? headerColumn.headerPadding : 0
        color: "transparent"
        anchors.top: scenarioName.bottom
        anchors.left: parent.left
    }

    ListSeparator {
        anchors.top: pad2.bottom
    }
}
