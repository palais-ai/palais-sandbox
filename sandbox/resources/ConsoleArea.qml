import QtQuick 2.0

Rectangle {
    color: colors.darkGray

    ListView {
        id: consoleListView
        anchors.fill: parent
        model: ConsoleModel
        boundsBehavior: Flickable.StopAtBounds

        header: ConsoleTableHeader {
            width: parent.width
        }

        delegate: ConsoleTableCell {
            width: parent.width
        }
    }
}
