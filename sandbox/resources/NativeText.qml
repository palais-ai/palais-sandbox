import QtQuick 2.0

Text {
    // Native rendering appears to be more robust on Mac OS.
    renderType: Text.NativeRendering

    Component.onCompleted: {
        // Scale fonts down on windows because the default DPI is higher
        // to archieve a homogenous design across platforms.
        var scaleFactor = Qt.platform.os === "windows" ? 0.85 : 1.0
        font.pointSize = font.pointSize * scaleFactor
    }
}
