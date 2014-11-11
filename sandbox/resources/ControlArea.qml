import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Image {
    source: "qrc:/images/controls-bg.jpg"
    fillMode: Image.Tile

    ListModel {
        id: model

        ListElement {
            name: "Bill Smith"
            number: "555 3264"
        }
        ListElement {
            name: "John Brown"
            number: "555 8426"
        }
        ListElement {
            name: "Sam Wise"
            number: "555 0473"
        }
    }

    ListView {
        width: parent.width;
        height: childrenRect.height

        model: model
        delegate: Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: name + ": " + number
        }
    }
}
