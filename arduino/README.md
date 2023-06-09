# Arduino CLI Scripts

I use scripts here to help me with the arduino cli tool. Everything
is from the Windows cmd.exe prompt.

Building and running font_demo should be easy to do with the IDE;
I just have not done that.

0. You may need git and python3 available at your cmd.exe prompt,
depending on what is being downloaded and run in ac-init. Installing
[earlephilhower arduino-pico](https://github.com/earlephilhower/arduino-pico)
(what I use for rp2040) requires both; not sure if either is required
for anything else.

1. Get [arduino-cli.exe](https://github.com/arduino/arduino-cli/releases)
and put it in this directory. This has to be available before running
ac-init.

2. Edit ac-init.bat to add/remove board manager URLs, and arduino
cores. Typically this means commenting/uncommenting one or more of
the "goto skip_..." lines to skip or not skip some setup.

3. Run ac-init.bat. It will download a bunch of stuff. Based on
the board you are using (in the "Install Cores" section), it will
also create a brd-\*.bat file.

4. Run the brd-\*.bat init file. All this does is set the FQBN in
your environment.

5. Run ac-build \<sketch-name\> \<com-port\> to build and download a
sketch. You can leave off com-port and it will just build.

6. Other stuff:
   * ac-list lists boards if arduino-cli can detect them.
   * ac-serial connects to a board's virtual serial port.
