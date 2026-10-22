#pragma once
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <windows.h>

using json = nlohmann::json;
#include <vector>

enum class Edge { Left, Right, Top, Bottom };

inline const char *EdgeName(Edge e) {
  switch (e) {
  case Edge::Left: return "left";
  case Edge::Right: return "right";
  case Edge::Top: return "top";
  case Edge::Bottom: return "bottom";
  }
  return "left";
}

struct SideConfig {
  bool enabled = true;
  int width = 30;     // thickness (px) of the edge handle
  int size = 100;     // extent along the edge (%)
  int position = 50;  // center offset along the edge (%)
  std::string color = "#000000";
};

struct AppConfig {
  float tension = 0.35f;
  float friction = 0.65f;
  float triggerThreshold = 90.0f;
  float maxWaveX = 160.0f;
  int verticalRange = 50;

  // Split Mode: 0=None, 1=Two, 2=Three (applies to left/right vertical edges)
  int splitMode = 0;

  // Swipe Thresholds
  float longSwipeThreshold = 450.0f;
  float shortSwipeThreshold = 30.0f;

  SideConfig left;
  SideConfig right;
  SideConfig top;
  SideConfig bottom;

  // Gestures key: "left_right", "bottom_left_up", "top_right_down", etc.
  // value: action name (e.g. "Back", "Launch_MyApp")
  std::map<std::string, std::string> gestureMap;

  // Actions key: action name (e.g. "Back", "Launch_MyApp")
  // value: command string (e.g. "alt+left", "launch:C:\app\app.exe")
  std::map<std::string, std::string> actionMap;

  std::vector<std::string> blacklist;
};

class ConfigManager {
public:
  static ConfigManager &Get() {
    static ConfigManager instance;
    return instance;
  }

  void Load();
  void LoadProfile(const std::string &appName);
  AppConfig &Current() { return m_config; }
  std::string CurrentProfileName() const { return m_currentProfile; }

  using ProfileChangeCallback = std::function<void(const std::string &)>;
  void SetProfileChangeCallback(ProfileChangeCallback cb) {
    m_profileChangeCb = cb;
  }

  COLORREF GetColorRef(const std::string &hexFunc);

  SideConfig &Side(Edge e);

private:
  ConfigManager() = default;
  void LoadFromPath(const std::string &path);

  AppConfig m_config;
  std::string m_configPath = "config.json";
  std::string m_configsDir = "configs";
  std::string m_currentProfile = "default";
  ProfileChangeCallback m_profileChangeCb;
};
