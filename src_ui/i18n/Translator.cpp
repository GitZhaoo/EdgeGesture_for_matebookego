#include "Translator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

// Embedded translation tables. We avoid the heavyweight Qt .ts/.qm toolchain
// (which requires lupdate/lrelease at build time) by storing strings inline
// in JSON loaded at runtime. This keeps the build self-contained.

static const char *kZhCN =
    "{"
    "\"app_title\":\"EdgeGesture 设置\","
    "\"nav_home\":\"主页\","
    "\"nav_handles\":\"触发区\","
    "\"nav_gestures\":\"手势\","
    "\"nav_actions\":\"动作\","
    "\"nav_advanced\":\"高级\","
    "\"general\":\"通用\","
    "\"enable_engine\":\"启用手势引擎\","
    "\"vertical_trigger_range\":\"纵向触发范围\","
    "\"split_mode\":\"分区模式\","
    "\"split_none\":\"不分区\","
    "\"split_two\":\"两段（左右半）\","
    "\"split_three\":\"三段（左中右）\","
    "\"short_swipe_threshold\":\"短滑阈值\","
    "\"long_swipe_threshold\":\"长滑阈值\","
    "\"trigger_threshold\":\"触发阈值\","
    "\"trigger_threshold_hint\":\"拉出多远才执行动作\","
    "\"handles\":\"触发区\","
    "\"application\":\"应用程序\","
    "\"exit_app\":\"退出应用程序\","
    "\"left_handle\":\"左侧触发区\","
    "\"right_handle\":\"右侧触发区\","
    "\"top_handle\":\"顶部触发区\","
    "\"bottom_handle\":\"底部触发区\","
    "\"appearance_position\":\"外观与位置\","
    "\"select_side\":\"选择边：\","
    "\"width\":\"宽度\","
    "\"size\":\"尺寸\","
    "\"position\":\"位置\","
    "\"color\":\"颜色\","
    "\"gesture_settings\":\"手势设置\","
    "\"select_edge\":\"选择边：\","
    "\"select_zone\":\"选择区段：\","
    "\"zone_global\":\"全局\","
    "\"zone_top\":\"上半\","
    "\"zone_bottom\":\"下半\","
    "\"zone_middle\":\"中段\","
    "\"zone_left\":\"左半\","
    "\"zone_right\":\"右半\","
    "\"short_swipe\":\"短滑\","
    "\"long_swipe\":\"长滑\","
    "\"straight\":\"直滑\","
    "\"diag_up\":\"斜向上\","
    "\"diag_down\":\"斜向下\","
    "\"diag_left\":\"斜向左\","
    "\"diag_right\":\"斜向右\","
    "\"long_straight\":\"长直滑\","
    "\"none\":\"无\","
    "\"actions\":\"动作\","
    "\"register_shortcuts\":\"在此注册自定义快捷键或启动应用。\","
    "\"add_new_action\":\"添加新动作\","
    "\"add_action\":\"添加动作\","
    "\"action_name\":\"动作名称（例如：复制）\","
    "\"action_type\":\"动作类型：\","
    "\"type_shortcut\":\"快捷键\","
    "\"type_launch\":\"启动应用\","
    "\"shortcut\":\"快捷键\","
    "\"shortcut_hint\":\"快捷键（例如 ctrl+c）\","
    "\"app_path\":\"程序路径\","
    "\"app_args\":\"启动参数（可选）\","
    "\"browse\":\"浏览…\","
    "\"record_shortcut\":\"录制快捷键\","
    "\"stop_recording\":\"停止录制\","
    "\"press_record\":\"按“录制”开始录制快捷键\","
    "\"press_keys\":\"按键中…\","
    "\"add\":\"添加\","
    "\"cancel\":\"取消\","
    "\"advanced_settings\":\"高级设置\","
    "\"animation\":\"动画\","
    "\"start_with_windows\":\"开机自启\","
    "\"start_minimized_hint\":\"开启后将静默启动并最小化到通知栏\","
    "\"language\":\"语言\","
    "\"per_app_profiles\":\"应用专属配置\","
    "\"per_app_desc\":\"为指定应用创建独立的手势配置。当该应用处于前台时，会自动应用其配置。\","
    "\"app_executable\":\"应用可执行文件（例如 notepad.exe）\","
    "\"find\":\"查找\","
    "\"refresh\":\"刷新\","
    "\"you_are_editing\":\"正在编辑：\","
    "\"use_default_profile\":\"使用默认配置\","
    "\"no_profiles\":\"尚未配置应用专属配置。\\n点击“添加”或“查找”创建一个。\","
    "\"blacklist\":\"黑名单（应用例外）\","
    "\"open_settings\":\"打开设置\","
    "\"quit\":\"退出\","
    "\"edge_swipe_hint\":\"从屏幕边缘滑动以返回上一页、打开任务视图或触发自定义动作。\","
    "\"long\":\"px\","
    "\"percent\":\"%\","
    "\"add_launch_hint\":\"选择一个 exe 文件作为启动目标。\","
    "\"name_placeholder\":\"显示名称\","
    "\"delete\":\"删除\""
    "}";

static const char *kEnUS =
    "{"
    "\"app_title\":\"EdgeGesture Config\","
    "\"nav_home\":\"Home\","
    "\"nav_handles\":\"Handles\","
    "\"nav_gestures\":\"Gestures\","
    "\"nav_actions\":\"Actions\","
    "\"nav_advanced\":\"Advanced\","
    "\"general\":\"General\","
    "\"enable_engine\":\"Enable Gesture Engine\","
    "\"vertical_trigger_range\":\"Vertical Trigger Range\","
    "\"split_mode\":\"Split Mode\","
    "\"split_none\":\"None\","
    "\"split_two\":\"2 Zones (Half)\","
    "\"split_three\":\"3 Zones (Thirds)\","
    "\"short_swipe_threshold\":\"Short Swipe Threshold\","
    "\"long_swipe_threshold\":\"Long Swipe Threshold\","
    "\"trigger_threshold\":\"Trigger Threshold\","
    "\"trigger_threshold_hint\":\"How far to pull before triggering\","
    "\"handles\":\"Handles\","
    "\"application\":\"Application\","
    "\"exit_app\":\"Exit Application\","
    "\"left_handle\":\"Left Handle\","
    "\"right_handle\":\"Right Handle\","
    "\"top_handle\":\"Top Handle\","
    "\"bottom_handle\":\"Bottom Handle\","
    "\"appearance_position\":\"Appearance & Position\","
    "\"select_side\":\"Select Side:\","
    "\"width\":\"Width\","
    "\"size\":\"Size\","
    "\"position\":\"Position\","
    "\"color\":\"Color\","
    "\"gesture_settings\":\"Gesture Settings\","
    "\"select_edge\":\"Select Edge:\","
    "\"select_zone\":\"Select Zone:\","
    "\"zone_global\":\"Global\","
    "\"zone_top\":\"Top\","
    "\"zone_bottom\":\"Bottom\","
    "\"zone_middle\":\"Middle\","
    "\"zone_left\":\"Left\","
    "\"zone_right\":\"Right\","
    "\"short_swipe\":\"Short Swipe\","
    "\"long_swipe\":\"Long Swipe\","
    "\"straight\":\"Straight\","
    "\"diag_up\":\"Diagonal Up\","
    "\"diag_down\":\"Diagonal Down\","
    "\"diag_left\":\"Diagonal Left\","
    "\"diag_right\":\"Diagonal Right\","
    "\"long_straight\":\"Long Straight\","
    "\"none\":\"None\","
    "\"actions\":\"Actions\","
    "\"register_shortcuts\":\"Register custom keyboard shortcuts or app launches here.\","
    "\"add_new_action\":\"Add New Action\","
    "\"add_action\":\"Add Action\","
    "\"action_name\":\"Action Name (e.g. Copy)\","
    "\"action_type\":\"Action Type:\","
    "\"type_shortcut\":\"Shortcut\","
    "\"type_launch\":\"Launch App\","
    "\"shortcut\":\"Shortcut\","
    "\"shortcut_hint\":\"Shortcut (e.g. ctrl+c)\","
    "\"app_path\":\"App Path\","
    "\"app_args\":\"Arguments (optional)\","
    "\"browse\":\"Browse…\","
    "\"record_shortcut\":\"Record Shortcut\","
    "\"stop_recording\":\"Stop Recording\","
    "\"press_record\":\"Press Record to start recording shortcut\","
    "\"press_keys\":\"Press keys...\","
    "\"add\":\"Add\","
    "\"cancel\":\"Cancel\","
    "\"advanced_settings\":\"Advanced Settings\","
    "\"animation\":\"Animation\","
    "\"start_with_windows\":\"Start with Windows\","
    "\"start_minimized_hint\":\"When enabled, starts silently and minimizes to the system tray\","
    "\"language\":\"Language\","
    "\"per_app_profiles\":\"Per-App Profiles\","
    "\"per_app_desc\":\"Create custom gesture configurations for specific applications. When an app with a profile is in focus, its settings will automatically be applied.\","
    "\"app_executable\":\"App Executable (e.g. notepad.exe)\","
    "\"find\":\"Find\","
    "\"refresh\":\"Refresh\","
    "\"you_are_editing\":\"You are editing:\","
    "\"use_default_profile\":\"Use Default Profile\","
    "\"no_profiles\":\"No app-specific profiles configured.\\nClick 'Add' or 'Find' to create one.\","
    "\"blacklist\":\"Blacklist (App Exceptions)\","
    "\"open_settings\":\"Open Settings\","
    "\"quit\":\"Quit\","
    "\"edge_swipe_hint\":\"Swipe from the edges to navigate back, open task view, or trigger custom actions.\","
    "\"long\":\"px\","
    "\"percent\":\"%\","
    "\"add_launch_hint\":\"Choose an exe file as the launch target.\","
    "\"name_placeholder\":\"Display Name\","
    "\"delete\":\"Delete\""
    "}";

Translator::Translator(QObject *parent) : QObject(parent) {
  loadSavedLanguage();
  loadLanguageData(m_currentLanguage);
}

void Translator::setCurrentLanguage(const QString &lang) {
  if (m_currentLanguage != lang && m_availableLanguages.contains(lang)) {
    m_currentLanguage = lang;
    loadLanguageData(lang);
    saveLanguage(lang);
    emit currentLanguageChanged();
  }
}

QString Translator::tr(const QString &source) const {
  auto it = m_strings.constFind(source);
  if (it != m_strings.constEnd())
    return it.value().toString();
  return source;
}

void Translator::loadLanguageData(const QString &lang) {
  const char *raw = (lang == "en_US") ? kEnUS : kZhCN;
  QJsonDocument doc = QJsonDocument::fromJson(QByteArray(raw));
  QVariantMap map = doc.object().toVariantMap();
  m_strings.clear();
  for (auto it = map.begin(); it != map.end(); ++it) {
    // Keyed by the English source string for en_US, and also by the same key
    // for zh_CN. QML always looks up by the English source key.
    m_strings.insert(it.key(), it.value());
  }
}

void Translator::loadSavedLanguage() {
  QFile f(QCoreApplication::applicationDirPath() + "/lang.txt");
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QString lang = QString::fromUtf8(f.readAll()).trimmed();
    if (m_availableLanguages.contains(lang))
      m_currentLanguage = lang;
  }
}

void Translator::saveLanguage(const QString &lang) {
  QFile f(QCoreApplication::applicationDirPath() + "/lang.txt");
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(lang.toUtf8());
  }
}
