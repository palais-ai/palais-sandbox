import QtQuick 2.0
import QtGraphicalEffects 1.0

Text {
    property Text source: parent
    property color shadowColor: Qt.rgba(source.color.r, source.color.g, source.color.b, 0.5)
    property real xOffset: 0
    property real yOffset: 1

    text: source.text
    color: shadowColor
    width: source.width
    height: source.height
    anchors.horizontalCenter: source.horizontalCenter
    anchors.horizontalCenterOffset: xOffset
    anchors.verticalCenter: source.verticalCenter
    anchors.verticalCenterOffset: yOffset
    font: source.font;
    opacity: source.opacity;
    maximumLineCount: source.maximumLineCount
    elide: source.elide
    clip: source.clip
    z: source.z - 1 // Always behind its parent

    GaussianBlur {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            source: parent
            radius: 16
            samples: 24
    }
}
