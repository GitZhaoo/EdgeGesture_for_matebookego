#include "Visualizer.h"
#include <algorithm>
#include <iostream>

Visualizer::Visualizer() {}

Visualizer::~Visualizer() {
  if (m_pWaveBrush)
    m_pWaveBrush->Release();
  if (m_pActiveBrush)
    m_pActiveBrush->Release();
  if (m_pArrowBrush)
    m_pArrowBrush->Release();
  if (m_pDCRT)
    m_pDCRT->Release();
  if (m_pD2DFactory)
    m_pD2DFactory->Release();
  if (m_hwnd)
    DestroyWindow(m_hwnd);
}

LRESULT CALLBACK Visualizer::WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                     LPARAM lParam) {
  if (message == WM_TIMER) {
    Visualizer *pThis = (Visualizer *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (pThis && pThis->m_onTimer) {
      pThis->m_onTimer();
    }
    return 0;
  } else if (message == WM_USER + 101) {
    ConfigManager::Get().Load();
    return 0;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

bool Visualizer::Init(int screenW, int screenH) {
  m_screenWidth = screenW;
  m_screenHeight = screenH;

  WNDCLASSEXW wcex = {sizeof(WNDCLASSEX)};
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = GetModuleHandle(NULL);
  wcex.lpszClassName = L"OHOVisualizer";
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassExW(&wcex);

  // Create a screen-sized window; we position/clip it as needed for each edge.
  m_hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW |
                               WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
                           L"OHOVisualizer", L"", WS_POPUP, 0, 0,
                           screenW, screenH, nullptr, nullptr,
                           GetModuleHandle(NULL), this);

  if (!m_hwnd)
    return false;

  SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

  HRESULT hr =
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
  if (FAILED(hr))
    return false;

  CreateResources();
  return true;
}

void Visualizer::CreateResources() {
  if (!m_pDCRT) {
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                          D2D1_ALPHA_MODE_PREMULTIPLIED));
    m_pD2DFactory->CreateDCRenderTarget(&props, &m_pDCRT);
  }
}

void Visualizer::Update(float currentX, float currentA, float anchorA,
                        Edge edge, bool isTriggered,
                        const std::string &gestureIcon) {
  m_drawX = currentX;
  AppConfig &cfg = ConfigManager::Get().Current();
  int range = cfg.verticalRange;
  if (currentA < anchorA - range)
    m_drawA = anchorA - range;
  else if (currentA > anchorA + range)
    m_drawA = anchorA + range;
  else
    m_drawA = currentA;

  m_anchorA = anchorA;
  m_edge = edge;
  m_triggered = isTriggered;
  m_gestureIcon = gestureIcon;

  // Position the window to cover the relevant side strip.
  int screenW = GetSystemMetrics(SM_CXSCREEN);
  int screenH = GetSystemMetrics(SM_CYSCREEN);
  int x = 0, y = 0, w = screenW, h = screenH;
  switch (edge) {
  case Edge::Left: x = 0; y = 0; w = m_width; h = screenH; break;
  case Edge::Right: x = screenW - m_width; y = 0; w = m_width; h = screenH; break;
  case Edge::Top: x = 0; y = 0; w = screenW; h = m_height; break;
  case Edge::Bottom: x = 0; y = screenH - m_height; w = screenW; h = m_height; break;
  }
  SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, w, h,
               SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void Visualizer::Render() {
  if (!m_pD2DFactory)
    return;

  int screenW = GetSystemMetrics(SM_CXSCREEN);
  int screenH = GetSystemMetrics(SM_CYSCREEN);

  int winW, winH;
  RECT rcPos;
  GetWindowRect(m_hwnd, &rcPos);
  winW = rcPos.right - rcPos.left;
  winH = rcPos.bottom - rcPos.top;

  HDC hScreenDC = GetDC(NULL);
  HDC hMemDC = CreateCompatibleDC(hScreenDC);

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = winW;
  bmi.bmiHeader.biHeight = winH;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void *pBits = nullptr;
  HBITMAP hBitmap =
      CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

  CreateResources();
  RECT rc = {0, 0, winW, winH};
  m_pDCRT->BindDC(hMemDC, &rc);
  m_pDCRT->BeginDraw();
  m_pDCRT->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));

  if (m_drawX > 2.0f) {
    AppConfig &cfg = ConfigManager::Get().Current();
    const SideConfig *sideCfg = nullptr;
    switch (m_edge) {
    case Edge::Left: sideCfg = &cfg.left; break;
    case Edge::Right: sideCfg = &cfg.right; break;
    case Edge::Top: sideCfg = &cfg.top; break;
    case Edge::Bottom: sideCfg = &cfg.bottom; break;
    }
    std::string hex = sideCfg ? sideCfg->color : "#000000";
    COLORREF color = ConfigManager::Get().GetColorRef(hex);
    float r = GetRValue(color) / 255.0f;
    float g = GetGValue(color) / 255.0f;
    float b = GetBValue(color) / 255.0f;

    if (m_pWaveBrush) m_pWaveBrush->Release();
    if (m_pActiveBrush) m_pActiveBrush->Release();
    if (m_pArrowBrush) m_pArrowBrush->Release();

    m_pDCRT->CreateSolidColorBrush(D2D1::ColorF(r, g, b, 0.5f), &m_pWaveBrush);
    m_pDCRT->CreateSolidColorBrush(D2D1::ColorF(r, g, b, 0.8f), &m_pActiveBrush);
    m_pDCRT->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.9f),
                                   &m_pArrowBrush);

    DrawWave();
    DrawArrow();
  }

  m_pDCRT->EndDraw();

  POINT ptSrc = {0, 0};
  SIZE sizeWnd = {winW, winH};
  POINT ptDst = {rcPos.left, rcPos.top};

  BLENDFUNCTION blend = {0};
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = 255;
  blend.AlphaFormat = AC_SRC_ALPHA;

  UpdateLayeredWindow(m_hwnd, hScreenDC, &ptDst, &sizeWnd, hMemDC, &ptSrc, 0,
                      &blend, ULW_ALPHA);

  SelectObject(hMemDC, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hMemDC);
  ReleaseDC(NULL, hScreenDC);
}

// Draws a curved wave for the current edge.
// The wave grows straight out from the edge (perpendicular), centered on
// the anchor point. It does NOT follow the finger's along-edge movement,
// so there is no diagonal tilt — just a natural vertical curve.
void Visualizer::DrawWave() {
  ID2D1SolidColorBrush *currentBrush =
      m_triggered ? m_pActiveBrush : m_pWaveBrush;

  float waveHalf = 140.0f;
  float edgeTension = waveHalf * 0.5f;
  float peakTension = waveHalf * 0.35f;
  float pull = m_drawX;

  // Use the ANCHOR position (not drawA) so the wave stays centered and
  // grows straight without tilting.
  float centerPos = m_anchorA;

  RECT rcPos;
  GetWindowRect(m_hwnd, &rcPos);
  int originA = (m_edge == Edge::Left || m_edge == Edge::Right)
                    ? rcPos.top
                    : rcPos.left;
  centerPos -= originA;

  ID2D1PathGeometry *pGeo = nullptr;
  m_pD2DFactory->CreatePathGeometry(&pGeo);
  ID2D1GeometrySink *pSink = nullptr;
  pGeo->Open(&pSink);

  if (m_edge == Edge::Left) {
    float baseTopY = centerPos - waveHalf;
    float baseBottomY = centerPos + waveHalf;
    float peakX = pull;
    pSink->BeginFigure(D2D1::Point2F(0, baseTopY), D2D1_FIGURE_BEGIN_FILLED);
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(0, baseTopY + edgeTension),
        D2D1::Point2F(peakX, centerPos - peakTension),
        D2D1::Point2F(peakX, centerPos)));
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(peakX, centerPos + peakTension),
        D2D1::Point2F(0, baseBottomY - edgeTension),
        D2D1::Point2F(0, baseBottomY)));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
  } else if (m_edge == Edge::Right) {
    float startX = (float)(rcPos.right - rcPos.left);
    float peakX = startX - pull;
    float baseTopY = centerPos - waveHalf;
    float baseBottomY = centerPos + waveHalf;
    pSink->BeginFigure(D2D1::Point2F(startX, baseTopY),
                       D2D1_FIGURE_BEGIN_FILLED);
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(startX, baseTopY + edgeTension),
        D2D1::Point2F(peakX, centerPos - peakTension),
        D2D1::Point2F(peakX, centerPos)));
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(peakX, centerPos + peakTension),
        D2D1::Point2F(startX, baseBottomY - edgeTension),
        D2D1::Point2F(startX, baseBottomY)));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
  } else if (m_edge == Edge::Top) {
    float baseLeftX = centerPos - waveHalf;
    float baseRightX = centerPos + waveHalf;
    float peakY = pull;
    pSink->BeginFigure(D2D1::Point2F(baseLeftX, 0), D2D1_FIGURE_BEGIN_FILLED);
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(baseLeftX + edgeTension, 0),
        D2D1::Point2F(centerPos - peakTension, peakY),
        D2D1::Point2F(centerPos, peakY)));
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(centerPos + peakTension, peakY),
        D2D1::Point2F(baseRightX - edgeTension, 0),
        D2D1::Point2F(baseRightX, 0)));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
  } else { // Bottom
    float startY = (float)(rcPos.bottom - rcPos.top);
    float peakY = startY - pull;
    float baseLeftX = centerPos - waveHalf;
    float baseRightX = centerPos + waveHalf;
    pSink->BeginFigure(D2D1::Point2F(baseLeftX, startY),
                       D2D1_FIGURE_BEGIN_FILLED);
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(baseLeftX + edgeTension, startY),
        D2D1::Point2F(centerPos - peakTension, peakY),
        D2D1::Point2F(centerPos, peakY)));
    pSink->AddBezier(D2D1::BezierSegment(
        D2D1::Point2F(centerPos + peakTension, peakY),
        D2D1::Point2F(baseRightX - edgeTension, startY),
        D2D1::Point2F(baseRightX, startY)));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
  }

  pSink->Close();
  m_pDCRT->FillGeometry(pGeo, currentBrush);

  pSink->Release();
  pGeo->Release();
}

void Visualizer::DrawArrow() {
  if (!m_pArrowBrush || m_drawX < 20.0f)
    return;

  AppConfig &cfg = ConfigManager::Get().Current();

  // Anchor position in window-local coordinates
  RECT rcPos;
  GetWindowRect(m_hwnd, &rcPos);
  int originA = (m_edge == Edge::Left || m_edge == Edge::Right)
                    ? rcPos.top
                    : rcPos.left;
  float drawPosLocal = m_anchorA - originA;

  // Center of the bar in window-local coordinates
  float centerX = 0, centerY = 0;
  float pull = m_drawX * 0.6f;
  switch (m_edge) {
  case Edge::Left:
    centerX = pull; centerY = drawPosLocal; break;
  case Edge::Right: {
    float startX = (float)(rcPos.right - rcPos.left);
    centerX = startX - pull; centerY = drawPosLocal; break;
  }
  case Edge::Top:
    centerX = drawPosLocal; centerY = pull; break;
  case Edge::Bottom: {
    float startY = (float)(rcPos.bottom - rcPos.top);
    centerX = drawPosLocal; centerY = startY - pull; break;
  }
  }

  // Straight arrow — no diagonal tilt
  float baseAngle = 0.0f;
  if (m_edge == Edge::Top) baseAngle = 90.0f;
  if (m_edge == Edge::Bottom) baseAngle = -90.0f;
  if (m_edge == Edge::Right) baseAngle = 180.0f;

  D2D1::Matrix3x2F oldTransform;
  m_pDCRT->GetTransform(&oldTransform);
  m_pDCRT->SetTransform(
      D2D1::Matrix3x2F::Rotation(baseAngle, D2D1::Point2F(centerX, centerY)) *
      oldTransform);

  ID2D1PathGeometry *pGeo = nullptr;
  m_pD2DFactory->CreatePathGeometry(&pGeo);
  ID2D1GeometrySink *pSink = nullptr;
  pGeo->Open(&pSink);

  float arrowSize = 12.0f;
  int arrowCount = (m_drawX >= cfg.longSwipeThreshold) ? 2 : 1;
  float offset = 8.0f;
  for (int i = 0; i < arrowCount; i++) {
    float shiftX = i * offset;
    pSink->BeginFigure(
        D2D1::Point2F(centerX + shiftX + arrowSize * 0.6f, centerY - arrowSize),
        D2D1_FIGURE_BEGIN_HOLLOW);
    pSink->AddLine(D2D1::Point2F(centerX + shiftX - arrowSize * 0.4f, centerY));
    pSink->AddLine(D2D1::Point2F(centerX + shiftX + arrowSize * 0.6f,
                                 centerY + arrowSize));
    pSink->EndFigure(D2D1_FIGURE_END_OPEN);
  }

  pSink->Close();
  m_pDCRT->DrawGeometry(pGeo, m_pArrowBrush, 3.0f);

  pSink->Release();
  pGeo->Release();
  m_pDCRT->SetTransform(oldTransform);
}

void Visualizer::SetWindowVisible(bool visible) {
  if (m_hwnd) {
    if (visible) {
      ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
      SetWindowPos(m_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    } else {
      ShowWindow(m_hwnd, SW_HIDE);
    }
  }
}
