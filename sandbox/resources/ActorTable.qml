import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

ScrollView {
    width: parent.width
    height: parent.height

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
        id: listView
        width: parent.width
        height: parent.height
        model: ActorModel
        boundsBehavior: Flickable.StopAtBounds

        header: ActorTableHeader {
            anchors.horizontalCenter: parent.horizontalCenter
        }

        delegate: ActorTableCell {
            width: sizes.sidebarWidth - separator.width
            height: sizes.sidebarCellHeight
        }

        Component.onCompleted: listView.positionViewAtBeginning()
    }
}
