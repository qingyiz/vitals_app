# Vitals

Vitals is a C++ / Qt / CMake desktop system monitor framework built around a host application, plugin SDK, metric center, and extensible UI panels.

The host application owns the window, navigation, plugin lifecycle, and metric display containers. Concrete monitoring capabilities should be implemented as plugins.

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

The application is emitted to:

```text
build/bin/Vitals
```

Plugins are emitted to:

```text
build/bin/plugins/
```

## Current Status

The initial framework includes:

- SDK interfaces for the base plugin contract, monitor/panel/taskbar/settings capabilities, app context, and metrics.
- Core plugin manager and metric center.
- Platform-aware plugin loading through `supportedPlatforms` metadata.
- Qt host application with dashboard and navigation.
- Cross-platform taskbar/tray/menu-bar indicator fed by `MetricCenter`, with capability-aware plugin integration.
- CPU, network, and system information plugins loaded through the plugin runtime using the new "plugin shell + capability objects" structure.

See `docs/TASK_PLAN.md` for the current task plan.

If you want to build a custom plugin, start with [docs/PLUGIN_DEVELOPMENT.md](/Users/qingyizhu/workspace/vitals_new/docs/PLUGIN_DEVELOPMENT.md:1).
