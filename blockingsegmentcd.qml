import QtQuick 2.0

Rectangle {
    id: rectangle
    color: "red"
    width: 200
    height: 500

    Text {
        id: text
        text: "This is QML code.\n(Click to pause)"
        font.pointSize: 14
        anchors.centerIn: parent
        PropertyAnimation {
            id: animation
            target: text
            property: "rotation"
            from: 0; to: 360; duration: 5000
            loops: Animation.Infinite
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: animation.paused ? animation.resume() : animation.pause()
    }
    Component.onCompleted: animation.start()
}
