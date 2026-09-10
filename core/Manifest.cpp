#include "Manifest.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mclauncher {

LauncherManifest ParseLauncherManifest(const std::string& text) {
    json j = json::parse(text);
    LauncherManifest m;
    m.latestVersion = j.value("latestVersion", "");
    m.downloadUrl   = j.value("downloadUrl", "");
    m.sha256        = j.value("sha256", "");
    m.changelog     = j.value("changelog", "");
    return m;
}

ModManifest ParseModManifest(const std::string& text) {
    json j = json::parse(text);
    ModManifest m;
    m.mcVersion     = j.value("mcVersion", "1.8.9");
    m.forgeVersion  = j.value("forgeVersion", "");

    for (const auto& e : j.value("mods", json::array())) {
        ModEntry entry;
        entry.id          = e.value("id", "");
        entry.displayName = e.value("displayName", entry.id);
        entry.fileName    = e.value("fileName", "");
        entry.url         = e.value("url", "");
        entry.sha256      = e.value("sha256", "");
        entry.version     = e.value("version", "");
        entry.enabled     = e.value("enabled", true);
        m.mods.push_back(std::move(entry));
    }
    return m;
}

} // namespace mclauncher
