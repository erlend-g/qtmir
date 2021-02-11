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

#include "mirserverhooks.h"

#include "screenscontroller.h"
#include "logging.h"
#include "inputdeviceobserver.h"
#include "promptsessionlistener.h"


namespace mg = mir::graphics;
namespace ms = mir::scene;

namespace qtmir {
    
MirServerHooks::MirServerHooks()
    : miroil::MirServerHooks(&PromptSessionListener::create)
{
}

}

/*
QSharedPointer<ScreensController> qtmir::MirServerHooks::createScreensController(QSharedPointer<ScreensModel> const &screensModel) const
{
    return QSharedPointer<ScreensController>(
        new ScreensController(screensModel, theMirDisplay(), self->m_mirDisplayConfigurationController.lock()));
}
*/
