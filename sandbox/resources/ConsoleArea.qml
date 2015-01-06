import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

Rectangle {
    id: consoleArea
    color: colors.darkGray

    ConsoleTableHeader {
        id: consoleTableHeader
        width: parent.width
        z: consoleListView.z + 1
    }

    ScrollView {
        id: scrollView
        width: consoleArea.width
        height: consoleArea.height - consoleTableHeader.height
        anchors.top: consoleTableHeader.bottom

        style: ScrollViewStyle {
            transientScrollBars: true
            handle: Item {
                implicitWidth: 14
                implicitHeight: 26
                Rectangle {
                    color: colors.primaryColor
                    anchors.fill: parent
                    anchors.topMargin: 6
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4
                    anchors.bottomMargin: 6
                }
            }

            scrollBarBackground: Item {
                implicitWidth: 14
                implicitHeight: 26
            }
        }

        ListView {
            id: consoleListView
            anchors.fill: parent
            boundsBehavior: Flickable.StopAtBounds

            property bool completed: false

            model: ConsoleModel

            delegate: ConsoleTableCell {
                width: parent.width
            }

            Component.onCompleted: {
                consoleListView.positionViewAtEnd()
                completed = true
            }

            Timer {
                id: timer
                // We need to schedule this call for after ListView has
                // finished all of its work. Using a symbolic 1 ms.
                interval: 1
                running: false
                repeat: false
                onTriggered: {
                     if(consoleListView.completed) {
                         consoleListView.positionViewAtEnd()
                     }
                 }
             }

            Connections {
                target: ConsoleModel
                onOnFinishedMessage: timer.restart()
            }
        }
    }
}
