/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
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

#include "eventbuilder.h"

#include "logging.h"

// common
#include <timestamp.h>

#include <QDebug>

namespace {

MirPointerAction mirPointerActionFromMouseEventType(QEvent::Type eventType)
{
    switch (eventType) {
    case QEvent::MouseButtonPress:
        return mir_pointer_action_button_down;
    case QEvent::MouseButtonRelease:
        return mir_pointer_action_button_up;
    case QEvent::HoverEnter:
        return mir_pointer_action_enter;
    case QEvent::HoverLeave:
        return mir_pointer_action_leave;
    case QEvent::MouseMove:
    case QEvent::HoverMove:
    default:
        return mir_pointer_action_motion;
    };
}

// Would be better if QMouseEvent had nativeModifiers
MirInputEventModifiers
getMirModifiersFromQt(Qt::KeyboardModifiers mods)
{
    MirInputEventModifiers m_mods = mir_input_event_modifier_none;
    if (mods & Qt::ShiftModifier)
        m_mods |= mir_input_event_modifier_shift;
    if (mods & Qt::ControlModifier)
        m_mods |= mir_input_event_modifier_ctrl;
    if (mods & Qt::AltModifier)
        m_mods |= mir_input_event_modifier_alt;
    if (mods & Qt::MetaModifier)
        m_mods |= mir_input_event_modifier_meta;

    return m_mods;
}

MirPointerButtons
getMirButtonsFromQt(Qt::MouseButtons buttons)
{
    MirPointerButtons result = 0;
    if (buttons & Qt::LeftButton)
        result |= mir_pointer_button_primary;
    if (buttons & Qt::RightButton)
        result |= mir_pointer_button_secondary;
    if (buttons & Qt::MiddleButton)
        result |= mir_pointer_button_tertiary;
    if (buttons & Qt::BackButton)
        result |= mir_pointer_button_back;
    if (buttons & Qt::ForwardButton)
        result |= mir_pointer_button_forward;

    return result;
}

} // anonymous namespace

using namespace qtmir;

EventBuilder *EventBuilder::m_instance = nullptr;

EventBuilder *EventBuilder::instance()
{
    if (!m_instance) {
        m_instance = new EventBuilder;
    }
    return m_instance;
}

EventBuilder::EventBuilder()
{
}

EventBuilder::~EventBuilder()
{
    m_instance = nullptr;
}

mir::EventUPtr EventBuilder::reconstructMirEvent(QMouseEvent *qtEvent)
{
    auto buttons = getMirButtonsFromQt(qtEvent->buttons());
    return makeMirEvent(qtEvent, qtEvent->x(), qtEvent->y(), buttons);
}

mir::EventUPtr EventBuilder::reconstructMirEvent(QHoverEvent *qtEvent)
{
    return makeMirEvent(qtEvent, qtEvent->pos().x(), qtEvent->pos().y(), 0 /*buttons*/);
}

mir::EventUPtr EventBuilder::makeMirEvent(QInputEvent *qtEvent, int x, int y, MirPointerButtons buttons)
{
    MirPointerAction action = mirPointerActionFromMouseEventType(qtEvent->type());

    auto timestamp = uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtEvent->timestamp()));
    auto modifiers = getMirModifiersFromQt(qtEvent->modifiers());

    float relativeX = 0.;
    float relativeY = 0.;
    MirInputDeviceId deviceId = 0;
    std::vector<uint8_t> cookie{};

    // Timestamp will be zero in case of synthetic events. Particularly synthetic QHoverEvents caused
    // by item movement under a stationary mouse pointer.
    if (qtEvent->timestamp() != 0) {
        auto eventInfo = find_info(qtEvent->timestamp());
        if (eventInfo) {
            relativeX = eventInfo->relative_x;
            relativeY = eventInfo->relative_y;
            deviceId = eventInfo->device_id;
            cookie = eventInfo->cookie;
        } else {
            qCWarning(QTMIR_MIR_INPUT) << "EventBuilder::makeMirEvent didn't find EventInfo with timestamp" << qtEvent->timestamp();
        }
    }

    return make_pointer_event(deviceId, timestamp, cookie, modifiers, action,
                      buttons, x, y, 0 /*hscroll*/, 0 /*vscroll*/, relativeX, relativeY);
}

mir::EventUPtr EventBuilder::makeMirEvent(QWheelEvent *qtEvent)
{
    auto timestamp = uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtEvent->timestamp()));
    auto modifiers = getMirModifiersFromQt(qtEvent->modifiers());
    auto buttons = getMirButtonsFromQt(qtEvent->buttons());

    MirInputDeviceId deviceId = 0;
    std::vector<uint8_t> cookie{};

    QPointF mirScroll(qtEvent->angleDelta());
    // QWheelEvent::DefaultDeltasPerStep = 120 but not defined on vivid
    mirScroll /= 120.0f;

    if (qtEvent->timestamp() != 0) {
        auto eventInfo = find_info(qtEvent->timestamp());
        if (eventInfo) {
            deviceId = eventInfo->device_id;
            cookie = eventInfo->cookie;
        } else {
            qCWarning(QTMIR_MIR_INPUT) << "EventBuilder::makeMirEvent didn't find EventInfo with timestamp" << qtEvent->timestamp();
        }
    }

    return make_pointer_event(deviceId, timestamp, cookie, modifiers, mir_pointer_action_motion,
                      buttons, qtEvent->x(), qtEvent->y(),
                      mirScroll.x(), mirScroll.y(),
                      0, 0);
}

mir::EventUPtr EventBuilder::makeMirEvent(QKeyEvent *qtEvent)
{
    MirKeyboardAction action = mir_keyboard_action_down;
    switch (qtEvent->type())
    {
    case QEvent::KeyPress:
        action = mir_keyboard_action_down;
        break;
    case QEvent::KeyRelease:
        action = mir_keyboard_action_up;
        break;
    default:
        break;
    }
    if (qtEvent->isAutoRepeat())
        action = mir_keyboard_action_repeat;
    MirInputDeviceId deviceId = 0;
    std::vector<uint8_t> cookie{};

    if (qtEvent->timestamp() != 0) {
        auto eventInfo = find_info(qtEvent->timestamp());
        if (eventInfo) {
            deviceId = eventInfo->device_id;
            cookie = eventInfo->cookie;
        } else {
            qCWarning(QTMIR_MIR_INPUT) << "EventBuilder::makeMirEvent didn't find EventInfo with timestamp" << qtEvent->timestamp();
        }
    }

    return make_key_event(deviceId, uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtEvent->timestamp())),
                          cookie, action, qtEvent->nativeVirtualKey(),
                          qtEvent->nativeScanCode(),
                          qtEvent->nativeModifiers());
}

mir::EventUPtr EventBuilder::makeMirEvent(Qt::KeyboardModifiers qmods,
                            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                            Qt::TouchPointStates /* qtTouchPointStates */,
                            ulong qtTimestamp)
{
    MirInputDeviceId deviceId = 0;
    std::vector<uint8_t> cookie{};

    if (qtTimestamp != 0) {
        auto eventInfo = find_info(qtTimestamp);
        if (eventInfo) {
            deviceId = eventInfo->device_id;
            cookie = eventInfo->cookie;
        } else {
            qCWarning(QTMIR_MIR_INPUT) << "EventBuilder::makeMirEvent didn't find EventInfo with timestamp" << qtTimestamp;
        }
    }

    auto modifiers = getMirModifiersFromQt(qmods);
    auto ev = make_touch_event(deviceId, uncompressTimestamp<qtmir::Timestamp>(qtmir::Timestamp(qtTimestamp)),
                               cookie, modifiers);

    for (int i = 0; i < qtTouchPoints.count(); ++i) {
        auto touchPoint = qtTouchPoints.at(i);
        auto id = touchPoint.id();

        MirTouchAction action = mir_touch_action_change;
        if (touchPoint.state() == Qt::TouchPointReleased)
        {
            action = mir_touch_action_up;
        }
        if (touchPoint.state() == Qt::TouchPointPressed)
        {
            action = mir_touch_action_down;
        }

        MirTouchTooltype tooltype = mir_touch_tooltype_finger;
        if (touchPoint.flags() & QTouchEvent::TouchPoint::Pen)
            tooltype = mir_touch_tooltype_stylus;

        add_touch(*ev, id, action, tooltype,
                  touchPoint.pos().x(), touchPoint.pos().y(),
                  touchPoint.pressure(),
                  touchPoint.rect().width(),
                  touchPoint.rect().height(),
                  0 /* size */);
    }

    return ev;
}
