#include "renderer.hpp"
#include "board.hpp"
#include "ai.hpp"

#include <argparse/argparse.hpp>
#include <centurion.hpp>
#include <fmt/core.h>

namespace defaults {
    constexpr std::size_t board_size = 8;
    constexpr std::size_t square_resolution = 64;
    constexpr std::string_view fen_8x8 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    constexpr bool anarchy = false;
    constexpr bool bot = false;
}

int main(int argc, char** argv) {
    // Retrieve any additional parameters from the command line if present.
    argparse::ArgumentParser program("bongcloud");

    program.add_argument("-s", "--size")
        .required()
        .help("the size of the board")
        .scan<'u', std::size_t>()
        .default_value(defaults::board_size);

    program.add_argument("-r", "--resolution")
        .required()
        .help("the resolution of each square")
        .scan<'u', std::size_t>()
        .default_value(defaults::square_resolution);

    program.add_argument("-f", "--fen")
        .required()
        .help("the FEN string to load")
        .default_value(defaults::fen_8x8.data());

    program.add_argument("-a", "--anarchy")
        .required()
        .help("ignore all rules of chess")
        .default_value(defaults::anarchy)
        .implicit_value(!defaults::anarchy);

    program.add_argument("-b", "--bot")
        .required()
        .help("play the built-in bot")
        .default_value(defaults::bot)
        .implicit_value(!defaults::bot);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::exit(1);
    }

    auto board_size = program.get<std::size_t>("size");
    auto square_res = program.get<std::size_t>("resolution");
    auto fen_string = program.get<const char*>("fen");
    auto anarchy = program.get<bool>("anarchy");
    auto bot = program.get<bool>("bot");

    // Initialise the board and its associated renderer.
    bongcloud::renderer renderer(square_res, board_size);
    bongcloud::board board(board_size, anarchy);
    bongcloud::random_ai engine(board);
    board.load(fen_string);

    // Initialise an event handler and then loop.
    cen::event_handler handler;
    bool running = true;

    while(running) {
        while(handler.poll()) {
            if(handler.is<cen::quit_event>()) {
                running = false;
                break;
            }

            if(handler.is<cen::keyboard_event>()) {
                auto& event = handler.get<cen::keyboard_event>();

                if(event.pressed()) {
                    bool ctrl_or_cmd = {
                        event.is_active(cen::key_mod::lctrl) ||
                        event.is_active(cen::key_mod::lgui)
                    };

                    // Pressing Ctrl+P will print the current board state.
                    if(ctrl_or_cmd && event.is_active(cen::scancodes::p)) {
                        board.print();
                    }

                    // Pressing Ctrl+Z will undo the last move.
                    if(ctrl_or_cmd && event.is_active(cen::scancodes::z)) {
                        board.undo();

                        if(bot) {
                            board.undo();
                        }
                    }
                }
            }

            if(handler.is<cen::mouse_button_event>()) {
                auto& event = handler.get<cen::mouse_button_event>();

                // Make sure it was a left-click.
                if(event.button() == cen::mouse_button::left && event.pressed()) {
                    auto x = static_cast<std::size_t>(event.x() * renderer.scale());
                    auto y = static_cast<std::size_t>(event.y() * renderer.scale());
                    auto i = renderer.square_at(board, x, y);
                    fmt::print("[bongcloud] left-click at ({}, {}) yields square {}\n", x, y, i);

                    auto stored = renderer.cursor();

                    if(!stored && board[i]) {
                        renderer.cursor(i);
                    } else if(i == stored || (stored && board.mutate(*stored, i))) {
                        renderer.cursor(std::nullopt);
                    }
                }
            }
        }

        if(bot && board.color() == bongcloud::piece::color::black) {
            bool success = false;

            while(!success) {
                // Let the AI decide a move for black.
                auto move = engine.generate();
                if(move.from != move.to && board[move.from]) {
                    success = board.mutate(move.from, move.to);
                }
            }
        }

        renderer.render(board);
    }

    return 0;
}
