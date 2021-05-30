#include <cstddef>

#include "renderer.h"
#include "board.h"

int main(void) {
    constexpr std::size_t squares = 8;
    constexpr std::size_t pixels = 60;
    Renderer renderer = Renderer(squares, pixels);
    Board board = Board(squares);

    // A place to store pieces being moved.
    Piece storePiece = Piece();
    std::size_t storeOrigin = 0;

    // For a standard 8x8 chess board.
    board.loadfen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // For a more modest 16x16 chess board.
    // board.loadfen("rrnnbbqqkkbbnnrr/pppppppppppppppp/97/97/97/97/97/97/97/97/97/97/97/97/PPPPPPPPPPPPPPPP/RRNNBBQQKKBBNNRR w KQkq - 0 1");

    // For an absolutely epic 20x20 chess board.
    // board.loadfen("2rrnnbbqqkkbbnnrr2/pppppppppppppppppppp/992/992/992/992/992/992/992/992/992/992/992/992/992/992/992/992/PPPPPPPPPPPPPPPPPPPP/6RNBQKBNR6 w KQkq - 0 1");

    while(true) {
        SDL_Event event;
        if(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: {
                    return 0;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    std::size_t location = renderer.square(event.button.x, event.button.y);
                    Piece &piece = board.square(location);

                    if(storePiece.type == Piece::Type::empty) {
                        storeOrigin = location;
                        storePiece.set(piece.colour, piece.type);
                        piece.set(Piece::Colour::white, Piece::Type::empty);
                    } else {
                        piece.set(storePiece.colour, storePiece.type);
                        storePiece.set(Piece::Colour::white, Piece::Type::empty);
                        renderer.lastmove(storeOrigin, location);
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
