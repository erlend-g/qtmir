/*
 * Copyright (C) 2015-2017 Canonical, Ltd.
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

#ifndef QMIRSERVER_P_H
#define QMIRSERVER_P_H

// Qt
#include <QThread>
#include <QSharedPointer>

// std
#include <condition_variable>
#include <mutex>

// local
#include "appnotifier.h"
#include "screensmodel.h"
#include "windowcontroller.h"
#include "windowmodelnotifier.h"
#include "screenscontroller.h"
#include "sessionauthorizer.h"
#include "mirdisplayconfigurationstorage.h"

//miral
#include <miral/application_authorizer.h>
#include <miral/runner.h>

// miroil
#include <miroil/compositor.h>
#include <miroil/mirserverhooks.h>
#include <miroil/promptsessionmanager.h>
#include <miroil/openglcontextfactory.h>

class MirServerThread;
class QOpenGLContext;

namespace qtmir
{
using SetSessionAuthorizer = miral::SetApplicationAuthorizer<SessionAuthorizer>;
class PromptSessionListener;
}

class QMirServerPrivate
{
public:
    QMirServerPrivate();
    const QSharedPointer<ScreensModel> screensModel{new ScreensModel()};
    QSharedPointer<ScreensController> screensController;
    MirServerThread *serverThread;

    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const;

    void run(const std::function<void()> &startCallback);
    void stop();

    qtmir::PromptSessionListener *promptSessionListener() const;
    std::shared_ptr<miroil::PromptSessionManager> promptSessionManager() const;

    std::shared_ptr<SessionAuthorizer> theApplicationAuthorizer() const
        { return m_sessionAuthorizer.the_custom_application_authorizer(); }

    qtmir::WindowModelNotifier *windowModelNotifier() const
        { return &m_windowModelNotifier; }

    qtmir::AppNotifier *appNotifier() const
        { return &m_appNotifier; }

    qtmir::WindowControllerInterface *windowController() const
        { return &m_windowController; }

private:
    qtmir::SetSessionAuthorizer  m_sessionAuthorizer;
    miroil::MirServerHooks       m_mirServerHooks;
    miroil::OpenGLContextFactory m_openGLContextFactory;    
    std::shared_ptr<qtmir::MirDisplayConfigurationStorage> m_DisplayConfigutaionStorage;

    miral::MirRunner runner;

    mutable qtmir::AppNotifier m_appNotifier;
    mutable qtmir::WindowModelNotifier m_windowModelNotifier;
    mutable qtmir::WindowController m_windowController;
};

class MirServerThread : public QThread
{
    Q_OBJECT

public:
    MirServerThread(QMirServerPrivate* server)
        : server(server)
    {}

    bool waitForMirStartup();

Q_SIGNALS:
    void stopped();

public Q_SLOTS:
    void run() override;

private:
    std::mutex mutex;
    std::condition_variable started_cv;
    bool mir_running{false};

    QMirServerPrivate* const server;
};

#endif // QMIRSERVER_P_H
