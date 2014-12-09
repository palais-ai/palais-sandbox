import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    id: headerColumn
    property real headerPadding: 8
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
        Layout.alignment: Layout.Center
        text: ApplicationWrapper.sceneLoaded ? "Scenario" : "No Scene Is Loaded Yet."
        font.family: openSans.name
        font.capitalization: Font.SmallCaps
        font.pointSize: 14
        color: "white"
        anchors.top: pad1.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        renderType: Text.NativeRendering

        //TextShadow {}
    }

    Text {
        id: scenarioName
        text: Scene.name
        font.family: openSans.name
        font.pointSize: 16
        color: "white"
        maximumLineCount: 1
        elide: Text.ElideRight
        clip: true
        anchors.top: scenarioText.bottom
        anchors.horizontalCenter: scenarioText.horizontalCenter
        renderType: Text.NativeRendering

        //TextShadow {}
    }

    Rectangle {
        width: parent.width
        height: headerColumn.headerPadding
        color: "transparent"
        anchors.top: scenarioName.bottom
        anchors.left: parent.left
    }
}
