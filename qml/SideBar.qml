import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2

Item {
    id: root

    // Information about tasks
    property int numTotal: 0
    property int numSelected: 0
    property int numFinished: 0

    // Properties for thread selector
    property int threadSelectorVal: 10
    property int threadSelectorMinN: 1
    property int threadSelectorMaxN: 99

    // State of thread pool
    property bool threadPoolActive: false

    // Signal for "Add tasks" button
    signal addTasks

    // Signal for "Remove selected" button
    signal removeTasks

    Rectangle { color: "silver"; height: parent.height; anchors.right: parent.right; width: 1; }
    Column {
        anchors.fill: parent
        anchors.topMargin: 8
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 10

        // Button that starts the pool
        Button {
            text: qsTr("Start")
            width: parent.width
            enabled: !root.threadPoolActive
            onClicked: { root.threadPoolActive = true; }
        }

        // Button that stops the pool
        Button {
            text: qsTr("Stop")
            width: parent.width
            enabled: root.threadPoolActive
            onClicked: { root.threadPoolActive = false; }
        }

        // Button to open task creation dialog
        Button {
            text: qsTr("Add tasks")
            width: parent.width
            onClicked: addTasks()
        }

        // Button to stop selected tasks
        // Should be enabled only if at least one task is selected
        Button {
            text: (root.numSelected == 0) ? qsTr("Remove selected") : qsTr("Remove %1").arg(root.numSelected)
            width: parent.width
            enabled: (root.numSelected != 0)
            onClicked: removeTasks()
        }

        // Just label
        Text {
            text: qsTr("Number of threads")
        }

        // Thread selector
        Item {
            width: parent.width
            height: 30

            // Hover hint
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    if (root.threadPoolActive) {
                        parent.ToolTip.text = qsTr("Stop the pool first");
                        parent.ToolTip.visible = true;
                    }
                }
                onExited: {
                    parent.ToolTip.visible = false;
                }
            }

            RowLayout {
                anchors.fill: parent

                // Disable when pool is active
                enabled: !root.threadPoolActive

                Slider {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    value: root.threadSelectorVal
                    from: root.threadSelectorMinN
                    to: root.threadSelectorMaxN
                    stepSize: 1

                    // Update num threads value from slider
                    onValueChanged: { root.threadSelectorVal = value; }
                }
                TextInput {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 25
                    text: root.threadSelectorVal
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    // Allow only numeric values in range [minN, maxN]
                    validator: IntValidator {bottom: root.threadSelectorMinN; top: root.threadSelectorMaxN}

                    // Update num threads value from textinput
                    onEditingFinished: { root.threadSelectorVal = text; }

                    // Draw unerline
                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: parent.acceptableInput ? "gray" : "red"
                    }
                }
            }
        }

        // Progress bar
        Item {
            width: parent.width
            height: label.height + progress.height + percentage.height

            Text {
                id: label
                anchors.top: parent.top
                text: qsTr("Progress bar - %1%").arg(parseInt(progress.value * 100))
            }
            ProgressBar {
                id: progress
                anchors.top: label.bottom
                anchors.topMargin: 4
                width: parent.width
                value: root.numFinished / (root.numTotal > 0 ? root.numTotal : 1);
            }
            Text {
                id: percentage
                anchors.top: progress.bottom
                anchors.topMargin: 4
                text: numFinished + "/" + numTotal
                visible: numTotal != 0
            }
        }
    }
}