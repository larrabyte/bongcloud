# bongcloud
A simple and elegant chess GUI written in C++ with [Centurion](https://github.com/albin-johansson/centurion).

## Dependencies
Make sure to execute `git clone` with the `--recurse-submodules` flag to install Centurion.
```
$ git clone --recurse-submodules https://github.com/larrabyte/bongcloud
```

Then, install `fmtlib` and `SDL2` (along with its extension libraries) using your favourite package manager.

To compile, simply invoke the Makefile using `make`. If you're on Windows, try sacrificing one of your PC components to ensure good luck before attempting to compile using MSVC. I managed to get it working at one point but for some reason it just kept throwing linker errors and I just gave up after a while.

## Additional Sprites
Bongcloud relies on the existence of a `data` folder to load the sprites for each piece. Each sprite should be in the form of a BMP file, with the prefix `w` for white pieces and `b` for black pieces.

- `p` corresponds to a pawn, eg. `wp.bmp` is the sprite for a white pawn.
- `n` corresponds to a knight, eg. `bn.bmp` is the sprite for a black knight.
- `b` corresponds to a bishop, eg. `wb.bmp` is the sprite for a white bishop.
- `r` corresponds to a rook, eg. `br.bmp` corresponds to a black rook.
- `q` corresponds to a queen, eg. `wq.bmp` corresponds to a white queen.
- `k` corresponds to a king, eg. `bk.bmp` corresponds to a black king.
