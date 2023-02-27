import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2
import TaskModel 1.0

Popup {
    id: root

    // Link to task model
    property QtObject taskModel

    // Dimensions and margins
    width: parent.width - leftMargin - rightMargin
    height: parent.height - topMargin - bottomMargin
    leftMargin: parent.width * 0.1
    rightMargin: leftMargin
    topMargin: parent.height * 0.2
    bottomMargin: topMargin

    // Behavior
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    modal: true
    focus: true

    // Random integer generator
    // Returns a random (non-uniform distribution) integer between min (inclusive) and max (inclusive).
    function getRandomInt(min, max) {
        min = Math.ceil(min);
        max = Math.floor(max);
        return Math.floor(Math.random() * (max - min + 1)) + min;
    }
    
    Column {
        anchors.fill: parent
        spacing: 10

        Label {
            text: qsTr("Add task manually")
            font.bold: true
            font.pixelSize: 16
        }

        // Manual task entry
        RowLayout {
            width: parent.width
            height: 35
            spacing: 5

            // Task selector
            ComboBox {
                id: taskSelector
                model: taskModel.taskTypes
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.3
                onCurrentIndexChanged: { input.text = "1"; }
            }

            // Task argument
            TextInput {
                id: input
                Layout.fillHeight: true
                Layout.fillWidth: true
                text: "1"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                validator: IntValidator { bottom: 0; top: 99999 }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: parent.acceptableInput ? "gray" : "red"
                }
            }

            // Randomize task argument
            Button {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.15
                text: qsTr("Rand")

                onReleased: { 
                    input.text = getRandomInt(input.validator.bottom, input.validator.top); 
                }
            }

            // Add task
            Button {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.15
                text: qsTr("Add")
                enabled: input.acceptableInput
                ToolTip.timeout: 700
                onReleased: {
                    if (taskModel.addTask(taskSelector.currentIndex, input.text)) {
                        ToolTip.text = qsTr("Added");
                    } else {
                        ToolTip.text = qsTr("Fail");
                    }
                    ToolTip.visible = true;
                }
            }
        }

        Label {
            height: 30
            verticalAlignment: Text.AlignBottom
            text: qsTr("Add task automatically (randomly)")
            font.bold: true
            font.pixelSize: 16
        }

        // Automatic task generation (random)
        RowLayout {
            id: configurator

            property int numTasks : 1024
            property int minValue : 0
            property int maxValue : 65536

            width: parent.width
            height: 35
            spacing: 5

            Label {
                Layout.fillHeight: true
                text: "Number of tasks: "
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            TextInput {
                id: num
                Layout.fillHeight: true
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                validator: IntValidator { bottom: 0; top: 99999 }
                text: configurator.numTasks
                onEditingFinished: { configurator.numTasks = text; }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: parent.acceptableInput ? "gray" : "red"
                }
            }

            Label {
                Layout.fillHeight: true
                text: "Min value: "
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            TextInput {
                id: min
                Layout.fillHeight: true
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                validator: IntValidator { bottom: 0; top: 999999 }
                text: configurator.minValue
                onEditingFinished: { configurator.minValue = text; }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: parent.acceptableInput ? "gray" : "red"
                }
            }

            Label {
                Layout.fillHeight: true
                text: "Max value: "
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
            TextInput {
                id: max
                Layout.fillHeight: true
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                validator: IntValidator { bottom: 0; top: 999999 }
                text: configurator.maxValue
                onEditingFinished: { configurator.maxValue = text; }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: parent.acceptableInput ? "gray" : "red"
                }
            }
        }

        Button {
            anchors.left: parent.left
            anchors.right: parent.right
            text: qsTr("Generate tasks")
            onReleased: taskModel.addTasksRandom(configurator.numTasks, configurator.minValue, configurator.maxValue)
        }
    }
}