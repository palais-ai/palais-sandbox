import QtQuick 2.0

ListView {
    id: listView
    width: parent.width - separator.width
    height: parent.height
    model: ActorModel

    header: ActorTableHeader {
        anchors.horizontalCenter: parent.horizontalCenter
    }

    delegate: ActorTableCell {
        width: sizes.sidebarWidth - separator.width
        height: sizes.sidebarCellHeight
    }
}
