#include "events.hpp"

#include <fmt/core.h>
#include <cstddef>

bcl::event_dispatcher::event_dispatcher(board& b, ai& e, renderer& r) noexcept :
    m_board {b},
    m_engine {e},
    m_renderer {r} {

    m_dispatcher.bind<cen::quit_event>().to<&event_dispatcher::on_quit_event>(this);
    m_dispatcher.bind<cen::keyboard_event>().to<&event_dispatcher::on_keyboard_event>(this);
    m_dispatcher.bind<cen::mouse_button_event>().to<&event_dispatcher::on_mouse_button_event>(this);
}

void bcl::event_dispatcher::on_quit_event(const cen::quit_event& event) noexcept {
    m_running = false;
}

void bcl::event_dispatcher::on_keyboard_event(const cen::keyboard_event& event) noexcept {
    if(event.pressed() && event.is_active(cen::key_mod::lctrl | cen::key_mod::lgui)) {
        if(event.is_active(cen::scancodes::p)) {
            m_board.print();
        }

        else if(event.is_active(cen::scancodes::z)) {
            if(!m_board.history().empty() && (!m_engine.enabled || !m_engine.future.valid())) {
                popup = false;
                m_board.undo();

                if(m_engine.enabled) {
                    m_board.undo();
                }
            }
        }

        else if(event.is_active(cen::scancodes::e)) {
            fmt::print("[bongcloud] current evaluation: {:+}\n", m_engine.evaluate(m_board));
        }
    }
}

void bcl::event_dispatcher::on_mouse_button_event(const cen::mouse_button_event& event) noexcept {
    using namespace std::chrono_literals;

    if(event.pressed() && event.button() == cen::mouse_button::left) {
        if(!m_engine.future.valid() || m_engine.future.wait_for(0ms) == std::future_status::ready) {
            auto x = static_cast<std::size_t>(event.x());
            auto y = static_cast<std::size_t>(event.y());
            auto i = m_renderer.square(m_board, x, y);

            if(!m_renderer.clicked_square && m_board[i]) {
                m_renderer.clicked_square = i;
            }

            else {
                if(m_renderer.clicked_square != i) {
                    m_board.move(*m_renderer.clicked_square, i);
                }

                m_renderer.clicked_square = std::nullopt;
            }
        }
    }
}
