#pragma once

#include "board.hpp"

#include <centurion.hpp>
#include <cstddef>

namespace bongcloud {
    class renderer {
        public:
            // The renderer's constructor.
            renderer(const std::size_t, const std::size_t);

        private:
            // The SDL libraries.
            const cen::sdl m_sdl;
            const cen::img m_img;
            const cen::mix m_mix;
            const cen::ttf m_ttf;

            // A window/renderer pair.
            cen::window m_window;
            cen::renderer m_renderer;

            // The size of a square.
            std::size_t m_resolution;
    };
}
