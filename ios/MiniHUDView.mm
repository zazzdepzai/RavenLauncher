#import "MiniHUDView.h"

// Faceted blue palette pulled from the Adobe-XD-style icon reference.
static UIColor *FacetColor(int i) {
    NSArray<UIColor *> *palette = @[
        [UIColor colorWithRed:0.522 green:0.718 blue:0.922 alpha:1.0], // #85B7EB
        [UIColor colorWithRed:0.216 green:0.541 blue:0.867 alpha:1.0], // #378ADD
        [UIColor colorWithRed:0.094 green:0.373 blue:0.647 alpha:1.0], // #185FA5
        [UIColor colorWithRed:0.047 green:0.267 blue:0.486 alpha:1.0], // #0C447C
        [UIColor colorWithRed:0.016 green:0.173 blue:0.325 alpha:1.0], // #042C53
    ];
    return palette[i % palette.count];
}

@interface MiniHUDView ()
@property (nonatomic, strong) CAShapeLayer *badgeShadeLayer; // swaps color w/ loader mode
@property (nonatomic, strong) UILabel *wordmarkLabel;         // "ravenxd"
@property (nonatomic, strong) UILabel *badgePillLabel;        // "forge" / "vanilla" pill
@property (nonatomic, strong) UIView *badgePillView;
@property (nonatomic, strong) UILabel *statusLabel;
@property (nonatomic, strong) UIProgressView *progressView;
@end

@implementation MiniHUDView

+ (instancetype)attachToWindow:(UIWindow *)window {
    CGFloat width = 260, height = 68;
    CGFloat x = window.bounds.size.width - width - 16;
    CGFloat y = 44; // below status bar / notch
    MiniHUDView *hud = [[MiniHUDView alloc] initWithFrame:CGRectMake(x, y, width, height)];
    [window addSubview:hud];
    return hud;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor colorWithRed:0.047 green:0.165 blue:0.290 alpha:0.92]; // deep navy panel
        self.layer.cornerRadius = 16;
        self.layer.masksToBounds = YES;
        self.layer.borderWidth = 0.5;
        self.layer.borderColor = [UIColor colorWithRed:0.216 green:0.541 blue:0.867 alpha:0.35].CGColor;

        [self buildFacetedBadge];
        [self buildLabelsAndProgress:frame];
    }
    return self;
}

// Faceted polygon icon, echoing the reference logo: overlapping
// triangular shards fanning out around a small monogram chip.
- (void)buildFacetedBadge {
    CGFloat s = 44; // badge box side
    CGFloat ox = 12, oy = 12;

    NSArray *facetPoints = @[
        @[@[@0,@30],@[@25,@0],@[@55,@10],@[@45,@45],@[@10,@55]],
        @[@[@25,@0],@[@55,@10],@[@65,@35],@[@45,@45]],
        @[@[@10,@55],@[@45,@45],@[@50,@75],@[@20,@80]],
        @[@[@45,@45],@[@65,@35],@[@70,@65],@[@50,@75]],
        @[@[@0,@30],@[@10,@55],@[@20,@80],@[-10,@75],@[-15,@45]],
    ];

    for (NSUInteger i = 0; i < facetPoints.count; i++) {
        UIBezierPath *path = [UIBezierPath bezierPath];
        NSArray *pts = facetPoints[i];
        for (NSUInteger p = 0; p < pts.count; p++) {
            NSArray *xy = pts[p];
            CGFloat px = ox + [xy[0] floatValue] * (s / 70.0);
            CGFloat py = oy + [xy[1] floatValue] * (s / 70.0);
            if (p == 0) [path moveToPoint:CGPointMake(px, py)];
            else [path addLineToPoint:CGPointMake(px, py)];
        }
        [path closePath];

        CAShapeLayer *facet = [CAShapeLayer layer];
        facet.path = path.CGPath;
        facet.fillColor = FacetColor((int)i).CGColor;
        [self.layer addSublayer:facet];
    }

    // Monogram chip sits on top of the facets, holds the loader glyph.
    self.badgeShadeLayer = [CAShapeLayer layer];
    self.badgeShadeLayer.path = [UIBezierPath bezierPathWithRoundedRect:CGRectMake(ox + 6, oy + 22, 28, 22)
                                                            cornerRadius:3].CGPath;
    self.badgeShadeLayer.fillColor = [UIColor colorWithRed:0.047 green:0.165 blue:0.290 alpha:1.0].CGColor;
    [self.layer addSublayer:self.badgeShadeLayer];

    UILabel *glyph = [[UILabel alloc] initWithFrame:CGRectMake(ox + 6, oy + 22, 28, 22)];
    glyph.text = @"X";
    glyph.textAlignment = NSTextAlignmentCenter;
    glyph.font = [UIFont boldSystemFontOfSize:14];
    glyph.textColor = FacetColor(0);
    [self addSubview:glyph];
}

- (void)buildLabelsAndProgress:(CGRect)frame {
    CGFloat leftInset = 64; // clears the badge

    _wordmarkLabel = [[UILabel alloc] initWithFrame:CGRectMake(leftInset, 10, 100, 18)];
    _wordmarkLabel.text = @"ravenxd";
    _wordmarkLabel.font = [UIFont systemFontOfSize:15 weight:UIFontWeightMedium];
    _wordmarkLabel.textColor = FacetColor(0);
    [self addSubview:_wordmarkLabel];

    _badgePillView = [[UIView alloc] initWithFrame:CGRectMake(frame.size.width - 66, 10, 54, 20)];
    _badgePillView.backgroundColor = FacetColor(3);
    _badgePillView.layer.cornerRadius = 10;
    [self addSubview:_badgePillView];

    _badgePillLabel = [[UILabel alloc] initWithFrame:_badgePillView.bounds];
    _badgePillLabel.text = @"vanilla";
    _badgePillLabel.font = [UIFont systemFontOfSize:10 weight:UIFontWeightMedium];
    _badgePillLabel.textColor = FacetColor(0);
    _badgePillLabel.textAlignment = NSTextAlignmentCenter;
    [_badgePillView addSubview:_badgePillLabel];

    _statusLabel = [[UILabel alloc] initWithFrame:CGRectMake(leftInset, 32, frame.size.width - leftInset - 12, 16)];
    _statusLabel.font = [UIFont systemFontOfSize:11];
    _statusLabel.textColor = [UIColor colorWithWhite:0.92 alpha:1.0];
    _statusLabel.text = @"ready";
    [self addSubview:_statusLabel];

    _progressView = [[UIProgressView alloc] initWithFrame:CGRectMake(leftInset, 52, frame.size.width - leftInset - 12, 4)];
    _progressView.progressTintColor = FacetColor(0);
    _progressView.trackTintColor = [UIColor colorWithWhite:1.0 alpha:0.12];
    _progressView.layer.cornerRadius = 2;
    _progressView.clipsToBounds = YES;
    _progressView.progress = 0.0;
    [self addSubview:_progressView];
}

- (void)setStatusText:(NSString *)text {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.statusLabel.text = [text lowercaseString];
    });
}

- (void)setProgress:(float)progress {
    dispatch_async(dispatch_get_main_queue(), ^{
        self.progressView.hidden = (progress < 0);
        if (progress >= 0) {
            [self.progressView setProgress:progress animated:YES];
        }
    });
}

// badgeText: "FORGE" or "VANILLA" (kept as the public API contract).
- (void)setLoaderBadge:(NSString *)badgeText {
    dispatch_async(dispatch_get_main_queue(), ^{
        BOOL isForge = [badgeText isEqualToString:@"FORGE"];
        self.badgePillLabel.text = isForge ? @"forge" : @"vanilla";
        self.badgePillView.backgroundColor = isForge ? FacetColor(1) : FacetColor(3);
        self.badgePillLabel.textColor = isForge ? FacetColor(4) : FacetColor(0);
    });
}

- (void)fadeOutAndRemove {
    dispatch_async(dispatch_get_main_queue(), ^{
        [UIView animateWithDuration:0.3 animations:^{
            self.alpha = 0.0;
        } completion:^(BOOL finished) {
            [self removeFromSuperview];
        }];
    });
}

@end
