import QtQuick 2.0
import QtQuick.Layouts 1.1

Item {

    FontLoader {
        id: openSansSemibold; source: "qrc:/fonts/open-sans/OpenSans-Semibold.ttf"
    }

    FontLoader {
        id: deftone; source: "qrc:/fonts/Deftone-Stylus/DEFTONE.ttf"
    }

    Text {
        id: sdkName
        Layout.alignment: Qt.AlignCenter
        text: "brainworks"
        font.pointSize: 36
        color: "white"
        font.family: deftone.name
    }

    Text {
        id: editorName
        Layout.alignment: Qt.AlignCenter
        text: "Sandbox"
        font.pointSize: 30
        color: "#f58723"
        font.family: openSansSemibold.name
        font.weight: Font.DemiBold
        anchors.horizontalCenter: sdkName.horizontalCenter
        anchors.top: sdkName.bottom
        anchors.topMargin: -12
    }
}
