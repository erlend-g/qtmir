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

#ifndef QTMIR_EVENT_REGISTRY_H
#define QTMIR_EVENT_REGISTRY_H
#include <miroil/event_builder.h>

#include <QtGlobal>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTouchEvent>
#include <QVector>

namespace qtmir {

/*
    Creates Mir input events out of Qt input events

    One important feature is that it's able to match a QInputEvent with the MirInputEvent that originated it, so
    it can make a MirInputEvent version of a QInputEvent containing also information that the latter does not carry,
    such as relative axis movement for pointer devices.
*/
class EventBuilder : public miroil::EventBuilder {
public:
    static EventBuilder *instance();
    EventBuilder();
    virtual ~EventBuilder();

    /*
        Builds a MirEvent version of the given QInputEvent using also extra data from the
        MirPointerEvent that caused it.

        See also: store()
     */
    mir::EventUPtr reconstructMirEvent(QMouseEvent *event);
    mir::EventUPtr reconstructMirEvent(QHoverEvent *qtEvent);

    /*
        Makes a MirEvent version of the given QInputEvent using also extra data from the
        MirInputEvent that caused it.

        See also: store()
     */
    mir::EventUPtr makeMirEvent(QWheelEvent *qtEvent);
    mir::EventUPtr makeMirEvent(QKeyEvent *qtEvent);
    mir::EventUPtr makeMirEvent(Qt::KeyboardModifiers qmods,
                                const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                                Qt::TouchPointStates /* qtTouchPointStates */,
                                ulong qtTimestamp);

private:
    mir::EventUPtr makeMirEvent(QInputEvent *qtEvent, int x, int y, MirPointerButtons buttons);

    static EventBuilder *m_instance;
};

} // namespace qtmir

#endif // QTMIR_EVENT_REGISTRY_H
