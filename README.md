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

- SDK interfaces for plugins, panels, monitor plugins, app context, and metrics.
- Core plugin manager and metric center.
- Platform-aware plugin loading through `supportedPlatforms` metadata.
- Qt host application with dashboard and navigation.
- Cross-platform taskbar/tray/menu-bar indicator fed by `MetricCenter`.
- A dynamically loaded `HelloPlugin`.

See `docs/TASK_PLAN.md` for the current task plan.
