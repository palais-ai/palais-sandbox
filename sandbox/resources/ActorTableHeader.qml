import QtQuick 2.0
import QtQuick.Layouts 1.1

Rectangle {
    id: headerColumn
    height: childrenRect.height
    width: parent.width
    color: "transparent"

    property real headerPadding: 8

    // Need actual elements for padding to make childrenRect calculate the proper height.
    Rectangle {
        id: pad1
        width: parent.width
        height: headerColumn.headerPadding - 2
        color: "transparent"
        anchors.top: parent.top
        anchors.left: parent.left
    }

    NativeText {
        id: scenarioText
        Layout.alignment: Layout.Left
        text: ApplicationWrapper.sceneLoaded ? "Scenario" : "No Scene Is Loaded Yet."
        font.family: openSans.name
        font.capitalization: Font.AllUppercase
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
        anchors.top: pad1.bottom
        anchors.topMargin: headerColumn.headerPadding - 2
        anchors.left: parent.left
        anchors.leftMargin: 8
    }

    NativeText {
        id: scenarioName
        width: headerColumn.width - anchors.leftMargin*2
        text: ActorModel.name
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
