# EdgeGesture for MateBook E Go

<div align="center">

![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white)
![Qt6](https://img.shields.io/badge/Qt6-2CDE85?logo=Qt&logoColor=fff)
![QML](https://img.shields.io/badge/QML-2CDE85?logo=Qt&logoColor=fff)
![Windows](https://img.shields.io/badge/Windows-0078D6?logo=windows&logoColor=white)
![ARM64](https://img.shields.io/badge/ARM64-FF8C00)

**为华为 MateBook E Go 等 ARM64 Windows 触屏设备打造的四边边缘手势工具**

</div>

---

## ✨ 功能特性

- **四边边缘手势**：支持屏幕左、右、上、下四个边缘的触摸滑动手势
- **左右分区**：左/右边缘支持垂直分区（2段/3段），顶/底边缘自动分为左半/右半独立设置
- **直出 & 长滑**：每段区域支持「短滑直出」和「长滑直出」两种手势
- **自定义动作**：
  - 键盘快捷键（支持 Win 键及组合键，如 `win+tab`、`ctrl+c`）
  - 启动自定义应用程序（`launch:<path> [args]`）
- **可视化反馈**：贝塞尔曲线波浪动画，直向生长不倾斜
- **开机自启**：支持开机静默启动并最小化到系统通知栏（托盘）
- **中英双语**：简体中文（默认）/ English，随时切换
- **ARM64 原生**：使用 Qt 6 + MSVC ARM64 交叉编译，NSIS 安装包内置 VC++ ARM64 运行时

## 📦 安装

1. 从 [Releases](https://github.com/GitZhaoo/EdgeGesture_for_matebookego/releases) 页面下载最新版安装包
2. 运行 `EdgeGesture-x.x.x-arm64-setup.exe`
3. 安装完成后 SettingsUI 会自动启动（同时拉起 GestureEngine 引擎）
4. 在系统通知栏可以看到 EdgeGesture 托盘图标

> ⚠️ **重要**：使用前请在 Windows 设置中关闭系统自带的边缘手势（至少关闭左侧），避免冲突。

## 🚀 使用

- 从屏幕四个边缘向内滑动即可触发手势
- 双击托盘图标或从托盘右键菜单选择「打开设置」进入配置界面
- 在设置中可分别配置各边缘各区段的动作
- 高级设置中可开启「开机自启」、管理黑名单和应用专属配置

## 🔧 构建说明

### 依赖

- **Qt 6.10+**（ARM64 / msvc2022_arm64）：Core, Gui, Qml, Quick, Network, Svg, Widgets, QuickDialogs2, Core5Compat
- **Visual Studio 2022 Build Tools**（ARM64 工具链）
- **CMake** + **Ninja**
- **NSIS 3.x**（打包安装程序）
- **FluentUI**（已内置于 `src_ui/FluentUI/`）

### 构建

```bat
:: 1. 配置（在 VS ARM64 开发者命令提示符中）
cmake -B build-arm64 -G Ninja -DCMAKE_PREFIX_PATH=C:\Qt\6.10.1\msvc2022_arm64

:: 2. 编译
cmake --build build-arm64

:: 3. 部署 Qt 依赖
windeployqt6 --release --qmldir src_ui build-arm64\SettingsUI.exe

:: 4. 打包安装程序
makensis installer.nsi
```

## 📂 项目结构

```
├── src_engine/          # 手势引擎（纯 Win32 / Direct2D，无 Qt 依赖）
│   ├── core/            # ConfigManager, EngineCore
│   ├── input/           # InputWindow（边缘触摸捕获窗口）
│   ├── ui/              # Visualizer（D2D 波浪可视化）
│   └── actions/         # ActionDispatcher（快捷键 / 启动应用）
├── src_ui/              # 设置界面（Qt6 / QML / FluentUI）
│   ├── pages/           # 各设置页面 QML
│   ├── i18n/            # 国际化（中/英）
│   └── utils/           # 配置桥接、系统操作等 C++ 后端
├── CMakeLists.txt
└── LICENSE
```

## 🙏 致谢

本项目基于 **[felix-sky/EdgeGesture](https://github.com/felix-sky/EdgeGesture)** 原项目进行二次开发，在此向原作者表示衷心感谢。

原项目提供了左侧边缘手势的基础架构和插件系统，本项目在此基础上进行了以下主要改动：

- 移除了插件子系统，功能改为内置直接动作
- 扩展为四边边缘手势（原项目仅支持左侧）
- 新增键盘快捷键录制（含 Win 键）和自定义应用启动两种动作类型
- 新增简体中文/英文国际化切换
- 适配 ARM64 Windows，提供 NSIS 安装包
- 移除斜向手势，简化为直出 + 长滑直出
- 支持开机自启并最小化到通知栏

同时感谢以下开源项目：

- **[FluentUI](https://github.com/zhuzichu520/FluentUI)** — FluentUI for QML，提供了精美的 UI 组件
- **[Qt](https://www.qt.io/)** — 跨平台应用开发框架

## 📄 许可证

本项目继承原项目许可证，采用 **GPL v3** 许可证。详见 [LICENSE](LICENSE)。

## 🐛 问题反馈

如有 Bug 或功能建议，请提交 [Issue](https://github.com/GitZhaoo/EdgeGesture_for_matebookego/issues)。
