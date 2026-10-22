import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import FluentUI 1.0
import EdgeGesture.i18n 1.0
import "pages"
import Qt.labs.platform 1.1

FluWindow {
    id: window
    title: Translator.tr("app_title")
    appBar: FluAppBar {
        title: window.title
        showDark: true
        closeClickListener: () => {
            window.hide();
        }
    }
    width: 600
    height: 750
    color: "transparent"
    windowIcon: "qrc:/icon.ico"

    background: Item {}

    Component.onCompleted: {
        FluTheme.darkMode = FluThemeType.System;
        ConfigBridge.setWindowDark(FluTheme.dark);
    }

    Connections {
        target: FluTheme
        function onDarkChanged() {
            ConfigBridge.setWindowDark(FluTheme.dark);
        }
    }

    FluNavigationView {
        id: nav_view
        anchors.fill: parent
        items: FluObject {
            FluPaneItem {
                title: Translator.tr("nav_home")
                icon: FluentIcons.Home
                url: "qrc:/pages/HomePage.qml"
                onTap: { nav_view.push(url); }
            }
            FluPaneItem {
                title: Translator.tr("nav_handles")
                icon: FluentIcons.Edit
                url: "qrc:/pages/HandleSettingsPage.qml"
                onTap: { nav_view.push(url); }
            }
            FluPaneItem {
                title: Translator.tr("nav_gestures")
                icon: FluentIcons.Touch
                url: "qrc:/pages/GestureSettingsPage.qml"
                onTap: { nav_view.push(url); }
            }
            FluPaneItem {
                title: Translator.tr("nav_actions")
                icon: FluentIcons.KeyboardClassic
                url: "qrc:/pages/ActionsPage.qml"
                onTap: { nav_view.push(url); }
            }
            FluPaneItem {
                title: Translator.tr("nav_advanced")
                icon: FluentIcons.Settings
                url: "qrc:/pages/AdvancedPage.qml"
                onTap: { nav_view.push(url); }
            }
        }

        Component.onCompleted: {
            nav_view.setCurrentIndex(0);
            nav_view.push("qrc:/pages/HomePage.qml");
        }
    }

    // System Tray
    SystemTrayIcon {
        visible: true
        icon.source: "qrc:/icon.ico"
        tooltip: "EdgeGesture"

        menu: Menu {
            MenuItem {
                text: Translator.tr("open_settings")
                onTriggered: {
                    window.show();
                    window.raise();
                    window.requestActivate();
                }
            }
            MenuItem {
                text: Translator.tr("quit")
                onTriggered: Qt.quit()
            }
        }

        onActivated: {
            window.show();
            window.raise();
            window.requestActivate();
        }
    }
}
