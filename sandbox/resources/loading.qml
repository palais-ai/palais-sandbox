import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    color: "#4D000000"

    LoadingIndicator {
        anchors.centerIn: parent
        id: indicator
        width: 120
        height: 120
        Layout.alignment: Qt.AlignCenter
        currentValue: ApplicationWrapper.loadingProgress
        primaryColor: "#f58723"
        secondaryColor: "#C7BA99"
    }
}
