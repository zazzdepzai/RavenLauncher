#pragma once
#include <string>
#include <vector>
#include "Manifest.hpp"
#include "LauncherProfile.hpp"

namespace mclauncher {

enum class UpdateStatus {
    UpToDate,
    UpdateAvailable,
    UpdateDownloaded,
    Failed
};

struct UpdateReport {
    UpdateStatus status = UpdateStatus::UpToDate;
    std::string message;
    std::vector<std::string> updatedMods; // display names of mods that changed
};

class UpdateManager {
public:
    UpdateManager(std::string launcherManifestUrl,
                  std::string modManifestUrlTemplate, // "{mc}/{loader}/mods.json"
                  std::string currentLauncherVersion);

    // Checks Anthropic-hosted... (your own) manifest for a newer launcher build.
    // On UpdateAvailable it also downloads the package to `stagingDir` and
    // reports UpdateDownloaded once verified, ready for the OS-level installer
    // (e.g. AltStore/TrollStore refresh, or in-house patcher) to apply it.
    UpdateReport CheckAndUpdateLauncher(const std::string& stagingDir);

    // Diffs the profile's currently-installed mods against the remote
    // ModManifest for that mc/loader combo, downloads anything new/changed,
    // and removes mods no longer listed (if pruneRemoved is true).
    UpdateReport CheckAndUpdateMods(LauncherProfile& profile, bool pruneRemoved = true);

private:
    std::string launcherManifestUrl_;
    std::string modManifestUrlTemplate_;
    std::string currentLauncherVersion_;

    std::string ResolveModManifestUrl(const LauncherProfile& profile) const;
};

} // namespace mclauncher
