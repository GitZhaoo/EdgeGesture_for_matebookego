#include "ActionDispatcher.h"
#include "core/ConfigManager.h"
#include <iostream>
#include <shellapi.h>
#include <sstream>

ActionDispatcher::ActionDispatcher() {}

ActionDispatcher::~ActionDispatcher() {}

void ActionDispatcher::Trigger(const std::string &actionName) {
  std::cout << "[Action] Triggering: " << actionName << std::endl;

  AppConfig &cfg = ConfigManager::Get().Current();
  std::string command = actionName;

  if (cfg.actionMap.count(actionName)) {
    command = cfg.actionMap[actionName];
    std::cout << "  -> Resolved to: " << command << std::endl;
  }

  if (command == "none" || command.empty()) {
    return;
  }

  // Launch application: "launch:<path> [args...]"
  if (command.rfind("launch:", 0) == 0) {
    LaunchApp(command);
    return;
  }

  // Parse and execute key sequence
  std::vector<INPUT> inputs = ParseKeySequence(command);
  if (!inputs.empty()) {
    SendKeySequence(inputs);
  } else {
    std::cerr << "[Action] Parse failed or empty sequence for: " << command
              << std::endl;
  }
}

void ActionDispatcher::LaunchApp(const std::string &command) {
  // Strip "launch:" prefix
  std::string rest = command.substr(7);

  // Trim leading whitespace
  while (!rest.empty() && (rest[0] == ' ' || rest[0] == '\t'))
    rest.erase(rest.begin());

  if (rest.empty())
    return;

  std::string exePath;
  std::string args;

  if (rest[0] == '"') {
    // Quoted path
    size_t endQuote = rest.find('"', 1);
    if (endQuote == std::string::npos) {
      exePath = rest.substr(1);
    } else {
      exePath = rest.substr(1, endQuote - 1);
      if (endQuote + 1 < rest.size())
        args = rest.substr(endQuote + 1);
    }
  } else {
    // Unquoted: split on first space
    size_t sp = rest.find(' ');
    if (sp == std::string::npos) {
      exePath = rest;
    } else {
      exePath = rest.substr(0, sp);
      args = rest.substr(sp + 1);
    }
  }

  // Trim args
  while (!args.empty() && (args[0] == ' ' || args[0] == '\t'))
    args.erase(args.begin());

  std::cout << "[Action] Launching: " << exePath << " | Args: " << args
            << std::endl;

  // Use ShellExecuteW for robustness with UAC / paths containing spaces.
  std::wstring wExe(exePath.begin(), exePath.end());
  std::wstring wArgs(args.begin(), args.end());

  HINSTANCE hInst = ShellExecuteW(
      nullptr, L"open", wExe.c_str(),
      wArgs.empty() ? nullptr : wArgs.c_str(), nullptr, SW_SHOWNORMAL);

  // ShellExecuteW returns a value > 32 on success.
  if ((INT_PTR)hInst <= 32) {
    std::cerr << "[Action] ShellExecute failed, code: " << (INT_PTR)hInst
              << std::endl;
  }
}

std::vector<INPUT>
ActionDispatcher::ParseKeySequence(const std::string &sequence) {
  std::vector<INPUT> inputs;
  std::vector<WORD> keysDown;

  std::stringstream ss(sequence);
  std::string segment;

  while (std::getline(ss, segment, '+')) {
    WORD vk = GetKeyCode(segment);
    if (vk == 0) {
      std::cerr << "  -> Unknown key: " << segment << std::endl;
      continue;
    }

    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    inputs.push_back(input);
    keysDown.push_back(vk);
  }

  for (auto it = keysDown.rbegin(); it != keysDown.rend(); ++it) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = *it;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    inputs.push_back(input);
  }

  return inputs;
}

WORD ActionDispatcher::GetKeyCode(const std::string &keyName) {
  std::string k = keyName;
  // Lowercase the string for comparison
  for (auto &c : k)
    c = (char)tolower(c);

  if (k == "ctrl" || k == "control")
    return VK_CONTROL;
  if (k == "alt" || k == "menu")
    return VK_MENU;
  if (k == "shift")
    return VK_SHIFT;
  if (k == "win" || k == "cmd" || k == "super")
    return VK_LWIN;
  if (k == "tab")
    return VK_TAB;
  if (k == "esc")
    return VK_ESCAPE;
  if (k == "enter" || k == "return")
    return VK_RETURN;
  if (k == "space")
    return VK_SPACE;
  if (k == "backspace")
    return VK_BACK;
  if (k == "delete" || k == "del")
    return VK_DELETE;

  if (k == "left" || k == "key_left")
    return VK_LEFT;
  if (k == "right" || k == "key_right")
    return VK_RIGHT;
  if (k == "up" || k == "key_up")
    return VK_UP;
  if (k == "down" || k == "key_down")
    return VK_DOWN;

  if (k.length() == 1) {
    char c = toupper(k[0]);
    if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
      return (WORD)c;
    }
  }

  if (k.length() >= 2 && (k[0] == 'f')) {
    try {
      int num = std::stoi(k.substr(1));
      if (num >= 1 && num <= 24)
        return (WORD)(VK_F1 + (num - 1));
    } catch (...) {
    }
  }

  if (k == "volume_up" || k == "vol_up")
    return VK_VOLUME_UP;
  if (k == "volume_down" || k == "vol_down")
    return VK_VOLUME_DOWN;
  if (k == "volume_mute" || k == "mute")
    return VK_VOLUME_MUTE;
  if (k == "media_next" || k == "next")
    return VK_MEDIA_NEXT_TRACK;
  if (k == "media_prev" || k == "prev")
    return VK_MEDIA_PREV_TRACK;
  if (k == "media_play_pause" || k == "play_pause")
    return VK_MEDIA_PLAY_PAUSE;
  if (k == "media_stop")
    return VK_MEDIA_STOP;

  return 0;
}

void ActionDispatcher::SendKeySequence(const std::vector<INPUT> &inputs) {
  if (!inputs.empty())
    SendInput((UINT)inputs.size(), (LPINPUT)inputs.data(), sizeof(INPUT));
}
