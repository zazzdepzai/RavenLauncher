#import "LauncherBridge.h"
#import "MiniHUDView.h"
#include "../core/UpdateManager.hpp"
#include "../core/GameLauncher.hpp"
#include "../core/LauncherProfile.hpp"
#include <thread>

using namespace mclauncher;

@interface LauncherBridge ()
@property (nonatomic, strong) MiniHUDView *hud;
@end

@implementation LauncherBridge {
    LauncherProfile _profile;
}

- (instancetype)initWithWindow:(UIWindow *)window {
    self = [super init];
    if (self) {
        _hud = [MiniHUDView attachToWindow:window];

        NSString *docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        _profile.mcVersion = "1.8.9";
        _profile.gameDir = [[docs stringByAppendingPathComponent:@"minecraft"] UTF8String];
        _profile.loader = LoaderType::Vanilla; // default: "forger none"
    }
    return self;
}

- (void)setForgeEnabled:(BOOL)enabled {
    _profile.loader = enabled ? LoaderType::Forge : LoaderType::Vanilla;
    [self.hud setLoaderBadge:enabled ? @"FORGE" : @"VANILLA"];
}

// Runs launcher + mod update checks off the main thread, updating the HUD,
// then calls completion(YES) if ready to launch.
- (void)runUpdateChecksWithCompletion:(void (^)(BOOL readyToLaunch))completion {
    [self.hud setStatusText:@"Checking launcher update..."];
    [self.hud setProgress:-1];

    std::thread([self, completion]() {
        UpdateManager updater(
            "https://your-server.example.com/launcher/manifest.json",
            "https://your-server.example.com/mods/{mc}/{loader}/mods.json",
            "1.0.0" // current launcher version, wire up to your Info.plist/CFBundleVersion
        );

        NSString *docs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        std::string staging = [[docs stringByAppendingPathComponent:@"staging"] UTF8String];

        auto launcherReport = updater.CheckAndUpdateLauncher(staging);
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.hud setStatusText:[NSString stringWithUTF8String:launcherReport.message.c_str()]];
        });

        [self.hud setStatusText:@"Checking mod updates..."];
        auto modReport = updater.CheckAndUpdateMods(_profile, /*pruneRemoved=*/true);
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.hud setStatusText:[NSString stringWithUTF8String:modReport.message.c_str()]];
            [self.hud setProgress:1.0];
        });

        bool failed = (launcherReport.status == UpdateStatus::Failed) ||
                      (modReport.status == UpdateStatus::Failed);

        dispatch_async(dispatch_get_main_queue(), ^{
            if (completion) completion(failed ? NO : YES);
        });
    }).detach();
}

- (void)launchWithUsername:(NSString *)username uuid:(NSString *)uuid accessToken:(NSString *)token {
    LaunchCommand cmd = GameLauncher::BuildLaunchCommand(
        _profile, [username UTF8String], [uuid UTF8String], [token UTF8String]);

    // NOTE: iOS cannot fork/exec a JVM the way desktop launchers do.
    // Hand `cmd` (mainClass / classpath / jvmArgs / gameArgs) to whatever
    // bundled JRE-for-iOS runtime you're shipping (e.g. an embedded JVM
    // linked into this same process, invoked via its own C API) --
    // that runtime is out of scope for this launcher-core scaffold.
    [self.hud setStatusText:@"Launching..."];
    NSLog(@"Main class: %s", cmd.mainClass.c_str());
}

- (void)hideHUD {
    [self.hud fadeOutAndRemove];
}

@end
