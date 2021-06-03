#include <cstddef>

#include "renderer.h"
#include "board.h"
#include "piece.h"

int main(void) {
    // Size of the actual and graphical board.
    constexpr std::size_t squares = 8;
    constexpr std::size_t pixels = 60;
    Renderer renderer = Renderer(squares, pixels);
    Board board = Board(squares);
    renderer.draw(board);

    // A choice of a variety of FEN strings for different board sizes.
    // const char *fenstring = "rbqkbr/pppppp/6/6/PPPPPP/RBQKBR w KQkq - 0 1";
    const char *fenstring = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // const char *fenstring = "rrnnbbqqkkbbnnrr/pppppppppppppppp/97/97/97/97/97/97/97/97/97/97/97/97/PPPPPPPPPPPPPPPP/RRNNBBQQKKBBNNRR w KQkq - 0 1";

    if(!board.loadfen(fenstring)) {
        const char *title = "Invalid FEN String";
        const char *content = "The FEN string loaded could not be parsed properly.";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, content, nullptr);
    }

    while(true) {
        SDL_Event event;
        if(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: {
                    return 0;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    // Convert mouse coordinates to std::size_t before asking for an index.
                    std::size_t x = static_cast<std::size_t>(event.button.x);
                    std::size_t y = static_cast<std::size_t>(event.button.y);
                    std::size_t index = renderer.square(x, y);
                    Piece &piece = board.square(index);

                    if(renderer.store.type == Piece::Type::empty) {
                        renderer.tempidx = index;
                        renderer.store.set(piece.colour, piece.type);
                        piece.set(Piece::Colour::white, Piece::Type::empty);
                    } else {
                        piece.set(renderer.store.colour, renderer.store.type);
                        renderer.store.set(Piece::Colour::white, Piece::Type::empty);

                        if(renderer.tempidx != index) {
                            renderer.origin = renderer.tempidx;
                            renderer.dest = index;
                        }
                    }

                    break;
                }
            }
        }

        // All events have been processed. Start rendering.
        renderer.draw(board);
    }
}
