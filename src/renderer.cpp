#include "renderer.hpp"

#include <fmt/core.h>

namespace colors {
    constexpr cen::color light_square {0xEC, 0xDB, 0xB9};
    constexpr cen::color dark_square {0xAE, 0x89, 0x68};
}

namespace ctors {
    cen::window make_window(const std::size_t resolution) {
        cen::iarea area = {
            static_cast<int>(resolution),
            static_cast<int>(resolution)
        };

        std::uint32_t flags = cen::window::allow_high_dpi | cen::window::resizable;
        return cen::window("bongcloud", area, flags);
    }

    cen::renderer make_renderer(cen::window& window) {
        std::uint32_t flags = cen::renderer::accelerated | cen::renderer::vsync;
        return window.make_renderer(flags);
    }

    double compute_scale(const cen::window& window, const cen::renderer& renderer) {
        // It is assumed that the scale factor is the same vertically and horizontally.
        auto renderer_width = static_cast<double>(renderer.output_size().width);
        auto window_width = static_cast<double>(window.width());
        return renderer_width / window_width;
    }
}

bongcloud::renderer::renderer(const std::size_t square_res, const std::size_t board_size)
    : m_window {ctors::make_window(square_res * board_size)},
      m_renderer {ctors::make_renderer(m_window)},
      m_scale {ctors::compute_scale(m_window, m_renderer)},
      m_resolution {static_cast<std::size_t>(square_res * m_scale)} {

    cen::iarea scaled = m_renderer.output_size();
    fmt::print("[bongcloud] resolution scale factor: {}\n", m_scale);
    fmt::print("[bongcloud] square resolution set to: {}x{}\n", m_resolution, m_resolution);
    fmt::print("[bongcloud] screen resolution set to: {}x{}\n", scaled.width, scaled.height);
    m_window.show();

    // Load textures from disk and store them in the texture array.
    const std::array<std::string, 16> paths {
        // White textures.
        "data/wp.bmp",
        "data/wn.bmp",
        "data/wb.bmp",
        "data/wr.bmp",
        "data/wq.bmp",
        "data/wk.bmp",
        "",
        "",

        // Black textures.
        "data/bp.bmp",
        "data/bn.bmp",
        "data/bb.bmp",
        "data/br.bmp",
        "data/bq.bmp",
        "data/bk.bmp",
        "",
        ""
    };

    for (const auto& path : paths) {
        if(path.size() == 0) {
            // Reserved entries should be set to not present.
            m_textures.push_back(std::nullopt);
            continue;
        }

        fmt::print("[bongcloud] loading texture at {}...\n", path);
        cen::surface surface(path);
        cen::texture texture = m_renderer.make_texture(surface);
        m_textures.push_back(std::move(texture));
    }
}

void bongcloud::renderer::render(const board& surface) {
    m_renderer.clear_with(cen::colors::black);

    std::size_t y = m_renderer.output_size().height - m_resolution;
    std::size_t i = 0, x = 0;

    for (const auto& square : surface) {
        // Construct a Centurion rectangle to represent this square.
        cen::irect rect(x, y, m_resolution, m_resolution);

        // Compute whether the square should be light or dark.
        std::size_t rank = i / 8;
        auto predicate = [=] { return (rank % 2 == 0) ? i % 2 == 0 : i % 2 != 0; };
        cen::color color = predicate() ? colors::dark_square : colors::light_square;

        // Render the square.
        m_renderer.set_color(color);
        m_renderer.fill_rect(rect);

        // Get the appropriate texture for the piece on the square (if present) and render it.
        if(square.container) {
            auto color = static_cast<int>(square.container->color);
            auto type = static_cast<int>(square.container->type);
            auto offset = (color << 3) | type;

            // Assume that the texture is always present.
            auto& texture = *m_textures[offset];
            m_renderer.render(texture, rect);
        }

        // Advance the i, x and y values to the next square.
        if (++i % surface.length != 0) {
            x += m_resolution;
        } else {
            y -= m_resolution;
            x = 0;
        }
    }

    m_renderer.present();
}
