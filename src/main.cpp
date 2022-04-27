#include "board.hpp"

#include <centurion.hpp>

int main(int argc, char** argv) {
    // Initialise the SDL libraries.
    const cen::sdl sdl;
    const cen::img img;
    const cen::mix mix;
    const cen::ttf ttf;

    // Initialise a window/renderer pair.
    cen::window window;
    cen::renderer renderer = window.make_renderer();
    window.show();

    // Initialise the board.
    bongcloud::board board(8);

    // Initialise an event handler and then loop.
    cen::event_handler handler;
    bool running = true;

    while (running) {
        while (handler.poll()) {
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
