import QtQuick 2.0
import QtQuick.Controls 1.2

ApplicationWindow {
    id: window
    width: 1024
    height: 800
    visible: true
    title: "sandbox"

    Loader {
        id: mainLoader
        anchors.centerIn: parent
        source: "loading.qml"

        function onFullSizeContentAdded() {
            anchors.fill = parent;
        }
    }

    function onOgreIsReady() {
        mainLoader.source = "example.qml"
        mainLoader.onFullSizeContentAdded()
    }

    onClosing: Qt.quit()
}
