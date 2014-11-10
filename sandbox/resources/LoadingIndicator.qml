import QtQuick 2.0
import QtGraphicalEffects 1.0

// draws two arcs (portion of a circle)
// fills the circle with a lighter secondary color
// when pressed
Canvas {
    id: canvas
    width: 240
    height: 240
    antialiasing: true

    property color primaryColor: "orange"
    property color secondaryColor: "lightblue"

    property real lineWidth: 2

    property real centerWidth: width / 2
    property real centerHeight: height / 2
    property real radius: (Math.min(canvas.width, canvas.height) - canvas.lineWidth) / 2

    property real minimumValue: 0
    property real maximumValue: 100
    property real currentValue: 0

    // this is the angle that splits the circle in two arcs
    // first arc is drawn from 0 radians to angle radians
    // second arc is angle radians to 2*PI radians
    property real angle: (currentValue - minimumValue) / (maximumValue - minimumValue) * 2 * Math.PI

    // we want both circle to start / end at 12 o'clock
    // without this offset we would start / end at 9 o'clock
    property real angleOffset: -Math.PI / 2

    property string text: currentValue

    signal clicked()

    onPrimaryColorChanged: requestPaint()
    onSecondaryColorChanged: requestPaint()
    onMinimumValueChanged: requestPaint()
    onMaximumValueChanged: requestPaint()
    onCurrentValueChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d");
        ctx.save();

        ctx.clearRect(0, 0, canvas.width, canvas.height);


        // First, thinner arc
        // From angle to 2*PI
        ctx.beginPath();
        ctx.lineWidth = canvas.lineWidth;
        ctx.strokeStyle = secondaryColor;
        ctx.arc(canvas.centerWidth,
                canvas.centerHeight,
                canvas.radius,
                angleOffset + canvas.angle,
                angleOffset + 2*Math.PI);
        ctx.stroke();


        // Second, thicker arc
        // From 0 to angle
        ctx.beginPath();
        ctx.lineWidth = canvas.lineWidth;
        ctx.strokeStyle = primaryColor;
        ctx.arc(canvas.centerWidth,
                canvas.centerHeight,
                canvas.radius,
                canvas.angleOffset,
                canvas.angleOffset + canvas.angle);
        ctx.stroke();

        ctx.restore();
    }

    Text {
        anchors.centerIn: parent

        font.family: openSans.name
        font.pointSize: 36
        text: canvas.text
        color: canvas.primaryColor
    }
}
