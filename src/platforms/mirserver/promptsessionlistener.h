/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#ifndef QTMIR_PROMPTSESSIONLISTENER_H
#define QTMIR_PROMPTSESSIONLISTENER_H
#include <memory>

// Qt
#include <QHash>
#include <QObject>

#include "promptsession.h"
#include <miroil/prompt_session_listener.h>
#include <miral/application.h>

namespace qtmir {

class PromptSessionListener : public QObject, public miroil::PromptSessionListener
{
Q_OBJECT
public:
    explicit PromptSessionListener(QObject *parent = 0);
    ~PromptSessionListener();

    // PromptSessionListener functions    
    void starting(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void stopping(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void suspending(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void resuming(std::shared_ptr<mir::scene::PromptSession> const& prompt_session) override;
    void prompt_provider_added(mir::scene::PromptSession const& prompt_session,
                               std::shared_ptr<mir::scene::Session> const& prompt_provider) override;
    void prompt_provider_removed(mir::scene::PromptSession const& prompt_session,
                                 std::shared_ptr<mir::scene::Session> const& prompt_provider) override;

Q_SIGNALS:
    void promptSessionStarting(qtmir::PromptSession const &session);
    void promptSessionStopping(qtmir::PromptSession const &session);
    void promptSessionSuspending(qtmir::PromptSession const &session);
    void promptSessionResuming(qtmir::PromptSession const &session);

    void promptProviderAdded(qtmir::PromptSession const&, miral::Application const&);
    void promptProviderRemoved(qtmir::PromptSession const&, miral::Application const&);
    
private:
    QHash<const mir::scene::PromptSession *, qtmir::PromptSession> m_mirPromptToSessionHash;
};

}

Q_DECLARE_METATYPE(qtmir::PromptSession)
Q_DECLARE_METATYPE(miral::Application)

#endif // QTMIR_PROMPTSESSIONLISTENER_H
