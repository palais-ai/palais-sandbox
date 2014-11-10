import QtQuick 2.0
import QtQuick.Controls 1.2

ApplicationWindow {
    id: appWindow
    width: 1280
    height: 800
    visible: true
    title: ""

    minimumWidth: 1280
    minimumHeight: 800

    FontLoader {
        id: openSans; source: "qrc:/fonts/open-sans/OpenSans-Regular.ttf"
    }

    FontLoader {
        source: "qrc:/fonts/FontAwesome/fontawesome-webfont.ttf"
    }

    Image {
        id: back
        anchors.fill: parent
        source: "qrc:/images/sandbox-bg.jpg"
        Behavior on opacity { NumberAnimation { } }
    }

    AppLogo {
        anchors.top: parent.top;
        anchors.left: parent.left;
        anchors.leftMargin: 8
    }

    Loader {
        id: mainLoader
        width: 840
        height: 530
        anchors.right: parent.right;
        anchors.rightMargin: 70
        anchors.bottom: parent.bottom;
        anchors.bottomMargin: 40
        source: "loading.qml"
    }

    function onOgreIsReady() {
        mainLoader.source = "Ogre3DView.qml"
    }

    onClosing: Qt.quit()
}
