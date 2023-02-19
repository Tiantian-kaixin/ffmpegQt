import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13
import shaderGraph 1.0

Window {
    id: root
    visible: true
    width: 600
    height: 600
    title: qsTr("Hello World")
    property var models: false

    FBOItem{
        id: fboItem
        anchors.fill: parent
        fragment: ':/asset/shader/triangle_mirror.frag'
        vertex: ':/asset/shader/triangle_mirror.vert'
        json: {
            "uniform" : {
                "width": 0.4
            },
            "file" : {
                "imgTexture" : ":/asset/video/testVideo.mov"
            }
        }
    }

}
