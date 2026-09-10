# MCLauncherIOS — 1.8.9 Launcher Core (Vanilla / Forge, Mini HUD, Auto-Update)

## What this is
A C++ launcher **core** (platform-independent) plus an iOS Objective-C++ **bridge**
implementing:

- Vanilla / Forge profile switch for Minecraft **1.8.9** (`LauncherProfile`, `GameLauncher`)
- A mini corner HUD (`MiniHUDView`) showing loader badge, status text, progress bar
- **Launcher self-update** (`UpdateManager::CheckAndUpdateLauncher`)
- **Mod auto-update** for the Forge profile — diffs against a remote manifest,
  downloads changed/new mods, prunes removed ones (`UpdateManager::CheckAndUpdateMods`)

## Important reality check before you build on this
Minecraft **Java Edition** requires a JVM. There is no official JVM for iOS, and
Apple's App Store guidelines prohibit shipping a general-purpose interpreter/JIT.
This is why real iOS Minecraft launchers (PojavLauncher's iOS fork, Zink-based
projects) only run on **jailbroken devices or via sideloading** (AltStore /
TrollStore / enterprise cert), using:
- A community JRE-8-for-iOS build, and
- A GL-to-Metal/Vulkan translation shim (e.g. MobGlass/Zink) since 1.8.9's
  renderer expects desktop OpenGL.

This scaffold gives you the **launcher/updater/HUD layer only** — the part that's
legitimately just "download files, verify hashes, manage config, show progress."
You still need to integrate an existing iOS JVM runtime (`GameLauncher::BuildLaunchCommand`
hands you `mainClass` / `classpath` / `jvmArgs` / `gameArgs` to feed into it) —
that piece is a large, separate undertaking and is not included here.

## Layout
```
MCLauncherIOS/
├── core/                  # Platform-independent C++ (buildable via CMake)
│   ├── LauncherProfile.hpp    # Vanilla/Forge profile + mod list model
│   ├── Manifest.hpp/.cpp      # JSON parsing for launcher/mod manifests
│   ├── DownloadManager.hpp/.cpp   # libcurl downloads + SHA-256 verification
│   ├── UpdateManager.hpp/.cpp     # self-update + mod auto-update logic
│   └── GameLauncher.hpp/.cpp      # builds JVM launch args for 1.8.9 (+Forge)
├── ios/                   # Objective-C++, add these files to your Xcode target
│   ├── MiniHUDView.h/.mm      # compact corner HUD (badge/status/progress)
│   └── LauncherBridge.h/.mm   # glues UIKit <-> C++ core, drives the HUD
├── manifests/             # Example JSON you host on your own server
│   ├── launcher_manifest.example.json
│   └── mods_1.8.9_forge.example.json
└── CMakeLists.txt         # builds core/ as a static lib (desktop testing)
```

## Wiring it up in Xcode
1. Create an iOS App target (Swift or Obj-C), enable Objective-C++ (rename
   `AppDelegate.m` → `.mm` if needed, or just add these `.mm` files directly).
2. Add all of `ios/*.h`, `ios/*.mm` and `core/*.hpp`, `core/*.cpp` to the target.
3. Add `libcurl`, `OpenSSL` (or swap `DownloadManager`'s hashing for
   `CommonCrypto`/`CryptoKit` to avoid the OpenSSL dependency on-device),
   and `nlohmann/json` (header-only, easiest via SPM or vendoring).
4. In your root view controller / SceneDelegate:
   ```objc
   self.launcher = [[LauncherBridge alloc] initWithWindow:self.window];
   [self.launcher setForgeEnabled:userPickedForge]; // toggle from your UI
   [self.launcher runUpdateChecksWithCompletion:^(BOOL ready) {
       if (ready) {
           [self.launcher launchWithUsername:name uuid:uuid accessToken:token];
       }
   }];
   ```
5. Host `launcher_manifest.json` and `mods/{mc}/{loader}/mods.json` on your own
   server, matching the example files in `manifests/`, and point
   `LauncherBridge.mm`'s `UpdateManager(...)` constructor at your real URLs.

## Desktop HUD preview (no Mac, no Xcode, no .ipa)
`desktop/` is a standalone SFML app that renders the same faceted `ravenxd`
badge, wordmark, loader pill, and progress bar as `ios/MiniHUDView.mm`, in a
plain Windows/Linux window. It simulates an update cycle locally rather than
hitting a real server, so you can see and interact with the HUD design
without any iOS toolchain.

**Linux:**
```bash
sudo apt install libsfml-dev cmake
cd desktop
cmake -B build
cmake --build build
./build/ravenxd_hud_desktop
```

**Windows (vcpkg):**
```powershell
vcpkg install sfml
cd desktop
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build
build\Debug\ravenxd_hud_desktop.exe
```

Controls: `F` toggles Forge/Vanilla, `Space` runs a simulated update cycle
(status text + progress bar), `Esc` quits.

CI builds this automatically in the `desktop-hud` job of
`.github/workflows/build.yml` and uploads the Linux binary as an artifact.

## Building/testing the core on desktop (sanity check, not the iOS app)
```bash
cd MCLauncherIOS
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```
This validates the manifest parsing / download / update-diff logic without
needing Xcode, since that logic is 100% portable C++.

## Extending
- Swap `DownloadManager`'s OpenSSL SHA-256 for `CommonCrypto` if you want to
  drop the OpenSSL dependency entirely on-device.
- Add a `LauncherBridge` method to expose per-mod enable/disable toggles to
  a SwiftUI settings screen, backed by `LauncherProfile::mods[i].enabled`.
- If you want auto-update to *apply itself* without a manual relaunch, you'll
  need your sideloading tool's refresh API (e.g. AltStore's) — plain iOS apps
  can't replace their own binary.
