#include "UpdateManager.hpp"
#include "DownloadManager.hpp"
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace mclauncher {

UpdateManager::UpdateManager(std::string launcherManifestUrl,
                              std::string modManifestUrlTemplate,
                              std::string currentLauncherVersion)
    : launcherManifestUrl_(std::move(launcherManifestUrl)),
      modManifestUrlTemplate_(std::move(modManifestUrlTemplate)),
      currentLauncherVersion_(std::move(currentLauncherVersion)) {}

std::string UpdateManager::ResolveModManifestUrl(const LauncherProfile& profile) const {
    std::string url = modManifestUrlTemplate_;
    std::string loaderStr = profile.isForge() ? "forge" : "vanilla";

    auto replace = [](std::string& s, const std::string& from, const std::string& to) {
        size_t pos = s.find(from);
        if (pos != std::string::npos) s.replace(pos, from.size(), to);
    };
    replace(url, "{mc}", profile.mcVersion);
    replace(url, "{loader}", loaderStr);
    return url;
}

UpdateReport UpdateManager::CheckAndUpdateLauncher(const std::string& stagingDir) {
    UpdateReport report;

    std::string raw;
    if (!DownloadManager::FetchText(launcherManifestUrl_, raw)) {
        report.status = UpdateStatus::Failed;
        report.message = "could not reach launcher manifest";
        return report;
    }

    LauncherManifest manifest = ParseLauncherManifest(raw);
    if (manifest.latestVersion.empty() || manifest.latestVersion == currentLauncherVersion_) {
        report.status = UpdateStatus::UpToDate;
        report.message = "launcher is up to date (" + currentLauncherVersion_ + ")";
        return report;
    }

    fs::create_directories(stagingDir);
    std::string dest = stagingDir + "/launcher_" + manifest.latestVersion + ".pkg";

    auto dl = DownloadManager::DownloadFile(manifest.downloadUrl, dest, manifest.sha256);
    if (!dl.success) {
        report.status = UpdateStatus::Failed;
        report.message = "launcher download failed: " + dl.error;
        return report;
    }

    report.status = UpdateStatus::UpdateDownloaded;
    report.message = "downloaded launcher " + manifest.latestVersion +
                      " -> " + dest + ". Restart to apply.";
    return report;
}

UpdateReport UpdateManager::CheckAndUpdateMods(LauncherProfile& profile, bool pruneRemoved) {
    UpdateReport report;

    if (!profile.isForge()) {
        report.status = UpdateStatus::UpToDate;
        report.message = "vanilla profile selected, no mods to update";
        return report;
    }

    std::string url = ResolveModManifestUrl(profile);
    std::string raw;
    if (!DownloadManager::FetchText(url, raw)) {
        report.status = UpdateStatus::Failed;
        report.message = "could not reach mod manifest: " + url;
        return report;
    }

    ModManifest remote = ParseModManifest(raw);
    std::string modsDir = profile.gameDir + "/mods";
    fs::create_directories(modsDir);

    // Update / add mods.
    std::vector<ModEntry> newList;
    for (const auto& remoteMod : remote.mods) {
        auto it = std::find_if(profile.mods.begin(), profile.mods.end(),
            [&](const ModEntry& m) { return m.id == remoteMod.id; });

        bool needsFetch = (it == profile.mods.end()) || (it->version != remoteMod.version);
        std::string destPath = modsDir + "/" + remoteMod.fileName;

        if (needsFetch) {
            auto dl = DownloadManager::DownloadFile(remoteMod.url, destPath, remoteMod.sha256);
            if (!dl.success) {
                report.status = UpdateStatus::Failed;
                report.message = "mod download failed (" + remoteMod.displayName + "): " + dl.error;
                return report;
            }
            report.updatedMods.push_back(remoteMod.displayName);
        }
        newList.push_back(remoteMod);
    }

    // Prune mods that are no longer in the remote list.
    if (pruneRemoved) {
        for (const auto& localMod : profile.mods) {
            bool stillListed = std::any_of(remote.mods.begin(), remote.mods.end(),
                [&](const ModEntry& m) { return m.id == localMod.id; });
            if (!stillListed) {
                fs::remove(modsDir + "/" + localMod.fileName);
            }
        }
    }

    profile.mods = std::move(newList);
    profile.forgeVersion = remote.forgeVersion;

    report.status = report.updatedMods.empty() ? UpdateStatus::UpToDate
                                                 : UpdateStatus::UpdateDownloaded;
    report.message = report.updatedMods.empty()
        ? "mods already up to date"
        : std::to_string(report.updatedMods.size()) + " mod(s) updated";
    return report;
}

} // namespace mclauncher
