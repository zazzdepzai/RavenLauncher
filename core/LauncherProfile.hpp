#pragma once
#include <string>
#include <vector>

namespace mclauncher {

// Which loader the user wants to launch with.
enum class LoaderType {
    Vanilla,   // "forge none"
    Forge      // "forge option"
};

struct ModEntry {
    std::string id;
    std::string displayName;
    std::string fileName;      // e.g. "optifine-1.8.9.jar"
    std::string url;           // download URL
    std::string sha256;        // integrity check
    std::string version;       // mod's own version string
    bool enabled = true;
};

struct LauncherProfile {
    std::string profileName;      // "1.8.9 Vanilla" / "1.8.9 Forge"
    std::string mcVersion = "1.8.9";
    LoaderType loader = LoaderType::Vanilla;
    std::string forgeVersion;     // e.g. "11.15.1.2318" (empty if Vanilla)
    std::string gameDir;          // sandboxed app dir on iOS
    std::string javaArgs = "-Xmx1024M -Xms512M";
    std::vector<ModEntry> mods;   // only relevant if loader == Forge

    bool isForge() const { return loader == LoaderType::Forge; }
};

} // namespace mclauncher
