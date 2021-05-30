# bongcloud
A simple and elegant chess GUI written in C++ with SDL2.

## Data Folder
The executable relies on the existence of an external `data` folder residing in the same folder as the executable to load the sprites for each piece. Each sprite comes in the form of a BMP file, with the prefix `w` for white pieces and `b` for black pieces.

- `p` corresponds to a pawn, eg. `wp.bmp` is the sprite for a white pawn.
- `n` corresponds to a knight, eg. `bn.bmp` is the sprite for a black knight.
- `b` corresponds to a bishop, eg. `wb.bmp` is the sprite for a white bishop.
- `r` corresponds to a rook, eg. `br.bmp` corresponds to a black rook.
- `q` corresponds to a queen, eg. `wq.bmp` corresponds to a white queen.
- `k` corresponds to a king, eg. `bk.bmp` corresponds to a black king.

## Prerequisites and Compilation
Ensure that `SDL2` is installed before compiling.

```
$ sudo pacman -S sdl2 (on Arch Linux)
$ brew install sdl2 (on macOS)
```

To compile, simply invoke the Makefile using `make`. If you're on Windows, try sacrificing one of your PC components to ensure good luck before attempting to compile using MSVC. I managed to get it working at one point but for some reason it just kept throwing linker errors and I just gave up after a while.
