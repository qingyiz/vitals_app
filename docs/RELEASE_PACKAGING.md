# Release Packaging

Vitals uses GitHub Actions to build installer-grade release packages for macOS,
Windows, and Linux.

## Outputs

Tag builds such as `v0.1.0` publish these assets:

```text
Vitals-macos.dmg
Vitals-windows-setup.exe
Vitals-linux-x86_64.AppImage
```

Branch builds on `develop_mac` run the same build and packaging jobs, but they do
not publish a GitHub Release.

## macOS Signing And Notarization

macOS tag releases require Apple Developer ID signing and notarization. Configure
these repository secrets before publishing a tag:

```text
APPLE_CERTIFICATE_BASE64
APPLE_CERTIFICATE_PASSWORD
APPLE_SIGNING_IDENTITY
APPLE_ID
APPLE_APP_PASSWORD
APPLE_TEAM_ID
```

`APPLE_CERTIFICATE_BASE64` should contain a base64-encoded `.p12` certificate
exported from a Developer ID Application certificate.

Example local encoding command:

```bash
base64 -i DeveloperIDApplication.p12 | pbcopy
```

`APPLE_SIGNING_IDENTITY` should look like:

```text
Developer ID Application: Your Name (TEAMID)
```

`APPLE_APP_PASSWORD` is an app-specific password for the Apple ID used by
`xcrun notarytool`.

For non-tag branch builds, the workflow can create an unsigned `.dmg` when these
secrets are absent. Tag releases fail fast if any required macOS signing secret
is missing.

## Windows Installer

Windows packages are built with Inno Setup and include the Qt runtime deployed by
`windeployqt`.

## Linux AppImage

Linux packages are built as AppImage artifacts with `linuxdeploy` and the Qt
plugin. The workflow currently builds on Ubuntu 22.04 to keep the generated
AppImage compatible with a wider range of distributions.
