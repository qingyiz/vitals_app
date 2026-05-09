# Vitals 插件开发指南

这份文档面向两类开发者：

- 想给 Vitals 主仓库贡献新插件的开发者
- 想基于 Vitals SDK 自定义本地插件的开发者

当前项目采用的是 `插件壳 + capability 对象` 架构。  
插件本身只负责生命周期和能力装配；监控、页面、任务栏、设置等能力通过独立 capability 对象暴露。

## 1. 先理解 4 个核心概念

### 1.1 `IPlugin`

这是每个插件必须实现的唯一基础接口，定义在 [sdk/IPlugin.h](/Users/qingyizhu/workspace/vitals_new/sdk/IPlugin.h:1)。

职责只有两类：

- 返回插件元信息
- 参与宿主生命周期：`initialize()`、`start()`、`stop()`、`shutdown()`

另外它还能返回可选 capability：

- `monitorCapability()`
- `panelCapability()`
- `taskbarCapability()`
- `settingsCapability()`

### 1.2 `IMonitorCapability`

定义在 [sdk/IMonitorCapability.h](/Users/qingyizhu/workspace/vitals_new/sdk/IMonitorCapability.h:1)。

适用于需要采集和发布指标的插件，比如 CPU、Network、Disk、Battery。

它负责：

- 声明插件会发布哪些指标
- 管理采集周期
- 启停监控

### 1.3 `IPanelCapability`

定义在 [sdk/IPanelCapability.h](/Users/qingyizhu/workspace/vitals_new/sdk/IPanelCapability.h:1)。

适用于需要在左侧导航里提供一个页面的插件。

它负责：

- 页面 id
- 页面标题
- 图标 key
- 创建 QWidget 页面

### 1.4 `ITaskbarCapability` 和 `ISettingsCapability`

分别定义在：

- [sdk/ITaskbarCapability.h](/Users/qingyizhu/workspace/vitals_new/sdk/ITaskbarCapability.h:1)
- [sdk/ISettingsCapability.h](/Users/qingyizhu/workspace/vitals_new/sdk/ISettingsCapability.h:1)

它们都是可选能力：

- `ITaskbarCapability` 负责托盘 / 菜单栏摘要文本、tooltip 和详情内容
- `ISettingsCapability` 负责插件设置页入口

## 2. 推荐目录结构

推荐按 capability 拆目录，而不是把所有逻辑堆进一个类：

```text
plugins/myplugin/
├── MyPlugin.h
├── MyPlugin.cpp
├── my_plugin.json
├── CMakeLists.txt
├── monitor/
│  ├── MyMonitorCapability.h
│  ├── MyMonitorCapability.cpp
│  ├── IMyCollector.h
│  ├── MyCollectorFactory.h
│  ├── MyCollectorFactory.cpp
│  └── platform/
│     ├── windows/
│     ├── macos/
│     └── linux/
├── panel/
│  ├── MyPanelCapability.h
│  ├── MyPanelCapability.cpp
│  ├── MyPanelWidget.h
│  └── MyPanelWidget.cpp
├── taskbar/
│  ├── MyTaskbarCapability.h
│  └── MyTaskbarCapability.cpp
└── settings/
   ├── MySettingsCapability.h
   └── MySettingsCapability.cpp
```

不是每个插件都必须有这四类目录。

最小插件可以只有：

```text
plugins/myplugin/
├── MyPlugin.h
├── MyPlugin.cpp
├── my_plugin.json
└── CMakeLists.txt
```

## 3. 一个最小可加载插件

下面是最小插件壳的结构示例：

```cpp
#pragma once

#include "IPlugin.h"

#include <QObject>

namespace Vitals {

class MyPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "my_plugin.json")
    Q_INTERFACES(Vitals::IPlugin)

public:
    explicit MyPlugin(QObject* parent = nullptr);
    ~MyPlugin() override;

    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;
};

} // namespace Vitals
```

```cpp
#include "MyPlugin.h"

namespace Vitals {

MyPlugin::MyPlugin(QObject* parent)
    : QObject(parent)
{
}

MyPlugin::~MyPlugin() = default;

PluginMetaInfo MyPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.example.myplugin"),
        QStringLiteral("My Plugin"),
        QStringLiteral("Example custom plugin for Vitals."),
        QStringLiteral("0.1.0"),
        QStringLiteral("YourName"),
        QStringLiteral("custom"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        false
    };
}

bool MyPlugin::initialize(IAppContext* context)
{
    return context != nullptr;
}

void MyPlugin::start()
{
}

void MyPlugin::stop()
{
}

void MyPlugin::shutdown()
{
}

} // namespace Vitals
```

这个插件可以被宿主发现和加载，但不会发布指标，也不会提供页面。

## 4. 如何做一个监控插件

如果你的插件需要采集指标，推荐提供 `IMonitorCapability`。

### 4.1 先定义 collector 接口

collector 是平台差异的承载点。  
capability 负责调度和发布；不同系统怎么采集，放进 collector。

```cpp
class IMyCollector
{
public:
    virtual ~IMyCollector() = default;
    virtual bool initialize() = 0;
    virtual MySnapshot collect() = 0;
};
```

### 4.2 用工厂分发平台实现

```cpp
std::unique_ptr<IMyCollector> MyCollectorFactory::create()
{
#if defined(Q_OS_WIN)
    return std::make_unique<WindowsMyCollector>();
#elif defined(Q_OS_MAC)
    return std::make_unique<MacMyCollector>();
#elif defined(Q_OS_LINUX)
    return std::make_unique<LinuxMyCollector>();
#else
    return nullptr;
#endif
}
```

### 4.3 capability 负责发布统一指标

插件不要直接改 UI。  
监控数据统一通过 [sdk/IMetricSink.h](/Users/qingyizhu/workspace/vitals_new/sdk/IMetricSink.h:1) 发布：

```cpp
MetricFrame frame;
frame.pluginId = QStringLiteral("com.example.myplugin");
frame.timestamp = QDateTime::currentDateTime();
frame.values = {
    {QStringLiteral("myplugin.value"), 42, frame.timestamp, {}}
};

context->metricSink()->publishFrame(frame);
```

宿主会通过 `MetricCenter` 缓存这些值，并驱动 Dashboard、面板和任务栏显示。

## 5. 如何提供插件页面

如果你的插件需要一个导航页面，提供 `IPanelCapability`。

一个典型 capability 会做两件事：

- 返回页面元信息：`panelId()`、`panelName()`、`panelIconKey()`
- 创建一个 QWidget：`createPanel()`

推荐优先复用现有 `widgets/` 里的组件，而不是每个插件从零拼布局。  
如果你的页面只是“标题 + 信息区 + 卡片区”这种信息型页面，可以参考现有 CPU / Network / SystemInfo 插件的 `PanelWidget` 写法。

## 6. 如何参与任务栏 / 托盘显示

如果你的插件希望把摘要显示到托盘或菜单栏，提供 `ITaskbarCapability`。

它需要实现三类内容：

- `displayText()`
- `tooltip()`
- `detailContent()`

注意：

- 插件只负责“内容语义”
- 宿主负责“平台渲染”

也就是说，插件返回的是摘要文本和结构化详情，不要在插件里直接操作托盘图标或菜单栏原生 API。

## 7. 如何提供设置页

如果你的插件需要独立设置页，提供 `ISettingsCapability`。

当前宿主的统一设置中心还在演进中，但 capability 接口已经就位。  
你现在可以先返回一个独立的设置 QWidget，后续宿主接入统一入口后就能直接复用。

## 8. 插件元信息怎么写

每个插件都必须有一个 JSON 元信息文件，例如：

```json
{
  "id": "com.example.myplugin",
  "name": "My Plugin",
  "description": "Example custom plugin for Vitals.",
  "version": "0.1.0",
  "author": "YourName",
  "category": "custom",
  "supportsTaskbarDisplay": true,
  "supportedPlatforms": ["macos"],
  "requiredHostVersion": "0.1.0"
}
```

字段含义与 [sdk/PluginMetaInfo.h](/Users/qingyizhu/workspace/vitals_new/sdk/PluginMetaInfo.h:1) 一致。

重点说明：

- `id` 必须稳定且唯一，推荐用反向域名格式
- `supportedPlatforms` 只写你真正实现并验证过的平台
- `supportsTaskbarDisplay` 表示插件是否提供任务栏能力

不要为了“先显示出来”就声明并未支持的平台。

## 9. CMake 怎么接

最简单的方式是参考现有插件写一个 `MODULE`：

```cmake
add_library(MyPlugin MODULE
    MyPlugin.h
    MyPlugin.cpp
    monitor/MyMonitorCapability.h
    monitor/MyMonitorCapability.cpp
    panel/MyPanelCapability.h
    panel/MyPanelCapability.cpp
    my_plugin.json
)

if(APPLE)
    target_sources(MyPlugin
        PRIVATE
            monitor/platform/macos/MacMyCollector.h
            monitor/platform/macos/MacMyCollector.mm
    )
endif()

target_include_directories(MyPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MyPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

set_target_properties(MyPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)
```

如果是主仓库内插件，还需要在 [plugins/CMakeLists.txt](/Users/qingyizhu/workspace/vitals_new/plugins/CMakeLists.txt:1) 里加：

```cmake
add_subdirectory(myplugin)
```

## 10. 怎么让宿主加载到它

当前宿主会扫描运行目录下的：

```text
build/bin/plugins/
```

因此你需要保证插件最终产物输出到这里。

常见文件名：

- Windows: `MyPlugin.dll`
- macOS: `libMyPlugin.dylib` 或 `libMyPlugin.so`
- Linux: `libMyPlugin.so`

宿主会在加载前先检查：

- 文件扩展名是否是插件库
- 元信息是否包含当前平台

## 11. 推荐开发顺序

建议按这个顺序开发：

1. 先做最小 `IPlugin`，确保能被加载
2. 再加 `IMonitorCapability`，先把指标发出来
3. 再加 `IPanelCapability`，做详情页面
4. 再加 `ITaskbarCapability`
5. 最后加 `ISettingsCapability`

这样最容易排查问题，不会一开始就把 UI、采集、设置、任务栏混在一起。

## 12. 设计原则

写自定义插件时，尽量遵守下面几条：

1. 插件不要直接依赖 `MainWindow` 或其他宿主 UI 类。
2. 插件不要直接改 Dashboard 或托盘组件。
3. 平台差异尽量只放在 collector / platform 目录。
4. capability 负责“能力组织”，collector 负责“平台实现”。
5. 优先复用统一 `MetricData` 模型，不要私造一套宿主外协议。
6. 新插件优先走 capability 模式，不要再回到“一个插件类继承一堆接口”的旧写法。

## 13. 常见问题

### 13.1 插件被扫描到了，但没有加载

优先检查：

- `Q_PLUGIN_METADATA` 是否写了正确的 `IID`
- JSON 元信息文件名是否匹配
- `supportedPlatforms` 是否包含当前平台
- `initialize()` 是否返回了 `false`

### 13.2 插件加载成功，但页面没出现

检查：

- 是否实现了 `panelCapability()`
- `panelId()` 是否为空
- `createPanel()` 是否真的返回了 QWidget

### 13.3 插件加载成功，但 Dashboard 没数据

检查：

- 是否实现了 `monitorCapability()`
- 是否调用了 `metricSink()->publishFrame()`
- metric key 是否稳定且非空

### 13.4 我能不能只做页面插件，不做监控插件

可以。  
`monitorCapability()`、`panelCapability()`、`taskbarCapability()`、`settingsCapability()` 都是可选的。

### 13.5 我能不能做仓库外的第三方插件

可以，但当前项目还没有完整的安装型 SDK 导出流程。  
现阶段最稳的方式仍然是：

- 在主仓库内新增插件目录开发
- 或者在你自己的 CMake 工程里直接引用本仓库的 `sdk/`、`widgets/` 约定和目标链接方式

如果后续项目补齐 `install/export` 能力，第三方插件接入会更顺滑。

## 14. 参考实现

当前仓库里最适合作为参考的插件：

- `plugins/cpu/`
- `plugins/network/`
- `plugins/systeminfo/`

其中：

- CPU 插件适合参考 collector + panel + taskbar 的完整链路
- Network 插件适合参考字节速率、累计流量和任务栏摘要格式
- SystemInfo 插件适合参考信息型页面和只读摘要内容
