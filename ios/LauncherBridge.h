#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

// Objective-C-facing bridge to the C++ launcher core. Instantiate once
// from your root view controller / SceneDelegate.
@interface LauncherBridge : NSObject

- (instancetype)initWithWindow:(UIWindow *)window;

// YES = Forge profile ("forge option"), NO = Vanilla ("forge none").
- (void)setForgeEnabled:(BOOL)enabled;

// Checks + applies launcher self-update and mod auto-update, showing
// progress in the mini HUD. completion(YES) means safe to launch.
- (void)runUpdateChecksWithCompletion:(void (^)(BOOL readyToLaunch))completion;

- (void)launchWithUsername:(NSString *)username
                       uuid:(NSString *)uuid
                accessToken:(NSString *)token;

- (void)hideHUD;

@end

NS_ASSUME_NONNULL_END
