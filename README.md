# Smooth Fonts

## Overview

Create cpp source containing smoothed font information from a TrueType
font. 

The overall process uses both Windows and Linux programs.

There are no fonts included in this repo because I assume there are
license considerations. Using generated fonts for your own projects
seems like it should be okay. I'm sure there are free fonts that
could be include here but I have not looked.

## Requirements

### Windows

[BMFont](https://www.angelcode.com/products/bmfont/). This reads a
TrueType font file (maybe other kinds) and creates two files: a PNG
containing the glyphs, and a description file containing character
information, including where in the PNG a glyph is. I tried FontBuilder,
but the y-offset information seemed funky.

### Linux

cmake and libpng-dev, to build make\_font. make\_font is what takes
the output of BMFont and creates cpp and h files you can build into
your projects.

## Build

On Linux:

```
$ cd make_font
$ mkdir build
$ cmake -B build
$ make -C build
```

## Usage

1. On Windows, use BMFont to create the .fnt and .png files for a font.
2. Copy those files to your Linux machine.
3. Use make\_font to create cpp and h files for the font.
4. Copy those to wherever they are used, e.g. the arduino sketch herein.

### Example

Start BMFont.

Options/Font Settings:

<img src="images/bmfont_fontsettings.png" height="432">

Options/Export Options:

<img src="images/bmfont_exportoptions.png" height="488">

Options/Save bitmap font as..., and give it a nice filename,
e.g. consolas\_36 for this example.

You should now have consolas\_36.fnt and consolas\_36\_0.png.

That's all I've done with BMFont. It can write multiple "page" of pngs,
but make\_font assumes only one page, so the png file is always \*\_0.png.

Copy those two files to your Linux machine where you built make\_font.

$ build/make\_font consolas\_36

You should now have consolas\_36.h and consolas\_36.cpp. Use these
in your program, like in the arduino sample in this repo.

If you do all this and build a sketch that seems to render characters
nicely, but is rendering the wrong characters, go look at the .png
file you gave to make\_font and see if it has what looks like an ascii
character set. If not, try starting BMFont fresh, regenerating the .fnt
and .png files for the font, regenerating the .cpp and .h files with
make\_font, and rebuilding the sketch. In at least one instance, I
somehow ended up with a .png file for a font that had strange characters,
like a different code page or whatever. Most likely my mistake.

## Limitations

Check a font's licensing against how you plan to use it.

The font data created by make\_font should be pretty flexible as
far as how you can use it. There are limitations in the arduino
example as far as drawing only on a solid background, or how glyphs
might be clipped if they extend left/right too far (which many do).

No kerning information is used.

A smarter program could add kerning, or be able to draw glyphs that
overlap each other in the x direction.
