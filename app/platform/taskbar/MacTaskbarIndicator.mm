#include "platform/taskbar/MacTaskbarIndicator.h"

#import <Cocoa/Cocoa.h>

#include <QBuffer>
#include <QPixmap>
#include <QWidget>

namespace Vitals {
class MacTaskbarIndicator;
}

@interface VitalsStatusItemTarget : NSObject
@property(nonatomic, assign) Vitals::MacTaskbarIndicator* indicator;
- (void)showWindow:(id)sender;
- (void)quitApp:(id)sender;
@end

namespace Vitals {

namespace {

NSString* nsStringFromQString(const QString& value)
{
    return [NSString stringWithUTF8String:value.toUtf8().constData()];
}

NSImage* nsImageFromQPixmap(const QPixmap& pixmap, bool isTemplate)
{
    if (pixmap.isNull()) {
        return nil;
    }

    QByteArray pngBytes;
    QBuffer buffer(&pngBytes);
    if (!buffer.open(QIODevice::WriteOnly)) {
        return nil;
    }

    if (!pixmap.save(&buffer, "PNG")) {
        return nil;
    }

    NSData* imageData = [NSData dataWithBytes:pngBytes.constData() length:static_cast<NSUInteger>(pngBytes.size())];
    NSImage* image = [[[NSImage alloc] initWithData:imageData] autorelease];
    if (image) {
        const qreal devicePixelRatio = pixmap.devicePixelRatio();
        [image setTemplate:isTemplate ? YES : NO];
        [image setSize:NSMakeSize(pixmap.width() / devicePixelRatio, pixmap.height() / devicePixelRatio)];
    }
    return image;
}

NSColor* colorFromHex(const QString& hex, NSColor* fallback)
{
    QString value = hex;
    if (value.startsWith(QLatin1Char('#'))) {
        value.remove(0, 1);
    }
    if (value.size() != 6) {
        return fallback;
    }

    bool ok = false;
    const int rgb = value.toInt(&ok, 16);
    if (!ok) {
        return fallback;
    }

    return [NSColor colorWithCalibratedRed:((rgb >> 16) & 0xff) / 255.0
                                     green:((rgb >> 8) & 0xff) / 255.0
                                      blue:(rgb & 0xff) / 255.0
                                     alpha:1.0];
}

NSTextField* labelField(const QString& text, NSFont* font, NSColor* color)
{
    NSTextField* label = [NSTextField labelWithString:nsStringFromQString(text)];
    [label setFont:font];
    [label setTextColor:color];
    [label setLineBreakMode:NSLineBreakByTruncatingTail];
    return label;
}

NSTextField* framedLabel(const QString& text, NSRect frame, NSFont* font, NSColor* color, NSTextAlignment alignment = NSTextAlignmentLeft)
{
    NSTextField* label = labelField(text, font, color);
    [label setFrame:frame];
    [label setAlignment:alignment];
    return label;
}

NSStackView* verticalStack(CGFloat spacing)
{
    NSStackView* stack = [[[NSStackView alloc] init] autorelease];
    [stack setOrientation:NSUserInterfaceLayoutOrientationVertical];
    [stack setAlignment:NSLayoutAttributeWidth];
    [stack setSpacing:spacing];
    return stack;
}

NSStackView* horizontalStack(CGFloat spacing)
{
    NSStackView* stack = [[[NSStackView alloc] init] autorelease];
    [stack setOrientation:NSUserInterfaceLayoutOrientationHorizontal];
    [stack setAlignment:NSLayoutAttributeCenterY];
    [stack setSpacing:spacing];
    return stack;
}

NSView* badgeView(const TaskbarDetailBadge& badge)
{
    NSStackView* stack = verticalStack(1.0);
    [stack setEdgeInsets:NSEdgeInsetsMake(6.0, 8.0, 6.0, 8.0)];
    [stack addArrangedSubview:labelField(badge.label, [NSFont systemFontOfSize:9.0 weight:NSFontWeightSemibold], [NSColor secondaryLabelColor])];
    [stack addArrangedSubview:labelField(badge.value, [NSFont monospacedDigitSystemFontOfSize:12.0 weight:NSFontWeightSemibold], [NSColor labelColor])];
    [stack setWantsLayer:YES];
    [stack layer].cornerRadius = 7.0;
    [stack layer].backgroundColor = [[NSColor controlBackgroundColor] CGColor];
    return stack;
}

NSView* rowView(const TaskbarDetailRow& row)
{
    NSStackView* stack = verticalStack(4.0);
    NSStackView* top = horizontalStack(8.0);

    NSTextField* title = labelField(row.label, [NSFont systemFontOfSize:10.0 weight:NSFontWeightSemibold], [NSColor secondaryLabelColor]);
    NSTextField* value = labelField(row.value, [NSFont monospacedDigitSystemFontOfSize:12.0 weight:NSFontWeightSemibold], [NSColor labelColor]);
    [title setContentHuggingPriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
    [value setContentHuggingPriority:NSLayoutPriorityRequired forOrientation:NSLayoutConstraintOrientationHorizontal];
    [top addArrangedSubview:title];
    [top addArrangedSubview:value];
    [stack addArrangedSubview:top];

    if (!row.detail.isEmpty()) {
        [stack addArrangedSubview:labelField(row.detail, [NSFont systemFontOfSize:10.0], [NSColor tertiaryLabelColor])];
    }

    if (row.progress >= 0.0) {
        NSProgressIndicator* progress = [[[NSProgressIndicator alloc] init] autorelease];
        [progress setIndeterminate:NO];
        [progress setMinValue:0.0];
        [progress setMaxValue:100.0];
        [progress setDoubleValue:qBound(0.0, row.progress, 100.0)];
        [progress setControlSize:NSControlSizeSmall];
        [progress setStyle:NSProgressIndicatorStyleBar];
        [[progress heightAnchor] constraintEqualToConstant:6.0].active = YES;
        [stack addArrangedSubview:progress];
    }

    return stack;
}

NSView* contentView(const TaskbarDetailContent& content)
{
    constexpr CGFloat width = 318.0;
    constexpr CGFloat inset = 14.0;
    CGFloat height = 78.0;
    if (!content.badges.isEmpty()) {
        height += 42.0;
    }
    for (const TaskbarDetailSection& section : content.sections) {
        if (!section.title.isEmpty()) {
            height += 24.0;
        }
        for (const TaskbarDetailRow& row : section.rows) {
            height += row.progress >= 0.0 ? 31.0 : 22.0;
            if (!row.detail.isEmpty()) {
                height += 12.0;
            }
        }
    }

    NSView* panel = [[[NSView alloc] initWithFrame:NSMakeRect(0.0, 0.0, width, height)] autorelease];
    CGFloat y = height - inset - 30.0;

    [panel addSubview:framedLabel(content.title,
        NSMakeRect(inset, y + 10.0, 170.0, 18.0),
        [NSFont systemFontOfSize:14.0 weight:NSFontWeightBold],
        [NSColor labelColor])];
    if (!content.subtitle.isEmpty()) {
        [panel addSubview:framedLabel(content.subtitle,
            NSMakeRect(inset, y - 6.0, 180.0, 16.0),
            [NSFont systemFontOfSize:11.0],
            [NSColor secondaryLabelColor])];
    }
    if (!content.primaryValue.isEmpty()) {
        [panel addSubview:framedLabel(content.primaryValue,
            NSMakeRect(width - inset - 118.0, y + 2.0, 118.0, 34.0),
            [NSFont monospacedDigitSystemFontOfSize:30.0 weight:NSFontWeightBold],
            colorFromHex(content.accentColor, [NSColor labelColor]),
            NSTextAlignmentRight)];
        [panel addSubview:framedLabel(content.primaryLabel,
            NSMakeRect(width - inset - 118.0, y - 11.0, 118.0, 14.0),
            [NSFont systemFontOfSize:9.0 weight:NSFontWeightSemibold],
            [NSColor secondaryLabelColor],
            NSTextAlignmentRight)];
    }

    y -= 40.0;
    if (!content.badges.isEmpty()) {
        const CGFloat badgeWidth = (width - inset * 2.0) / static_cast<CGFloat>(content.badges.size());
        for (int index = 0; index < content.badges.size(); ++index) {
            const TaskbarDetailBadge& badge = content.badges.at(index);
            const CGFloat x = inset + badgeWidth * index;
            [panel addSubview:framedLabel(badge.label,
                NSMakeRect(x, y + 17.0, badgeWidth - 4.0, 13.0),
                [NSFont systemFontOfSize:9.0 weight:NSFontWeightSemibold],
                [NSColor secondaryLabelColor],
                NSTextAlignmentCenter)];
            [panel addSubview:framedLabel(badge.value,
                NSMakeRect(x, y - 1.0, badgeWidth - 4.0, 19.0),
                [NSFont monospacedDigitSystemFontOfSize:16.0 weight:NSFontWeightSemibold],
                [NSColor labelColor],
                NSTextAlignmentCenter)];
        }
        y -= 36.0;
    }

    for (const TaskbarDetailSection& section : content.sections) {
        if (!section.title.isEmpty()) {
            [panel addSubview:framedLabel(section.title,
                NSMakeRect(inset, y, width - inset * 2.0, 15.0),
                [NSFont systemFontOfSize:10.0 weight:NSFontWeightBold],
                [NSColor secondaryLabelColor])];
            y -= 22.0;
        }

        for (const TaskbarDetailRow& row : section.rows) {
            [panel addSubview:framedLabel(row.label,
                NSMakeRect(inset, y, 120.0, 16.0),
                [NSFont systemFontOfSize:11.0 weight:NSFontWeightSemibold],
                [NSColor secondaryLabelColor])];
            [panel addSubview:framedLabel(row.value,
                NSMakeRect(width - inset - 76.0, y, 76.0, 16.0),
                [NSFont monospacedDigitSystemFontOfSize:13.0 weight:NSFontWeightSemibold],
                [NSColor labelColor],
                NSTextAlignmentRight)];
            if (!row.detail.isEmpty()) {
                [panel addSubview:framedLabel(row.detail,
                    NSMakeRect(inset, y - 13.0, width - inset * 2.0, 13.0),
                    [NSFont systemFontOfSize:10.0],
                    [NSColor tertiaryLabelColor])];
                y -= 13.0;
            }
            if (row.progress >= 0.0) {
                NSProgressIndicator* progress = [[[NSProgressIndicator alloc] initWithFrame:NSMakeRect(inset, y - 9.0, width - inset * 2.0, 6.0)] autorelease];
                [progress setIndeterminate:NO];
                [progress setMinValue:0.0];
                [progress setMaxValue:100.0];
                [progress setDoubleValue:qBound(0.0, row.progress, 100.0)];
                [progress setControlSize:NSControlSizeSmall];
                [progress setStyle:NSProgressIndicatorStyleBar];
                [panel addSubview:progress];
                y -= 29.0;
            } else {
                y -= 22.0;
            }
        }
    }

    return panel;
}

NSView* detailMenuView(const QList<TaskbarDetailContent>& contents)
{
    constexpr CGFloat width = 318.0;
    CGFloat height = 18.0;
    QList<NSView*> views;
    for (const TaskbarDetailContent& content : contents) {
        if (!content.isEmpty()) {
            NSView* view = contentView(content);
            views.append(view);
            height += view.frame.size.height + 8.0;
        }
    }
    if (contents.isEmpty()) {
        height = 56.0;
    }

    NSView* container = [[[NSView alloc] initWithFrame:NSMakeRect(0.0, 0.0, width, height)] autorelease];

    if (contents.isEmpty()) {
        [container addSubview:framedLabel(QStringLiteral("Waiting for metrics"),
            NSMakeRect(14.0, 18.0, width - 28.0, 18.0),
            [NSFont systemFontOfSize:12.0],
            [NSColor secondaryLabelColor],
            NSTextAlignmentCenter)];
        return container;
    }

    CGFloat y = height - 10.0;
    for (NSView* view : views) {
        y -= view.frame.size.height;
        [view setFrameOrigin:NSMakePoint(0.0, y)];
        [container addSubview:view];
        y -= 8.0;
    }
    return container;
}

} // namespace

class MacTaskbarIndicator::NativeBridge
{
public:
    explicit NativeBridge(MacTaskbarIndicator* owner)
        : indicator(owner)
    {
    }

    MacTaskbarIndicator* indicator = nullptr;
    struct Entry
    {
        QString pluginId;
        NSStatusItem* statusItem = nil;
        NSMenu* menu = nil;
        NSMenuItem* detailItem = nil;
    };

    static void disposeEntry(const Entry& entry)
    {
        if (entry.detailItem) {
            [entry.detailItem setView:nil];
        }
        if (entry.statusItem) {
            [entry.statusItem setMenu:nil];
            [[NSStatusBar systemStatusBar] removeStatusItem:entry.statusItem];
            [entry.statusItem release];
        }
        if (entry.menu) {
            [entry.menu release];
        }
        if (entry.detailItem) {
            [entry.detailItem release];
        }
    }

    QList<Entry> entries;
    bool isRefreshing = false;
    bool refreshPending = false;
};

} // namespace Vitals

@implementation VitalsStatusItemTarget

- (void)showWindow:(id)sender
{
    Q_UNUSED(sender);
    if (self.indicator) {
        self.indicator->emitShowRequested();
    }
}

- (void)quitApp:(id)sender
{
    Q_UNUSED(sender);
    if (self.indicator) {
        self.indicator->emitQuitRequested();
    }
}

@end

namespace Vitals {

MacTaskbarIndicator::~MacTaskbarIndicator()
{
    if (!m_nativeBridge) {
        return;
    }

    for (const NativeBridge::Entry& entry : m_nativeBridge->entries) {
        NativeBridge::disposeEntry(entry);
    }

    delete m_nativeBridge;
    m_nativeBridge = nullptr;
}

void MacTaskbarIndicator::initialize(QWidget* mainWindow)
{
    setMainWindow(mainWindow);

    if (!m_nativeBridge) {
        m_nativeBridge = new NativeBridge(this);
    }

    if (!m_nativeBridge->entries.isEmpty()) {
        return;
    }
    refresh();
}

QString MacTaskbarIndicator::platformName() const
{
    return QStringLiteral("macOS menu bar");
}

QString MacTaskbarIndicator::idleText() const
{
    return QStringLiteral("Vitals");
}

QColor MacTaskbarIndicator::accentColor() const
{
    return QColor(QStringLiteral("#f5f5f7"));
}

bool MacTaskbarIndicator::prefersTextOnlyDisplay() const
{
    return true;
}

bool MacTaskbarIndicator::prefersSystemTintedText() const
{
    return true;
}

int MacTaskbarIndicator::maximumVisibleLabelLength() const
{
    return 18;
}

void MacTaskbarIndicator::refresh()
{
    if (!m_nativeBridge) {
        TaskbarIndicator::refresh();
        return;
    }

    if (m_nativeBridge->isRefreshing) {
        m_nativeBridge->refreshPending = true;
        return;
    }

    m_nativeBridge->isRefreshing = true;

    auto rebuildEntries = [this]() {
        for (const NativeBridge::Entry& entry : m_nativeBridge->entries) {
            NativeBridge::disposeEntry(entry);
        }
        m_nativeBridge->entries.clear();

        for (const TaskbarPluginDisplay& display : pluginDisplays()) {
            NativeBridge::Entry entry;
            entry.pluginId = display.pluginId;
            entry.statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];
            entry.menu = [[NSMenu alloc] initWithTitle:nsStringFromQString(display.pluginName)];
            entry.detailItem = [[NSMenuItem alloc] initWithTitle:nsStringFromQString(display.pluginName)
                                                           action:nil
                                                    keyEquivalent:@""];
            [entry.detailItem setEnabled:NO];
            [entry.menu addItem:entry.detailItem];
            [entry.statusItem setMenu:entry.menu];
            m_nativeBridge->entries.append(entry);
        }
    };

    do {
        m_nativeBridge->refreshPending = false;

        const QList<TaskbarPluginDisplay> displays = pluginDisplays();
        const QHash<QString, MetricValue> values = latestValues();

        bool needsRebuild = m_nativeBridge->entries.size() != displays.size();
        if (!needsRebuild) {
            for (int index = 0; index < m_nativeBridge->entries.size(); ++index) {
                if (m_nativeBridge->entries.at(index).pluginId != displays.at(index).pluginId) {
                    needsRebuild = true;
                    break;
                }
            }
        }
        if (needsRebuild) {
            rebuildEntries();
        }

        if (displays.isEmpty()) {
            continue;
        }

        const int entryCount = qMin(displays.size(), m_nativeBridge->entries.size());
        for (int index = 0; index < entryCount; ++index) {
            const TaskbarPluginDisplay& display = displays.at(index);
            NativeBridge::Entry& entry = m_nativeBridge->entries[index];
            if (!entry.statusItem) {
                continue;
            }

            const QString label = labelForDisplay(display, values).left(maximumVisibleLabelLength());
            const QString tooltip = tooltipForDisplay(display, values);

            NSStatusBarButton* button = [entry.statusItem button];
            if (!button) {
                continue;
            }

            const QString visibleTitle = label.isEmpty() ? idleText() : label;
            const QPixmap pixmap = buildPixmap(visibleTitle);
            [button setTitle:@""];
            [button setToolTip:nsStringFromQString(tooltip)];
            [button setImage:nsImageFromQPixmap(pixmap, prefersSystemTintedText())];
            [button setImagePosition:NSImageOnly];

            const TaskbarDetailContent content = detailContentForDisplay(display, values);
            [entry.detailItem setTitle:@""];
            [entry.detailItem setView:detailMenuView(content.isEmpty() ? QList<TaskbarDetailContent>{}
                                                                       : QList<TaskbarDetailContent>{content})];
            [entry.statusItem setVisible:YES];
        }
    } while (m_nativeBridge->refreshPending);

    m_nativeBridge->isRefreshing = false;
}

} // namespace Vitals
