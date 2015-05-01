import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: inspectorCell

    signal pressed()
    property color textColor: colors.slightlyDimmedTextColor
    property int horizontalMargin: 8
    property int verticalMargin: 8

    color: colors.darkGray
    height: childrenRect.height

    function typeNameForObject(value) {
        if(value instanceof Object &&
           value.hasOwnProperty("objectName")) {
            return value.objectName
        } else if(value instanceof Array) {
            return typeof(value[0]) + "[" + value.length + "]"
        }
        return typeof(value)
    }

    Behavior on color {
        ColorAnimation {
            duration: 200
        }
    }

    MouseArea {
        id: cellMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: inspectorCell.pressed()
    }

    NativeText {
        id: knowledgeName
        anchors.top: parent.top
        anchors.topMargin: verticalMargin
        anchors.left: parent.left
        anchors.leftMargin: horizontalMargin
        text: key
        font.family: openSans.name
        font.pointSize: 11
        color: textColor
    }

    NativeText {
        anchors.top: parent.top
        anchors.topMargin: verticalMargin
        anchors.right: parent.right
        anchors.rightMargin: horizontalMargin
        text: typeNameForObject(value)
        font.family: openSans.name
        font.pointSize: 10
        color: colors.lightGray
    }


    ComboBox {
        id: textualRepresentation
        anchors.top: knowledgeName.bottom
        anchors.topMargin: verticalMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - horizontalMargin*2
        visible: typeNameForObject(value) !== "object"

        model: {
            if(value === null && typeof value === "object") {
                return "NULL";
            } else if(value.hasOwnProperty("getTextualRepresentation")) {
                return value.getTextualRepresentation();
            } else if(value instanceof Array) {
                return value;
            } else if(typeNameForObject(value) === "object") {
                return [];
            } else {
                return [value];
            }
        }

        style: ComboBoxStyle {
            background: Rectangle {
                radius: 2
                border.width: 1
                border.color: colors.darkerGray
                color: colors.lightGray
                implicitHeight: 25

                NativeText {
                    visible: textualRepresentation.count > 1
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: -2
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    color: "black"
                    font.family: fontAwesome.name
                    font.pointSize: 12
                    text: "\uf0dd"
                }
            }
        }
    }

    ListSeparator {
        id: separator
        anchors.top: textualRepresentation.bottom
        anchors.topMargin: 8
    }

    states: [
        State {
            name: 'HOVER'
            //when: cellMouseArea.containsMouse
            PropertyChanges {
                target: inspectorCell
                color: colors.darkerGray
            }
        }
    ]
}
