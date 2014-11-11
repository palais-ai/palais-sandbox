import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Image {
    source: "qrc:/images/controls-bg.jpg"
    fillMode: Image.Tile

    ListModel {
        id: model

        ListElement {
            name: "Actor 1"
        }
        ListElement {
            name: "Actor 2"
        }
        ListElement {
            name: "Actor 3"
        }
        ListElement {
            name: "Actor 4"
        }
        ListElement {
            name: "Actor 5"
        }
        ListElement {
            name: "Actor 6"
        }
        ListElement {
            name: "Actor 7"
        }
    }

    ListView {
        width: parent.width;
        height: parent.height
        model: model

        header: ActorTableHeader {
            anchors.horizontalCenter: parent.horizontalCenter
        }

        delegate: ActorTableCell {
            width: parent.width
            height: 34
        }
    }
}
