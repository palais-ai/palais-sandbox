import QtQuick 2.0

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
