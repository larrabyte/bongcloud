#include "renderer.hpp"

#include <fmt/core.h>

namespace ctor {
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
}


bongcloud::renderer::renderer(const std::size_t square_res, const std::size_t board_size)
    : m_window {ctor::make_window(square_res * board_size)},
      m_renderer {ctor::make_renderer(m_window)},
      m_resolution {square_res} {

    fmt::print("[bongcloud] square resolution set to: {}x{}\n", m_resolution, m_resolution);
    fmt::print("[bongcloud] screen resolution set to: {}x{}\n", m_window.width(), m_window.height());
    m_window.show();
}
