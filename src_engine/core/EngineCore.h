#pragma once
#include "actions/ActionDispatcher.h"
#include "core/ConfigManager.h"
#include "input/InputWindow.h"
#include "ui/Visualizer.h"
#include <memory>
#include <string>
#include <windows.h>

class EngineCore {
public:
  EngineCore();
  ~EngineCore();

  void Run();
  void ReloadConfig();

private:
  void OnZoneState(bool inZone, Edge edge);
  void OnGestureStart(Edge edge, int anchorA, int anchorB);
  void OnGestureUpdate(int x, int y);
  void OnGestureEnd();

  void PhysicsLoop();

  bool IsBlacklistedAppActive();
  std::string GetActiveProcessName();

  void DetermineGesture();

  // Convert current drag state into a gesture key string.
  std::string BuildGestureKey();

  Visualizer m_vis;
  ActionDispatcher m_dispatcher;

  // currentX = pull depth (always positive, perpendicular to edge)
  float m_currentX = 0;
  // currentA = along-edge coordinate (vertical for left/right, horizontal for
  // top/bottom)
  float m_currentA = 0;
  float m_velocityX = 0;
  float m_velocityA = 0;
  float m_targetX = 0;
  float m_targetA = 0;
  float m_anchorA = 0;

  bool m_isDragging = false;
  bool m_isAnimating = false;
  Edge m_edge = Edge::Left;

  UINT_PTR m_timerId = 1001;
  UINT_PTR m_profileTimerId = 2001;

  std::string m_currentAction;
  std::string m_currentGestureName;
  std::string m_lastAppName;
};
