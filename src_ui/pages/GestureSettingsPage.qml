import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import FluentUI 1.0
import EdgeGesture.i18n 1.0

FluScrollablePage {
    id: gesturePage
    title: Translator.tr("gesture_settings")
    leftPadding: 20
    rightPadding: 20

    // currentEdgeIndex: 0=Left,1=Right,2=Top,3=Bottom
    property int currentEdgeIndex: 0
    property string currentEdgeName: ["left","right","top","bottom"][currentEdgeIndex]
    property string currentZone: "Global"
    property int splitMode: ConfigBridge.splitMode

    property var actionList: []
    property int reloadCount: 0

    function refreshActionList() {
        var names = ConfigBridge.actionRegistry.getActionNames();
        names.push(Translator.tr("none"));
        actionList = names;
    }

    Connections {
        target: ConfigBridge
        function onSettingsChanged() {
            refreshActionList();
            splitMode = ConfigBridge.splitMode;
            reloadCount++;
        }
    }
    Connections {
        target: ConfigBridge.actionRegistry
        function onActionsChanged() { refreshActionList(); }
    }
    Connections {
        target: Translator
        function onCurrentLanguageChanged() { reloadCount++; }
    }

    Component.onCompleted: { refreshActionList(); }

    // ---- Gesture key building ----
    // Only straight and long-straight gestures remain (diagonals removed).
    function straightSuffix() {
        if (currentEdgeName === "left") return "_right";
        if (currentEdgeName === "right") return "_left";
        if (currentEdgeName === "top") return "_down";
        return "_up"; // bottom
    }
    function longStraightSuffix() {
        return "_long" + straightSuffix();
    }

    function getFullKey(suffix) {
        var zone = "";
        var isHorizontalEdge = (currentEdgeName === "top" || currentEdgeName === "bottom");
        if (isHorizontalEdge || splitMode > 0) {
            zone = "_" + currentZone.toLowerCase();
        }
        return currentEdgeName + zone + suffix;
    }

    // ---- Zone model ----
    //  - top/bottom edges: ALWAYS split into left/right (2 zones)
    //  - left/right edges: use splitMode (0=none, 1=two, 2=three vertical zones)
    property var zoneModel: {
        var isHorizontalEdge = (currentEdgeName === "top" || currentEdgeName === "bottom");
        if (isHorizontalEdge) {
            return [Translator.tr("zone_left"), Translator.tr("zone_right")];
        }
        if (splitMode === 0) return [Translator.tr("zone_global")];
        if (splitMode === 1) {
            return [Translator.tr("zone_top"), Translator.tr("zone_bottom")];
        }
        if (splitMode === 2) {
            return [Translator.tr("zone_top"), Translator.tr("zone_middle"), Translator.tr("zone_bottom")];
        }
        return [Translator.tr("zone_global")];
    }

    function zoneToken(displayName) {
        if (displayName === Translator.tr("zone_global")) return "Global";
        if (displayName === Translator.tr("zone_top")) return "top";
        if (displayName === Translator.tr("zone_bottom")) return "bottom";
        if (displayName === Translator.tr("zone_middle")) return "middle";
        if (displayName === Translator.tr("zone_left")) return "left";
        if (displayName === Translator.tr("zone_right")) return "right";
        return "Global";
    }

    // Update currentZone when zoneModel changes (edge switched or splitMode changed)
    onZoneModelChanged: {
        var found = false;
        for (var i = 0; i < zoneModel.length; i++) {
            if (zoneToken(zoneModel[i]) === currentZone) {
                found = true;
                break;
            }
        }
        if (!found) {
            currentZone = zoneToken(zoneModel[0]);
        }
    }

    // Helper: find index of a token in a model of display names
    function indexOfToken(model, token) {
        for (var i = 0; i < model.length; i++) {
            if (zoneToken(model[i]) === token) return i;
        }
        return 0;
    }

    // Helper: find index of an action name in actionList
    function indexOfAction(actionName) {
        for (var i = 0; i < actionList.length; i++) {
            if (actionList[i] === actionName) return i;
        }
        return actionList.length - 1; // "none" is last
    }

    property var edgeLabels: [
        Translator.tr("left_handle"),
        Translator.tr("right_handle"),
        Translator.tr("top_handle"),
        Translator.tr("bottom_handle")
    ]

    ColumnLayout {
        spacing: 20
        width: parent.width

        // ---- Edge selector ----
        RowLayout {
            spacing: 10
            FluText {
                text: Translator.tr("select_edge")
                font: FluTextStyle.BodyStrong
            }
            FluComboBox {
                id: edgeCombo
                model: edgeLabels
                onActivated: {
                    currentEdgeIndex = currentIndex;
                }
                Connections {
                    target: gesturePage
                    function onCurrentEdgeIndexChanged() {
                        edgeCombo.currentIndex = currentEdgeIndex;
                    }
                    function onReloadCountChanged() {
                        edgeCombo.currentIndex = currentEdgeIndex;
                    }
                }
                Component.onCompleted: currentIndex = currentEdgeIndex
            }
        }

        // ---- Zone selector ----
        RowLayout {
            spacing: 10
            visible: splitMode > 0 || currentEdgeName === "top" || currentEdgeName === "bottom"
            FluText {
                text: Translator.tr("select_zone")
                font: FluTextStyle.BodyStrong
            }
            FluComboBox {
                id: zoneCombo
                model: zoneModel
                onActivated: {
                    currentZone = zoneToken(currentText);
                }
                function updateIndex() {
                    zoneCombo.currentIndex = indexOfToken(zoneCombo.model, currentZone);
                }
                Connections {
                    target: gesturePage
                    function onCurrentZoneChanged() { zoneCombo.updateIndex(); }
                    function onZoneModelChanged() { zoneCombo.updateIndex(); }
                    function onReloadCountChanged() { zoneCombo.updateIndex(); }
                }
                Component.onCompleted: updateIndex()
            }
        }

        FluText {
            text: Translator.tr("short_swipe")
            font: FluTextStyle.Subtitle
        }

        FluFrame {
            Layout.fillWidth: true
            padding: 15

            ColumnLayout {
                width: parent.width
                spacing: 10

                // Straight (short swipe)
                RowLayout {
                    FluIcon { iconSource: FluentIcons.Forward }
                    FluText {
                        text: Translator.tr("straight")
                        Layout.fillWidth: true
                    }
                    FluComboBox {
                        id: straightCombo
                        model: actionList
                        onActivated: {
                            var key = getFullKey(straightSuffix());
                            ConfigBridge.actionRegistry.setGesture(key, currentText);
                            ConfigBridge.applySettings();
                        }
                        function updateIndex() {
                            var key = getFullKey(straightSuffix());
                            var actionName = ConfigBridge.actionRegistry.getGesture(key);
                            straightCombo.currentIndex = indexOfAction(actionName);
                        }
                        Connections {
                            target: gesturePage
                            function onCurrentEdgeIndexChanged() { straightCombo.updateIndex(); }
                            function onCurrentZoneChanged() { straightCombo.updateIndex(); }
                            function onReloadCountChanged() { straightCombo.updateIndex(); }
                        }
                        Component.onCompleted: updateIndex()
                    }
                }
            }
        }

        FluText {
            text: Translator.tr("long_swipe")
            font: FluTextStyle.Subtitle
        }

        FluFrame {
            Layout.fillWidth: true
            padding: 15
            ColumnLayout {
                width: parent.width
                spacing: 10

                RowLayout {
                    FluIcon { iconSource: FluentIcons.Forward }
                    FluText {
                        text: Translator.tr("long_straight")
                        Layout.fillWidth: true
                    }
                    FluComboBox {
                        id: longCombo
                        model: actionList
                        onActivated: {
                            var key = getFullKey(longStraightSuffix());
                            ConfigBridge.actionRegistry.setGesture(key, currentText);
                            ConfigBridge.applySettings();
                        }
                        function updateIndex() {
                            var key = getFullKey(longStraightSuffix());
                            var actionName = ConfigBridge.actionRegistry.getGesture(key);
                            longCombo.currentIndex = indexOfAction(actionName);
                        }
                        Connections {
                            target: gesturePage
                            function onCurrentEdgeIndexChanged() { longCombo.updateIndex(); }
                            function onCurrentZoneChanged() { longCombo.updateIndex(); }
                            function onReloadCountChanged() { longCombo.updateIndex(); }
                        }
                        Component.onCompleted: updateIndex()
                    }
                }
            }
        }
    }
}
