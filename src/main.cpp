#include <centurion.hpp>
#include <fmt/core.h>

#include <cstddef>
#include <random>

int main(int argc, char** argv) {
    // Initialise the SDL libraries.
    const cen::sdl sdl;
    const cen::img img;
    const cen::mix mix;
    const cen::ttf ttf;

    cen::window window;
    cen::renderer renderer = window.make_renderer();
    window.show();

    std::random_device entropy_source;
    auto seed = entropy_source();
    std::default_random_engine rng(seed);

    cen::event_handler handler;
    bool running = true;

    while (running) {
        while (handler.poll()) {
            if (handler.is<cen::mouse_button_event>()) {
                auto& event = handler.get<cen::mouse_button_event>();
                if (event.pressed()) {
                    auto x = rng();
                    fmt::print("[bongcloud] random number requested: {}\n", x);
                }
            }

            if (handler.is<cen::quit_event>()) {
                running = false;
                break;
            }
        }

        renderer.clear_with(cen::colors::coral);
        renderer.present();
    }

    window.hide();
    return 0;
}
