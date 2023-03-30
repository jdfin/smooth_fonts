# Waveshare 2.4 Inch LCD

This is an Arduino library for the
[Waveshare 2.4 Inch LCD module](https://www.waveshare.com/wiki/2.4inch_LCD_Module).

Waveshare has a
[library and examples](https://www.waveshare.com/w/upload/e/e9/LCD_Module_code.7z)
for the screen. The Arduino code is not very efficient with the SPI
bus, and seemed a bit awkward as far as extending it (e.g. chip select
usage, data/command might be set over here and assumed correct over
there). Some just looks wrong, like initialization commands that don't
have the right number of parameters. But it works.

What I wanted was the ability to easily add more fonts without typing
in bitmasks. This allows generating source for an arbitrary font from
one on your PC, then rendering it on the LCD. There are no other
graphics operations (e.g. lines, circles), just rendering smoothed
text. The only part of the Waveshare software used here is the values
written to various registers at init time. Some doesn't look right,
but I don't think one can fix it (or see that it's correct) without
more information from Waveshare and/or errata from ILITEK.

If you need more features, you can:
* Add them here.
* Use the Waveshare software to get lines and circles.
* Get a display from Adafruit and use their software. I don't think they
  they do smoothed fonts, but it's probably the most complete package
  otherwise.

I don't know if you can use one vendor's ILI9341 software on another
vendor's ILI9341-based module. Some of the initialization appears to
be screen-specific. Plus you should really buy the hardware from
whoever wrote the software.

# Usage

Create one or more fonts. See the README at the top level of this repo.
Put the font(s) in arduino/library/fonts/.

Create a sketch similar to "LCD Demo", or modify it to use your font
and your gpios.

I only use arduino-cli (not the gui). There are some scripts in the
arduino directory that set things up and build/download a sketch.
See arduino/README for more.
