#pragma once

#include <SDL.h>
#include <cstdint>
#include <cstddef>

#include "board.h"

class Renderer {
    public:
        // Renderer constructor.
        Renderer(std::size_t, std::size_t);

        // Draw a board to the screen.
        void draw(Board&);

        // Get the index of the square at the given x and y mouse coordinates.
        std::size_t square(std::size_t, std::size_t);

        // Temporary piece storage.
        std::size_t tempidx;
        std::size_t origin;
        std::size_t dest;
        Piece store;

    private:
        // Get the offset of a texture in the texture array.
        std::size_t texoffset(Piece::Colour, Piece::Type);

        // Load a texture given a filepath and piece information.
        void loadtex(Piece::Colour, Piece::Type, const char*);

        // Get the texture of a piece.
        SDL_Texture* readtex(Piece&);

        // Return an casted SDL rectangle given a (size_t) position and (size_t) size.
        SDL_Rect rect(std::size_t, std::size_t, std::size_t, std::size_t);

        // Set the renderer's brush given a hex colour.
        void brush(std::uint32_t);

        SDL_Texture* textures[16];
        SDL_Renderer* renderer;
        SDL_Window* window;

        std::size_t pixels;
        std::size_t scale;
        std::size_t stride;
};
