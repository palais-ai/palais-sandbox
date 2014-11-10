import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0

ColumnLayout{
    anchors.centerIn: parent
    spacing: 6

    Label {
        id: loadLabel
        Layout.alignment: Qt.AlignCenter
        text: "Loading Sandbox ..."
        font.pointSize: 13
        color: "darkgray"
    }

    ProgressBar {
        Layout.alignment: Qt.AlignCenter
        indeterminate: true
    }
}
