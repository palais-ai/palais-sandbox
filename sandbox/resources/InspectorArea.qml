import QtQuick 2.0

Rectangle {
    color: colors.darkGray

    ListView {
        id: inspectorListView
        width: parent.width - separator.width
        height: parent.height
        model: InspectorModel
        boundsBehavior: Flickable.StopAtBounds

        header: InspectorTableHeader {
            anchors.horizontalCenter: parent.horizontalCenter
        }

        delegate: InspectorTableCell {
            width: sizes.sidebarWidth - separator.width
        }
    }

    ListSeparator {
        id: separator
        handleSize: 1
        handleColor: colors.black95
        orientation: Qt.Vertical
        anchors.left: parent.left
        anchors.top: parent.top
    }
}
