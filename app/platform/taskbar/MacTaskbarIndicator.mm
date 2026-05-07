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

} // namespace

class MacTaskbarIndicator::NativeBridge
{
public:
    explicit NativeBridge(MacTaskbarIndicator* owner)
        : indicator(owner)
    {
    }

    MacTaskbarIndicator* indicator = nullptr;
    NSStatusItem* statusItem = nil;
    NSMenu* menu = nil;
    NSMenuItem* summaryItem = nil;
    NSMenuItem* showItem = nil;
    NSMenuItem* quitItem = nil;
    id target = nil;
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

    if (m_nativeBridge->statusItem) {
        [[NSStatusBar systemStatusBar] removeStatusItem:m_nativeBridge->statusItem];
    }

    if (m_nativeBridge->target) {
        [m_nativeBridge->target release];
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

    if (m_nativeBridge->statusItem) {
        return;
    }

    m_nativeBridge->statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    m_nativeBridge->menu = [[NSMenu alloc] initWithTitle:@"Vitals"];

    m_nativeBridge->summaryItem = [[NSMenuItem alloc] initWithTitle:@"Vitals"
                                                             action:nil
                                                      keyEquivalent:@""];
    [m_nativeBridge->summaryItem setEnabled:NO];
    [m_nativeBridge->menu addItem:m_nativeBridge->summaryItem];
    [m_nativeBridge->menu addItem:[NSMenuItem separatorItem]];

    VitalsStatusItemTarget* target = [[VitalsStatusItemTarget alloc] init];
    target.indicator = this;
    m_nativeBridge->target = target;

    m_nativeBridge->showItem = [[NSMenuItem alloc] initWithTitle:@"Show Vitals"
                                                          action:@selector(showWindow:)
                                                   keyEquivalent:@""];
    [m_nativeBridge->showItem setTarget:target];
    [m_nativeBridge->menu addItem:m_nativeBridge->showItem];

    m_nativeBridge->quitItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                          action:@selector(quitApp:)
                                                   keyEquivalent:@""];
    [m_nativeBridge->quitItem setTarget:target];
    [m_nativeBridge->menu addItem:m_nativeBridge->quitItem];

    [m_nativeBridge->statusItem setMenu:m_nativeBridge->menu];
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
    if (!m_nativeBridge || !m_nativeBridge->statusItem) {
        TaskbarIndicator::refresh();
        return;
    }

    if (!hasPluginDisplays()) {
        [m_nativeBridge->statusItem setVisible:NO];
        [m_nativeBridge->summaryItem setTitle:@"Vitals: taskbar display disabled"];
        return;
    }

    const QString label = currentLabel().left(maximumVisibleLabelLength());
    const QString tooltip = currentTooltip();
    const QString summary = tooltip.split(QStringLiteral("\n")).join(QStringLiteral("  |  "));

    NSStatusBarButton* button = [m_nativeBridge->statusItem button];
    if (button) {
        const QString visibleTitle = label.isEmpty() ? idleText() : label;
        const QPixmap pixmap = buildPixmap(visibleTitle);
        [button setTitle:@""];
        [button setToolTip:nsStringFromQString(tooltip)];
        [button setImage:nsImageFromQPixmap(pixmap, prefersSystemTintedText())];
        [button setImagePosition:NSImageOnly];
    }

    [m_nativeBridge->summaryItem setTitle:nsStringFromQString(summary)];
    [m_nativeBridge->statusItem setVisible:YES];
}

} // namespace Vitals
