#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

// Small, semi-transparent overlay shown while the launcher checks for
// updates / downloads mods / prepares to launch. "Mini" = compact pill
// in the corner, not a full-screen progress view.
// Visual style: deep navy panel with a faceted blue polygon badge
// (echoing an Adobe-XD-style icon) and a "ravenxd" wordmark.
@interface MiniHUDView : UIView

+ (instancetype)attachToWindow:(UIWindow *)window;

// Update the single status line, e.g. "Checking launcher update..."
- (void)setStatusText:(NSString *)text;

// Update the progress bar (0.0 - 1.0). Pass a negative value to hide the bar
// (indeterminate / idle state).
- (void)setProgress:(float)progress;

// Toggle Forge/Vanilla badge shown in the HUD corner.
- (void)setLoaderBadge:(NSString *)badgeText; // "FORGE" or "VANILLA"

- (void)fadeOutAndRemove;

@end

NS_ASSUME_NONNULL_END
