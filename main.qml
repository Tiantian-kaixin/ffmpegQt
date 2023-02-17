import QtQuick 2.13
import QtQuick.Window 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13

CWindows {
    id: root
    visible: true
    width: 2000
    height: 2000
    title: qsTr("Hello World")
    property var models: false

    Button {
        text: "asda"
        onClicked: {
        }
    }

}
