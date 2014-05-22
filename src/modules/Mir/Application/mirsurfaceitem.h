/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef MIRSURFACEITEM_H
#define MIRSURFACEITEM_H

#include <memory>

// Qt
#include <QMutex>
#include <QSet>
#include <QQuickItem>

// mir
#include <mir/scene/surface.h>
#include <mir/scene/surface_observer.h>
#include <mir_toolkit/common.h>

#include "ubuntukeyboardinfo.h"

class MirSurfaceManager;
class QSGMirSurfaceNode;
class QMirSurfaceTextureProvider;
class Application;

class MirSurfaceObserver : public mir::scene::SurfaceObserver {
public:
    MirSurfaceObserver();

    void setListener(QObject *listener);

    void attrib_changed(MirSurfaceAttrib, int) override {}
    void resized_to(mir::geometry::Size const&) override {}
    void moved_to(mir::geometry::Point const&) override {}
    void hidden_set_to(bool) override {}

    // Get new frame notifications from Mir, called from a Mir thread.
    void frame_posted() override;

    void alpha_set_to(float) override {}
    void transformation_set_to(glm::mat4 const&) override {}
private:
    QObject *m_listener;
};

class MirSurfaceItem : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(Type)
    Q_ENUMS(State)

    Q_PROPERTY(Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(Application* application READ application CONSTANT)

public:
    explicit MirSurfaceItem(std::shared_ptr<mir::scene::Surface> surface, Application* application,
                            QQuickItem *parent = 0);
    ~MirSurfaceItem();

    enum Type {
        Normal = mir_surface_type_normal,
        Utility = mir_surface_type_utility,
        Dialog = mir_surface_type_dialog,
        Overlay = mir_surface_type_overlay,
        Freestyle = mir_surface_type_freestyle,
        Popover = mir_surface_type_popover,
        InputMethod = mir_surface_type_inputmethod,
        };

    enum State {
        Unknown = mir_surface_state_unknown,
        Restored = mir_surface_state_restored,
        Minimized = mir_surface_state_minimized,
        Maximized = mir_surface_state_maximized,
        VertMaximized = mir_surface_state_vertmaximized,
        /* SemiMaximized = mir_surface_state_semimaximized, // see mircommon/mir_toolbox/common.h*/
        Fullscreen = mir_surface_state_fullscreen,
    };

    //getters
    Application* application() const;
    Type type() const;
    State state() const;
    QString name() const;

    // Item surface/texture management
    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

Q_SIGNALS:
    void typeChanged();
    void stateChanged();
    void nameChanged();
    void surfaceDestroyed();
    void surfaceFirstFrameDrawn(MirSurfaceItem *); // so MirSurfaceManager can notify QML

public Q_SLOTS:
    void release(); // For QML to destroy this surface

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(QTouchEvent *event) override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private Q_SLOTS:
    void surfaceDamaged();

private:
    bool updateTexture();
    void ensureProvider();

    void setType(const Type&);
    void setState(const State&);

    // called by MirSurfaceManager
    void setAttribute(const MirSurfaceAttrib, const int);
    void setSurfaceValid(const bool);

    bool hasTouchInsideUbuntuKeyboard(QTouchEvent *event);

    QMutex m_mutex;

    std::shared_ptr<mir::scene::Surface> m_surface;
    Application* m_application;
    int m_pendingClientBuffersCount;
    bool m_firstFrameDrawn;

    QMirSurfaceTextureProvider *m_textureProvider;

    static UbuntuKeyboardInfo *m_ubuntuKeyboardInfo;

    std::shared_ptr<MirSurfaceObserver> m_surfaceObserver;

    friend class MirSurfaceManager;
};

Q_DECLARE_METATYPE(MirSurfaceItem*)

#endif // MIRSURFACEITEM_H
