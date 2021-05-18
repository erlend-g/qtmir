/*
 * Copyright (C) 2015,2017 Canonical, Ltd.
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

#include "qmirserver_p.h"

// local
#include "logging.h"
#include "inputdeviceobserver.h"
#include "mirdisplayconfigurationpolicy.h"
#include "namedcursor.h"
#include "mirglconfig.h"
#include "miropenglcontext.h"
#include "windowmanagementpolicy.h"
#include "promptsessionlistener.h"
#include "screenscontroller.h"
#include "qtcompositor.h"

#include <miroil/prompt_session_manager.h>
#include <miroil/persist_display_config.h>
#include <miroil/set_compositor.h>
#include <miroil/display_listener_wrapper.h>

// miral
#include <miral/add_init_callback.h>
#include <miral/set_terminator.h>
#include <miral/version.h>
#include <miral/x11_support.h>
#if MIRAL_VERSION > MIR_VERSION_NUMBER(1,3,1)
#include <miral/set_window_management_policy.h>
#else
#include <miral/set_window_managment_policy.h>
#endif

// Qt
#include <QCoreApplication>
#include <QOpenGLContext>

#include <valgrind.h>

static int qtmirArgc{1};
static const char *qtmirArgv[]{"qtmir"};

void MirServerThread::run()
{
    auto start_callback = [this]
    {
        std::lock_guard<std::mutex> lock(mutex);
        mir_running = true;
        started_cv.notify_one();
    };

    server->run(start_callback);

    Q_EMIT stopped();
}

bool MirServerThread::waitForMirStartup()
{
    const int timeout = RUNNING_ON_VALGRIND ? 100 : 10; // else timeout triggers before Mir ready

    std::unique_lock<decltype(mutex)> lock(mutex);
    started_cv.wait_for(lock, std::chrono::seconds{timeout}, [&]{ return mir_running; });
    return mir_running;
}

QPlatformOpenGLContext *QMirServerPrivate::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    QSurfaceFormat            format     = context->format();
    mir::graphics::Display  * mirDisplay = m_mirServerHooks.the_mir_display().get();
    mir::graphics::GLConfig * gl_config  = m_openGLContext.the_open_gl_config().get();
    
    if (!gl_config)
        throw std::logic_error("No gl config available. Server not running?");
    
    return new MirOpenGLContext(*mirDisplay, *gl_config, format);
}

std::shared_ptr<miroil::PromptSessionManager> QMirServerPrivate::promptSessionManager() const
{
    return std::make_shared<miroil::PromptSessionManager>(m_mirServerHooks.the_prompt_session_manager());
}

QMirServerPrivate::QMirServerPrivate() :
    m_openGLContext(new MirGLConfig()),    
    runner(qtmirArgc, qtmirArgv)
{
    m_DisplayConfigutaionStorage = std::make_shared<qtmir::MirDisplayConfigurationStorage>();
}

qtmir::PromptSessionListener *QMirServerPrivate::promptSessionListener() const
{
    return dynamic_cast<qtmir::PromptSessionListener*>(m_mirServerHooks.the_prompt_session_listener());
}

void QMirServerPrivate::run(const std::function<void()> &startCallback)
{
    m_mirServerHooks.create_prompt_session_listener(std::dynamic_pointer_cast<miroil::PromptSessionListener>(std::make_shared<qtmir::PromptSessionListener>()));
    m_mirServerHooks.create_named_cursor([](std::string const & name)
        {
            // We are not responsible for loading cursors. This is left for shell to do as it's drawing its own QML cursor.
            // So here we work around Mir API by storing just the cursor name in the CursorImage.
            return std::make_shared<qtmir::NamedCursor>(name.c_str());
        }
    );
    

    miral::AddInitCallback addInitCallback{[&, this]
    {
        qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
        qCDebug(QTMIR_MIR_MESSAGES) << "Command line arguments passed to Qt:" << QCoreApplication::arguments();
    }};

    miral::SetTerminator setTerminator{[](int)
    {
        qDebug() << "Signal caught by Mir, stopping Mir server..";
        QCoreApplication::quit();
    }};

    runner.set_exception_handler([this]
    {
        try {
            throw;
        } catch (const std::exception &ex) {
            qCritical() << ex.what();
            exit(1);
        }
    });

    runner.add_start_callback([&]
    {
        screensModel->update();
        screensController = QSharedPointer<ScreensController>(new ScreensController(screensModel, m_mirServerHooks.the_mir_display(), m_mirServerHooks.the_display_configuration_controller()));
        std::shared_ptr<miroil::InputDeviceObserver> ptr = std::make_shared<qtmir::MirInputDeviceObserver>();
        m_mirServerHooks.create_input_device_observer(ptr);
    });

    runner.add_start_callback(startCallback);

    runner.add_stop_callback([&]
    {
        screensModel->terminate();
        screensController.clear();
    });

    runner.run_with(
        {
            m_sessionAuthorizer,
            m_openGLContext,
            m_mirServerHooks,
            miral::set_window_management_policy<WindowManagementPolicy>(m_windowModelNotifier, m_windowController,
                    m_appNotifier, screensModel),
            addInitCallback,
            miroil::SetCompositor(
                // Create the the QtCompositor 
                    [this]()
                    -> std::shared_ptr<miroil::Compositor>
                {
                    std::shared_ptr<miroil::Compositor> result = std::make_shared<QtCompositor>();
                    return result;
                }
                ,
                // Initialization called by mir when the new compositor is setup up              
                    [this](const std::shared_ptr<mir::graphics::Display>& display,
                           const std::shared_ptr<miroil::Compositor> & compositor,
                           const std::shared_ptr<mir::compositor::DisplayListener>& displayListener)
                {
                    
                    std::shared_ptr<QtCompositor> qtCompsitor = std::dynamic_pointer_cast<QtCompositor>(compositor);
                    
                    this->screensModel->init(display, qtCompsitor, std::make_shared<miroil::DisplayListenerWrapper>(displayListener));
                }
            ),
            setTerminator,
            miroil::PersistDisplayConfig(m_DisplayConfigutaionStorage, &qtmir::wrapDisplayConfigurationPolicy),
            miral::X11Support{},
        });
}

void QMirServerPrivate::stop()
{
    runner.stop();
}
