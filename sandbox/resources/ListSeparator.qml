import QtQuick 2.0

Rectangle {
    property int handleSize: 1
    property color handleColor: colors.darkerGray
    property int orientation: Qt.Horizontal

    width: orientation == Qt.Horizontal ? parent.width : handleSize
    height: orientation == Qt.Horizontal ? handleSize : parent.height

    Rectangle {
        color: handleColor
        width: orientation == Qt.Horizontal ? parent.width : handleSize
        height: orientation == Qt.Horizontal ? handleSize : parent.height

        anchors.bottom: parent.bottom
    }
}
