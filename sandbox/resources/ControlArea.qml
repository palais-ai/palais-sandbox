import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    color: colors.darkGray

    ListModel {
        id: emptyModel
    }

    ListView {
        id: listView
        width: parent.width;
        height: parent.height
        model: ActorModel

        header: ActorTableHeader {
            anchors.horizontalCenter: parent.horizontalCenter
        }

        delegate: ActorTableCell {
            width: 210
            height: 25
        }
    }
}
