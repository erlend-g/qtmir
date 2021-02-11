/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef QTMIR_MIRSERVERHOOKS_H
#define QTMIR_MIRSERVERHOOKS_H
#include <miroil/mirserverhooksbase.h>
#include <memory>
#include <QSharedPointer>

class ScreensController;
class ScreensModel;

namespace qtmir
{
class MirServerHooks : public miroil::MirServerHooksBase
{
public:
    MirServerHooks();

    QSharedPointer<ScreensController> createScreensController(QSharedPointer<ScreensModel> const &screensModel) const;
};
}

#endif //QTMIR_MIRSERVERHOOKS_H
