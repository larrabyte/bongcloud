#pragma once

#include "renderer.hpp"
#include "board.hpp"
#include "ai.hpp"

#include <centurion.hpp>

namespace bcl {
    class event_dispatcher {
        public:
            // The event dispatcher's default constructor.
            event_dispatcher(board&, ai&, renderer&) noexcept;

            // Called when a quit event is dispatched.
            void on_quit_event(const cen::quit_event&) noexcept;

            // Called when a keyboard event is dispatched.
            void on_keyboard_event(const cen::keyboard_event&) noexcept;

            // Called when a mouse button event is dispatched.
            void on_mouse_button_event(const cen::mouse_button_event&) noexcept;

            // A shortcut for the dispatcher's polling mechanism.
            void poll(void) {
                m_dispatcher.poll();
            }

            // Returns whether the event loop should continue to run.
            [[nodiscard]] bool running(void) const noexcept {
                return m_running;
            }

            // Whether a stale/checkmating popup has appeared.
            bool popup = false;

        private:
            using dispatcher = cen::event_dispatcher<
                cen::quit_event,
                cen::keyboard_event,
                cen::mouse_button_event
            >;

            // The event dispatcher.
            dispatcher m_dispatcher;

            // References to game objects (that can be used by event-handling functions).
            bcl::board& m_board;
            bcl::ai& m_engine;
            bcl::renderer& m_renderer;

            // Whether the event loop is still active.
            bool m_running = true;
    };
}
