# Vitals 任务计划书

## 当前目标

根据 `agent.md` 的架构约束，搭建一个可继续演进的 C++ / Qt / CMake 插件化系统监控框架，并逐步把“宿主框架 + 插件 + 统一 UI 组件 + 跨平台采集层”做实。

当前阶段重点验证：

- 主程序只承担宿主框架职责。
- 插件通过 SDK 接口接入。
- 插件可以被动态加载。
- 插件可以提供 QWidget 页面。
- 插件可以通过统一 Metric 数据模型向宿主发布数据。
- 插件页面可以复用宿主提供的通用页面骨架，而不是每个插件都手写布局。

## 已完成

### 0. 构建验证

已在当前环境完成配置和构建验证：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

当前环境使用 Qt 5，构建产物已生成：

```text
build/bin/Vitals
build/bin/plugins/libCpuMonitorPlugin.so
build/bin/plugins/libSystemInfoPlugin.so
build/bin/plugins/libDiskMonitorPlugin.so
```

### 1. 项目基础结构

已创建推荐的基础目录：

```text
app/
core/
sdk/
widgets/
plugins/
docs/
```

根目录 `CMakeLists.txt` 已接入 Qt 6 / Qt 5.15 兼容查找逻辑，并统一设置构建输出目录。

### 2. SDK 契约

已完成第一版插件 SDK：

- `IPlugin`
- `IMonitorCapability`
- `IPanelCapability`
- `ITaskbarCapability`
- `ISettingsCapability`
- `IAppContext`
- `IMetricSink`
- `MetricData`
- `PluginMetaInfo`

这些接口作为宿主和插件之间的稳定边界，后续新增能力优先扩展接口，而不是让插件直接依赖主窗口。

当前插件架构已升级为：

- `IPlugin` 只负责生命周期和 capability 入口。
- 具体能力通过 `monitorCapability()`、`panelCapability()`、`taskbarCapability()`、`settingsCapability()` 暴露。
- 宿主优先消费 capability，新旧插件接口仍可兼容共存。

### 3. 核心框架

已完成核心模块雏形：

- `PluginManager`：扫描插件目录、读取插件元信息、按 `supportedPlatforms` 跳过不支持当前平台的插件、动态加载插件、初始化、启动、停止和卸载插件。
- `MetricCenter`：作为统一指标入口，接收插件发布的 `MetricFrame`，缓存最新指标并通知 UI。
- `MetricRegistry`：预留指标描述注册能力。
- `ConfigManager`：提供插件配置路径。
- `EventBus` / `Logger`：作为后续框架能力占位。

### 4. 主程序框架

已完成 Qt 宿主程序：

- `MainWindow`：应用主窗口和页面容器。
- `NavigationWidget`：左侧导航。
- `DashboardWidget`：从 `MetricCenter` 接收指标并展示。
- `AppContext`：向插件暴露宿主服务。
- `TaskbarIndicator`：跨平台任务栏 / 托盘 / macOS 菜单栏指标显示抽象。
- Dashboard 已支持把 `*.bytes` 指标自动格式化为 `KB / MB / GB / TB` 这种可读单位。

主程序目前不包含任何 CPU、内存、网络等具体监控采集逻辑。

任务栏显示能力目前由宿主订阅 `MetricCenter`，将关键指标显示在系统托盘 tooltip、菜单和动态图标中。平台差异由 `WindowsTaskbarIndicator`、`MacTaskbarIndicator`、`LinuxTaskbarIndicator` 隔离。Windows 当前支持每个启用 taskbar capability 的插件拥有独立文本按钮，并点击打开对应插件详情。

### 5. 通用 UI 组件

已创建 `widgets` 模块，并实现：

- `CardWidget`
- `InfoPanelWidget`

其中 `InfoPanelWidget` 已经承担插件详情页的通用页面骨架职责，统一封装：

- 页面标题 / 副标题
- Hero 概览区
- Key-Value 快照区
- Tile 网格区

这样后续插件页面可以优先“传数据”，而不是每个插件都从零开始拼 Qt 布局。

同时预留了图表、仪表盘、表格、小型曲线组件的类型入口，后续可逐步补齐实现。

### 6. 监控插件

已完成 `CpuMonitorPlugin`、`NetworkMonitorPlugin`、`SystemInfoPlugin` 与 macOS 首版 `DiskMonitorPlugin`：

- 作为动态插件构建。
- 使用“插件壳 + capability 对象”结构。
- CPU 插件发布总使用率、每核使用率、逻辑核心数等指标。
- Network 插件发布主网卡、实时上下行速率、累计流量等指标。
- SystemInfo 插件发布设备、系统、硬件和运行时长等指标。
- Disk 插件发布当前挂载卷、挂载路径、文件系统、容量和使用率等指标，并支持选择要监控的挂载卷。
- 每个插件内部都已拆出 `monitor/`、`panel/`、`taskbar/`、`settings/` 目录。
- 监控类 capability 继续通过 `Collector + Factory + platform/*` 组织跨平台采集实现。
- 插件元信息分别位于 `plugins/cpu/cpu_plugin.json`、`plugins/network/network_plugin.json`、`plugins/systeminfo/systeminfo_plugin.json`、`plugins/disk/disk_plugin.json`。
- 当前已实现平台通过 `supportedPlatforms` 明确声明。
- CPU 插件当前已支持 macOS 与 Windows，Windows 采集层位于 `plugins/cpu/platform/windows/`。
- Disk 插件当前声明支持 macOS；外接硬盘在系统完成挂载后会作为可选挂载卷进入监控列表。

插件输出目录已统一为：

```text
build/bin/plugins/
```

### 7. 系统信息插件（macOS 首版）

已完成 `SystemInfoPlugin` 第一版：

- 插件目录位于 `plugins/systeminfo/`。
- 当前仅支持 macOS，并通过 `supportedPlatforms: ["macos"]` 明确声明。
- 插件通过 capability 组合提供监控数据、详情页、任务栏和设置入口能力。
- 采集层使用 `ISystemInfoCollector` + `SystemInfoCollectorFactory` + `platform/macos/` 分层组织。
- 当前已上报设备名、macOS 版本、CPU 型号、GPU 型号、总内存、系统运行时长。
- 插件详情页改为复用 `InfoPanelWidget`，展示同一份统一快照数据。
- macOS GPU 信息通过 Metal 默认设备名进行采集。

该插件已经完成编译接入，并会输出到运行目录的 `plugins/` 下。

## 当前已实现功能清单

### 框架层

- Qt/CMake 工程骨架已经建立。
- 宿主、SDK、核心框架、插件目录、通用 `widgets` 目录已经分离。
- 插件支持动态加载、初始化、启动、停止、卸载。
- 插件元信息支持 `supportedPlatforms`，宿主会在加载前做平台过滤。
- 统一 `MetricFrame` / `MetricValue` / `MetricDescriptor` 数据模型已经建立。

### 宿主程序

- 主窗口、导航、Dashboard、插件页面容器已经可用。
- 任务栏 / 托盘 / 菜单栏显示抽象已经建立。
- Dashboard 可以显示插件发布的实时指标。
- Dashboard 已支持对字节类指标进行可读单位格式化。
- 宿主已支持优先消费 capability，并兼容旧式多接口插件。

### UI 组件

- `CardWidget` 已用于 Dashboard 指标卡片。
- `InfoPanelWidget` 已用于构建信息类详情页。
- 插件页面已经开始从“手写布局”迁移到“复用宿主组件”模式。

### 插件

- `CpuMonitorPlugin`、`NetworkMonitorPlugin`、`SystemInfoPlugin` 已完成 capability 化重构。
- `DiskMonitorPlugin` 已完成 macOS 首版接入，并支持 monitor/panel/taskbar/settings capability。
- 这些插件都已经验证“插件壳 + monitor/panel/taskbar/settings capability”模式。
- capability 化后，平台差异继续只下沉在 collector / platform 目录，不回流到插件主类。

## 下一阶段计划

### 第一阶段补强：框架跑通与工程稳定

- 启动 GUI，确认主程序可以实际加载 CPU / Network / SystemInfo 插件并正确显示页面。
- 在 Windows、macOS、Linux 分别验证任务栏 / 托盘 / 菜单栏显示行为。
- 增加基础自动化测试或最小 smoke test，避免后续改动破坏插件加载链路。
- 为插件加载失败增加更完整的错误展示页面。
- 为插件管理页展示已加载插件、失败插件和状态。
- 为 `settingsCapability` 增加宿主侧统一设置入口。
- 继续完善宿主统一样式，使 Dashboard、插件页、任务栏提示风格更一致。

### 第二阶段：数据链路跑通

- 新增 `MockMonitorPlugin`。
- 让模拟插件定时发布 CPU、内存、网络等假数据。
- Dashboard 根据统一 Metric key 自动展示核心指标。
- 任务栏显示同步展示 Mock 指标摘要。
- 加入指标刷新节流和历史点缓存。
- 让 `InfoPanelWidget` 支持更多可选区块，例如图表区、状态标签区、附加列表区。

### 第三阶段：系统信息插件

- 继续补充 Linux 平台系统信息采集实现，并完善 Windows/macOS 采集字段覆盖。
- 将系统信息详情页扩展为更完整的硬件与宿主摘要页。
- 根据需要增加更多稳定指标，例如架构、内核版本、序列化标识等。
- 继续抽离 `InfoTileWidget`、`InfoRowWidget` 等更细粒度组件，降低后续插件页面开发成本。

### 第四阶段：CPU / Network 插件深化

- 完善 CPU Linux collector，并继续补充 Windows/macOS 下的频率、温度等扩展指标。
- 完善 Network Windows / Linux collector。
- 把 capability 间重复逻辑进一步抽成可复用基类或 helper。
- 为设置 capability 接入真实设置项，而不是占位 widget。

### 第五阶段：核心监控插件扩展

- MemoryMonitorPlugin
- DiskMonitorPlugin 平台扩展与读写速率采集
- BatteryMonitorPlugin

### 第六阶段：复杂插件后置

- GPU 插件按供应商或平台拆分策略设计。
- ProcessMonitorPlugin 引入进程列表、排序、过滤和高频刷新优化。

## 当前边界说明

当前版本已经不再只是纯骨架，已经包含三类可编译接入插件、一套 capability 化插件架构，以及可复用的宿主页面骨架组件。

但项目整体仍处于早期阶段，当前边界仍然包括：

- 真实跨平台能力还未补齐，CPU 与 SystemInfo 仍缺 Linux collector，Network 仍缺 Windows / Linux collector。
- 插件管理页仍是占位版本。
- 自动化测试仍未建立。
- 多平台运行验证还未完成。
- 设置 capability 还未接入统一宿主设置中心。
- 通用 UI 组件体系刚开始搭建，仍有继续抽象空间。
