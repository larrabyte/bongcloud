#pragma once

#include <SDL.h>
#include <cstdint>
#include <cstddef>

#include "board.h"

class Renderer {
    public:
        // Renderer constructor.
        Renderer(std::size_t, std::size_t);

        // Clear the renderer's screen.
        void clear(void);

        // Tell the renderer to wrap up and start drawing.
        void finish(void);

        // Stick a piece onto the cursor.
        void lift(Piece&);

        // Draw a board to the screen.
        void draw(Board&);

        // Update the last move highlighting.
        void lastmove(std::size_t begin, std::size_t end);

        // Get the index of the square at the given x and y mouse coordinates.
        std::size_t square(std::int32_t, std::int32_t);

    private:
        // Get the offset of a texture in the texture array.
        std::size_t texoffset(Piece::Colour, Piece::Type);

        // Load a texture given a filepath and piece information.
        void loadtex(Piece::Colour, Piece::Type, const char*);

        // Get the texture of a piece.
        SDL_Texture *gettex(Piece&);

        // Set the renderer's brush given a hex colour.
        void brush(int);

        SDL_Texture *textures[16] = { nullptr };
        SDL_Renderer *renderer = nullptr;
        SDL_Window *window = nullptr;
        std::size_t pixels;
        std::size_t scale;
        std::size_t squares;
        std::size_t origin;
        std::size_t dest;
};
