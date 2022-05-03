#pragma once

#include "board.hpp"

#include <centurion.hpp>
#include <cstddef>

namespace bongcloud {
    class renderer {
        public:
            // The renderer's constructor.
            renderer(const std::size_t, const std::size_t);

            // Renders a board to the screen.
            void render(const board&);

            // Attaches the index of the square to the mouse.
            void cursor(std::optional<std::size_t>);

            // Returns the index of the square attached to the mouse.
            std::optional<std::size_t> cursor(void) const noexcept;

            // Returns the index of the square at the given (scaled) mouse position.
            std::size_t square_at(const board&, const std::size_t, const std::size_t) const;

            // Returns the current resolution scale.
            double scale(void) const noexcept;

        private:
            // The SDL libraries.
            const cen::sdl m_sdl;
            const cen::img m_img;
            const cen::mix m_mix;
            const cen::ttf m_ttf;

            // A window/renderer pair.
            cen::window m_window;
            cen::renderer m_renderer;

            // Piece texture array.
            std::vector<std::optional<cen::texture>> m_textures;

            // The index of the hidden square.
            std::optional<std::size_t> m_mouse;

            // Resolution scale factor.
            double m_scale;

            // The scaled resolution of a single board square.
            std::size_t m_resolution;
    };
}
