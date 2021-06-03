#include "renderer.h"
#include "board.h"
#include "piece.h"

namespace colours {
    constexpr std::uint32_t lastMoveLight = 0xCED287;
    constexpr std::uint32_t lastMoveDark = 0xA9A356;
    constexpr std::uint32_t regularLight = 0xECDBB9;
    constexpr std::uint32_t regularDark = 0xAE8968;
}

SDL_Texture *Renderer::readtex(Piece& piece) {
    std::size_t offset = this->texoffset(piece.colour, piece.type);
    return this->textures[offset];
}

std::size_t Renderer::texoffset(Piece::Colour c, Piece::Type t) {
    std::uint32_t cb = static_cast<std::uint32_t>(c);
    std::uint32_t tb = static_cast<std::uint32_t>(t);
    return (cb << 3) | tb;
}

void Renderer::loadtex(Piece::Colour c, Piece::Type t, const char* file) {
    SDL_Surface *surface = SDL_LoadBMP(file);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(this->renderer, surface);
    if(surface != nullptr) SDL_FreeSurface(surface);
    this->textures[this->texoffset(c, t)] = tex;
}

void Renderer::brush(std::uint32_t hex) {
    std::uint8_t red = (hex >> 16) & 0xFF;
    std::uint8_t green = (hex >> 8) & 0xFF;
    std::uint8_t blue = hex & 0xFF;
    SDL_SetRenderDrawColor(this->renderer, red, green, blue, 255);  
}

SDL_Rect Renderer::rect(std::size_t x, std::size_t y, std::size_t w, std::size_t l) {
    // The renderer works using std::size_t types. SDL uses machine-wide integers,
    // so we perform a static cast to make it happy. If something breaks because
    // of overflow, expect the problem to be around here.
    SDL_Rect rect = {
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(w),
        static_cast<int>(l)
    };

    return rect;
}

void Renderer::draw(Board& board) {
    std::size_t length = this->pixels * this->scale;
    SDL_Texture *storetex = this->readtex(this->store);

    // Clear the screen.
    this->brush(0x000000);
    SDL_RenderClear(this->renderer);

    // Render the board along with its pieces.
    for(std::size_t i = 0, x = 0, y = 0; auto& piece : board) {
        SDL_Rect graphic = this->rect(x, y, length, length);

        // For move highlighting to be possible, origin and destination 
        // must not be equal AND one must match to a valid board index.
        bool lastmove = this->origin != this->dest && (i == this->origin || i == this->dest);
        bool colour = this->stride % 2 == 0 ? ((i / this->stride) + i) % 2 == 0 : i % 2 == 0;

        // Create a checkerboard pattern for any number of squares or highlight last move.
        if(lastmove) this->brush(colour ? colours::lastMoveLight : colours::lastMoveDark);
        else this->brush(colour ? colours::regularLight : colours::regularDark);
        SDL_RenderFillRect(this->renderer, &graphic);

        SDL_Texture *tex = this->readtex(piece);
        if(tex != nullptr) SDL_RenderCopy(this->renderer, tex, nullptr, &graphic);

        if(++i % this->stride == 0) {
            y += length; x = 0;
        } else {
            x += length;
        }
    }

    // Render the store piece onto the cursor.
    if(storetex != nullptr) {
        SDL_Rect mouse = this->rect(0, 0, length, length);
        SDL_GetMouseState(&mouse.x, &mouse.y);
        mouse.x = (mouse.x * this->scale) - (length / 2);
        mouse.y = (mouse.y * this->scale) - (length / 2);
        SDL_RenderCopy(this->renderer, storetex, nullptr, &mouse);
    }

    // Finish rendering and present to screen.
    SDL_RenderPresent(this->renderer);
}

std::size_t Renderer::square(std::size_t x, std::size_t y) {
    // Get an appropriate board index by dividing by the pixel size of the square.
    // Mouse coordinates in SDL2 are given in the non-scaled form.
    std::size_t rank = (y / this->pixels) * this->stride;
    std::size_t file = x / this->pixels;
    return rank + file;
}

Renderer::Renderer(std::size_t squares, std::size_t pixels) {
    // Attempt to initialise SDL, exit if unable.
    if(SDL_Init(SDL_INIT_VIDEO) < 0) exit(-1);
    std::uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    int resolution = static_cast<int>(squares * pixels);
    int scaledres;

    // If the SDL window or renderer couldn't initialise, exit.
    SDL_CreateWindowAndRenderer(resolution, resolution, flags, &this->window, &this->renderer);
    if(this->window == nullptr || this->renderer == nullptr) exit(-1);

    // Get the actual resolution that the OS has decided to use.
    SDL_GetRendererOutputSize(this->renderer, &scaledres, nullptr);
    this->scale = static_cast<std::size_t>(scaledres / resolution);

    // The value of these doesn't matter, only that they are
    // equal to each other to disable move highlighting.
    this->origin = this->dest;
    this->stride = squares;
    this->pixels = pixels;
    this->tempidx = 0;

    // Load piece textures into an array.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    for(auto texture : this->textures) {
        texture = nullptr;
    }

    this->loadtex(Piece::Colour::white, Piece::Type::empty, "data/we.bmp");
    this->loadtex(Piece::Colour::white, Piece::Type::pawn, "data/wp.bmp");
    this->loadtex(Piece::Colour::white, Piece::Type::knight, "data/wn.bmp");
    this->loadtex(Piece::Colour::white, Piece::Type::bishop, "data/wb.bmp");
    this->loadtex(Piece::Colour::white, Piece::Type::rook, "data/wr.bmp");
    this->loadtex(Piece::Colour::white, Piece::Type::queen, "data/wq.bmp");
    this->loadtex(Piece::Colour::white, Piece::Type::king, "data/wk.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::empty, "data/be.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::pawn, "data/bp.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::knight, "data/bn.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::bishop, "data/bb.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::rook, "data/br.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::queen, "data/bq.bmp");
    this->loadtex(Piece::Colour::black, Piece::Type::king, "data/bk.bmp");
}
