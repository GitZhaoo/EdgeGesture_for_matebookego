#include "EngineCore.h"
#include <cmath>
#include <iostream>
#include <psapi.h>
#include <string>

EngineCore::EngineCore() {}

EngineCore::~EngineCore() {}

std::string EngineCore::GetActiveProcessName() {
  HWND hwnd = GetForegroundWindow();
  if (!hwnd)
    return "";

  DWORD pid = 0;
  GetWindowThreadProcessId(hwnd, &pid);
  if (pid == 0)
    return "";

  HANDLE hProcess = OpenProcess(
      PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (!hProcess)
    return "";

  char buffer[MAX_PATH];
  std::string result;
  if (GetModuleBaseNameA(hProcess, nullptr, buffer, MAX_PATH)) {
    result = buffer;
  }
  CloseHandle(hProcess);
  return result;
}

void EngineCore::Run() {
  ConfigManager::Get().Load();

  int screenW = GetSystemMetrics(SM_CXSCREEN);
  int screenH = GetSystemMetrics(SM_CYSCREEN);

  m_vis.Init(screenW, screenH);

  InputWindow::Get().SetCallbacks(
      [this](bool inZone, Edge edge) { OnZoneState(inZone, edge); },
      [this](Edge edge, int a, int b) { OnGestureStart(edge, a, b); },
      [this](int x, int y) { OnGestureUpdate(x, y); },
      [this]() { OnGestureEnd(); });
  InputWindow::Get().Initialize();

  m_profileTimerId = SetTimer(NULL, 0, 500, nullptr);
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    if (msg.message == (WM_USER + 101)) {
      ReloadConfig();
    }

    if (msg.message == WM_TIMER && msg.wParam == m_profileTimerId) {
      std::string appName = GetActiveProcessName();
      if (!appName.empty() && appName != m_lastAppName) {
        m_lastAppName = appName;
        ConfigManager::Get().LoadProfile(appName);
        InputWindow::Get().UpdateLayout();
      }
    }
  }
}

void EngineCore::ReloadConfig() {
  ConfigManager::Get().Load();
  InputWindow::Get().UpdateLayout();
  std::cout << "[Core] Config Reloaded" << std::endl;
}

void EngineCore::OnZoneState(bool inZone, Edge edge) {
  if (m_isDragging)
    return;
}

void EngineCore::OnGestureStart(Edge edge, int a, int b) {
  if (IsBlacklistedAppActive()) {
    std::cout << "[Core] Blacklisted app active, ignoring gesture."
              << std::endl;
    return;
  }

  std::cout << "\n=== Gesture Start === Edge: " << EdgeName(edge)
            << " | A: " << a << " B: " << b << std::endl;

  m_isDragging = true;
  m_edge = edge;

  // a = along-edge coordinate (the "anchor" along the edge)
  // b = perpendicular coordinate (ignored at start)
  m_anchorA = (float)a;

  m_targetX = 0;
  m_targetA = m_anchorA;

  m_currentX = 0;
  m_currentA = m_anchorA;
  m_velocityX = 0;
  m_velocityA = 0;

  m_vis.SetTimerCallback([this]() { this->PhysicsLoop(); });
  SetTimer(m_vis.GetHwnd(), m_timerId, 16, nullptr);
}

void EngineCore::OnGestureUpdate(int x, int y) {
  AppConfig &cfg = ConfigManager::Get().Current();
  int screenW = GetSystemMetrics(SM_CXSCREEN);

  float absX = (float)x;
  float absY = (float)y;
  float pull = 0.0f;
  float along = 0.0f;

  switch (m_edge) {
  case Edge::Left:
    pull = absX;
    along = absY;
    break;
  case Edge::Right:
    pull = (float)(screenW - absX);
    along = absY;
    break;
  case Edge::Top:
    pull = absY;
    along = absX;
    break;
  case Edge::Bottom: {
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    pull = (float)(screenH - absY);
    along = absX;
    break;
  }
  }

  m_targetX = pull;
  m_targetA = along;

  std::cout << "Update: X=" << x << " Y=" << y << " | pull=" << m_targetX
            << " along=" << m_targetA << std::endl;

  DetermineGesture();
}

void EngineCore::OnGestureEnd() {
  m_isDragging = false;
  AppConfig &cfg = ConfigManager::Get().Current();
  std::cout << "=== Gesture End === currentX: " << m_currentX
            << " | threshold: " << cfg.triggerThreshold << std::endl;
  if (m_currentX > cfg.triggerThreshold) {
    std::cout << "  -> TRIGGERING ACTION: " << m_currentAction << std::endl;
    m_dispatcher.Trigger(m_currentAction);
    // Snap back immediately so the next gesture can start right away
    m_currentX = 0;
    m_velocityX = 0;
    m_currentA = m_anchorA;
    m_velocityA = 0;
    KillTimer(m_vis.GetHwnd(), m_timerId);
    m_vis.SetWindowVisible(false);
    // Re-assert input windows above the taskbar (Visualizer may have pushed
    // them down in Z-order during the gesture).
    InputWindow::Get().ReassertTopmost();
  } else {
    std::cout << "  -> Below threshold, not triggering" << std::endl;
    // Let the spring-back animation play (it will hide when settled)
  }
}

std::string EngineCore::BuildGestureKey() {
  AppConfig &cfg = ConfigManager::Get().Current();
  std::string base = EdgeName(m_edge);
  std::string zone = "";

  // Top/bottom edges ALWAYS split into left/right halves (2 zones),
  // regardless of splitMode. splitMode only affects left/right edges.
  bool isHorizontalEdge = (m_edge == Edge::Top || m_edge == Edge::Bottom);

  if (isHorizontalEdge) {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    float rel = m_anchorA / (float)screenW;
    zone = (rel < 0.5f) ? "_left" : "_right";
  } else if (cfg.splitMode > 0) {
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    float rel = m_anchorA / (float)screenH;
    if (cfg.splitMode == 1) {
      zone = (rel < 0.5f) ? "_top" : "_bottom";
    } else if (cfg.splitMode == 2) {
      if (rel < 0.33f)
        zone = "_top";
      else if (rel < 0.66f)
        zone = "_middle";
      else
        zone = "_bottom";
    }
  }
  return base + zone;
}

void EngineCore::DetermineGesture() {
  float dx = m_targetX;
  AppConfig &cfg = ConfigManager::Get().Current();

  if (dx < cfg.shortSwipeThreshold)
    return;

  std::string base = BuildGestureKey();

  // Only straight pull gestures (diagonals removed).
  std::string straight;
  switch (m_edge) {
  case Edge::Left:   straight = "_right"; break;
  case Edge::Right:  straight = "_left";  break;
  case Edge::Top:    straight = "_down";  break;
  case Edge::Bottom: straight = "_up";    break;
  }

  bool isLong = (dx > cfg.longSwipeThreshold);
  std::string direction = straight;
  if (isLong)
    direction = "_long" + direction;

  std::string key = base + direction;
  std::cout << "Gesture: " << key << " | dx:" << dx << std::endl;

  if (cfg.gestureMap.count(key)) {
    m_currentAction = cfg.gestureMap[key];
    std::cout << "  -> Action: " << m_currentAction << std::endl;
  } else {
    std::cout << "  -> Gesture key NOT found in map: " << key << std::endl;
    m_currentAction = "none";
  }
}

void EngineCore::PhysicsLoop() {
  AppConfig &cfg = ConfigManager::Get().Current();

  float realTargetX = m_isDragging ? m_targetX : 0;
  if (realTargetX > cfg.maxWaveX)
    realTargetX = cfg.maxWaveX;

  float forceX = (realTargetX - m_currentX) * cfg.tension;
  m_velocityX += forceX;
  m_velocityX *= cfg.friction;
  m_currentX += m_velocityX;

  float realTargetA = m_isDragging ? m_targetA : m_anchorA;
  float forceA = (realTargetA - m_currentA) * cfg.tension;
  m_velocityA += forceA;
  m_velocityA *= cfg.friction;
  m_currentA += m_velocityA;

  if (!m_isDragging && std::abs(m_currentX) < 0.5f &&
      std::abs(m_velocityX) < 0.5f) {
    m_currentX = 0;
    KillTimer(m_vis.GetHwnd(), m_timerId);
    // Hide the visualizer so it doesn't block subsequent touch input
    m_vis.SetWindowVisible(false);
    // Re-assert input windows above the taskbar
    InputWindow::Get().ReassertTopmost();
  }

  bool triggered = (m_currentX > cfg.triggerThreshold);
  m_vis.Update(m_currentX, m_currentA, m_anchorA, m_edge, triggered, "");
  m_vis.Render();
}

bool EngineCore::IsBlacklistedAppActive() {
  std::string exeName = GetActiveProcessName();
  if (exeName.empty())
    return false;

  AppConfig &cfg = ConfigManager::Get().Current();
  for (const auto &blocked : cfg.blacklist) {
    if (_stricmp(exeName.c_str(), blocked.c_str()) == 0) {
      return true;
    }
  }

  return false;
}
