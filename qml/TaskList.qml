import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2

ListView {
    id: root

    // Columns in format [Column name, prefered width, minimum width, fill width]
    property var columns: [
        { column: qsTr("Name"), prefWidth: root.width * 0.2, minWidth: 180, maxWidth: 320, fillWidth: false },
        { column: qsTr("Status"), prefWidth: root.width * 0.15, minWidth: 160, maxWidth: 320, fillWidth: false },
        { column: qsTr("Result"), prefWidth: -1, minWidth: -1, maxWidth: -1, fillWidth: true }
    ]

    // Row height
    property int rowHeight: 50

    // Maximum number of characters to display
    property int maxResultDisplayLength: 100

    // Function to reset checkbox "Select All"
    function resetCheckboxSelectAll() {
        headerItem.checkboxSelectAll.checked = false;
    }

    // Vertical scrolling
    flickableDirection: Flickable.VerticalFlick
    ScrollBar.vertical: ScrollBar {}

    // Table header
    header: RowLayout {
        spacing: 1
        height: rowHeight
        width: root.width
        property Item checkboxSelectAll : checkbox
        CheckBox {
            id: checkbox
            Layout.fillHeight: true
            background: Rectangle { color: "silver" }
            onToggled: model.selectTasksAll(checked)
        }
        Repeater {
            model: columns
            delegate: Label {
                // Configure layout
                Layout.fillHeight: true
                Layout.preferredWidth: modelData.prefWidth
                Layout.minimumWidth: modelData.minWidth
                Layout.maximumWidth: modelData.maxWidth
                Layout.fillWidth: modelData.fillWidth
                // Configure text
                text: modelData.column
                font.bold: true
                font.pixelSize: 20
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                // Background color
                background: Rectangle { color: "silver" }
            }
        }
    }

    // Table body
    delegate: Column {
        id: delegate
        RowLayout {
            spacing: 1
            height: rowHeight
            width: root.width
            CheckBox { 
                checked: model.checked; 
                height: parent.height; 
                onToggled: {
                    resetCheckboxSelectAll();
                    model.checked = checked;
                } 
            }
            Label {
                // Configure layout
                Layout.fillHeight: true
                Layout.preferredWidth: columns[0].prefWidth
                Layout.minimumWidth:   columns[0].minWidth
                Layout.maximumWidth:   columns[0].maxWidth
                Layout.fillWidth:      columns[0].fillWidth
                leftPadding: 10
                rightPadding: 10
                // Configure text
                text: model.name
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                elide: Text.ElideRight
            }
            Label {
                // Configure layout
                Layout.fillHeight: true
                Layout.preferredWidth: columns[1].prefWidth
                Layout.minimumWidth:   columns[1].minWidth
                Layout.maximumWidth:   columns[1].maxWidth
                Layout.fillWidth:      columns[1].fillWidth
                leftPadding: 10
                rightPadding: 10
                // Configure text
                text: model.status
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight

                // color: "yellow"

                background: Rectangle {
                    anchors.fill: parent
                    property int intStatus: model.status
                    property var colors: ["white", "yellow", "lightgreen"]
                    color: colors[intStatus]
                }
            }
            Label {
                // Configure layout
                Layout.fillHeight: true
                Layout.preferredWidth: columns[2].prefWidth
                Layout.minimumWidth:   columns[2].minWidth
                Layout.maximumWidth:   columns[2].maxWidth
                Layout.fillWidth:      columns[2].fillWidth
                leftPadding: 10
                rightPadding: 10
                // Configure text
                text: {
                    const result = model.result.substring(0, maxResultDisplayLength);
                    const postfix = (result.length >= maxResultDisplayLength) ? "..." : "";
                    return result + postfix;
                }
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                // Clipboard
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        if (parent.text) {
                            parent.ToolTip.text = qsTr("Click to copy");
                            parent.ToolTip.visible = true;
                        }
                    }
                    onClicked: {
                        if (parent.text) {
                            parent.ToolTip.text = qsTr("Copied");
                            clipboard.setValue(model.result);
                        }
                    }
                    onExited: {
                        parent.ToolTip.visible = false;
                    }
                }
            }
        }
        Rectangle {
            color: "silver"
            width: parent.width
            height: 1
        }
        Component.onCompleted: {
            root.model.updateIdMap(index);
        }
    }

    // Clipboard hack
    TextEdit {
        id: clipboard
        function setValue(value) {
            text = value
            selectAll()
            copy()
        }
        visible: false
    }
}