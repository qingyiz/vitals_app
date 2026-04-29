# Vitals 任务计划书

## 当前目标

根据 `agent.md` 的架构约束，先搭建一个可继续演进的 C++ / Qt / CMake 项目框架，重点验证：

- 主程序只承担宿主框架职责。
- 插件通过 SDK 接口接入。
- 插件可以被动态加载。
- 插件可以提供 QWidget 页面。
- 插件可以通过统一 Metric 数据模型向宿主发布数据。

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
build/bin/plugins/libHelloPlugin.so
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
- `IPanelPlugin`
- `IMonitorPlugin`
- `IAppContext`
- `IMetricSink`
- `MetricData`
- `PluginMetaInfo`

这些接口作为宿主和插件之间的稳定边界，后续新增能力优先扩展接口，而不是让插件直接依赖主窗口。

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

主程序目前不包含任何 CPU、内存、网络等具体监控采集逻辑。

任务栏显示能力目前由宿主订阅 `MetricCenter`，将关键指标显示在系统托盘 tooltip、菜单和动态图标中。平台差异由 `WindowsTaskbarIndicator`、`MacTaskbarIndicator`、`LinuxTaskbarIndicator` 隔离。

### 5. 通用 UI 组件

已创建 `widgets` 模块，并实现：

- `CardWidget`

同时预留了图表、仪表盘、表格、小型曲线组件的类型入口，后续可逐步补齐实现。

### 6. 示例插件

已完成 `HelloPlugin`：

- 作为动态插件构建。
- 实现 `IPlugin` 和 `IPanelPlugin`。
- 提供独立 QWidget 页面。
- 启动时发布 `hello.plugin.status` 指标。
- 插件元信息位于 `plugins/hello/hello_plugin.json`。
- 通过 `supportedPlatforms` 声明支持 Windows、macOS、Linux。

插件输出目录已统一为：

```text
build/bin/plugins/
```

## 下一阶段计划

### 第一阶段补强：框架跑通与工程稳定

- 启动 GUI，确认主程序可以实际加载 `HelloPlugin` 并显示插件页面。
- 在 Windows、macOS、Linux 分别验证任务栏 / 托盘 / 菜单栏显示行为。
- 增加基础自动化测试或最小 smoke test，避免后续改动破坏插件加载链路。
- 为插件加载失败增加更完整的错误展示页面。
- 为插件管理页展示已加载插件、失败插件和状态。

### 第二阶段：数据链路跑通

- 新增 `MockMonitorPlugin`。
- 让模拟插件定时发布 CPU、内存、网络等假数据。
- Dashboard 根据统一 Metric key 自动展示核心指标。
- 任务栏显示同步展示 Mock 指标摘要。
- 加入指标刷新节流和历史点缓存。

### 第三阶段：系统信息插件

- 新增 `plugins/systeminfo/`。
- 设计 `ISystemInfoCollector`。
- 分别实现 Windows、macOS、Linux 平台采集入口。
- 提供系统信息详情页。

### 第四阶段：CPU 插件

- 新增 `plugins/cpu/`。
- 建立 `ICpuCollector` 和平台工厂。
- 实现 CPU 总使用率、每核心使用率、CPU 型号。
- 将采集与 UI 展示解耦，数据统一进入 `MetricCenter`。

### 第五阶段：核心监控插件扩展

- MemoryMonitorPlugin
- DiskMonitorPlugin
- NetworkMonitorPlugin
- BatteryMonitorPlugin

### 第六阶段：复杂插件后置

- GPU 插件按供应商或平台拆分策略设计。
- ProcessMonitorPlugin 引入进程列表、排序、过滤和高频刷新优化。

## 当前边界说明

当前版本是框架骨架，不实现真实系统监控采集。这样做是为了先稳定宿主、插件、SDK、Metric 数据链路，避免把监控逻辑提前写进主程序，破坏插件化架构。
