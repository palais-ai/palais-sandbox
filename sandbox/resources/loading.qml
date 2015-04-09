import QtQuick 2.0
import QtQuick.Controls 1.2

Rectangle {
    color: colors.gray

    NativeText {
        id: loadingIcon
        anchors.centerIn: parent
        font.pointSize: 48
        font.family: fontAwesome.name
        color: colors.primaryColorDarker
        text: "\uf1ce"
    }

    RotationAnimation {
        target: loadingIcon
        property: "rotation"
        from: 0
        to: 360
        direction: RotationAnimation.Clockwise
        duration: 1000
        loops: Animation.Infinite
        running: true
    }
}
