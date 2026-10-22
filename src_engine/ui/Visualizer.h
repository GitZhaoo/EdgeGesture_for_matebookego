#pragma once
#include "core/ConfigManager.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <functional>
#include <windows.h>

#pragma comment(lib, "d2d1.lib")

class Visualizer {
public:
  Visualizer();
  ~Visualizer();

  bool Init(int screenW, int screenH);
  void Update(float currentX, float currentA, float anchorA, Edge edge,
              bool isTriggered, const std::string &gestureIcon);
  void Render();

  void SetTimerCallback(std::function<void()> cb) { m_onTimer = cb; }

  HWND GetHwnd() const { return m_hwnd; }
  void SetWindowVisible(bool visible);

private:
  std::function<void()> m_onTimer;
  HWND m_hwnd = nullptr;
  ID2D1Factory *m_pD2DFactory = nullptr;
  ID2D1DCRenderTarget *m_pDCRT = nullptr;

  ID2D1SolidColorBrush *m_pWaveBrush = nullptr;
  ID2D1SolidColorBrush *m_pActiveBrush = nullptr;
  ID2D1SolidColorBrush *m_pArrowBrush = nullptr;

  int m_width = 300;   // window width (covers screen-wide for top/bottom)
  int m_height = 300;  // window height (covers screen-tall for left/right)
  int m_screenWidth = 0;
  int m_screenHeight = 0;

  float m_drawX = 0.0f;   // pull depth
  float m_drawA = 0.0f;   // along-edge position
  float m_anchorA = 0.0f; // along-edge anchor
  Edge m_edge = Edge::Left;
  bool m_triggered = false;
  std::string m_gestureIcon;

  void CreateResources();
  void DrawWave();
  void DrawArrow();
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
};
