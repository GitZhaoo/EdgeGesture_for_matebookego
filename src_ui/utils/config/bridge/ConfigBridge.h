#ifndef CONFIGBRIDGE_H
#define CONFIGBRIDGE_H

#include <QObject>
#include <QVariantMap>

#include "../models/HandleSettings.h"
#include "../models/PhysicsSettings.h"
#include "../services/ActionRegistry.h"
#include "../services/ProfileManager.h"
#include "../system/SystemEventListener.h"

class EngineControl;
class SettingsManager;
class QTimer;

class ConfigBridge : public QObject {
  Q_OBJECT

  Q_PROPERTY(ActionRegistry *actionRegistry READ actionRegistry CONSTANT)
  Q_PROPERTY(ProfileManager *profileManager READ profileManager CONSTANT)

  Q_PROPERTY(PhysicsSettings *physics READ physics CONSTANT)
  Q_PROPERTY(HandleSettings *leftHandle READ leftHandle CONSTANT)
  Q_PROPERTY(HandleSettings *rightHandle READ rightHandle CONSTANT)
  Q_PROPERTY(HandleSettings *topHandle READ topHandle CONSTANT)
  Q_PROPERTY(HandleSettings *bottomHandle READ bottomHandle CONSTANT)

  Q_PROPERTY(bool engineEnabled READ engineEnabled WRITE setEngineEnabled NOTIFY
                 engineEnabledChanged)
  Q_PROPERTY(
      int splitMode READ splitMode WRITE setSplitMode NOTIFY splitModeChanged)

public:
  explicit ConfigBridge(QObject *parent = nullptr);
  ~ConfigBridge() override;

  ActionRegistry *actionRegistry() const { return m_actionRegistry; }
  ProfileManager *profileManager() const { return m_profileManager; }

  PhysicsSettings *physics() const { return m_physics; }
  HandleSettings *leftHandle() const { return m_leftHandle; }
  HandleSettings *rightHandle() const { return m_rightHandle; }
  HandleSettings *topHandle() const { return m_topHandle; }
  HandleSettings *bottomHandle() const { return m_bottomHandle; }

  bool engineEnabled() const;
  void setEngineEnabled(bool enabled);

  int splitMode() const;
  void setSplitMode(int mode);

  Q_INVOKABLE void requestSave();
  Q_INVOKABLE void applySettings();
  Q_INVOKABLE void setPreviewHandle(bool pressed, int edgeIndex);
  Q_INVOKABLE void setWindowDark(bool dark);

signals:
  void engineEnabledChanged();
  void splitModeChanged();
  void settingsChanged();

private slots:
  void delayedSave();

private:
  void loadConfig();
  void generateConfig();

  ActionRegistry *m_actionRegistry;
  ProfileManager *m_profileManager;

  PhysicsSettings *m_physics;
  HandleSettings *m_leftHandle;
  HandleSettings *m_rightHandle;
  HandleSettings *m_topHandle;
  HandleSettings *m_bottomHandle;

  EngineControl *m_engineControl;
  SettingsManager *m_settingsManager;
  SystemEventListener *m_systemListener;

  QVariantMap m_configCache;
  QTimer *m_saveTimer;

  bool m_engineEnabled;
  int m_splitMode;
  bool m_loading;
};

#endif // CONFIGBRIDGE_H
