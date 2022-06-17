#include "renderer.hpp"
#include "extras.hpp"

#include <type_traits>
#include <fmt/core.h>
#include <cstdint>
#include <utility>
#include <bit>

namespace detail {
    cen::window make_window(const std::size_t resolution) noexcept {
        cen::iarea area = {
            static_cast<int>(resolution),
            static_cast<int>(resolution)
        };

        std::uint32_t flags = cen::window::allow_high_dpi;
        return cen::window("bongcloud", area, flags);
    }

    cen::renderer make_renderer(cen::window& window) noexcept {
        std::uint32_t flags = cen::renderer::accelerated;
        return window.make_renderer(flags);
    }

    double compute_scale(const cen::window& window, const cen::renderer& renderer) noexcept {
        // It is assumed that the scale factor is the same vertically and horizontally.
        auto renderer_width = static_cast<double>(renderer.output_size().width);
        auto window_width = static_cast<double>(window.width());
        return renderer_width / window_width;
    }

    std::underlying_type_t<bcl::piece::type> compute_texture_offset(const bcl::piece& piece) noexcept {
        using piece_type = std::underlying_type_t<bcl::piece::type>;

        constexpr auto offset = []() {
            // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2.
            piece_type x = ext::to_underlying(bcl::piece::type::last);
            piece_type e = 0;

            if(x != 0) {
                --x;
            } else {
                return e;
            }

            for(piece_type i = 0; i < std::bit_width(x); ++i) {
                x |= x >> (1 << i);
            }

            x += 1;
            while((x >>= 1) != 0) {
                ++e;
            }

            return e;
        }();

        auto color = ext::to_underlying(piece.hue);
        auto type = ext::to_underlying(piece.variety);
        return static_cast<piece_type>((color << offset) | type);
    }
}

bcl::renderer::renderer(const std::size_t resolution, const std::size_t size) noexcept :
    m_window {detail::make_window(resolution * size)},
    m_renderer {detail::make_renderer(m_window)},
    m_scale {detail::compute_scale(m_window, m_renderer)},
    m_resolution {static_cast<std::size_t>(resolution * m_scale)} {

    cen::iarea scaled = m_renderer.output_size();
    fmt::print("[bongcloud] resolution scale factor: {}\n", m_scale);
    fmt::print("[bongcloud] square resolution set to: {}x{}\n", m_resolution, m_resolution);
    fmt::print("[bongcloud] screen resolution set to: {}x{}\n", scaled.width, scaled.height);
    m_window.show();

    std::size_t rounded_size = 1 << std::bit_width(constants::white_textures.size());
    std::size_t maximum_index = rounded_size * 2;
    m_textures.reserve(maximum_index);

    for(std::size_t i = 0; i < maximum_index; ++i) {
        bool oob_white = i >= constants::white_textures.size() && i < rounded_size;
        bool oob_black = i >= constants::black_textures.size() + rounded_size;

        if(oob_white || oob_black) {
            m_textures.push_back(std::nullopt);
            continue;
        }

        const auto path = (i < rounded_size) ? constants::white_textures[i] : constants::black_textures[i - rounded_size];
        fmt::print("[bongcloud] loading texture at {}...\n", path);
        cen::surface surface(path);
        cen::texture texture = m_renderer.make_texture(surface);
        m_textures.push_back(std::move(texture));
    }
}

void bcl::renderer::render(const bcl::board& board) noexcept {
    m_renderer.clear_with(cen::colors::black);

    std::size_t y = static_cast<std::size_t>(m_renderer.output_size().height) - m_resolution;
    std::size_t i = 0;
    std::size_t x = 0;

    for(const auto& piece : board) {
        cen::irect rect = {
            static_cast<int>(x),
            static_cast<int>(y),
            static_cast<int>(m_resolution),
            static_cast<int>(m_resolution)
        };

        // Compute whether the square should be light or dark or highlighted.
        const auto& last = board.latest();
        bool green = last && (i == last->from || i == last->to);

        bool dark = {
            (board.length % 2 != 0) ?
            (i % 2 == 0) :
            (i / board.length % 2 == 0) ? i % 2 == 0 : i % 2 != 0
        };

        cen::color color = {
            (green) ?
            (dark) ? constants::dark_last_move : constants::light_last_move :
            (dark) ? constants::dark_square : constants::light_square
        };

        // Render the square.
        m_renderer.set_color(color);
        m_renderer.fill_rect(rect);

        // Get the appropriate texture for the piece on the square (if present) and render it.
        // Assume that the piece's texture is always present.
        if(piece && i != clicked_square) {
            auto offset = detail::compute_texture_offset(*piece);
            const auto& texture = *m_textures[offset];
            m_renderer.render(texture, rect);
        }

        // Advance the i, x and y values to the next square.
        if(++i % board.length != 0) {
            x += m_resolution;
        } else {
            y -= m_resolution;
            x = 0;
        }
    }

    // If a cursor square is specified, render it last.
    if(clicked_square) {
        cen::mouse mouse;
        mouse.update();

        cen::irect rect(
            // Set the top-left of the rectangle to the middle of the square.
            static_cast<int>(mouse.x() * m_scale - (m_resolution / 2)),
            static_cast<int>(mouse.y() * m_scale - (m_resolution / 2)),
            static_cast<int>(m_resolution),
            static_cast<int>(m_resolution)
        );

        const auto& piece = board[*clicked_square];
        auto offset = detail::compute_texture_offset(*piece);

        // Assume that the texture is always present.
        const auto& texture = *m_textures[offset];
        m_renderer.render(texture, rect);
    }

    // If a piece selection menu is active, render it.
    if(promotion_square) {
        this->promote(*promotion_square, board);
    }

    m_renderer.present();
}

std::size_t bcl::renderer::square(const bcl::board& board, const std::size_t x, const std::size_t y) const noexcept {
    // Compute the square at the given x and y coordinates.
    auto screen_height = static_cast<std::size_t>(m_renderer.output_size().height);
    auto scaled_y = static_cast<std::size_t>(y * m_scale);
    auto scaled_x = static_cast<std::size_t>(x * m_scale);

    std::size_t rank = (screen_height - scaled_y) / m_resolution;
    std::size_t file = scaled_x / m_resolution;
    return (rank * board.length) + file;
}

void bcl::renderer::promote(const std::size_t square, const bcl::board& board) noexcept {
    std::size_t h = static_cast<std::size_t>(m_renderer.output_size().height);
    std::size_t x = (square % board.length) * m_resolution;
    std::size_t y = h - (((square / board.length) + 1) * (m_resolution));
    std::size_t s = constants::promotion_pieces.size();

    // m_renderer.draw_rect() only renders the outline of a square.
    m_renderer.set_color(cen::colors::black);
    for(std::size_t border = 1; border < constants::promotion_menu_border; ++border) {
        std::size_t horizontal = m_resolution + (border * 2);
        std::size_t vertical = (m_resolution * s + (border * 2));

        cen::irect outline = {
            static_cast<int>(x - border),
            static_cast<int>(y - border),
            static_cast<int>(horizontal),
            static_cast<int>(vertical)
        };

        m_renderer.draw_rect(outline);
    }

    cen::irect primary = {
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(m_resolution),
        static_cast<int>(m_resolution * s)
    };

    m_renderer.set_color(cen::colors::white);
    m_renderer.fill_rect(primary);

    // Render each piece on top.
    for(std::size_t i = 0; i < s; ++i) {
        cen::irect place = {
            static_cast<int>(x),
            static_cast<int>(y + (i * m_resolution)),
            static_cast<int>(m_resolution),
            static_cast<int>(m_resolution)
        };

        bcl::piece piece = {board.color(), constants::promotion_pieces[i]};
        auto offset = detail::compute_texture_offset(piece);
        const auto& texture = *m_textures[offset];
        m_renderer.render(texture, place);
    }
}
