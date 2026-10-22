#include "ConfigBridge.h"

#include "../services/ActionRegistry.h"
#include "../services/ProfileManager.h"
#include "../services/SettingsManager.h"
#include "../system/EngineControl.h"
#include "../system/SystemEventListener.h"
#include "../system/WindowsUtils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

ConfigBridge::ConfigBridge(QObject *parent)
    : QObject(parent), m_loading(false) {
  m_engineControl = new EngineControl(this);
  m_actionRegistry = new ActionRegistry(this);
  m_settingsManager = new SettingsManager(this);
  m_profileManager = new ProfileManager(this);

  m_profileManager->setSettingsManager(m_settingsManager);

  connect(m_profileManager, &ProfileManager::configSwitched, this, [this]() {
    loadConfig();
    m_engineControl->notifyChanges();
  });

  m_systemListener = new SystemEventListener(this);
  QCoreApplication::instance()->installNativeEventFilter(m_systemListener);

  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(500);
  connect(m_saveTimer, &QTimer::timeout, this, &ConfigBridge::delayedSave);

  m_physics = new PhysicsSettings(this);
  connect(m_physics, &PhysicsSettings::settingsChanged, this,
          &ConfigBridge::requestSave);

  auto makeHandle = [this]() {
    auto *h = new HandleSettings(this);
    connect(h, &HandleSettings::settingsChanged, this,
            &ConfigBridge::requestSave);
    return h;
  };
  m_leftHandle = makeHandle();
  m_rightHandle = makeHandle();
  m_topHandle = makeHandle();
  m_bottomHandle = makeHandle();

  connect(m_actionRegistry, &ActionRegistry::gesturesChanged, this,
          &ConfigBridge::requestSave);
  connect(m_actionRegistry, &ActionRegistry::actionsChanged, this,
          &ConfigBridge::requestSave);
  connect(m_actionRegistry, &ActionRegistry::blacklistChanged, this,
          &ConfigBridge::requestSave);

  connect(m_settingsManager, &SettingsManager::configChanged, this,
          [this](const QJsonObject &root) {
            Q_UNUSED(root);
            loadConfig();
          });

  m_engineControl->cleanUp();
  loadConfig();
}

ConfigBridge::~ConfigBridge() {
  QCoreApplication::instance()->removeNativeEventFilter(m_systemListener);
}

bool ConfigBridge::engineEnabled() const { return m_engineEnabled; }
void ConfigBridge::setEngineEnabled(bool enabled) {
  if (m_engineEnabled != enabled) {
    m_engineEnabled = enabled;
    emit engineEnabledChanged();
    requestSave();
    m_engineControl->setEnabled(enabled);
  }
}

int ConfigBridge::splitMode() const { return m_splitMode; }
void ConfigBridge::setSplitMode(int mode) {
  if (m_splitMode != mode) {
    m_splitMode = mode;
    emit splitModeChanged();
    requestSave();
  }
}

void ConfigBridge::requestSave() {
  if (m_loading)
    return;
  if (!m_saveTimer->isActive()) {
    m_saveTimer->start();
  }
}

void ConfigBridge::delayedSave() {
  generateConfig();
  QJsonObject root = QJsonObject::fromVariantMap(m_configCache);
  m_settingsManager->save(root);
  if (m_engineControl)
    m_engineControl->notifyChanges();
}

void ConfigBridge::applySettings() {
  requestSave();
  if (m_engineControl) {
    m_engineControl->notifyChanges();
  }
}

void ConfigBridge::setPreviewHandle(bool pressed, int edgeIndex) {
  if (m_engineControl) {
    m_engineControl->setPreviewHandle(pressed, edgeIndex);
  }
}

void ConfigBridge::setWindowDark(bool dark) {
  WindowsUtils::setWindowDark(dark);
}

void ConfigBridge::loadConfig() {
  m_loading = true;

  QJsonObject root = m_settingsManager->load();
  m_configCache = root.toVariantMap();

  setEngineEnabled(root["engine_enabled"].toBool(true));

  m_physics->loadFromConfig(root);
  m_leftHandle->loadFromConfig(root.value("left_handle").toObject());
  m_rightHandle->loadFromConfig(root.value("right_handle").toObject());
  m_topHandle->loadFromConfig(root.value("top_handle").toObject());
  m_bottomHandle->loadFromConfig(root.value("bottom_handle").toObject());
  m_actionRegistry->loadFromConfig(root);

  if (root.contains("general")) {
    setSplitMode(root["general"].toObject()["split_mode"].toInt(0));
  } else {
    setSplitMode(0);
  }

  m_engineControl->setEnabled(m_engineEnabled);

  m_loading = false;
  emit settingsChanged();
}

void ConfigBridge::generateConfig() {
  QJsonObject root;

  root["engine_enabled"] = m_engineEnabled;

  m_physics->saveToConfig(root);

  QJsonObject leftObj, rightObj, topObj, bottomObj;
  m_leftHandle->saveToConfig(leftObj);
  m_rightHandle->saveToConfig(rightObj);
  m_topHandle->saveToConfig(topObj);
  m_bottomHandle->saveToConfig(bottomObj);
  root["left_handle"] = leftObj;
  root["right_handle"] = rightObj;
  root["top_handle"] = topObj;
  root["bottom_handle"] = bottomObj;

  m_actionRegistry->saveToConfig(root);

  QJsonObject general = root["general"].toObject();
  general["split_mode"] = m_splitMode;
  root["general"] = general;

  m_configCache = root.toVariantMap();
}
