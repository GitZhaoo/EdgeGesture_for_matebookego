#pragma once
#include <string>
#include <vector>
#include <windows.h>

enum class ActionType { None, Back, TaskView, ShowDesktop, QuickPanel };

class ActionDispatcher {
public:
  ActionDispatcher();
  ~ActionDispatcher();

  void Trigger(const std::string &actionName);

  // Launch an executable. Supports optional arguments separated by a space
  // after the path. Path may be quoted if it contains spaces.
  // command format: launch:C:\path\app.exe arg1 arg2
  void LaunchApp(const std::string &command);

private:
  void SendKeySequence(const std::vector<INPUT> &inputs);
  std::vector<INPUT> ParseKeySequence(const std::string &sequence);
  WORD GetKeyCode(const std::string &keyName);
};
