import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import FluentUI 1.0
import EdgeGesture.i18n 1.0

FluScrollablePage {
    title: Translator.tr("actions")
    property bool isRecording: false
    property string recordedShortcut: ""
    // 0 = shortcut, 1 = launch app
    property int addActionType: 0

    ListModel { id: actionModel }

    property string tempActionName: ""
    property string tempActionShortcut: ""
    property string tempActionExePath: ""
    property string tempActionArgs: ""

    function refreshActions() {
        actionModel.clear();
        var actions = ConfigBridge.actionRegistry.getActions();
        for (var key in actions) {
            var cmd = actions[key];
            var display = cmd;
            var isLaunch = cmd.indexOf("launch:") === 0;
            if (isLaunch) {
                display = cmd.substring(7);
            }
            actionModel.append({
                "name": key,
                "shortcut": display,
                "isLaunch": isLaunch
            });
        }
    }

    Component.onCompleted: { refreshActions(); }
    Connections {
        target: ConfigBridge.actionRegistry
        function onActionsChanged() { refreshActions(); }
    }
    Connections {
        target: ConfigBridge
        function onSettingsChanged() { refreshActions(); }
    }

    // File dialog for selecting an exe to launch
    FileDialog {
        id: exeFileDialog
        title: Translator.tr("add_launch_hint")
        nameFilters: ["Executables (*.exe)", "All Files (*)"]
        onAccepted: {
            var path = exeFileDialog.file.toString();
            if (path.startsWith("file:///")) path = path.substring(8);
            else if (path.startsWith("file://")) path = path.substring(7);
            tempActionExePath = path;
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: 20

        FluText {
            text: Translator.tr("register_shortcuts")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        FluButton {
            text: Translator.tr("add_new_action")
            onClicked: {
                tempActionName = "";
                tempActionShortcut = "";
                tempActionExePath = "";
                tempActionArgs = "";
                addActionType = 0;
                isRecording = false;
                addActionDialog.open();
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            interactive: false
            model: actionModel
            clip: true
            spacing: 10

            delegate: Rectangle {
                width: parent.width
                height: 50
                color: "transparent"
                radius: 4

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.color = FluTheme.dark ? Qt.rgba(1,1,1,0.05) : Qt.rgba(0,0,0,0.05)
                    onExited: parent.color = "transparent"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 20

                    FluIcon {
                        iconSource: model.isLaunch ? FluentIcons.Apps : FluentIcons.KeyboardClassic
                        iconSize: 16
                    }

                    FluText {
                        text: model.name
                        font: FluTextStyle.BodyStrong
                        Layout.preferredWidth: 150
                    }

                    FluText {
                        text: model.shortcut
                        font: FluTextStyle.Body
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    FluIconButton {
                        iconSource: FluentIcons.Delete
                        onClicked: {
                            ConfigBridge.actionRegistry.removeAction(model.name);
                        }
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: FluTheme.dark ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
                }
            }
        }
    }

    FluContentDialog {
        id: addActionDialog
        title: Translator.tr("add_action")
        message: Translator.tr("press_record")
        negativeText: Translator.tr("cancel")
        positiveText: Translator.tr("add")

        contentDelegate: Component {
            ColumnLayout {
                spacing: 10
                width: parent.width

                // Type selector
                RowLayout {
                    Layout.fillWidth: true
                    FluText {
                        text: Translator.tr("action_type")
                        font: FluTextStyle.BodyStrong
                    }
                    FluComboBox {
                        model: [Translator.tr("type_shortcut"), Translator.tr("type_launch")]
                        currentIndex: addActionType
                        onActivated: {
                            addActionType = currentIndex;
                            isRecording = false;
                        }
                    }
                }

                FluTextBox {
                    id: nameInput
                    placeholderText: Translator.tr("name_placeholder")
                    Layout.fillWidth: true
                    text: tempActionName
                    onTextChanged: tempActionName = text
                }

                // ---- Shortcut mode ----
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    visible: addActionType === 0

                    FluTextBox {
                        id: shortcutInput
                        placeholderText: Translator.tr("shortcut_hint")
                        Layout.fillWidth: true
                        text: tempActionShortcut
                        // Allow manual editing so users can type shortcuts that
                        // Windows intercepts (e.g. win+e) and can't be recorded.
                        readOnly: false
                        onTextChanged: tempActionShortcut = text
                    }

                    FluButton {
                        text: isRecording ? Translator.tr("stop_recording") : Translator.tr("record_shortcut")
                        Layout.fillWidth: true
                        onClicked: {
                            isRecording = !isRecording;
                            if (isRecording) {
                                tempActionShortcut = "";
                                keyListener.forceActiveFocus();
                            }
                        }
                    }

                    Item {
                        id: keyListener
                        focus: isRecording
                        // Track which modifier keys are currently held down
                        // (Qt's event.modifiers doesn't include the modifier
                        //  key being pressed itself on its own key-press event)
                        property bool winDown: false
                        property bool ctrlDown: false
                        property bool altDown: false
                        property bool shiftDown: false

                        Keys.onPressed: event => {
                            if (!isRecording) return;

                            // Track modifier key states
                            if (event.key === Qt.Key_Meta) { keyListener.winDown = true; }
                            if (event.key === Qt.Key_Control) { keyListener.ctrlDown = true; }
                            if (event.key === Qt.Key_Alt) { keyListener.altDown = true; }
                            if (event.key === Qt.Key_Shift) { keyListener.shiftDown = true; }

                            // Build modifier list from tracked state
                            var keys = [];
                            if (keyListener.ctrlDown) keys.push("ctrl");
                            if (keyListener.altDown) keys.push("alt");
                            if (keyListener.shiftDown) keys.push("shift");
                            if (keyListener.winDown) keys.push("win");

                            var keyName = "";
                            if (event.key === Qt.Key_Left) keyName = "left";
                            else if (event.key === Qt.Key_Right) keyName = "right";
                            else if (event.key === Qt.Key_Up) keyName = "up";
                            else if (event.key === Qt.Key_Down) keyName = "down";
                            else if (event.key === Qt.Key_Return) keyName = "enter";
                            else if (event.key === Qt.Key_Space) keyName = "space";
                            else if (event.key === Qt.Key_Tab) keyName = "tab";
                            else if (event.key === Qt.Key_Backspace) keyName = "backspace";
                            else if (event.key === Qt.Key_Delete) keyName = "delete";
                            else if (event.key === Qt.Key_Escape) keyName = "esc";
                            else if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) keyName = String.fromCharCode(event.key);
                            else if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z) keyName = String.fromCharCode(event.key).toLowerCase();
                            else if (event.key >= Qt.Key_F1 && event.key <= Qt.Key_F12) keyName = "f" + (event.key - Qt.Key_F1 + 1);
                            // Ignore standalone modifier key presses — they're
                            // handled on release (see Keys.onReleased below)
                            else if (event.key === Qt.Key_Control || event.key === Qt.Key_Alt || event.key === Qt.Key_Shift || event.key === Qt.Key_Meta) {
                                // Show partial shortcut while modifier is held
                                if (keys.length > 0) {
                                    tempActionShortcut = keys.join("+") + "+...";
                                }
                                return;
                            }
                            else if (event.text !== "" && event.text.charCodeAt(0) > 31) keyName = event.text.toLowerCase();

                            if (keyName !== "") {
                                keys.push(keyName);
                                tempActionShortcut = keys.join("+");
                                isRecording = false;
                                // Reset modifier tracking
                                keyListener.winDown = false;
                                keyListener.ctrlDown = false;
                                keyListener.altDown = false;
                                keyListener.shiftDown = false;
                            }
                        }

                        Keys.onReleased: event => {
                            if (!isRecording) return;

                            // If a modifier key is released and the current
                            // shortcut is still partial (ending with "+..."),
                            // record the modifier as a standalone key.
                            var isModifier = (event.key === Qt.Key_Meta || event.key === Qt.Key_Control || event.key === Qt.Key_Alt || event.key === Qt.Key_Shift);
                            if (isModifier && tempActionShortcut.endsWith("+...")) {
                                var mod = tempActionShortcut.replace("+...", "");
                                if (mod !== "") {
                                    tempActionShortcut = mod;
                                    isRecording = false;
                                }
                            }

                            // Update modifier tracking
                            if (event.key === Qt.Key_Meta) { keyListener.winDown = false; }
                            if (event.key === Qt.Key_Control) { keyListener.ctrlDown = false; }
                            if (event.key === Qt.Key_Alt) { keyListener.altDown = false; }
                            if (event.key === Qt.Key_Shift) { keyListener.shiftDown = false; }
                        }
                    }
                }

                // ---- Launch app mode ----
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    visible: addActionType === 1

                    FluText {
                        text: Translator.tr("add_launch_hint")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        opacity: 0.7
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        FluTextBox {
                            id: exePathInput
                            placeholderText: Translator.tr("app_path")
                            Layout.fillWidth: true
                            text: tempActionExePath
                            onTextChanged: tempActionExePath = text
                        }
                        FluButton {
                            text: Translator.tr("browse")
                            onClicked: exeFileDialog.open();
                        }
                    }

                    FluTextBox {
                        id: argsInput
                        placeholderText: Translator.tr("app_args")
                        Layout.fillWidth: true
                        text: tempActionArgs
                        onTextChanged: tempActionArgs = text
                    }
                }
            }
        }

        onPositiveClicked: {
            if (tempActionName !== "") {
                var cmd = "";
                if (addActionType === 0) {
                    // shortcut — accept any non-empty, non-partial shortcut
                    if (tempActionShortcut !== "" && !tempActionShortcut.endsWith("+...")) {
                        cmd = tempActionShortcut;
                    }
                } else {
                    // launch
                    if (tempActionExePath !== "") {
                        // quote path if it contains spaces
                        var path = tempActionExePath;
                        if (path.indexOf(" ") !== -1 && path.charAt(0) !== '"') {
                            path = '"' + path + '"';
                        }
                        cmd = "launch:" + path;
                        if (tempActionArgs !== "") {
                            cmd = cmd + " " + tempActionArgs;
                        }
                    }
                }
                if (cmd !== "") {
                    ConfigBridge.actionRegistry.addAction(tempActionName, cmd);
                    tempActionName = "";
                    tempActionShortcut = "";
                    tempActionExePath = "";
                    tempActionArgs = "";
                }
            }
        }
    }
}
