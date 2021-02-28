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

#ifndef QTMIR_MIR_INPUT_DEVICE_OBSERVER_H
#define QTMIR_MIR_INPUT_DEVICE_OBSERVER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMutex>

#include "miroil/input_device_observer.h"

namespace mir { namespace input { class Device; }}

namespace qtmir {

class MirInputDeviceObserver: public QObject, public miroil::InputDeviceObserver
{
    Q_OBJECT
public:
    MirInputDeviceObserver(QObject * parent = nullptr);
    ~MirInputDeviceObserver() = default;

private Q_SLOTS:
    void setKeymap(const QString &keymap);

protected:
    void applyKeymap();
    void applyKeymap(const std::shared_ptr<miroil::InputDevice> &device);
    void device_added(const std::shared_ptr<miroil::InputDevice> &device);
    void device_removed(const std::shared_ptr<miroil::InputDevice> &device);

    QString m_keymap;
    QVector<std::shared_ptr<miroil::InputDevice>> m_devices;
    QMutex m_mutex;
};

} // namespace qtmir

#endif
