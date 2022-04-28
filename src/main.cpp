#include "renderer.hpp"
#include "board.hpp"

#include <argparse/argparse.hpp>
#include <centurion.hpp>
#include <fmt/core.h>

constexpr std::size_t default_board_size = 8;
constexpr std::size_t default_square_resolution = 64;

int main(int argc, char** argv) {
    // Retrieve any additional parameters from the command line if present.
    argparse::ArgumentParser program("bongcloud");

    program.add_argument("-s", "--size")
        .required()
        .help("the size of the board")
        .scan<'u', std::size_t>()
        .default_value(default_board_size);

    program.add_argument("-r", "--resolution")
        .required()
        .help("the resolution of each square")
        .scan<'u', std::size_t>()
        .default_value(default_square_resolution);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::exit(1);
    }

    auto board_size = program.get<std::size_t>("size");
    auto square_res = program.get<std::size_t>("resolution");

    // Initialise the board and its associated renderer.
    bongcloud::renderer renderer(square_res, board_size);
    bongcloud::board board(board_size);

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
    }

    return 0;
}
