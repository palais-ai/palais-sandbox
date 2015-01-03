import QtQuick 2.0

Rectangle {
    property real headerPadding: 8
    property bool isSizeInformationVisible: inspectorListView.count === 0

    id: headerColumn
    height: isSizeInformationVisible ?
                childrenRect.height + scenarioText.anchors.topMargin :
                childrenRect.height - emptyLine.height - emptySeparator.height - headerPadding - 2
    width: parent.width
    color: "transparent"

    NativeText {
        id: scenarioText
        text: "Knowledge"
        font.family: openSans.name
        font.capitalization: Font.AllUppercase
        font.pointSize: 12
        font.weight: Font.Light
        color: colors.lightGray
        anchors.top: parent.top
        anchors.topMargin: headerPadding - 2
        anchors.left: parent.left
        anchors.leftMargin: 8
    }

    NativeText {
        id: scenarioName
        width: headerColumn.width - anchors.leftMargin*2
        text: InspectorModel.name
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

    ListSeparator {
        id: emptySeparator
        anchors.top: scenarioName.bottom
        anchors.topMargin: headerPadding
        visible: ApplicationWrapper.sceneLoaded
    }

    Item {
        id: emptyLine
        anchors.top: emptySeparator.bottom
        anchors.topMargin: headerPadding
        anchors.left: parent.left
        anchors.leftMargin: 8
        width: childrenRect.width
        height: childrenRect.height
        visible: isSizeInformationVisible

        NativeText {
            id: emptyIcon
            anchors.verticalCenter: emptyText.verticalCenter
            anchors.left: parent.left
            text: "\uf05a"
            font.family: fontAwesome.name
            font.pointSize: 12
            color: colors.lightGray
            visible: isSizeInformationVisible
        }

        NativeText {
            id: emptyText
            width: headerColumn.width - 8*3 - emptyIcon.width
            wrapMode: Text.WordWrap
            anchors.top: parent.top
            anchors.left: emptyIcon.right
            anchors.leftMargin: 8
            text: "Contains " + inspectorListView.count + " knowledge items."
            font.family: openSans.name
            font.pointSize: 12
            color: colors.lightGray
            visible: isSizeInformationVisible
        }
    }

    ListSeparator {
        anchors.top: emptyLine.bottom
        anchors.topMargin: headerPadding
        visible: isSizeInformationVisible
    }
}
