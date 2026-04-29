# Vitals 项目 Agent 指南

我是一名偏架构和底层方向的 C++/Qt 工程师，正在开发一款名为 **Vitals** 的跨平台系统监控软件。  
本项目的核心目标不是做一个简单的 CPU/内存监控界面，而是设计一套 **“宿主程序 + 插件系统 + 跨平台采集层 + 可扩展 UI 面板”** 的桌面监控框架。

> 重要原则：  
> **主程序只做框架，不直接写死任何具体监控逻辑；CPU、GPU、内存、网络、磁盘、系统信息等能力全部以插件形式接入，并且插件本身也要具备跨平台能力。**

---

## 1. 项目概述

### 1.1 项目定位

Vitals 是一个基于 **C++ / Qt / CMake** 的跨平台桌面系统监控框架，目标平台包括：

- Windows
- macOS
- Linux

软件初始界面只提供统一的应用框架、插件管理、导航布局、数据展示容器和基础配置能力。  
具体系统监控能力不应直接写在主程序中，而应作为插件独立开发、独立加载、独立启停。

### 1.2 核心能力

项目最终希望支持以下插件能力：

| 插件类型 | 说明 |
|----------|------|
| SystemInfoPlugin | 系统版本、设备名称、CPU 型号、内存大小、启动时间等 |
| CpuMonitorPlugin | CPU 总使用率、每核心使用率、频率、温度等 |
| MemoryMonitorPlugin | 内存总量、已用内存、可用内存、Swap 等 |
| GpuMonitorPlugin | GPU 型号、显存、使用率、温度等 |
| NetworkMonitorPlugin | 上传速度、下载速度、网卡信息、IP、累计流量等 |
| DiskMonitorPlugin | 磁盘容量、使用率、读写速度、挂载点等 |
| BatteryMonitorPlugin | 电池电量、健康度、循环次数、温度等 |
| ProcessMonitorPlugin | 进程列表、CPU/内存占用排行等，后续扩展 |

---

## 2. 总体架构

本项目采用插件化架构：

```text
Vitals
│
├── app                 # Qt 主程序，负责窗口、导航、页面容器
├── core                # 核心框架，负责插件管理、数据中心、配置、日志
├── sdk                 # 插件 SDK，定义插件接口和统一数据结构
├── widgets             # 通用 UI 组件，如卡片、曲线图、仪表盘、表格
├── plugins             # 各类系统监控插件
├── platform            # 宿主程序自身需要的平台封装，可选
├── docs                # 项目文档
├── tests               # 测试代码
└── cmake               # CMake 辅助脚本
```

推荐目录结构：

```text
project-root/
│
├── CMakeLists.txt
│
├── app/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── MainWindow.h
│   ├── MainWindow.cpp
│   ├── DashboardWidget.h
│   ├── DashboardWidget.cpp
│   ├── NavigationWidget.h
│   ├── NavigationWidget.cpp
│   └── platform/
│       └── taskbar/
│           ├── TaskbarIndicator.h
│           ├── TaskbarIndicator.cpp
│           ├── TaskbarIndicatorFactory.h
│           ├── TaskbarIndicatorFactory.cpp
│           ├── WindowsTaskbarIndicator.h
│           ├── MacTaskbarIndicator.h
│           └── LinuxTaskbarIndicator.h
│
├── core/
│   ├── CMakeLists.txt
│   ├── plugin/
│   │   ├── PluginManager.h
│   │   └── PluginManager.cpp
│   ├── metric/
│   │   ├── MetricCenter.h
│   │   ├── MetricCenter.cpp
│   │   ├── MetricRegistry.h
│   │   └── MetricRegistry.cpp
│   ├── config/
│   │   ├── ConfigManager.h
│   │   └── ConfigManager.cpp
│   ├── event/
│   │   ├── EventBus.h
│   │   └── EventBus.cpp
│   └── log/
│       ├── Logger.h
│       └── Logger.cpp
│
├── sdk/
│   ├── CMakeLists.txt
│   ├── IPlugin.h
│   ├── IMonitorPlugin.h
│   ├── IPanelPlugin.h
│   ├── IAppContext.h
│   ├── IMetricSink.h
│   ├── MetricData.h
│   └── PluginMetaInfo.h
│
├── widgets/
│   ├── CMakeLists.txt
│   ├── CardWidget.h
│   ├── LineChartWidget.h
│   ├── GaugeWidget.h
│   ├── MetricTableWidget.h
│   └── MiniChartWidget.h
│
└── plugins/
    ├── CMakeLists.txt
    ├── systeminfo/
    ├── cpu/
    ├── memory/
    ├── gpu/
    ├── network/
    ├── disk/
    └── battery/
```

---

## 3. 开发环境

### 3.1 基础依赖

- CMake 3.20+
- C++17 或更高版本
- Qt 5.15+ 或 Qt 6.x
- Ninja / Make / Visual Studio Build Tools
- Git

### 3.2 平台相关依赖

#### Windows

可能使用：

- Win32 API
- PDH
- WMI
- DXGI
- Performance Counters

#### macOS

可能使用：

- IOKit
- CoreFoundation
- Foundation
- SystemConfiguration
- sysctl
- host_processor_info

如涉及 Objective-C++，源文件使用 `.mm` 后缀。

#### Linux

可能使用：

- `/proc`
- `/sys`
- Netlink
- lm-sensors
- libudev，可选

---

## 4. 构建指引

推荐使用 CMake 组织工程。

### 4.1 基础构建

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

### 4.2 Debug 构建

```bash
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

### 4.3 Release 构建

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```

### 4.4 插件输出目录

所有插件应统一输出到运行目录的 `plugins/` 下：

```text
build/bin/
│
├── Vitals.exe / Vitals.app / vitals
└── plugins/
    ├── CpuMonitorPlugin.dll / dylib / so
    ├── MemoryMonitorPlugin.dll / dylib / so
    ├── NetworkMonitorPlugin.dll / dylib / so
    └── ...
```

---

## 5. 核心设计原则

### 5.1 主程序不直接实现监控逻辑

禁止在 `MainWindow`、`DashboardWidget` 或其他宿主 UI 类中直接写以下逻辑：

```cpp
getCpuUsage();
getMemoryUsage();
getGpuInfo();
getNetworkSpeed();
```

宿主程序只做：

```cpp
pluginManager->loadAllPlugins();
metricCenter->subscribe(...);
dashboard->bindMetricCenter(metricCenter);
```

具体数据由插件采集并上报。

---

### 5.2 插件不直接操作主窗口

插件禁止直接依赖主窗口对象。

禁止：

```cpp
mainWindow->setCpuUsage(value);
dashboard->updateNetworkSpeed(speed);
```

推荐：

```cpp
metricSink->publishFrame(frame);
```

插件只向框架发布数据，主程序自己决定如何展示。

---

### 5.3 平台差异只存在于插件内部

每个跨平台插件内部应使用统一接口隔离平台实现。

以 CPU 插件为例：

```text
plugins/cpu/
│
├── CpuMonitorPlugin.h
├── CpuMonitorPlugin.cpp
├── ICpuCollector.h
├── CpuCollectorFactory.h
├── CpuCollectorFactory.cpp
│
└── platform/
    ├── windows/
    │   ├── WindowsCpuCollector.h
    │   └── WindowsCpuCollector.cpp
    ├── macos/
    │   ├── MacCpuCollector.h
    │   └── MacCpuCollector.mm
    └── linux/
        ├── LinuxCpuCollector.h
        └── LinuxCpuCollector.cpp
```

统一接口：

```cpp
class ICpuCollector
{
public:
    virtual ~ICpuCollector() = default;

    virtual bool initialize() = 0;
    virtual double totalUsage() = 0;
    virtual QList<double> perCoreUsage() = 0;
    virtual QString cpuName() const = 0;
};
```

工厂创建：

```cpp
std::unique_ptr<ICpuCollector> CpuCollectorFactory::create()
{
#ifdef Q_OS_WIN
    return std::make_unique<WindowsCpuCollector>();
#elif defined(Q_OS_MAC)
    return std::make_unique<MacCpuCollector>();
#elif defined(Q_OS_LINUX)
    return std::make_unique<LinuxCpuCollector>();
#else
    return nullptr;
#endif
}
```

---

### 5.4 接口稳定优先

`sdk/` 目录中的接口是插件和宿主之间的契约。  
修改 SDK 接口前必须谨慎考虑兼容性。

优先新增接口，不轻易破坏已有接口。

---

### 5.5 数据模型统一

所有插件上报的数据必须使用统一的 Metric 数据模型。  
不要让 CPU 插件、网络插件、磁盘插件各自定义完全不同的数据格式。

### 5.6 宿主平台能力可以分平台实现

“平台差异只存在于插件内部”主要约束监控采集插件。  
宿主程序自身也可能需要少量平台相关能力，例如任务栏 / 系统托盘 / macOS 菜单栏显示、系统通知、开机启动等。

这类能力允许放在宿主的 `app/platform/` 或根级 `platform/` 目录中，但必须遵守：

- 只处理宿主程序能力，不直接实现 CPU、内存、网络等监控采集。
- 数据来源必须是 `MetricCenter` 或 SDK 数据模型。
- 平台差异通过抽象接口和 Factory 隔离。
- `MainWindow` 只依赖抽象接口，不直接写 `#ifdef Q_OS_WIN` / `Q_OS_MAC` / `Q_OS_LINUX`。

以任务栏显示为例：

```text
app/platform/taskbar/
│
├── TaskbarIndicator.h
├── TaskbarIndicator.cpp
├── TaskbarIndicatorFactory.h
├── TaskbarIndicatorFactory.cpp
├── WindowsTaskbarIndicator.h
├── WindowsTaskbarIndicator.cpp
├── MacTaskbarIndicator.h
├── MacTaskbarIndicator.cpp
├── LinuxTaskbarIndicator.h
└── LinuxTaskbarIndicator.cpp
```

宿主可以订阅 `MetricCenter`，将 CPU、内存、网络、电池等核心指标显示到系统任务栏区域，但不能在任务栏组件中直接采集系统数据。

---

## 6. 插件 SDK 规范

### 6.1 基础插件接口

```cpp
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    virtual PluginMetaInfo metaInfo() const = 0;

    virtual bool initialize(IAppContext* context) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void shutdown() = 0;
};

#define IPlugin_iid "com.vitals.plugin.IPlugin/1.0"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_iid)
```

### 6.2 监控插件接口

```cpp
class IMonitorPlugin : public IPlugin
{
public:
    virtual ~IMonitorPlugin() = default;

    virtual QList<MetricDescriptor> metricDescriptors() const = 0;

    virtual int defaultIntervalMs() const = 0;
    virtual void setIntervalMs(int intervalMs) = 0;
};

#define IMonitorPlugin_iid "com.vitals.plugin.IMonitorPlugin/1.0"
Q_DECLARE_INTERFACE(IMonitorPlugin, IMonitorPlugin_iid)
```

### 6.3 面板插件接口

```cpp
class IPanelPlugin : public IPlugin
{
public:
    virtual ~IPanelPlugin() = default;

    virtual QString panelId() const = 0;
    virtual QString panelName() const = 0;
    virtual QWidget* createPanel(QWidget* parent = nullptr) = 0;
};

#define IPanelPlugin_iid "com.vitals.plugin.IPanelPlugin/1.0"
Q_DECLARE_INTERFACE(IPanelPlugin, IPanelPlugin_iid)
```

一个插件可以同时实现 `IMonitorPlugin` 和 `IPanelPlugin`。

---

## 7. Metric 数据规范

### 7.1 MetricDescriptor

用于描述一个指标。

```cpp
enum class MetricValueType
{
    Integer,
    Double,
    String,
    Boolean,
    Percentage,
    Bytes,
    BytesPerSecond,
    Temperature,
    Frequency
};

struct MetricDescriptor
{
    QString key;
    QString name;
    QString description;
    QString unit;
    MetricValueType type;
};
```

### 7.2 MetricValue

用于表示一个指标值。

```cpp
struct MetricValue
{
    QString key;
    QVariant value;
    QDateTime timestamp;
    QMap<QString, QString> labels;
};
```

### 7.3 MetricFrame

用于插件一次性上报一组指标。

```cpp
struct MetricFrame
{
    QString pluginId;
    QDateTime timestamp;
    QList<MetricValue> values;
};
```

### 7.4 指标命名规范

指标 key 使用小写英文和点号分层。

推荐：

```text
cpu.usage.total
cpu.usage.core0
cpu.temperature
memory.usage.percent
network.upload.speed
network.download.speed
disk.read.speed
disk.write.speed
gpu.usage.percent
battery.level.percent
system.uptime.seconds
```

禁止：

```text
CPUUsage
cpuUsage
cpu_usage_total
当前CPU使用率
```

---

## 8. 插件元信息规范

每个插件应提供 JSON 元信息文件。

插件元信息不仅用于展示，也用于宿主程序的加载决策。  
宿主在真正实例化插件前，必须先读取插件 JSON 元信息中的 `supportedPlatforms` 字段，判断当前运行平台是否被支持。  
如果当前平台不在插件声明的平台列表中，宿主应跳过该插件，不调用 `initialize()`，也不把它视为加载失败。

例如 `cpu_plugin.json`：

```json
{
  "id": "com.vitals.cpu",
  "name": "CPU Monitor",
  "description": "Cross-platform CPU usage monitor plugin",
  "version": "1.0.0",
  "author": "YourName",
  "category": "monitor",
  "supportedPlatforms": ["windows", "macos", "linux"],
  "requiredHostVersion": "1.0.0"
}
```

### 8.1 平台支持声明

`supportedPlatforms` 是必填字段，用于声明插件已经实现并验证的平台能力。

允许值：

```text
windows
macos
linux
all
*
```

示例：只实现 macOS 的系统信息插件：

```json
{
  "id": "com.vitals.systeminfo",
  "name": "System Info",
  "description": "macOS system information plugin",
  "version": "1.0.0",
  "author": "YourName",
  "category": "monitor",
  "supportedPlatforms": ["macos"],
  "requiredHostVersion": "1.0.0"
}
```

示例：实现了 Windows、macOS、Linux 的 CPU 插件：

```json
{
  "id": "com.vitals.cpu",
  "name": "CPU Monitor",
  "description": "Cross-platform CPU usage monitor plugin",
  "version": "1.0.0",
  "author": "YourName",
  "category": "monitor",
  "supportedPlatforms": ["windows", "macos", "linux"],
  "requiredHostVersion": "1.0.0"
}
```

加载规则：

```text
宿主扫描插件文件
→ 读取 Qt 插件 JSON metadata
→ 检查 supportedPlatforms
→ 当前平台匹配：继续实例化并 initialize
→ 当前平台不匹配：跳过插件并记录原因
```

注意：

- `supportedPlatforms` 表示插件已经具备的平台实现，不表示未来计划支持的平台。
- 如果插件只写了 macOS collector，就只能声明 `["macos"]`。
- 不要为了让插件显示在列表里而声明未实现的平台。
- 平台不匹配属于正常跳过，不应作为错误弹窗打扰用户。
- 插件详情页和监控能力都应遵守同一个平台声明。

插件 ID 命名规范：

```text
com.vitals.cpu
com.vitals.memory
com.vitals.network
com.vitals.disk
com.vitals.gpu
com.vitals.systeminfo
```

---

## 9. 线程与性能规范

### 9.1 UI 线程原则

UI 只能在主线程操作。

禁止在采集线程中直接更新 QWidget。

错误：

```cpp
workerThread->setLabelText(...);
panelWidget->updateChart(...);
```

正确：

```cpp
emit metricCollected(frame);
```

或者：

```cpp
metricSink->publishFrame(frame);
```

由主程序使用 Qt 队列连接处理 UI 更新。

---

### 9.2 采集线程原则

以下操作不应阻塞 UI 线程：

- 系统 API 查询
- 进程列表扫描
- 磁盘 IO 查询
- 网络接口枚举
- GPU 查询
- 大量历史数据处理

推荐做法：

- 轻量定时采集可使用 `QTimer`
- 较重采集放入 `QThread` / worker
- 更复杂任务可以后续引入线程池

---

### 9.3 UI 刷新节流

不要每收到一条数据就立即刷新界面。

推荐：

```text
数据采集频率：500ms / 1000ms
UI 刷新频率：500ms / 1000ms
曲线刷新频率：1000ms
历史数据保留：最近 60 秒 / 5 分钟 / 1 小时
```

高频数据必须做：

- 批量刷新
- 缓冲区
- 最大点数限制
- 必要时降采样

---

## 10. UI 设计规范

### 10.1 主界面原则

主界面是插件容器，不是固定业务页面。

推荐结构：

```text
MainWindow
│
├── 左侧导航栏
│   ├── 总览 Dashboard
│   ├── 插件提供的页面
│   └── 插件管理
│
├── 中间内容区
│   └── PluginPanelHost
│
└── 状态栏
    ├── 当前平台
    ├── 插件数量
    ├── 数据刷新状态
    └── 日志状态
```

### 10.2 Dashboard 设计

Dashboard 由宿主提供，展示所有插件的核心指标。

例如：

```text
CPU 使用率
内存使用率
GPU 使用率
网络上传/下载速度
磁盘使用率
电池电量
系统运行时间
```

Dashboard 不关心数据来自哪个平台 API，只关心 MetricCenter 中是否有对应指标。

### 10.3 插件详情页

插件可以提供自己的 QWidget 页面。

例如：

- CPU 插件提供 CPU 详情页
- 网络插件提供网络详情页
- 磁盘插件提供磁盘详情页
- 系统信息插件提供系统信息页

但插件页面仍然不能绕过统一数据模型直接控制主程序。

### 10.3.1 插件页面应优先复用宿主通用组件

不要让每个插件都从零开始手写一整套 QWidget、QFrame、QLabel、QGridLayout、QVBoxLayout 页面结构。  
对于结构相似的详情页，宿主应在 `widgets/` 中提供可复用的通用页面组件，插件只负责提供内容数据。

推荐做法：

- 页面级骨架组件放在 `widgets/`，例如 `InfoPanelWidget`。
- 单个指标卡片、tile、表格、曲线图等重复元素，也应逐步沉淀到 `widgets/`。
- 插件页面优先通过“传数据”的方式驱动这些组件，而不是在插件里重复拼布局。

例如 `InfoPanelWidget` 适合封装以下通用页面结构：

```text
页面标题
副标题
Hero 概览区
Key-Value 快照区
Tile 网格区
```

插件推荐只提供：

- 页面标题和副标题
- hero 区展示数据
- key-value 行数据
- tile 数据

而不在插件里重复实现：

- 页面整体布局
- 间距与对齐规则
- 卡片边框与样式
- 多列栅格排列
- 不同区块之间的响应式组织

目标原则：

```text
插件负责声明内容；
宿主通用组件负责布局、样式与一致性。
```

这样做的好处：

- 后续新增插件时开发量更小
- 页面风格更统一
- 对齐、间距、卡片尺寸等问题可以集中修复
- UI 优化可以一次改动影响多个插件页面

如果某个插件页面只是少量差异化展示，应优先扩展已有通用组件，而不是重新从头写一套新布局。

### 10.4 任务栏 / 托盘 / 菜单栏显示

系统监控软件必须支持“随时可见”的轻量信息显示。

宿主程序应提供任务栏显示能力：

- Windows：优先使用系统托盘 / taskbar notification area。
- macOS：优先使用菜单栏状态项 / menu bar extra。
- Linux：优先使用系统托盘 / StatusNotifier / desktop environment tray 能力。

该能力属于宿主 UI 外壳，不属于某个监控插件。  
任务栏组件只能展示来自 `MetricCenter` 的指标，不允许直接调用系统 API 采集 CPU、内存、网络等数据。

推荐展示内容：

```text
CPU 使用率
内存使用率
网络上传 / 下载速度
电池电量
插件运行状态
```

推荐交互：

- 单击或双击显示主窗口。
- 右键菜单显示核心指标摘要。
- 提供显示主窗口和退出应用操作。
- 当平台不支持系统托盘时，记录日志并保持主程序正常运行。

任务栏显示也需要刷新节流，不应每个 Metric 更新都进行昂贵重绘。第一阶段可以先用轻量图标和 tooltip，后续再根据平台扩展更原生的实现。

---

## 11. CMake 开发规范

### 11.1 模块组织

每个模块必须有自己的 `CMakeLists.txt`。

```text
sdk/CMakeLists.txt
core/CMakeLists.txt
widgets/CMakeLists.txt
app/CMakeLists.txt
plugins/cpu/CMakeLists.txt
```

### 11.2 平台代码添加

平台代码在 CMake 中按平台添加。

```cmake
if(WIN32)
    target_sources(CpuMonitorPlugin PRIVATE
        platform/windows/WindowsCpuCollector.cpp
    )
elseif(APPLE)
    target_sources(CpuMonitorPlugin PRIVATE
        platform/macos/MacCpuCollector.mm
    )

    target_link_libraries(CpuMonitorPlugin
        PRIVATE
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework Foundation"
    )
elseif(UNIX)
    target_sources(CpuMonitorPlugin PRIVATE
        platform/linux/LinuxCpuCollector.cpp
    )
endif()
```

### 11.3 Qt 版本兼容

项目应尽量兼容 Qt 5.15+ 与 Qt 6。

推荐封装 Qt5/Qt6 差异，不要在业务代码中到处写版本判断。

---

## 12. 代码风格规范

### 12.1 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | 大驼峰 | `PluginManager` |
| 函数名 | 小驼峰 | `loadAllPlugins()` |
| 成员变量 | `m_` 前缀 | `m_metricCenter` |
| 常量 | 大驼峰或全大写 | `DefaultIntervalMs` |
| 文件名 | 与类名一致 | `PluginManager.h` |
| 插件目录 | 小写 | `plugins/cpu/` |

### 12.2 Qt 宏规范

推荐统一使用 Qt 官方宏风格：

```cpp
Q_OBJECT
Q_SIGNALS:
Q_SLOTS:
Q_EMIT signalName();
```

避免混用：

```cpp
signals:
slots:
emit signalName();
```

### 12.3 头文件规范

头文件应尽量保持轻量。

推荐：

- 使用前向声明
- 减少不必要 include
- 不在头文件中写复杂实现
- SDK 头文件保持稳定和清晰

### 12.4 注释规范

项目代码必须带有规范化注释，不能长期维持“几乎无注释”的状态。

总体要求：

- `sdk/`、`core/`、宿主平台抽象层、插件平台抽象接口等关键类型，必须写类级注释。
- 公共接口、虚函数、线程敏感函数、生命周期函数、平台分发函数，必须写函数级注释。
- 简单 getter / setter / 一眼可懂的小函数，不要求都写大段注释，但至少要保持命名清晰；必要时可加一行短注释。
- 注释应解释职责、边界、线程要求、生命周期、调用约束，不要只重复代码字面意思。
- 新增复杂逻辑块时，可以在实现文件中加入 1 到 2 行短注释帮助阅读。

关键类与关键接口默认采用中英双语 Doxygen 风格，格式参考：

```cpp
/**
 * \if ENGLISH
 * @brief Theme manager for ImPlot visual styles with Qt integration
 *
 * Provides a Qt-friendly interface to customize non-auto colors in ImPlot themes.
 * \endif
 *
 * \if CHINESE
 * @brief ImPlot 可视化样式的 Qt 集成主题管理器
 *
 * 提供 Qt 友好的接口来自定义 ImPlot 主题中的非自动颜色。
 * \endif
 */
```

适用范围建议：

- 类注释：使用上述双语 Doxygen 风格。
- 重要 public / protected 函数：优先使用双语 Doxygen 风格。
- 普通辅助函数：可以使用简短的 `//` 注释。

推荐：

```cpp
/**
 * \if ENGLISH
 * @brief Publishes a batch of metric values to the host application
 * \endif
 *
 * \if CHINESE
 * @brief 向宿主程序发布一批指标值
 * \endif
 */
virtual void publishFrame(const MetricFrame& frame) = 0;
```

禁止：

- 完全没有注释的公共框架头文件。
- 只有“赋值给变量”“调用某函数”这类无信息量注释。
- 注释与实现明显不一致且长期不维护。

---

## 13. 日志与错误处理

### 13.1 插件加载错误必须记录

PluginManager 加载失败时必须输出：

- 插件路径
- 错误原因
- Qt loader error string
- 插件元信息，如果能读取

示例：

```cpp
qWarning() << "Failed to load plugin:" << filePath
           << loader->errorString();
```

### 13.2 插件异常不能影响主程序

某个插件初始化失败，不应导致整个主程序崩溃。

推荐策略：

```text
插件加载失败 → 记录日志 → 标记为不可用 → 主程序继续启动
```

如果插件声明的 `supportedPlatforms` 不包含当前平台，应采用更轻量的跳过策略：

```text
平台不匹配 → 记录跳过原因 → 不实例化插件 → 主程序继续启动
```

### 13.3 系统 API 查询失败

系统 API 查询失败时，应返回空数据或错误状态，不要直接崩溃。

例如：

```cpp
if (!m_collector->initialize()) {
    qWarning() << "CPU collector initialize failed";
    return false;
}
```

---

## 14. 配置管理规范

### 14.1 配置目录

推荐：

```text
config/
│
├── app.json
└── plugins/
    ├── com.vitals.cpu.json
    ├── com.vitals.memory.json
    ├── com.vitals.network.json
    └── com.vitals.gpu.json
```

### 14.2 插件配置

CPU 插件示例：

```json
{
  "enabled": true,
  "intervalMs": 1000,
  "showPerCore": true,
  "historySeconds": 300
}
```

网络插件示例：

```json
{
  "enabled": true,
  "intervalMs": 1000,
  "preferredInterface": "auto",
  "showTotalTraffic": true
}
```

配置读取由框架提供路径，具体字段由插件自己解释。

---

## 15. 推荐开发顺序

### 第一阶段：框架跑通

优先实现：

1. `sdk`
2. `core`
3. `PluginManager`
4. `IPlugin`
5. `IPanelPlugin`
6. 一个 `HelloPlugin`
7. 主程序加载插件并显示插件页面

目标：

```text
证明宿主程序可以加载插件，插件可以提供 QWidget 页面。
```

### 第二阶段：数据链路跑通

实现：

1. `MetricData`
2. `IMetricSink`
3. `MetricCenter`
4. 一个模拟数据插件 `MockMonitorPlugin`
5. Dashboard 从 MetricCenter 读取数据

目标：

```text
证明插件可以上报数据，主程序可以统一接收并展示。
```

### 第三阶段：系统信息插件

实现：

1. Windows / macOS / Linux 系统信息采集
2. 系统信息详情页
3. 系统信息 Metric 注册

目标：

```text
验证跨平台采集层设计。
```

### 第四阶段：CPU 插件

实现：

1. CPU 总使用率
2. 每核心使用率
3. CPU 型号
4. 定时采集
5. CPU 详情页面
6. CPU 曲线显示

目标：

```text
验证真实监控插件的数据采集、数据上报、UI 展示和线程模型。
```

### 第五阶段：内存、磁盘、网络插件

逐步补齐核心监控能力。

### 第六阶段：GPU 插件

GPU 跨平台差异最大，建议后置。

可以考虑拆分：

```text
GenericGpuPlugin
NvidiaGpuPlugin
AmdGpuPlugin
AppleGpuPlugin
```

---

## 16. 开发时必须避免的问题

### 16.1 避免把插件系统做成摆设

不要只是在形式上有插件，实际逻辑仍然写在主程序里。

错误：

```text
MainWindow 负责 CPU、内存、网络采集
插件只提供一个空页面
```

正确：

```text
插件负责能力本身
主程序负责加载、展示和调度
```

### 16.2 避免接口过早复杂化

第一阶段不要一上来设计过多接口。  
优先保证最小链路跑通：

```text
加载插件 → 创建页面 → 上报数据 → 显示数据
```

### 16.3 避免平台代码污染业务层

不要在插件主逻辑里到处写：

```cpp
#ifdef Q_OS_WIN
...
#elif defined(Q_OS_MAC)
...
#endif
```

平台判断应集中在 Factory 和 platform 目录中。

### 16.4 避免 UI 与采集强耦合

采集数据和 UI 展示要解耦。  
同一个 CPU 数据未来既可以显示在 Dashboard，也可以显示在悬浮窗、状态栏、历史曲线中。

---

## 17. Agent 编码行为规范

当 AI Agent 或开发者参与本项目编码时，必须遵守以下规则：

1. 修改代码前先判断改动属于 `app`、`core`、`sdk`、`widgets` 还是 `plugins`。
2. 如果是新增监控能力，优先设计为插件，不要直接写入主程序。
3. 如果涉及 Windows / macOS / Linux 差异，必须放入插件内部的 `platform/` 子目录。
4. 如果涉及插件和主程序通信，必须通过 SDK 接口或 Metric 数据模型完成。
5. 不要让插件直接依赖 `MainWindow`、`DashboardWidget` 等宿主 UI 类。
6. 不要让 UI 线程执行重型系统采集逻辑。
7. 新增 Metric 时必须遵守统一命名规范。
8. 新增插件时必须提供插件元信息，并准确填写 `supportedPlatforms`。
9. 新增平台 API 时必须在 CMake 中正确链接对应系统库或 Framework。
10. 修改 SDK 接口时必须考虑已有插件兼容性。
11. 新增任务栏、托盘、系统通知等宿主平台能力时，必须放入宿主平台抽象层，并从 `MetricCenter` 读取数据，不要直接采集系统信息。

---

## 18. 一句话核心原则

```text
主程序不做监控，主程序只做框架；
插件不控制主窗口，插件只声明能力、采集数据、提供面板；
平台差异不扩散，平台实现只存在于插件内部；
所有监控数据都通过统一 Metric 模型进入框架。
```
