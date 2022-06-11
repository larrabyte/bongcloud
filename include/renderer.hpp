#pragma once

#include "board.hpp"

#include <centurion.hpp>
#include <cstddef>

namespace bongcloud {
    class renderer {
        public:
            // The renderer's constructor.
            renderer(const std::size_t, const std::size_t) noexcept;

            // Renders a board to the screen.
            void render(const board&) noexcept;

            // Returns the index of the square at a given mouse coordinate.
            std::size_t square(const board&, const std::size_t, const std::size_t) const noexcept;

            // The index of the hidden square.
            std::optional<std::size_t> clicked_square;

            // The index of the piece selection menu square.
            std::optional<std::size_t> promotion_square;

        private:
            // Renders a piece selection menu to the screen at the square index passed in.
            void promote(const std::size_t, const board&) noexcept;

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

            // Resolution scale factor.
            double m_scale;

            // The scaled resolution of a single board square.
            std::size_t m_resolution;
    };

    namespace constants {
        // The thickness of the promotion menu border in pixels.
        constexpr std::size_t promotion_menu_border = 4;

        // The color code of the standard light square.
        constexpr cen::color light_square {0xEC, 0xDB, 0xB9};

        // The color code of the standard dark square.
        constexpr cen::color dark_square {0xAE, 0x89, 0x68};

        // The color code of the light-square move highlight.
        constexpr cen::color light_last_move {0xCE, 0xD2, 0x87};

        // The color code of the dark-square move highlight.
        constexpr cen::color dark_last_move {0xA9, 0xA3, 0x56};

        // A list of paths to the white piece textures.
        // Textures are ordered in the same way as the piece enum.
        constexpr std::string_view white_textures[] = {
            "data/wp.bmp",
            "data/wn.bmp",
            "data/wb.bmp",
            "data/wr.bmp",
            "data/wq.bmp",
            "data/wk.bmp"
        };

        // A list of paths to the black piece textures.
        // Textures are ordered in the same way as the piece enum.
        constexpr std::string_view black_textures[] = {
            "data/bp.bmp",
            "data/bn.bmp",
            "data/bb.bmp",
            "data/br.bmp",
            "data/bq.bmp",
            "data/bk.bmp"
        };

        static_assert(
            std::size(white_textures) == std::size(black_textures),
            "white and black must have the same number of textures"
        );
    }
}
