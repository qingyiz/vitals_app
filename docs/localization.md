# Localization

Vitals host UI text is loaded from JSON files under `assets/languages/`.

At build time CMake copies this directory to `build/bin/languages`, and the app also
keeps the same files in Qt resources as a fallback. The runtime directory wins,
so a packaged build can receive an extra language by adding a JSON file and
updating `languages/languages.json`.

## Add a Language

1. Copy `assets/languages/en-US.json` to a new file, for example `ja-JP.json`.
2. Translate values under the `translations` object. Keep keys unchanged.
3. Register the file in `assets/languages/languages.json`:

```json
{
  "code": "ja-JP",
  "name": "Japanese",
  "nativeName": "日本語",
  "file": "ja-JP.json"
}
```

The selected language is saved in `config/app.json` as `language`.

## Plugin Rules

Plugins must use the same language catalog as the host for all user-facing
static text.

- Panel titles, subtitles, section names, row labels, tile labels, badges,
  settings labels, empty states, taskbar/menu labels, and tooltips must use
  `IAppContext::translate(key, fallback)`.
- Plugins should not hard-code English or Chinese UI text in widgets or
  capabilities, except as the fallback argument passed to `translate()`.
- Plugin translation keys should be grouped by plugin area, for example
  `cpu.title`, `memory.usedMemory`, `network.currentThroughput`, or
  `systemInfo.currentSnapshot`.
- Values collected from the operating system, such as device name, CPU model,
  interface name, file path, or plugin ID, should stay as collected and should
  not be translated.
- When adding a plugin page or settings page, add both `en-US.json` and
  `zh-CN.json` entries in the same change.
