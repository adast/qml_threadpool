import QtQuick 2.9
import QtQuick.Controls 2.2
import TaskModel 1.0

ApplicationWindow {
    minimumWidth: 800
    minimumHeight: 480
    visible: true
    title: qsTr("Thread Pool")

    // Model for taskList
    TaskModel {
        id: taskModel
    }

    // Popup window for task creation
    TaskCreator {
        id: taskCreator
        taskModel: taskModel
    }

    // Table of tasks
    TaskList {
        id: taskList
        anchors.left: sidebar.right
        anchors.right: parent.right
        height: parent.height
        model: taskModel
    }

    // Sidebar with controls
    SideBar {
        id: sidebar
        anchors.left: parent.left
        height: parent.height
        width: 180
        numSelected: taskModel.numSelected
        numFinished: taskModel.numFinished
        numTotal: taskModel.numTotal

        onAddTasks: taskCreator.open()
        
        onRemoveTasks: {
            taskModel.removeTasks();
            taskList.resetCheckboxSelectAll();
        }

        onThreadPoolActiveChanged: {
            if (threadPoolActive) {
                taskModel.startPool(threadSelectorVal);
            } else {
                taskModel.stopPool();
            }
        }
    }
}
