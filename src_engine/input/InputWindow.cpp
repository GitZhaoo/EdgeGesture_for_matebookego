#include "InputWindow.h"
#include <iostream>

#define WINDOW_CLASS_NAME L"OHOInputOverlay"

InputWindow::InputWindow() {}

InputWindow::~InputWindow() { Shutdown(); }

void InputWindow::Initialize() {
  WNDCLASSEXW wcex = {sizeof(WNDCLASSEX)};
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = GetModuleHandle(NULL);
  wcex.lpszClassName = WINDOW_CLASS_NAME;
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  RegisterClassExW(&wcex);

  CreateWindows();
  UpdateLayout();

  if (m_hwndLeft)
    SetTimer(m_hwndLeft, TIMER_WATCHDOG, WATCHDOG_INTERVAL_MS, nullptr);
}

void InputWindow::Shutdown() {
  if (m_hwndLeft) {
    DestroyWindow(m_hwndLeft);
    m_hwndLeft = nullptr;
  }
  if (m_hwndRight) {
    DestroyWindow(m_hwndRight);
    m_hwndRight = nullptr;
  }
  if (m_hwndTop) {
    DestroyWindow(m_hwndTop);
    m_hwndTop = nullptr;
  }
  if (m_hwndBottom) {
    DestroyWindow(m_hwndBottom);
    m_hwndBottom = nullptr;
  }
  UnregisterClassW(WINDOW_CLASS_NAME, GetModuleHandle(NULL));
}

void InputWindow::SetCallbacks(ZoneStateCallback onZone,
                               GestureStartCallback onStart,
                               GestureUpdateCallback onUpdate,
                               GestureEndCallback onEnd) {
  OnZoneState = onZone;
  OnStart = onStart;
  OnUpdate = onUpdate;
  OnEnd = onEnd;
}

HWND &InputWindow::HwndFor(Edge e) {
  switch (e) {
  case Edge::Left: return m_hwndLeft;
  case Edge::Right: return m_hwndRight;
  case Edge::Top: return m_hwndTop;
  case Edge::Bottom: return m_hwndBottom;
  }
  return m_hwndLeft;
}

Edge InputWindow::EdgeFromHwnd(HWND hWnd) {
  if (hWnd == m_hwndLeft) return Edge::Left;
  if (hWnd == m_hwndRight) return Edge::Right;
  if (hWnd == m_hwndTop) return Edge::Top;
  if (hWnd == m_hwndBottom) return Edge::Bottom;
  return Edge::Left;
}

static HWND CreateEdgeWindow(const wchar_t *name, InputWindow *pThis) {
  return CreateWindowExW(
      WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED,
      WINDOW_CLASS_NAME, name, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr,
      GetModuleHandle(NULL), pThis);
}

void InputWindow::CreateWindows() {
  m_hwndLeft = CreateEdgeWindow(L"OHO_Left", this);
  m_hwndRight = CreateEdgeWindow(L"OHO_Right", this);
  m_hwndTop = CreateEdgeWindow(L"OHO_Top", this);
  m_hwndBottom = CreateEdgeWindow(L"OHO_Bottom", this);

  // Core logic: visually transparent window that still captures input.
  // Alpha 0 makes it disappear, so use 1.
  HWND handles[4] = {m_hwndLeft, m_hwndRight, m_hwndTop, m_hwndBottom};
  for (auto h : handles) {
    if (h)
      SetLayeredWindowAttributes(h, 0, 1, LWA_ALPHA);
  }
}

static void LayoutEdge(HWND hwnd, const SideConfig &cfg, int previewModeIdx,
                       int curPreviewMode, int x, int y, int w, int h) {
  if (!hwnd)
    return;
  if (cfg.enabled) {
    SetLayeredWindowAttributes(hwnd, 0,
                               (curPreviewMode == previewModeIdx) ? 128 : 1,
                               LWA_ALPHA);
    // Use SetWindowPos with HWND_TOPMOST to ensure the input window stays
    // above the taskbar and other topmost windows.
    SetWindowPos(hwnd, HWND_TOPMOST, x, y, w, h,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
  } else {
    ShowWindow(hwnd, SW_HIDE);
  }
}

void InputWindow::UpdateLayout() {
  int screenW = GetSystemMetrics(SM_CXSCREEN);
  int screenH = GetSystemMetrics(SM_CYSCREEN);
  AppConfig &cfg = ConfigManager::Get().Current();

  // Left/Right: thickness is horizontal, extent is vertical.
  if (m_hwndLeft) {
    int h = (int)((cfg.left.size / 100.0f) * screenH);
    int top = (int)((cfg.left.position / 100.0f) * screenH) - (h / 2);
    if (top < 0) top = 0;
    if (top + h > screenH) top = screenH - h;
    LayoutEdge(m_hwndLeft, cfg.left, 1, m_previewMode, 0, top, cfg.left.width, h);
  }
  if (m_hwndRight) {
    int h = (int)((cfg.right.size / 100.0f) * screenH);
    int top = (int)((cfg.right.position / 100.0f) * screenH) - (h / 2);
    if (top < 0) top = 0;
    if (top + h > screenH) top = screenH - h;
    LayoutEdge(m_hwndRight, cfg.right, 2, m_previewMode,
               screenW - cfg.right.width, top, cfg.right.width, h);
  }
  // Top/Bottom: thickness is vertical, extent is horizontal.
  if (m_hwndTop) {
    int w = (int)((cfg.top.size / 100.0f) * screenW);
    int left = (int)((cfg.top.position / 100.0f) * screenW) - (w / 2);
    if (left < 0) left = 0;
    if (left + w > screenW) left = screenW - w;
    LayoutEdge(m_hwndTop, cfg.top, 3, m_previewMode, left, 0, w, cfg.top.width);
  }
  if (m_hwndBottom) {
    int w = (int)((cfg.bottom.size / 100.0f) * screenW);
    int left = (int)((cfg.bottom.position / 100.0f) * screenW) - (w / 2);
    if (left < 0) left = 0;
    if (left + w > screenW) left = screenW - w;
    LayoutEdge(m_hwndBottom, cfg.bottom, 4, m_previewMode, left,
               screenH - cfg.bottom.width, w, cfg.bottom.width);
  }
}

void InputWindow::ReassertTopmost() {
  AppConfig &cfg = ConfigManager::Get().Current();
  struct H { HWND hwnd; bool enabled; };
  H arr[4] = {
    {m_hwndLeft, cfg.left.enabled},
    {m_hwndRight, cfg.right.enabled},
    {m_hwndTop, cfg.top.enabled},
    {m_hwndBottom, cfg.bottom.enabled},
  };
  for (auto &it : arr) {
    if (it.hwnd && it.enabled) {
      SetWindowPos(it.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
  }
}

bool InputWindow::IsTouchInput() {
  ULONG_PTR extraInfo = GetMessageExtraInfo();
  if ((extraInfo & 0xFF515700) == 0xFF515700) {
    return true;
  }
  return false;
}

LRESULT CALLBACK InputWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                      LPARAM lParam) {
  InputWindow *pThis = nullptr;

  if (message == WM_NCCREATE) {
    CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
    pThis = (InputWindow *)pCreate->lpCreateParams;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
  } else {
    pThis = (InputWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  }

  if (pThis) {
    switch (message) {

    case WM_LBUTTONDOWN: {
      pThis->m_isDragging = true;
      SetCapture(hWnd);

      Edge edge = pThis->EdgeFromHwnd(hWnd);
      POINT pt;
      pt.x = (short)LOWORD(lParam);
      pt.y = (short)HIWORD(lParam);
      ClientToScreen(hWnd, &pt);

      // a = along-edge coordinate, b = perpendicular
      int a = pt.y;
      int b = pt.x;
      if (edge == Edge::Top || edge == Edge::Bottom) {
        a = pt.x;
        b = pt.y;
      }

      if (pThis->OnStart) {
        pThis->OnStart(edge, a, b);
      }
      return 0;
    }

    case WM_MOUSEMOVE: {
      if (pThis->m_isDragging) {
        POINT pt;
        pt.x = (short)LOWORD(lParam);
        pt.y = (short)HIWORD(lParam);
        ClientToScreen(hWnd, &pt);
        if (pThis->OnUpdate)
          pThis->OnUpdate(pt.x, pt.y);
      }
      // Hover suppression removed — input windows must always stay visible
      // to receive the next touch gesture immediately.
      break;
    }

    case WM_MOUSELEAVE: {
      break;
    }

    case WM_TIMER: {
      if (wParam == TIMER_WATCHDOG) {
        AppConfig &cfg = ConfigManager::Get().Current();
        struct H { HWND hwnd; const SideConfig *cfg; };
        H arr[4] = {
          {pThis->m_hwndLeft, &cfg.left},
          {pThis->m_hwndRight, &cfg.right},
          {pThis->m_hwndTop, &cfg.top},
          {pThis->m_hwndBottom, &cfg.bottom},
        };
        for (auto &it : arr) {
          if (it.hwnd && it.cfg->enabled && !IsWindowVisible(it.hwnd)) {
            SetWindowPos(it.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
          }
        }
      }
      break;
    }

    case WM_LBUTTONUP: {
      if (pThis->m_isDragging) {
        pThis->m_isDragging = false;
        ReleaseCapture();
        if (pThis->OnEnd) {
          pThis->OnEnd();
        }
      }
      break;
    }

    case WM_DISPLAYCHANGE: {
      pThis->UpdateLayout();
      break;
    }
    case WM_USER + 101: {
      ConfigManager::Get().Load();
      pThis->UpdateLayout();
      break;
    }
    case WM_USER + 102: {
      // Preview Mode: wParam = 0 (Off), 1 (Left), 2 (Right), 3 (Top), 4 (Bottom)
      pThis->m_previewMode = (int)wParam;
      pThis->UpdateLayout();
      break;
    }
    }
  }

  return DefWindowProc(hWnd, message, wParam, lParam);
}
