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

#include <Qt>
#include <QTimer>

#include "inputdeviceobserver.h"
#include "mirsingleton.h"
#include "logging.h"

using namespace qtmir;
namespace mi = mir::input;

// Note: MirInputDeviceObserver has affinity to a Mir thread, but it is expected setKeymap will be called from the Qt GUI thread

MirInputDeviceObserver::MirInputDeviceObserver(QObject *parent):
    QObject(parent)
{
    // NB: have to use a Direct connection here, as it's called from Qt GUI thread
    connect(Mir::instance(), &Mir::currentKeymapChanged, this, &MirInputDeviceObserver::setKeymap, Qt::DirectConnection);
}

void MirInputDeviceObserver::setKeymap(const QString &keymap)
{
    QMutexLocker locker(&m_mutex); // lock so that Qt and Mir don't apply the keymap at the same time

    if (keymap != m_keymap) {
        qCDebug(QTMIR_MIR_KEYMAP) << "SET KEYMAP" << keymap;
        m_keymap = keymap;
        applyKeymap();
    }
}

void MirInputDeviceObserver::applyKeymap()
{
    Q_FOREACH(auto &device, m_devices) {
        applyKeymap(device);
    }
}

void MirInputDeviceObserver::applyKeymap(std::shared_ptr<miroil::InputDevice> device)
{
    if (!m_keymap.isEmpty()) {
        const QStringList stringList = m_keymap.split('+', QString::SkipEmptyParts);

        const QString &layout = stringList.at(0);
        QString variant;

        if (stringList.count() > 1) {
            variant = stringList.at(1);
        }

        qCDebug(QTMIR_MIR_KEYMAP) << "Applying keymap" <<  layout << variant << "on" << device->get_device_id() << QString::fromStdString(device->get_device_name());

        try
        {
            device->apply_keymap(layout.toStdString(), variant.toStdString());
            qCDebug(QTMIR_MIR_KEYMAP) << "Keymap applied";
        }
        catch(std::exception const& e)
        {
            qCWarning(QTMIR_MIR_KEYMAP) << "Keymap could not be applied:" << e.what();
        }
    }
}

void MirInputDeviceObserver::device_added(miroil::InputDevice device)
{
    QMutexLocker locker(&m_mutex);  // lock so that Qt and Mir don't apply the keymap at the same time
    
    if (device.is_keyboard() && device.is_alpha_numeric()) {
        qCDebug(QTMIR_MIR_KEYMAP) << "Device added" << device.get_device_id();
        m_devices.append(std::make_shared<miroil::InputDevice>(device));
        applyKeymap(std::make_shared<miroil::InputDevice>(device));
    }
}

void MirInputDeviceObserver::device_removed(miroil::InputDevice device)
{
    QMutexLocker locker(&m_mutex);  // lock so that Qt and Mir don't apply the keymap at the same time
    
    size_t n = m_devices.size();
    for (int i = 0; i < n;) {
        if (*m_devices[i].get() == device) {
            qCDebug(QTMIR_MIR_KEYMAP) << "Device removed" << device.get_device_id();
            m_devices.remove(i);
        }
        else {
            i++;            
        }
    }    
}
