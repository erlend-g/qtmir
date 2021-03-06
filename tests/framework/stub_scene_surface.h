/*
 * Copyright © 2014,2016 Canonical Ltd.
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
 * Original file authored by: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#ifndef MIR_SCENE_FAKESURFACE_H_
#define MIR_SCENE_FAKESURFACE_H_

#include <mir/scene/surface.h>

#include <memory>
#include <gmock/gmock.h>

namespace mir {
namespace scene {

class FakeSurface : public Surface
{
public:
    int fd;
    mir::input::InputReceptionMode input_mode{mir::input::InputReceptionMode::normal};

    FakeSurface(int fd=123)
        : channel(std::make_shared<mir::test::doubles::StubInputChannel>(fd)), fd(fd)
    {
    }

    std::shared_ptr<mir::input::InputChannel> input_channel() const override
    {
        return channel;
    }

    mir::input::InputReceptionMode reception_mode() const override
    {
        return input_mode;
    }

    std::string name() const override { return {}; }
    geometry::Point top_left() const override { return {}; }
    geometry::Size client_size() const override { return {};}
    geometry::Size size() const override { return {}; }
    geometry::Rectangle input_bounds() const override { return {{},{}}; }
    bool input_area_contains(mir::geometry::Point const&) const override { return false; }

    void set_streams(std::list<scene::StreamInfo> const&) override {}
    graphics::RenderableList generate_renderables(compositor::CompositorID) const override { return {}; }
    int buffers_ready_for_compositor(void const*) const override { return 0; }

    float alpha() const override { return 0.0f;}
    MirWindowType type() const override { return mir_window_type_normal; }
    MirWindowState state() const override { return mir_window_state_unknown; }

    void hide() override {}
    void show() override {}
    bool visible() const override { return true; }
    void move_to(geometry::Point const&) override {}
    void set_input_region(std::vector<geometry::Rectangle> const&) override {}
    void resize(geometry::Size const&) override {}
    void set_transformation(glm::mat4 const&) override {}
    void set_alpha(float) override {}
    void set_orientation(MirOrientation) {}

    void add_observer(std::shared_ptr<scene::SurfaceObserver> const&) override {}
    void remove_observer(std::weak_ptr<scene::SurfaceObserver> const&) override {}

    void set_reception_mode(input::InputReceptionMode mode) override { input_mode = mode; }
    void consume(MirEvent const*) override {}

    void set_cursor_image(std::shared_ptr<graphics::CursorImage> const& /* image */) {}
    std::shared_ptr<graphics::CursorImage> cursor_image() const { return {}; }

    void request_client_surface_close() override {}

    bool supports_input() const override { return true;}
    int client_input_fd() const override { return fd;}
    int configure(MirWindowAttrib, int) override { return 0; }
    int query(MirWindowAttrib) const override { return 0; }
    void with_most_recent_buffer_do(std::function<void(graphics::Buffer&)> const&) {}

    std::shared_ptr<Surface> parent() const override { return nullptr; }

    void set_keymap(MirInputDeviceId, std::string const&, std::string const&, std::string const&, std::string const&) override
    {}

    void set_cursor_stream(std::shared_ptr<frontend::BufferStream> const&, geometry::Displacement const&) {}
    void rename(std::string const&) {}
    std::shared_ptr<frontend::BufferStream> primary_buffer_stream() const override { return nullptr; }
};

} // namespace scene
} // namescape mir

#endif // MIR_SCENE_FAKESURFACE_H_
