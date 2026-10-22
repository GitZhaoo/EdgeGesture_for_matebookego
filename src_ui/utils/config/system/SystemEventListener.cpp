#include "SystemEventListener.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

SystemEventListener::SystemEventListener(QObject *parent) : QObject(parent) {}

SystemEventListener::~SystemEventListener() {}

bool SystemEventListener::nativeEventFilter(const QByteArray &eventType,
                                            void *message, qintptr *result) {
  Q_UNUSED(eventType);
  Q_UNUSED(result);
  return false;
}
