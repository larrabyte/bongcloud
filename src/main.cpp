#include "renderer.hpp"
#include "events.hpp"
#include "extras.hpp"
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

    bongcloud::board board(board_size, anarchy);
    bongcloud::ai engine(search_depth, bot);
    board.load(fen_string);

    // This must be done at the start to
    // determine which color the engine is to use.
    auto engine_color = ext::flip(board.color());

    if(perft) {
        // Run performance/correctness testing and then exit the program.
        for(std::size_t i = 1; i < engine.layers + 1; ++i) {
            auto n = board.positions(i);
            fmt::print("[bongcloud] no. of positions after {} ply: {}\n", i, n);
        }

        return 0;
    }

    bongcloud::renderer renderer(square_res, board_size);
    bongcloud::event_dispatcher dispatcher(board, engine, renderer);

    while(dispatcher.running()) {
        dispatcher.poll();

        if(engine.enabled && board.color() == engine_color) {
            if(engine.future.valid()) {
                // If the future is valid, then the AI could either
                // have a result for us or still be thinking.
                using namespace std::chrono_literals;
                if(engine.future.wait_for(0ms) == std::future_status::ready) {
                    if(auto move = engine.future.get()) {
                        board.move(move->from, move->to);
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

        if(auto status = board.state(); !dispatcher.popup && status != bongcloud::board::status::normal) {
            cen::message_box box;
            dispatcher.popup = true;

            if(status == bongcloud::board::status::checkmate) {
                box.set_title("Checkmate!");
                auto index = ext::to_underlying(board.color());
                auto color = bongcloud::constants::color_titles[index];
                auto message = fmt::format("Game: {} was checkmated.", color);
                box.set_message(message);
            }

            else if(status == bongcloud::board::status::stalemate) {
                box.set_title("Stalemate!");
                box.set_message("Game: draw by stalemate.");
            }

            box.show();
        }
    }

    return 0;
}
