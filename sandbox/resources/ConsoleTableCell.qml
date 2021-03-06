import QtQuick 2.0
import Console 1.0
import Inspector 1.0

Rectangle {
    color: colors.darkerGray
    height: childrenRect.height + separator.anchors.topMargin

    NativeText {
        id: logIcon
        anchors.verticalCenter: logText.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 8
        text: getIconForLogLevel(level)
        font.family: fontAwesome.name
        font.pointSize: 12
        color: getIconColorForLogLevel(level)

        function getIconForLogLevel(loglevel) {
            switch(loglevel)
            {
            case ConsoleModel.LogLevelInfo:
                return "\uf05a"
            case ConsoleModel.LogLevelDebug:
                return "\uf013"
            case ConsoleModel.LogLevelWarning:
                return "\uf071"
            case ConsoleModel.LogLevelError:
                return "\uf057"
            }
        }


        function getIconColorForLogLevel(loglevel) {
            switch(loglevel)
            {
            case ConsoleModel.LogLevelInfo:
            case ConsoleModel.LogLevelDebug:
                return colors.almostWhite;
            case ConsoleModel.LogLevelWarning:
                return "yellow"
            case ConsoleModel.LogLevelError:
                return "red"
            }
        }
    }

    TextEdit {
        id: logText
        width: parent.width - logIcon.width - logIcon.anchors.leftMargin * 2
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.left: logIcon.right
        anchors.leftMargin: 8
        text: message
        font.family: sourceSans.name
        wrapMode: Text.WordWrap
        font.pointSize: 12
        color: colors.almostWhite
        renderType: Text.NativeRendering
        readOnly: true
        selectByMouse: true
        selectedTextColor: "black"
        selectionColor: "white"
    }

    ListSeparator {
        id: separator
        handleColor: colors.black90
        anchors.top: logText.bottom
        anchors.topMargin: 2
    }
}
