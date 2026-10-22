import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import FluentUI 1.0
import EdgeGesture.i18n 1.0

FluScrollablePage {
    title: "EdgeGesture"
    leftPadding: 20
    rightPadding: 20

    // Helper for friendly language labels
    function langLabel(code) {
        if (code === "zh_CN") return "简体中文";
        if (code === "en_US") return "English";
        return code;
    }

    ColumnLayout {
        spacing: 20
        width: parent.width

        FluText {
            text: Translator.tr("general")
            font: FluTextStyle.Subtitle
        }

        FluFrame {
            Layout.fillWidth: true
            padding: 15

            ColumnLayout {
                width: parent.width
                spacing: 15

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("enable_engine")
                        font: FluTextStyle.BodyStrong
                        Layout.fillWidth: true
                    }
                    FluToggleSwitch {
                        checked: ConfigBridge.engineEnabled
                        onClicked: ConfigBridge.engineEnabled = checked
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("language")
                        Layout.preferredWidth: 150
                    }
                    FluComboBox {
                        model: Translator.availableLanguages.map(langLabel)
                        property var codes: Translator.availableLanguages
                        currentIndex: codes.indexOf(Translator.currentLanguage)
                        onActivated: {
                            Translator.currentLanguage = codes[currentIndex];
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("vertical_trigger_range")
                        Layout.preferredWidth: 150
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: ConfigBridge.physics.verticalRange
                        onMoved: {
                            ConfigBridge.physics.verticalRange = value;
                        }
                        onValueChanged: {
                            ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: ConfigBridge.physics.verticalRange + " " + Translator.tr("long")
                        Layout.preferredWidth: 60
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("split_mode")
                        Layout.fillWidth: true
                    }
                    FluComboBox {
                        model: [Translator.tr("split_none"), Translator.tr("split_two"), Translator.tr("split_three")]
                        currentIndex: ConfigBridge.splitMode
                        onActivated: {
                            ConfigBridge.splitMode = currentIndex;
                            ConfigBridge.applySettings();
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("short_swipe_threshold")
                        Layout.preferredWidth: 150
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 10
                        to: 150
                        value: ConfigBridge.physics.shortSwipeThreshold
                        onMoved: {
                            ConfigBridge.physics.shortSwipeThreshold = value;
                        }
                        onValueChanged: {
                            ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: Math.round(ConfigBridge.physics.shortSwipeThreshold) + " " + Translator.tr("long")
                        Layout.preferredWidth: 60
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("long_swipe_threshold")
                        Layout.preferredWidth: 150
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 100
                        to: 600
                        value: ConfigBridge.physics.longSwipeThreshold
                        onValueChanged: {
                            if (pressed)
                                ConfigBridge.physics.longSwipeThreshold = value;
                        }
                        onPressedChanged: {
                            if (!pressed)
                                ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: Math.round(ConfigBridge.physics.longSwipeThreshold) + " " + Translator.tr("long")
                        Layout.preferredWidth: 60
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("trigger_threshold")
                        Layout.preferredWidth: 150
                    }
                    FluSlider {
                        Layout.fillWidth: true
                        from: 30
                        to: 200
                        value: ConfigBridge.physics.triggerThreshold
                        onValueChanged: {
                            if (pressed)
                                ConfigBridge.physics.triggerThreshold = value;
                        }
                        onPressedChanged: {
                            if (!pressed)
                                ConfigBridge.applySettings();
                        }
                    }
                    FluText {
                        text: Math.round(ConfigBridge.physics.triggerThreshold) + " " + Translator.tr("long")
                        Layout.preferredWidth: 60
                    }
                }

                FluText {
                    text: Translator.tr("edge_swipe_hint")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    color: FluColors.Grey100
                    font: FluTextStyle.Caption
                }
            }
        }

        FluText {
            text: Translator.tr("handles")
            font: FluTextStyle.Subtitle
        }

        FluFrame {
            Layout.fillWidth: true
            padding: 15

            ColumnLayout {
                width: parent.width
                spacing: 15

                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("left_handle")
                        Layout.fillWidth: true
                    }
                    FluToggleSwitch {
                        checked: ConfigBridge.leftHandle.enabled
                        onClicked: ConfigBridge.leftHandle.enabled = checked
                    }
                }
                Rectangle {
                    Layout.fillWidth: true; height: 1
                    color: FluTheme.dark ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
                }
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("right_handle")
                        Layout.fillWidth: true
                    }
                    FluToggleSwitch {
                        checked: ConfigBridge.rightHandle.enabled
                        onClicked: ConfigBridge.rightHandle.enabled = checked
                    }
                }
                Rectangle {
                    Layout.fillWidth: true; height: 1
                    color: FluTheme.dark ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
                }
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("top_handle")
                        Layout.fillWidth: true
                    }
                    FluToggleSwitch {
                        checked: ConfigBridge.topHandle.enabled
                        onClicked: ConfigBridge.topHandle.enabled = checked
                    }
                }
                Rectangle {
                    Layout.fillWidth: true; height: 1
                    color: FluTheme.dark ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
                }
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("bottom_handle")
                        Layout.fillWidth: true
                    }
                    FluToggleSwitch {
                        checked: ConfigBridge.bottomHandle.enabled
                        onClicked: ConfigBridge.bottomHandle.enabled = checked
                    }
                }
            }
        }

        FluText {
            text: Translator.tr("application")
            font: FluTextStyle.Subtitle
        }

        FluFrame {
            Layout.fillWidth: true
            padding: 15

            ColumnLayout {
                width: parent.width
                spacing: 15

                FluFilledButton {
                    text: Translator.tr("exit_app")
                    Layout.alignment: Qt.AlignLeft
                    onClicked: Qt.quit()
                }
            }
        }
    }
}
