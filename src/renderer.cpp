#include "renderer.h"
#include "board.h"

namespace colours {
    constexpr int lastMoveLight = 0xCED287;
    constexpr int lastMoveDark = 0xA9A356;
    constexpr int regularLight = 0xECDBB9;
    constexpr int regularDark = 0xAE8968;
}

std::size_t Renderer::texoffset(Piece::Colour c, Piece::Type t) {
    return (c << 3) | t;
}

void Renderer::loadtex(Piece::Colour c, Piece::Type t, const char *file) {
    SDL_Surface *surface = SDL_LoadBMP(file);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(this->renderer, surface);
    if(surface != NULL) SDL_FreeSurface(surface);
    this->textures[this->texoffset(c, t)] = tex;
}

SDL_Texture *Renderer::gettex(Piece &piece) {
    std::size_t offset = this->texoffset(piece.colour, piece.type);
    return this->textures[offset];
}

void Renderer::brush(int hex) {
    std::uint8_t red = (hex >> 16) & 0xFF;
    std::uint8_t green = (hex >> 8) & 0xFF;
    std::uint8_t blue = hex & 0xFF;
    SDL_SetRenderDrawColor(this->renderer, red, green, blue, 255);  
}

void Renderer::clear(void) {
    this->brush(0x000000);
    SDL_RenderClear(this->renderer);
}

void Renderer::finish(void) {
    SDL_RenderPresent(this->renderer);
}

void Renderer::lift(Piece &piece) {
    SDL_Texture *tex = this->gettex(piece);

    if(tex != nullptr) {
        int length = this->pixels * this->scale;
        SDL_Rect graphic = {0, 0, length, length};
        SDL_GetMouseState(&graphic.x, &graphic.y);
        graphic.x = graphic.x * this->scale - this->pixels;
        graphic.y = graphic.y * this->scale - this->pixels;
        SDL_RenderCopy(this->renderer, tex, NULL, &graphic);
    }
}

void Renderer::draw(Board &board) {
    std::size_t i = 0;
    int x = 0, y = 0;

    for(auto piece : board) {
        int length = this->pixels * this->scale;
        SDL_Rect graphic = {x, y, length, length};

        // For move highlighting to be possible, origin and destination 
        // must not be equal AND one must match to a valid board index.
        bool highlight = (this->origin != this->dest) && (i == this->origin || i == this->dest);

        // Checkboard pattern: paint a dark square if it's
        // on an even rank or even file, but not both.
        bool dark = ((i / this->squares) % 2 == 0) ^ (i % 2 == 0);

        if(highlight) this->brush(dark ? colours::lastMoveDark : colours::lastMoveLight);
        else this->brush(dark ? colours::regularDark : colours::regularLight);
        SDL_RenderFillRect(this->renderer, &graphic);

        SDL_Texture *tex = this->gettex(piece);
        if(tex != nullptr) SDL_RenderCopy(this->renderer, tex, nullptr, &graphic);

        y = (++i % this->squares == 0) ? y + length : y;
        x = (i % this->squares == 0) ? 0 : x + length;
    }
}

void Renderer::lastmove(std::size_t a, std::size_t b) {
    this->origin = a;
    this->dest = b;
}

std::size_t Renderer::square(std::int32_t x, std::int32_t y) {
    // Get an appropriate board index by dividing by the pixel size of the square.
    std::size_t rank = (y / this->pixels) * this->squares;
    std::size_t file = x / this->pixels;
    return rank + file;
}

Renderer::Renderer(std::size_t squares, std::size_t pixels) {
    // Attempt to initialise SDL, exit if unable.
    if(SDL_Init(SDL_INIT_VIDEO) < 0) exit(-1);

    int resolution = static_cast<int>(squares * pixels);
    constexpr std::uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_CreateWindowAndRenderer(resolution, resolution, flags, &this->window, &this->renderer);

    // If the SDL window or renderer couldn't initialise, exit.
    if(this->window == nullptr || this->renderer == nullptr) exit(-1);

    int scaledResolution; // Get the actual resolution that the OS has decided to use.
    SDL_GetRendererOutputSize(this->renderer, &scaledResolution, nullptr);
    this->scale = scaledResolution / resolution;

    // The value of these doesn't matter: only that they are
    // equal to each other to disable move highlighting.
    this->origin = this->dest;

    // Set remaining class members.
    this->squares = squares;
    this->pixels = pixels;

    // Load piece textures into an array.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
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
