import QtQuick 2.0
import Console 1.0
import Inspector 1.0

Rectangle {
    width: childrenRect.width
    height: childrenRect.height

    NativeText {
        id: logIcon
        anchors.verticalCenter: emptyText.verticalCenter
        anchors.verticalCenterOffset: 2
        anchors.left: parent.left
        anchors.leftMargin: 8
        text: getIconForLogLevel(level)
        font.family: fontAwesome.name
        font.pointSize: 12
        color: colors.lightGray

        function getIconForLogLevel(loglevel) {
            switch(loglevel)
            {
            case Console.LogLevelInfo:
                return "\uf05a"
            case Console.LogLevelDebug:
                return "\uf013"
            case Console.LogLevelWarning:
                return "\uf071"
            case Console.LogLevelError:
                return "\uf057"
            }
        }
    }

    NativeText {
        id: logText
        width: headerColumn.width - emptyIcon.width - emptyIcon.anchors.leftMargin * 2
        anchors.top: parent.top
        anchors.left: emptyIcon.right
        anchors.leftMargin: 8
        text: message
        font.family: openSans.name
        wrapMode: Text.WordWrap
        font.pointSize: 12
        color: colors.lightGray
    }

    ListSeparator {
        id: separator
        anchors.top: logText.bottom
    }
}
