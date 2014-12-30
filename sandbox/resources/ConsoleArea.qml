import QtQuick 2.0

Rectangle {
    color: colors.darkGray

    ListView {
        id: consoleListView
        anchors.fill: parent
        model: ConsoleModel

        header: ConsoleTableHeader {
            color: "blue"
        }

        delegate: ConsoleTableCell {
            width: parent.width
        }
    }
}
