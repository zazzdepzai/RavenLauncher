#pragma once
#include <string>
#include <vector>
#include "LauncherProfile.hpp"

namespace mclauncher {

struct LaunchCommand {
    std::string mainClass;              // net.minecraft.client.main.Main or Forge's launch wrapper
    std::vector<std::string> classpath; // jars needed: client.jar, forge jars, libraries
    std::vector<std::string> jvmArgs;
    std::vector<std::string> gameArgs;  // --username, --uuid, --accessToken, etc.
};

class GameLauncher {
public:
    // Builds the argument set to hand to the on-device JVM runtime
    // (e.g. a bundled iOS-compatible JRE 8, invoked out-of-process).
    // This class does NOT execute the JVM itself -- on iOS that must be
    // done by whatever sandboxed runtime you ship (see README).
    static LaunchCommand BuildLaunchCommand(const LauncherProfile& profile,
                                             const std::string& username,
                                             const std::string& uuid,
                                             const std::string& accessToken);

private:
    static void AddVanillaClasspath(const LauncherProfile& profile, LaunchCommand& cmd);
    static void AddForgeClasspath(const LauncherProfile& profile, LaunchCommand& cmd);
};

} // namespace mclauncher
