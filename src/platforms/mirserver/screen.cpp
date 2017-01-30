/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// local
#include "screen.h"
#include "logging.h"
#include "nativeinterface.h"

// Mir
#include "mir/geometry/size.h"
#include "mir/graphics/buffer.h"
#include "mir/graphics/display_buffer.h"
#include "mir/graphics/display.h"
#include <mir/graphics/display_configuration.h>
#include <mir/renderer/gl/render_target.h>

// Qt
#include <QGuiApplication>
#include <qpa/qwindowsysteminterface.h>
#include <QThread>
#include <QtMath>

// Qt sensors
#include <QtSensors/QOrientationReading>
#include <QtSensors/QOrientationSensor>

namespace mg = mir::geometry;

#define DEBUG_MSG_SCREENS qCDebug(QTMIR_SCREENS).nospace() << "Screen[" << (void*)this <<"]::" << __func__
#define DEBUG_MSG_SENSORS qCDebug(QTMIR_SENSOR_MESSAGES).nospace() << "Screen[" << (void*)this <<"]::" << __func__
#define WARNING_MSG_SENSORS qCWarning(QTMIR_SENSOR_MESSAGES).nospace() << "Screen[" << (void*)this <<"]::" << __func__

namespace {
bool isLittleEndian() {
    unsigned int i = 1;
    char *c = (char*)&i;
    return *c == 1;
}
static mir::renderer::gl::RenderTarget *as_render_target(
    mir::graphics::DisplayBuffer *displayBuffer)
{
    auto const render_target =
        dynamic_cast<mir::renderer::gl::RenderTarget*>(
            displayBuffer->native_display_buffer());
    if (!render_target)
        throw std::logic_error("DisplayBuffer does not support GL rendering");

    return render_target;
}

enum QImage::Format qImageFormatFromMirPixelFormat(MirPixelFormat mirPixelFormat) {
    switch (mirPixelFormat) {
    case mir_pixel_format_abgr_8888:
        if (isLittleEndian()) {
            // 0xRR,0xGG,0xBB,0xAA
            return QImage::Format_RGBA8888;
        } else {
            // 0xAA,0xBB,0xGG,0xRR
            qFatal("[mirserver QPA] "
                   "Qt doesn't support mir_pixel_format_abgr_8888 in a big endian architecture");
        }
        break;
    case mir_pixel_format_xbgr_8888:
        if (isLittleEndian()) {
            // 0xRR,0xGG,0xBB,0xXX
            return QImage::Format_RGBX8888;
        } else {
            // 0xXX,0xBB,0xGG,0xRR
            qFatal("[mirserver QPA] "
                   "Qt doesn't support mir_pixel_format_xbgr_8888 in a big endian architecture");
        }
        break;
        break;
    case mir_pixel_format_argb_8888:
        // 0xAARRGGBB
        return QImage::Format_ARGB32;
        break;
    case mir_pixel_format_xrgb_8888:
        // 0xffRRGGBB
        return QImage::Format_RGB32;
        break;
    case mir_pixel_format_bgr_888:
        qFatal("[mirserver QPA] Qt doesn't support mir_pixel_format_bgr_888");
        break;
    default:
        qFatal("[mirserver QPA] Unknown mir pixel format");
        break;
    }
    return QImage::Format_Invalid;
}

QString displayTypeToString(qtmir::OutputTypes type)
{
    typedef qtmir::OutputTypes Type;
    switch (type) {
    case Type::VGA:           return QStringLiteral("VGP");
    case Type::DVII:          return QStringLiteral("DVI-I");
    case Type::DVID:          return QStringLiteral("DVI-D");
    case Type::DVIA:          return QStringLiteral("DVI-A");
    case Type::Composite:     return QStringLiteral("Composite");
    case Type::SVideo:        return QStringLiteral("S-Video");
    case Type::LVDS:          return QStringLiteral("LVDS");
    case Type::Component:     return QStringLiteral("Component");
    case Type::NinePinDIN:    return QStringLiteral("9 Pin DIN");
    case Type::DisplayPort:   return QStringLiteral("DisplayPort");
    case Type::HDMIA:         return QStringLiteral("HDMI-A");
    case Type::HDMIB:         return QStringLiteral("HDMI-B");
    case Type::TV:            return QStringLiteral("TV");
    case Type::EDP:           return QStringLiteral("EDP");
    case Type::Unknown:
    default:
        return QStringLiteral("Unknown");
    } //switch
}
} // namespace {


class OrientationReadingEvent : public QEvent {
public:
    OrientationReadingEvent(QEvent::Type type, QOrientationReading::Orientation orientation)
        : QEvent(type)
        , m_orientation(orientation) {
    }

    static const QEvent::Type m_type;
    QOrientationReading::Orientation m_orientation;
};

const QEvent::Type OrientationReadingEvent::m_type =
        static_cast<QEvent::Type>(QEvent::registerEventType());

bool Screen::skipDBusRegistration = false;

Screen::Screen(const mir::graphics::DisplayConfigurationOutput &screen)
    : QObject(nullptr)
    , m_used(false)
    , m_refreshRate(-1.0)
    , m_scale(1.0)
    , m_formFactor(mir_form_factor_unknown)
    , m_isActive(false)
    , m_renderTarget(nullptr)
    , m_displayGroup(nullptr)
    , m_orientationSensor(new QOrientationSensor(this))
    , m_unityScreen(nullptr)
{
    setMirDisplayConfiguration(screen, false);
    DEBUG_MSG_SCREENS << "(output=" << m_outputId.as_value() << ", geometry=" << m_geometry << ")";

    // Set the default orientation based on the initial screen dimmensions.
    m_nativeOrientation = (m_geometry.width() >= m_geometry.height())
        ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen - nativeOrientation is:" << m_nativeOrientation;

    // If it's a landscape device (i.e. some tablets), start in landscape, otherwise portrait
    m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation)
            ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
    qCDebug(QTMIR_SENSOR_MESSAGES) << "Screen - initial currentOrientation is:" << m_currentOrientation;

    if (internalDisplay()) { // only enable orientation sensor for device-internal display
        QObject::connect(m_orientationSensor, &QOrientationSensor::readingChanged,
                         this, &Screen::onOrientationReadingChanged);
        m_orientationSensor->start();
    }

    if (!skipDBusRegistration) {
        // FIXME This is a unity8 specific dbus call and shouldn't be in qtmir
        m_unityScreen = new QDBusInterface(QStringLiteral("com.canonical.Unity.Screen"),
                                         QStringLiteral("/com/canonical/Unity/Screen"),
                                         QStringLiteral("com.canonical.Unity.Screen"),
                                         QDBusConnection::systemBus(), this);

        m_unityScreen->connection().connect(QStringLiteral("com.canonical.Unity.Screen"),
                                          QStringLiteral("/com/canonical/Unity/Screen"),
                                          QStringLiteral("com.canonical.Unity.Screen"),
                                          QStringLiteral("DisplayPowerStateChange"),
                                          this,
                                          SLOT(onDisplayPowerStateChanged(int, int)));
    }
}

Screen::~Screen()
{
    //if a ScreenWindow associated with this screen, kill it
    Q_FOREACH (ScreenWindow* window, m_screenWindows) {
        window->window()->destroy(); // ends up destroying window
    }
}

bool Screen::orientationSensorEnabled()
{
    return m_orientationSensor->isActive();
}

void Screen::setUsed(bool used)
{
    if (m_used == used)
        return;
    Q_EMIT __usedChanged(used);
}

void Screen::setScale(float scale)
{
    if (qFuzzyCompare(scale, m_scale))
        return;
    Q_EMIT __scaleChanged(scale);
}

void Screen::setFormFactor(MirFormFactor formFactor)
{
    if (formFactor == m_formFactor)
        return;
    Q_EMIT __formFactorChanged(formFactor);
}

void Screen::setSize(const QSize &size)
{
    QList<QSize> sizes = availableSizes();
    for (int i = 0; i < sizes.size(); i++) {
        if (sizes[i] == size) {
            setCurrentModeIndex(i);
        }
    }
}

void Screen::setCurrentModeIndex(uint32_t currentModeIndex)
{
    if (m_currentModeIndex == currentModeIndex)
        return;
    Q_EMIT __currentModeIndexChanged(currentModeIndex);
}

void Screen::setActive(bool active)
{
    if (m_isActive == active)
        return;

    if (m_isActive) {
        QList<QScreen *> screens = QGuiApplication::screens();
        Q_FOREACH(auto screen, screens) {
            const auto platformScreen = static_cast<Screen *>(screen->handle());
            if (platformScreen->isActive()) {
                platformScreen->setActive(false);
            }
        }
    }
    m_isActive = active;
    Q_EMIT activeChanged(active);
}

void Screen::onDisplayPowerStateChanged(int status, int reason)
{
    Q_UNUSED(reason);
    if (internalDisplay()) {
        toggleSensors(status);
    }
}

void Screen::setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &screen,
                                        bool notify)
{
    // Note: DisplayConfigurationOutput will be destroyed after this function returns

    if (m_used != screen.used) {
        m_used = screen.used;
        Q_EMIT usedChanged();
    }

    // Output data - each output has a unique id and corresponding type. Can be multiple cards.
    m_outputId = screen.id;
    m_type = static_cast<qtmir::OutputTypes>(screen.type); //FIXME: need compile time check these are equivalent

    // Physical screen size
    m_physicalSize.setWidth(screen.physical_size_mm.width.as_int());
    m_physicalSize.setHeight(screen.physical_size_mm.height.as_int());

    // Screen capabilities
    if (m_currentModeIndex != screen.current_mode_index) {
        m_currentModeIndex = screen.current_mode_index;
        Q_EMIT currentModeIndexChanged();
    }

    // Current Pixel Format & depth
    m_format = qImageFormatFromMirPixelFormat(screen.current_format);
    m_depth = 8 * MIR_BYTES_PER_PIXEL(screen.current_format);

    // Power mode
    m_powerMode = screen.power_mode;

    QRect oldGeometry = m_geometry;
    // Position of screen in virtual desktop coordinate space
    m_geometry.setTop(screen.top_left.y.as_int());
    m_geometry.setLeft(screen.top_left.x.as_int());

    // Mode = current resolution & refresh rate
    mir::graphics::DisplayConfigurationMode mode = screen.modes.at(m_currentModeIndex);
    m_geometry.setWidth(mode.size.width.as_int());
    m_geometry.setHeight(mode.size.height.as_int());

    m_availableSizes.clear();
    Q_FOREACH(auto mode, screen.modes) {
        m_availableSizes.append(QSize(mode.size.width.as_int(), mode.size.height.as_int()));
    }

    // DPI - unnecessary to calculate, default implementation in QPlatformScreen is sufficient

    // Check for Screen geometry change
    if (m_geometry != oldGeometry) {
        if (notify) {
            QWindowSystemInterface::handleScreenGeometryChange(this->screen(), m_geometry, m_geometry);
        }

        Q_FOREACH (ScreenWindow* window, m_screenWindows) {
            window->setGeometry(m_geometry);
        }
        if (oldGeometry.size() != m_geometry.size()) {
            Q_EMIT sizeChanged();
        }
    }

    // Refresh rate
    if (m_refreshRate != mode.vrefresh_hz) {
        m_refreshRate = mode.vrefresh_hz;
        if (notify) {
            QWindowSystemInterface::handleScreenRefreshRateChange(this->screen(), mode.vrefresh_hz);
        }
    }

    // Scale, DPR & Form Factor
    // Update the scale & form factor native-interface properties for the windows affected
    // as there is no convenient way to emit signals for those custom properties on a QScreen
    m_devicePixelRatio = 1.0; //qCeil(m_scale); // FIXME: I need to announce this changing, probably by delete/recreate Screen

    if (screen.form_factor != m_formFactor) {
        m_formFactor = screen.form_factor;
        Q_EMIT formFactorChanged();
    }

    if (!qFuzzyCompare(screen.scale, m_scale)) {
        m_scale = screen.scale;
        Q_EMIT scaleChanged();
    }
}

void Screen::toggleSensors(const bool enable) const
{
    DEBUG_MSG_SENSORS << "(enable=" << enable << ")";
    if (enable) {
        m_orientationSensor->start();
    } else {
        m_orientationSensor->stop();
    }
}

void Screen::customEvent(QEvent* event)
{
    OrientationReadingEvent* oReadingEvent = static_cast<OrientationReadingEvent*>(event);
    switch (oReadingEvent->m_orientation) {
        case QOrientationReading::LeftUp: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::InvertedPortraitOrientation : Qt::LandscapeOrientation;
            break;
        }
        case QOrientationReading::TopUp: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::LandscapeOrientation : Qt::PortraitOrientation;
            break;
        }
        case QOrientationReading::RightUp: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::PortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        }
        case QOrientationReading::TopDown: {
            m_currentOrientation = (m_nativeOrientation == Qt::LandscapeOrientation) ?
                        Qt::InvertedLandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        }
        default: {
            WARNING_MSG_SENSORS << "() - unknown orientation.";
            event->accept();
            return;
        }
    }

    // Raise the event signal so that client apps know the orientation changed
    QWindowSystemInterface::handleScreenOrientationChange(screen(), m_currentOrientation);
    event->accept();
    DEBUG_MSG_SENSORS << "() - new orientation=" << m_currentOrientation;
}

void Screen::onOrientationReadingChanged()
{
    DEBUG_MSG_SENSORS << "()";

    // Make sure to switch to the main Qt thread context
    QCoreApplication::postEvent(this, new OrientationReadingEvent(
                                              OrientationReadingEvent::m_type,
                                    m_orientationSensor->reading()->orientation()));
}

void Screen::activate()
{
    setActive(true);
}

QPlatformCursor *Screen::cursor() const
{
    if (!m_cursor) {
        const_cast<Screen*>(this)->m_cursor.reset(new qtmir::Cursor);
    }
    return m_cursor.data();
}

QString Screen::name() const
{
    return displayTypeToString(m_type);
}

QWindow *Screen::topLevelAt(const QPoint &point) const
{
    QVector<ScreenWindow*>::const_iterator screen = m_screenWindows.constBegin();
    QVector<ScreenWindow*>::const_iterator end = m_screenWindows.constEnd();

    while (screen != end) {
        QWindow* window = (*screen)->window();
        if (window) {
            if (window->geometry().contains(point)) return window;
        }
        screen++;
    }
    return nullptr;
}

ScreenWindow *Screen::primaryWindow() const
{
    return m_screenWindows.value(0, nullptr);
}

void Screen::addWindow(ScreenWindow *window)
{
    if (!window || m_screenWindows.contains(window)) return;
    DEBUG_MSG_SCREENS << "(screenWindow=" << window << ")";
    m_screenWindows.push_back(window);

    auto nativeInterface = qGuiApp->platformNativeInterface();
    Q_EMIT nativeInterface->windowPropertyChanged(window, QStringLiteral("formFactor"));
    Q_EMIT nativeInterface->windowPropertyChanged(window, QStringLiteral("scale"));

    window->setGeometry(geometry());

    if (m_screenWindows.count() > 1) {
        DEBUG_MSG_SCREENS << "() - secondary window added to screen.";
    } else {
        primaryWindowChanged(m_screenWindows.at(0));
    }
}

void Screen::removeWindow(ScreenWindow *window)
{
    int index = m_screenWindows.indexOf(window);
    if (index >= 0) {
        DEBUG_MSG_SCREENS << "(screenWindow=" << window << ")";
        m_screenWindows.remove(index);
        if (index == 0) {
            Q_EMIT primaryWindowChanged(m_screenWindows.value(0, nullptr));
        }
    }
}

void Screen::setMirDisplayBuffer(mir::graphics::DisplayBuffer *buffer, mir::graphics::DisplaySyncGroup *group)
{
    DEBUG_MSG_SCREENS << "(renderTarget=" << as_render_target(buffer) << ", displayGroup=" << group << ")";
    // This operation should only be performed while rendering is stopped
    m_renderTarget = as_render_target(buffer);
    m_displayGroup = group;
}

void Screen::swapBuffers()
{
    m_renderTarget->swap_buffers();

    /* FIXME this exposes a QtMir architecture problem, as Screen is supposed to wrap a mg::DisplayBuffer.
     * We use Qt's multithreaded renderer, where each Screen is rendered to relatively independently, and
     * post() called also individually.
     *
     * But if this is a native server on Android, in the multimonitor case a DisplaySyncGroup can contain
     * 2+ DisplayBuffers, one post() call will submit all mg::DisplayBuffers in the group for flipping.
     * This will cause just one Screen to be updated, blocking the swap call for the other Screens, which
     * will slow rendering dramatically.
     *
     * Integrating the Qt Scenegraph renderer as a Mir renderer should solve this issue.
     */
    m_displayGroup->post();
}

void Screen::makeCurrent()
{
    m_renderTarget->make_current();
}

void Screen::doneCurrent()
{
    m_renderTarget->release_current();
}

bool Screen::internalDisplay() const
{
    using namespace mir::graphics;
    if (m_type == qtmir::OutputTypes::LVDS || m_type == qtmir::OutputTypes::EDP) {
        return true;
    }
    return false;
}
