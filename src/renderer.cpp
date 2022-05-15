#include "renderer.hpp"
#include "extras.hpp"

#include <fmt/core.h>
#include <bit>

namespace internal {
    constexpr cen::color light_square {0xEC, 0xDB, 0xB9};
    constexpr cen::color dark_square {0xAE, 0x89, 0x68};
    constexpr cen::color light_last_move {0xCE, 0xD2, 0x87};
    constexpr cen::color dark_last_move {0xA9, 0xA3, 0x56};

    constexpr std::string_view white_textures[] = {
        "data/wp.bmp",
        "data/wn.bmp",
        "data/wb.bmp",
        "data/wr.bmp",
        "data/wq.bmp",
        "data/wk.bmp"
    };

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

    cen::window make_window(const std::size_t resolution) {
        cen::iarea area = {
            static_cast<int>(resolution),
            static_cast<int>(resolution)
        };

        std::uint32_t flags = cen::window::allow_high_dpi;
        return cen::window("bongcloud", area, flags);
    }

    cen::renderer make_renderer(cen::window& window) {
        std::uint32_t flags = cen::renderer::accelerated;
        return window.make_renderer(flags);
    }

    double compute_scale(const cen::window& window, const cen::renderer& renderer) {
        // It is assumed that the scale factor is the same vertically and horizontally.
        auto renderer_width = static_cast<double>(renderer.output_size().width);
        auto window_width = static_cast<double>(window.width());
        return renderer_width / window_width;
    }

    std::size_t compute_texture_offset(const bongcloud::piece& piece) {
        constexpr auto offset = [] {
            // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2.
            auto x = ext::to_underlying(bongcloud::piece::type::last);
            std::size_t e = 0;

            if(x == 0) {
                return e;
            } else {
                x--;
            }

            for(std::size_t i = 0; i < std::bit_width(x); ++i) {
                x |= x >> (1 << i);
            }

            x += 1;
            while(x >>= 1) {
                ++e;
            }

            return e;
        }();

        auto color = ext::to_underlying(piece.hue);
        auto type = ext::to_underlying(piece.variety);
        return (color << offset) | type;
    }
}

bongcloud::renderer::renderer(const std::size_t square_res, const std::size_t board_size) :
    m_window {internal::make_window(square_res * board_size)},
    m_renderer {internal::make_renderer(m_window)},
    m_scale {internal::compute_scale(m_window, m_renderer)},
    m_resolution {static_cast<std::size_t>(square_res * m_scale)} {

    cen::iarea scaled = m_renderer.output_size();
    fmt::print("[bongcloud] resolution scale factor: {}\n", m_scale);
    fmt::print("[bongcloud] square resolution set to: {}x{}\n", m_resolution, m_resolution);
    fmt::print("[bongcloud] screen resolution set to: {}x{}\n", scaled.width, scaled.height);
    m_window.show();

    std::size_t rounded_size = 1 << std::bit_width(std::size(internal::white_textures));
    std::size_t maximum_index = rounded_size * 2;
    m_textures.reserve(maximum_index);

    for(std::size_t i = 0; i < maximum_index; ++i) {
        bool oob_white = i >= std::size(internal::white_textures) && i < rounded_size;
        bool oob_black = i >= std::size(internal::black_textures) + rounded_size;

        if(oob_white || oob_black) {
            m_textures.push_back(std::nullopt);
            continue;
        }

        const auto path = (i < rounded_size) ? internal::white_textures[i] : internal::black_textures[i - rounded_size];
        fmt::print("[bongcloud] loading texture at {}...\n", path);
        cen::surface surface(path.data());
        cen::texture texture = m_renderer.make_texture(surface);
        m_textures.push_back(std::move(texture));
    }
}

void bongcloud::renderer::render(const bongcloud::board& board) {
    m_renderer.clear_with(cen::colors::black);

    std::size_t y = m_renderer.output_size().height - m_resolution;
    std::size_t i = 0, x = 0;

    for(const auto& square : board) {
        // Construct a Centurion rectangle to represent this square.
        cen::irect rect(x, y, m_resolution, m_resolution);

        // Compute whether the square should be light or dark or highlighted.
        const auto& latest = board.latest();
        bool green = latest && (i == latest->from || i == latest->to);

        bool dark = {
            (board.length % 2 != 0) ?
            (i % 2 == 0) :
            (i / board.length % 2 == 0) ? i % 2 == 0 : i % 2 != 0
        };

        cen::color color = {
            (green) ?
            (dark) ? internal::dark_last_move : internal::light_last_move :
            (dark) ? internal::dark_square : internal::light_square
        };

        // Render the square.
        m_renderer.set_color(color);
        m_renderer.fill_rect(rect);

        // Get the appropriate texture for the piece on the square (if present) and render it.
        // Assume that the piece's texture is always present.
        if(square && i != m_mouse) {
            auto offset = internal::compute_texture_offset(*square);
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
    if(m_mouse) {
        cen::mouse mouse;
        mouse.update();

        cen::irect rect(
            // Set the top-left of the rectangle to the middle of the square.
            static_cast<int>(mouse.x() * m_scale - (m_resolution / 2)),
            static_cast<int>(mouse.y() * m_scale - (m_resolution / 2)),
            m_resolution,
            m_resolution
        );

        const auto& piece = board[*m_mouse];
        auto offset = internal::compute_texture_offset(*piece);

        // Assume that the texture is always present.
        const auto& texture = *m_textures[offset];
        m_renderer.render(texture, rect);
    }

    m_renderer.present();
}

std::size_t bongcloud::renderer::square_at(const bongcloud::board& board, const std::size_t x, const std::size_t y) const noexcept {
    // Compute the square at the given x and y coordinates.
    std::size_t rank = (m_renderer.output_size().height - y) / m_resolution;
    std::size_t file = x / m_resolution;
    return (rank * board.length) + file;
}
