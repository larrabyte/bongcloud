#include "renderer.hpp"
#include "board.hpp"
#include "ai.hpp"

#include <argparse/argparse.hpp>
#include <centurion.hpp>
#include <fmt/core.h>
#include <memory>
#include <future>
#include <chrono>

namespace defaults {
    constexpr std::size_t board_size = 8;
    constexpr std::size_t square_resolution = 64;
    constexpr std::size_t search_depth = 4;
    constexpr bool anarchy = false;
    constexpr bool bot = false;

    // Sadly, constexpr std::string isn't a thing yet.
    const std::string fen_8x8 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
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

    program.add_argument("-d", "--depth")
        .required()
        .help("set the search depth of the bot")
        .scan<'u', std::size_t>()
        .default_value(defaults::search_depth);

    program.add_argument("-f", "--fen")
        .required()
        .help("the FEN string to load")
        .default_value(defaults::fen_8x8);

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
    auto search_depth = program.get<std::size_t>("depth");
    auto fen_string = program.get<std::string>("fen");
    auto anarchy = program.get<bool>("anarchy");
    auto bot = program.get<bool>("bot");

    // Initialise the board and its associated renderer.
    bongcloud::renderer renderer(square_res, board_size);
    bongcloud::board board(board_size, anarchy);
    board.load(fen_string);

    // This can be any object as long as it inherits from the abstract AI class.
    std::unique_ptr<bongcloud::ai> engine(new bongcloud::classical_ai(board, search_depth));
    std::future<std::optional<bongcloud::move>> future;

    cen::event_handler handler;
    bool running = true;

    while(running) {
        while(handler.poll()) {
            if(handler.is<cen::quit_event>()) {
                running = false;
            }

            else if(handler.is<cen::keyboard_event>()) {
                auto& event = handler.get<cen::keyboard_event>();

                if(event.pressed()) {
                    bool ctrl_or_cmd = {
                        event.is_active(cen::key_mod::lctrl) ||
                        event.is_active(cen::key_mod::lgui)
                    };

                    if(ctrl_or_cmd && event.is_active(cen::scancodes::p)) {
                        // Pressing Ctrl+P will print the current board state.
                        board.print();
                    }

                    if(ctrl_or_cmd && event.is_active(cen::scancodes::z)) {
                        // Pressing Ctrl+Z will undo the last move.
                        if(board.history().size() > 0) {
                            board.undo();

                            if(bot && board.history().size() > 0) {
                                board.undo();
                            }
                        }
                    }

                    if(ctrl_or_cmd && event.is_active(cen::scancodes::e)) {
                        // Pressing Ctrl+E will print the current evaluation.
                        auto evaluation = engine->evaluate(board);
                        fmt::print("[bongcloud] current evaluation: {:+}\n", evaluation);
                    }
                }
            }

            else if(handler.is<cen::mouse_button_event>()) {
                auto& event = handler.get<cen::mouse_button_event>();

                // Make sure it was a left-click.
                if(event.button() == cen::mouse_button::left && event.pressed()) {
                    auto x = static_cast<std::size_t>(event.x() * renderer.scale());
                    auto y = static_cast<std::size_t>(event.y() * renderer.scale());
                    auto i = renderer.square(board, x, y);
                    auto stored = renderer.cursor();

                    if(!stored && board[i]) {
                        renderer.cursor(i);
                    } else if(i == stored || (stored && board.move(*stored, i))) {
                        renderer.cursor(std::nullopt);
                    }
                }
            }
        }

        if(bot && board.color() == bongcloud::piece::color::black) {
            if(future.valid()) {
                auto zero = std::chrono::milliseconds(0);
                if(future.wait_for(zero) == std::future_status::ready) {
                    if(auto move = future.get()) {
                        bool success = board.move(move->from, move->to);

                        if(!success) {
                            throw std::runtime_error("AI tried to play illegal move");
                        }
                    } else {
                        // The bot has no legal moves.
                        fmt::print("[bongcloud] no legal moves remaining.\n");
                        bot = false;
                    }
                }
            }

            else {
                auto subroutine = [&]() { return engine->generate(board); };
                future = std::async(std::launch::async, subroutine);
            }
        }

        renderer.render(board);
    }

    return 0;
}
