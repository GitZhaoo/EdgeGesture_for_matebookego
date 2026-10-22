import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import FluentUI 1.0
import EdgeGesture.i18n 1.0

FluScrollablePage {
    title: Translator.tr("nav_handles")
    leftPadding: 20
    rightPadding: 20

    // edgeIndex: 0=Left,1=Right,2=Top,3=Bottom
    property int currentEdgeIndex: 0
    property var edgeLabels: [
        Translator.tr("left_handle"),
        Translator.tr("right_handle"),
        Translator.tr("top_handle"),
        Translator.tr("bottom_handle")
    ]

    function handleObj(i) {
        if (i === 0) return ConfigBridge.leftHandle;
        if (i === 1) return ConfigBridge.rightHandle;
        if (i === 2) return ConfigBridge.topHandle;
        return ConfigBridge.bottomHandle;
    }

    ColumnLayout {
        spacing: 20
        width: parent.width

        RowLayout {
            spacing: 10
            FluText {
                text: Translator.tr("select_edge")
                font: FluTextStyle.BodyStrong
            }
            FluComboBox {
                model: edgeLabels
                currentIndex: currentEdgeIndex
                onCurrentIndexChanged: currentEdgeIndex = currentIndex
            }
        }

        FluText {
            text: Translator.tr("appearance_position")
            font: FluTextStyle.Subtitle
        }

        FluFrame {
            id: settingsFrame
            Layout.fillWidth: true
            padding: 15

            property var currentHandle: handleObj(currentEdgeIndex)

            ColumnLayout {
                width: parent.width
                spacing: 15

                // Width (thickness of the handle)
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("width")
                        Layout.preferredWidth: 100
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 2
                        to: 60
                        value: settingsFrame.currentHandle.width
                        onMoved: {
                            settingsFrame.currentHandle.width = value;
                        }
                        onPressedChanged: {
                            ConfigBridge.setPreviewHandle(pressed, currentEdgeIndex);
                            if (!pressed)
                                ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: settingsFrame.currentHandle.width + " " + Translator.tr("long")
                        Layout.preferredWidth: 60
                    }
                }

                // Size (extent along the edge, %)
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("size")
                        Layout.preferredWidth: 100
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 10
                        to: 100
                        value: settingsFrame.currentHandle.size
                        onMoved: {
                            settingsFrame.currentHandle.size = value;
                        }
                        onPressedChanged: {
                            ConfigBridge.setPreviewHandle(pressed, currentEdgeIndex);
                            if (!pressed)
                                ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: settingsFrame.currentHandle.size + " " + Translator.tr("percent")
                        Layout.preferredWidth: 60
                    }
                }

                // Position (center along the edge, %)
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("position")
                        Layout.preferredWidth: 100
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: settingsFrame.currentHandle.position
                        onMoved: {
                            settingsFrame.currentHandle.position = value;
                        }
                        onPressedChanged: {
                            ConfigBridge.setPreviewHandle(pressed, currentEdgeIndex);
                            if (!pressed)
                                ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: settingsFrame.currentHandle.position + " " + Translator.tr("percent")
                        Layout.preferredWidth: 60
                    }
                }

                // Color
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("color")
                        Layout.preferredWidth: 100
                    }
                    FluColorPicker {
                        Layout.alignment: Qt.AlignLeft
                        current: settingsFrame.currentHandle.color
                        onAccepted: {
                            settingsFrame.currentHandle.color = current.toString();
                            ConfigBridge.applySettings();
                        }
                    }
                }
            }
        }
    }
}
