import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import WordModel 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 600
    title: "Анализатор частоты слов"
    minimumWidth: 600
    minimumHeight: 400

    // Обработчик ошибок
    Connections {
        target: appController
        onErrorOccurred: {
            errorDialog.text = message
            errorDialog.open()
        }
        onProcessingFinished: {
            // Сбросить выбранный файл при отмене
            fileDialog.selectedFile = ""
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // Панель управления
        RowLayout {
            spacing: 10

            Button {
                text: "Открыть"
                enabled: !appController || !appController.isProcessing
                onClicked: fileDialog.open()
            }

            Button {
                id: startButton
                text: "Старт"
                enabled: (fileDialog.selectedFile !== "") && (!appController || !appController.isProcessing)
                onClicked: {
                    appController.openFile(fileDialog.selectedFile)
                    appController.startProcessing()
                }
            }

            Button {
                text: "Пауза"
                enabled: appController && appController.isProcessing && !appController.isPaused
                onClicked: appController.pauseProcessing()
            }

            Button {
                text: "Продолжить"
                enabled: appController && appController.isProcessing && appController.isPaused
                onClicked: appController.resumeProcessing()
            }

            Button {
                text: "Отмена"
                enabled: appController && appController.isProcessing
                onClicked: appController.cancelProcessing()
            }
        }

        // Прогресс бар
        ProgressBar {
            Layout.fillWidth: true
            value: appController ? appController.progress : 0
            from: 0
            to: 100
        }

        // Гистограмма
        ListView {
            id: histogram
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: appController.wordModel
            clip: true
            spacing: 5
            boundsBehavior: Flickable.StopAtBounds

            delegate: Item {
                width: histogram.width
                height: 40

                RowLayout {
                    anchors.fill: parent
                    spacing: 10

                    Label {
                        text: model.word
                        Layout.preferredWidth: 150
                        elide: Text.ElideRight
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 30
                        color: "#e0e0e0"
                        radius: 4

                        Rectangle {
                            width: {
                                if (appController.wordModel && appController.wordModel.maxCount > 0) {
                                    parent.width * (model.count / appController.wordModel.maxCount)
                                } else {
                                    0
                                }
                            }
                            height: parent.height
                            color: "#2196F3"
                            radius: 4
                            Behavior on width {
                                NumberAnimation {
                                    duration: 300
                                    easing.type: Easing.OutQuad
                                }
                            }
                        }

                        Label {
                            anchors.centerIn: parent
                            text: model.count
                            font.bold: true
                            color: "black"
                        }
                    }
                }
            }
        }
    }

    // Диалог выбора файла
    FileDialog {
        id: fileDialog
        title: "Выберите текстовый файл"
        nameFilters: ["Текстовые файлы (*.txt)", "Все файлы (*)"]
        property string selectedFile: ""
        onAccepted: {
            // Правильная обработка путей для разных платформ
            if (Qt.platform.os === "windows") {
                selectedFile = fileUrl.toString().replace("file:///", "");
            } else {
                selectedFile = fileUrl.toString().replace("file://", "");
            }
        }
    }

    // Диалог ошибок
    MessageDialog {
        id: errorDialog
        title: "Ошибка"
        icon: StandardIcon.Critical
    }
}
