#include "GameLauncher.hpp"

namespace mclauncher {

void GameLauncher::AddVanillaClasspath(const LauncherProfile& profile, LaunchCommand& cmd) {
    cmd.classpath.push_back(profile.gameDir + "/versions/1.8.9/1.8.9.jar");
    cmd.classpath.push_back(profile.gameDir + "/libraries"); // resolved recursively by loader
    cmd.mainClass = "net.minecraft.client.main.Main";
}

void GameLauncher::AddForgeClasspath(const LauncherProfile& profile, LaunchCommand& cmd) {
    // Forge 1.8.9 uses LaunchWrapper with a tweak class.
    cmd.classpath.push_back(profile.gameDir + "/versions/1.8.9/1.8.9.jar");
    cmd.classpath.push_back(profile.gameDir + "/versions/1.8.9-forge-" +
                             profile.forgeVersion + "/forge.jar");
    cmd.classpath.push_back(profile.gameDir + "/libraries");
    for (const auto& mod : profile.mods) {
        if (mod.enabled) {
            cmd.classpath.push_back(profile.gameDir + "/mods/" + mod.fileName);
        }
    }
    cmd.mainClass = "net.minecraft.launchwrapper.Launch";
    cmd.gameArgs.push_back("--tweakClass");
    cmd.gameArgs.push_back("net.minecraftforge.fml.common.launcher.FMLTweaker");
}

LaunchCommand GameLauncher::BuildLaunchCommand(const LauncherProfile& profile,
                                                const std::string& username,
                                                const std::string& uuid,
                                                const std::string& accessToken) {
    LaunchCommand cmd;

    // Split "-Xmx1024M -Xms512M" style string into discrete args.
    std::string arg;
    for (char c : profile.javaArgs) {
        if (c == ' ') {
            if (!arg.empty()) { cmd.jvmArgs.push_back(arg); arg.clear(); }
        } else {
            arg += c;
        }
    }
    if (!arg.empty()) cmd.jvmArgs.push_back(arg);

    if (profile.isForge()) {
        AddForgeClasspath(profile, cmd);
    } else {
        AddVanillaClasspath(profile, cmd);
    }

    cmd.gameArgs.push_back("--version");     cmd.gameArgs.push_back(profile.mcVersion);
    cmd.gameArgs.push_back("--gameDir");     cmd.gameArgs.push_back(profile.gameDir);
    cmd.gameArgs.push_back("--assetsDir");   cmd.gameArgs.push_back(profile.gameDir + "/assets");
    cmd.gameArgs.push_back("--username");    cmd.gameArgs.push_back(username);
    cmd.gameArgs.push_back("--uuid");        cmd.gameArgs.push_back(uuid);
    cmd.gameArgs.push_back("--accessToken"); cmd.gameArgs.push_back(accessToken);
    cmd.gameArgs.push_back("--userType");    cmd.gameArgs.push_back("mojang");

    return cmd;
}

} // namespace mclauncher
