import QtQuick 2.0

Item {
    height: nameInput.height
    width: parent.width

    property string key: 'KEY'
    property string value: 'PLACEHOLDER'
    property bool editable: true

    NativeText {
        id: keyText
        anchors.left: parent.left
        anchors.verticalCenter: nameInput.verticalCenter
        text: key
        width: parent.width - nameInput.width - 4
        font.family: openSans.name
        font.capitalization: Font.AllUppercase
        font.pointSize: 12
        font.weight: Font.DemiBold
        color: colors.darkerGray
        elide: Text.ElideRight
    }

    Rectangle {
        id: nameInput
        anchors.top: parent.top
        anchors.right: parent.right
        height: 24
        width: 162
        color: "white"
        border.color: colors.darkerGray
        border.width: 1
        clip: true

        TextInput {
            height: 20
            width: 150
            readOnly: !editable
            anchors.centerIn: parent
            font.family: openSans.name
            color: colors.darkerGray
            font.pointSize: 12
            selectByMouse: true
            text: value
            renderType: Text.NativeRendering
            onTextChanged: value = text
        }
    }
}

