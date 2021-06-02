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

    // Render a blank board to the screen.
    renderer.clear();
    renderer.draw(board);
    renderer.finish();

    // Allocate a place to store pieces being moved.
    Piece storePiece = Piece();
    std::size_t storeOrigin = 0;

    // Your choice of a variety of FEN strings for different board sizes.
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
                    std::size_t x = static_cast<std::size_t>(event.button.x);
                    std::size_t y = static_cast<std::size_t>(event.button.y);
                    std::size_t location = renderer.square(x, y);
                    Piece &piece = board.square(location);

                    if(storePiece.type == Piece::Type::empty) {
                        storeOrigin = location;
                        storePiece.set(piece.colour, piece.type);
                        piece.set(Piece::Colour::white, Piece::Type::empty);
                    } else {
                        piece.set(storePiece.colour, storePiece.type);
                        storePiece.set(Piece::Colour::white, Piece::Type::empty);
                        if(storeOrigin != location) renderer.lastmove(storeOrigin, location);
                    }

                    break;
                }
            }
        }

        // All events have been processed. Start rendering.
        renderer.clear();
        renderer.draw(board);
        renderer.lift(storePiece);
        renderer.finish();
    }
}
