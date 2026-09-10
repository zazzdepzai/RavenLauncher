#pragma once
#include <string>
#include <vector>
#include "LauncherProfile.hpp"

namespace mclauncher {

// Describes the latest available launcher build (self-update).
struct LauncherManifest {
    std::string latestVersion;    // "2.3.0"
    std::string downloadUrl;      // ipa / zip patch
    std::string sha256;
    std::string changelog;
};

// Describes the latest set of mods for a given profile (mod auto-update).
struct ModManifest {
    std::string mcVersion;        // "1.8.9"
    std::string forgeVersion;     // required forge build, empty = vanilla-only pack
    std::vector<ModEntry> mods;
};

// Parses raw JSON text (using a minimal embedded parser call site;
// swap in nlohmann/json in real build, see Manifest.cpp).
LauncherManifest ParseLauncherManifest(const std::string& json);
ModManifest ParseModManifest(const std::string& json);

} // namespace mclauncher
