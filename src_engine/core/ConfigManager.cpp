#include "ConfigManager.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

SideConfig &ConfigManager::Side(Edge e) {
  switch (e) {
  case Edge::Left: return m_config.left;
  case Edge::Right: return m_config.right;
  case Edge::Top: return m_config.top;
  case Edge::Bottom: return m_config.bottom;
  }
  return m_config.left;
}

void ConfigManager::Load() {
  if (!std::filesystem::exists(m_configPath)) {
    json j;
    j["physics"] = {{"tension", 0.35}, {"friction", 0.65}};
    j["general"] = {{"trigger_threshold", 90.0},
                    {"max_wave_x", 160.0},
                    {"long_swipe_threshold", 450.0},
                    {"short_swipe_threshold", 30.0}};

    auto sideObj = [](bool en, int w, int sz, int pos, const char *c) {
      return json{{"enabled", en},
                  {"width", w},
                  {"size", sz},
                  {"position", pos},
                  {"color", c}};
    };

    j["left_handle"] = sideObj(true, 12, 80, 50, "#000000");
    j["right_handle"] = sideObj(true, 12, 80, 50, "#000000");
    j["top_handle"] = sideObj(true, 12, 80, 50, "#000000");
    j["bottom_handle"] = sideObj(true, 12, 100, 50, "#000000");

    j["gestures"] = {
        {"left_right", "Back"},
        {"left_long_right", "QuickPanel"},
        {"right_left", "Forward"},
        {"right_long_left", "TaskView"},
        {"bottom_left_up", "TaskView"},
        {"bottom_right_up", "QuickPanel"},
        {"top_left_down", "Back"},
        {"top_right_down", "TaskView"}};

    j["actions"] = {
        {"Back", "alt+left"},        {"TaskView", "win+tab"},
        {"ShowDesktop", "win+d"},    {"QuickPanel", "win+a"},
        {"VolumeUp", "volume_up"},   {"VolumeDown", "volume_down"},
        {"Mute", "volume_mute"},     {"PlayPause", "media_play_pause"},
        {"NextTrack", "media_next"}, {"PrevTrack", "media_prev"}};

    j["language"] = "zh_CN";

    std::ofstream o(m_configPath);
    o << std::setw(4) << j << std::endl;
  }

  LoadFromPath(m_configPath);
  m_currentProfile = "default";
}

void ConfigManager::LoadProfile(const std::string &appName) {
  if (appName == m_currentProfile) {
    return;
  }

  std::string profilePath = m_configsDir + "/" + appName + ".json";
  std::string defaultPath = m_configsDir + "/default.json";

  if (std::filesystem::exists(profilePath)) {
    LoadFromPath(profilePath);
    m_currentProfile = appName;
    std::cout << "[Config] Loaded profile: " << appName << std::endl;
  } else if (std::filesystem::exists(defaultPath)) {
    LoadFromPath(defaultPath);
    m_currentProfile = "default";
    std::cout << "[Config] Profile not found, using default" << std::endl;
  } else {
    std::cout << "[Config] No profile or default found, keeping current"
              << std::endl;
    return;
  }

  if (m_profileChangeCb) {
    m_profileChangeCb(m_currentProfile);
  }
}

static void LoadSide(const json &j, const char *key, SideConfig &out) {
  if (!j.contains(key))
    return;
  auto &s = j[key];
  out.enabled = s.value("enabled", out.enabled);
  out.width = s.value("width", out.width);
  out.size = s.value("size", out.size);
  out.position = s.value("position", out.position);
  out.color = s.value("color", out.color);
}

void ConfigManager::LoadFromPath(const std::string &path) {
  try {
    std::ifstream i(path);
    json j;
    i >> j;

    bool modified = false;
    if (!j.contains("actions")) {
      j["actions"] = json::object();
      modified = true;
    }

    std::map<std::string, std::string> defaults = {
        {"Back", "alt+left"},        {"TaskView", "win+tab"},
        {"ShowDesktop", "win+d"},    {"QuickPanel", "win+a"},
        {"VolumeUp", "volume_up"},   {"VolumeDown", "volume_down"},
        {"Mute", "volume_mute"},     {"PlayPause", "media_play_pause"},
        {"NextTrack", "media_next"}, {"PrevTrack", "media_prev"}};

    for (const auto &[key, val] : defaults) {
      if (!j["actions"].contains(key)) {
        j["actions"][key] = val;
        modified = true;
      }
    }

    if (modified) {
      std::ofstream o(path);
      o << std::setw(4) << j << std::endl;
      std::cout << "[Config] Updated config with missing actions." << std::endl;
    }

    // Ensure default gestures exist (for configs created by older versions)
    if (!j.contains("gestures")) {
      j["gestures"] = {
          {"left_right", "Back"},
          {"left_long_right", "QuickPanel"},
          {"right_left", "Forward"},
          {"right_long_left", "TaskView"},
          {"bottom_left_up", "TaskView"},
          {"bottom_right_up", "QuickPanel"},
          {"top_left_down", "Back"},
          {"top_right_down", "TaskView"}};
      std::ofstream o(path);
      o << std::setw(4) << j << std::endl;
      std::cout << "[Config] Added default gestures." << std::endl;
    }

    if (j.contains("physics")) {
      m_config.tension = j["physics"].value("tension", 0.35f);
      m_config.friction = j["physics"].value("friction", 0.65f);
    }

    if (j.contains("general")) {
      m_config.triggerThreshold =
          j["general"].value("trigger_threshold", 90.0f);
      m_config.maxWaveX = j["general"].value("max_wave_x", 160.0f);
      m_config.verticalRange = j["general"].value("vertical_range", 50);
      m_config.splitMode = j["general"].value("split_mode", 0);
      m_config.longSwipeThreshold =
          j["general"].value("long_swipe_threshold", 450.0f);
      m_config.shortSwipeThreshold =
          j["general"].value("short_swipe_threshold", 30.0f);
    }

    LoadSide(j, "left_handle", m_config.left);
    LoadSide(j, "right_handle", m_config.right);
    LoadSide(j, "top_handle", m_config.top);
    LoadSide(j, "bottom_handle", m_config.bottom);

    if (j.contains("actions")) {
      m_config.actionMap.clear();
      for (auto &el : j["actions"].items()) {
        m_config.actionMap[el.key()] = el.value();
      }
    }

    if (j.contains("gestures")) {
      m_config.gestureMap.clear();
      for (auto &el : j["gestures"].items()) {
        m_config.gestureMap[el.key()] = el.value();
      }
    }

    m_config.blacklist.clear();
    if (j.contains("blacklist")) {
      for (const auto &val : j["blacklist"]) {
        m_config.blacklist.push_back(val.get<std::string>());
      }
    }

    std::cout << "[Config] Loaded from: " << path << std::endl;

  } catch (std::exception &e) {
    std::cerr << "[Config] Error loading config: " << e.what() << std::endl;
  }
}

COLORREF ConfigManager::GetColorRef(const std::string &hex) {
  if (hex.length() < 7 || hex[0] != '#')
    return RGB(0, 0, 0);
  int r, g, b;
  sscanf_s(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);
  return RGB(r, g, b);
}
