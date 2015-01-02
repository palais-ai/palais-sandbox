import QtQuick 2.0

Text {
    renderType: Qt.platform.os === "osx" ? Text.NativeRendering : Text.QtRendering
}
