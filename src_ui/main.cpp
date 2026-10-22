#include "utils/FileBridge.h"
#include "utils/SystemBridge.h"
#include "utils/config/bridge/ConfigBridge.h"
#include <FluentUI.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include "i18n/Translator.h"
#include "utils/ExeIconProvider.h"
#include "utils/config/system/WindowsUtils.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <dwmapi.h>
#include <windows.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")

void applyMica(QWindow *window) {
  if (!window)
    return;
  HWND hwnd = (HWND)window->winId();
  if (!hwnd)
    return;

  int backdropValue = 2;
  DwmSetWindowAttribute(hwnd, 38, &backdropValue, sizeof(backdropValue));

  MARGINS margins = {-1, -1, -1, -1};
  DwmExtendFrameIntoClientArea(hwnd, &margins);
}

int main(int argc, char *argv[]) {
  HANDLE hMutex =
      CreateMutexW(NULL, TRUE, L"EdgeGesture_SettingsUI_Singleton");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    HWND existingWindow = FindWindowW(NULL, L"EdgeGesture Config");
    if (existingWindow) {
      ShowWindow(existingWindow, SW_RESTORE);
      SetForegroundWindow(existingWindow);
    }
    return 0;
  }

  QGuiApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);
  app.setWindowIcon(QIcon(":/icon.ico"));

  // Check for --minimized flag (used by auto-start to launch silently to tray)
  bool startMinimized = false;
  for (int i = 1; i < argc; ++i) {
    if (QString::fromLocal8Bit(argv[i]) == "--minimized") {
      startMinimized = true;
      break;
    }
  }

  // Register Translator as a QML singleton (typed "Translator")
  qmlRegisterSingletonType<Translator>("EdgeGesture.i18n", 1, 0, "Translator",
                                       translatorProvider);

  QQmlApplicationEngine engine;
  engine.addImageProvider("exe_icon", new ExeIconProvider);

  ConfigBridge configBridge;
  SystemBridge systemBridge;
  FileBridge fileBridge;

  engine.rootContext()->setContextProperty("ConfigBridge", &configBridge);
  engine.rootContext()->setContextProperty("SystemBridge", &systemBridge);
  engine.rootContext()->setContextProperty("FileBridge", &fileBridge);

  const QUrl url(u"qrc:/Main.qml"_qs);
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url, startMinimized](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
          QCoreApplication::exit(-1);
        } else if (obj) {
          QWindow *window = qobject_cast<QWindow *>(obj);
          if (window) {
            applyMica(window);
            if (startMinimized) {
              window->hide();
            }
          }
        }
      },
      Qt::QueuedConnection);
  engine.load(url);

  int execResult = app.exec();

  WindowsUtils::killProcess("GestureEngine.exe");

  return execResult;
}
