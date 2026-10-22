#pragma once

#include <QObject>

class QProcess;

class EngineControl : public QObject {
  Q_OBJECT
public:
  explicit EngineControl(QObject *parent = nullptr);
  ~EngineControl();

  void setEnabled(bool enabled);
  bool isEnabled() const { return m_enabled; }

  // Clean up existing instances
  void cleanUp();

  // Notify engine components of config changes
  void notifyChanges();

  // Input overlay preview control: edgeIndex 0=Left,1=Right,2=Top,3=Bottom
  void setPreviewHandle(bool enabled, int edgeIndex);

private:
  void updateState();

  bool m_enabled = false;
  QProcess *m_process = nullptr;
};
