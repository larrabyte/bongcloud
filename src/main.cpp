#include "renderer.hpp"
#include "board.hpp"
#include "ai.hpp"

#include <argparse/argparse.hpp>
#include <centurion.hpp>
#include <fmt/core.h>
#include <future>
#include <chrono>

namespace defaults {
    constexpr std::size_t board_size = 8;
    constexpr std::size_t square_resolution = 64;
    constexpr std::size_t search_depth = 4;
    constexpr bool anarchy = false;
    constexpr bool bot = false;
    constexpr bool perft = false;

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

    program.add_argument("-p", "--perft")
        .required()
        .help("run perft up to the bot's depth")
        .default_value(defaults::perft)
        .implicit_value(!defaults::perft);

    // Let this throw if there are any runtime errors.
    program.parse_args(argc, argv);

    auto board_size = program.get<std::size_t>("size");
    auto square_res = program.get<std::size_t>("resolution");
    auto search_depth = program.get<std::size_t>("depth");
    auto fen_string = program.get<std::string>("fen");
    auto anarchy = program.get<bool>("anarchy");
    auto bot = program.get<bool>("bot");
    auto perft = program.get<bool>("perft");

    // Initialise game-related objects.
    bongcloud::board board(board_size, anarchy);
    bongcloud::ai engine(search_depth, bot);
    board.load(fen_string);

    if(bot) {
        fmt::print("[bongcloud] AI enabled, search depth set to {} ply.\n", search_depth);
    }

    if(perft) {
        for(std::size_t i = 1; i < engine.layers + 1; ++i) {
            auto n = engine.perft(board, i);
            fmt::print("[bongcloud] no. of positions after {} ply: {}\n", i, n);
        }

        return 0;
    }

    bongcloud::renderer renderer(square_res, board_size);
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
                        if(!board.history().empty()) {
                            board.undo();

                            if(engine.enabled && !board.history().empty()) {
                                board.undo();
                            }
                        }
                    }

                    if(ctrl_or_cmd && event.is_active(cen::scancodes::e)) {
                        // Pressing Ctrl+E will print the current evaluation.
                        auto evaluation = engine.evaluate(board);
                        fmt::print("[bongcloud] current evaluation: {:+}\n", evaluation);
                    }
                }
            }

            else if(handler.is<cen::mouse_button_event>()) {
                auto& event = handler.get<cen::mouse_button_event>();

                // Make sure it was a left-click.
                if(event.button() == cen::mouse_button::left && event.pressed()) {
                    auto i = renderer.square(
                        board,
                        static_cast<std::size_t>(event.x()),
                        static_cast<std::size_t>(event.y())
                    );

                    auto stored = renderer.cursor();

                    if(!stored && board[i]) {
                        renderer.cursor(i);
                    } else if(i == stored || (stored && board.move(*stored, i))) {
                        // Stop piece tracking if we're placing the piece
                        // back or a successful move was made.
                        renderer.cursor(std::nullopt);
                    }
                }
            }
        }

        if(engine.enabled && board.color() == bongcloud::piece::color::black) {
            if(engine.future.valid()) {
                // If the future is valid, then the AI could either
                // have a result for us or still be thinking.
                auto zero = std::chrono::milliseconds(0);
                bool ready = engine.future.wait_for(zero) == std::future_status::ready;

                if(ready) {
                    if(auto move = engine.future.get()) {
                        // A move has been generated! Play it.
                        board.move(move->from, move->to);
                    }

                    else {
                        // The bot has no legal moves.
                        fmt::print("[bongcloud] no legal moves remaining.\n");
                        engine.enabled = false;
                    }
                }
            }

            else {
                // Otherwise, spawn a new thread to evaluate this position.
                auto subroutine = [&]() { return engine.generate(board); };
                engine.future = std::async(std::launch::async, subroutine);
            }
        }

        renderer.render(board);
    }

    return 0;
}
