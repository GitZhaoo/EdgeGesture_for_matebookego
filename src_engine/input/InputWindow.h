#pragma once
#include <functional>
#include <windows.h>
#include "../core/ConfigManager.h"

using ZoneStateCallback = std::function<void(bool inZone, Edge edge)>;
using GestureStartCallback = std::function<void(Edge edge, int a, int b)>;
using GestureUpdateCallback = std::function<void(int x, int y)>;
using GestureEndCallback = std::function<void()>;

class InputWindow {
public:
  static InputWindow &Get() {
    static InputWindow instance;
    return instance;
  }

  void Initialize();
  void Shutdown();

  void SetCallbacks(ZoneStateCallback onZone, GestureStartCallback onStart,
                    GestureUpdateCallback onUpdate, GestureEndCallback onEnd);

  void UpdateLayout();
  // Re-assert all input windows as HWND_TOPMOST without changing their
  // position/size/alpha. Call this after the Visualizer is hidden to ensure
  // input windows are above the taskbar again.
  void ReassertTopmost();

private:
  InputWindow();
  ~InputWindow();

  void CreateWindows();
  // Create or get a handle for a given edge
  HWND &HwndFor(Edge e);

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);

  bool IsTouchInput();

  // Resolve which edge a given HWND belongs to; returns Edge::Left if unknown.
  Edge EdgeFromHwnd(HWND hWnd);

  HWND m_hwndLeft = nullptr;
  HWND m_hwndRight = nullptr;
  HWND m_hwndTop = nullptr;
  HWND m_hwndBottom = nullptr;
  int m_previewMode = 0; // 0=None, 1=Left, 2=Right, 3=Top, 4=Bottom

  ZoneStateCallback OnZoneState;
  GestureStartCallback OnStart;
  GestureUpdateCallback OnUpdate;
  GestureEndCallback OnEnd;

  bool m_isDragging = false;

  bool m_isHovering = false;
  bool m_isSuppressed = false;

  static const UINT_PTR TIMER_HOVER_CHECK = 1001;
  static const UINT_PTR TIMER_RECOVERY = 1002;
  static const UINT_PTR TIMER_WATCHDOG = 1003;

  const UINT HOVER_THRESHOLD_MS = 400;
  const UINT RECOVERY_DELAY_MS = 2000;
  const UINT WATCHDOG_INTERVAL_MS = 2000;
};
