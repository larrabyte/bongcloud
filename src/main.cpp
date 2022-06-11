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

namespace routines {
    namespace board {
        void print(const bongcloud::board& board) noexcept {
            board.print();
        }

        void undo(const bongcloud::ai& engine, bongcloud::board& board) noexcept {
            if(!board.history().empty()) {
                board.undo();

                if(engine.enabled && !board.history().empty()) {
                    board.undo();
                }
            }
        }

        void clicked(bongcloud::renderer& renderer, bongcloud::board& board, const std::size_t x, const std::size_t y) noexcept {
            auto i = renderer.square(board, x, y);

            if(!renderer.clicked_square && board[i]) {
                renderer.clicked_square = i;
            }

            else {
                if(renderer.clicked_square) {
                    board.move(*renderer.clicked_square, i);
                }

                renderer.clicked_square = std::nullopt;
            }
        }
    }

    namespace ai {
        [[noreturn]] void perft(const bongcloud::ai& engine, bongcloud::board& board) noexcept {
            // Run performance testing and then exit the program.
            for(std::size_t i = 1; i < engine.layers + 1; ++i) {
                auto n = engine.perft(board, i);
                fmt::print("[bongcloud] no. of positions after {} ply: {}\n", i, n);
            }

            std::terminate();
        }

        void evaluate(const bongcloud::ai& engine, bongcloud::board& board) noexcept {
            auto evaluation = engine.evaluate(board);
            fmt::print("[bongcloud] current evaluation: {:+}\n", evaluation);
        }

        void generate(bongcloud::ai& engine, bongcloud::board& board) noexcept {
            if(engine.future.valid()) {
                // If the future is valid, then the AI could either
                // have a result for us or still be thinking.
                auto zero = std::chrono::milliseconds(0);
                if(engine.future.wait_for(zero) == std::future_status::ready) {
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
    }
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
        routines::ai::perft(engine, board);
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
                if(auto& event = handler.get<cen::keyboard_event>(); event.pressed()) {
                    // Make sure that either the Left Control or Left GUI keys are being pressed.
                    if(event.is_active(cen::key_mod::lctrl) || event.is_active(cen::key_mod::lgui)) {
                        if(event.is_active(cen::scancodes::p)) {
                            routines::board::print(board);
                        } else if(event.is_active(cen::scancodes::z)) {
                            routines::board::undo(engine, board);
                        } else if(event.is_active(cen::scancodes::e)) {
                            routines::ai::evaluate(engine, board);
                        }
                    }
                }
            }

            else if(handler.is<cen::mouse_button_event>()) {
                // Make sure it the left-mouse button was pressed.
                if(auto& event = handler.get<cen::mouse_button_event>(); event.button() == cen::mouse_button::left && event.pressed()) {
                    auto x = static_cast<std::size_t>(event.x());
                    auto y = static_cast<std::size_t>(event.y());
                    routines::board::clicked(renderer, board, x, y);
                }
            }
        }

        if(engine.enabled && board.color() == bongcloud::piece::color::black) {
            routines::ai::generate(engine, board);
        }

        renderer.render(board);
    }

    return 0;
}
