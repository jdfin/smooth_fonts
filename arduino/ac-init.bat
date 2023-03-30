rmdir /s /q data downloads hardware
del config.yaml brd-*.bat

arduino-cli config init ^
    --dest-file .\config.yaml

arduino-cli config set directories.data "%cd%\data" ^
    --config-file .\config.yaml

arduino-cli config set directories.downloads "%cd%\downloads" ^
    --config-file .\config.yaml

arduino-cli config set directories.user "%cd%" ^
    --config-file .\config.yaml

::::: Add Board Manager URLs
::
:: Comment out a 'goto skip_...' line to add a URL.
::

goto skip_adafruit
arduino-cli config add board_manager.additional_urls ^
    https://adafruit.github.io/arduino-board-index/package_adafruit_index.json ^
    --config-file .\config.yaml
:skip_adafruit

goto skip_seeedstudio
arduino-cli config add board_manager.additional_urls ^
    https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json ^
    --config-file .\config.yaml
:skip_seeedstudio

goto skip_pjrc
arduino-cli config add board_manager.additional_urls ^
    https://www.pjrc.com/teensy/package_teensy_index.json ^
    --config-file .\config.yaml
:skip_pjrc

::goto skip_arduino_pico
:: For RP2040, I use the earlephilhower stuff.
:: It's not installed like other board managers.
mkdir hardware\pico
git clone https://github.com/earlephilhower/arduino-pico.git hardware/pico/rp2040
cd hardware/pico/rp2040
:: I pin it to a tag until I want to consciously change it
git checkout tags/3.1.0
git submodule update --init
cd pico-sdk
git submodule update --init
cd ..\tools
python3 .\get.py
cd ..\..\..\..
:skip_arduino_pico

::::: Install Cores
::
:: The hardest part about adding a new one of these is figuring out the FQBN.
:: Running the IDE to see what it uses is one way to find it.
::

arduino-cli core update-index ^
    --config-file .\config.yaml

goto skip_arduino_avr
arduino-cli core install --no-overwrite arduino:avr ^
    --config-file .\config.yaml
echo set FQBN=arduino:avr:leonardo > brd-beetle.bat
:skip_arduino_avr

goto skip_arduino_megaavr
arduino-cli core install --no-overwrite arduino:megaavr ^
    --config-file .\config.yaml
echo set FQBN=arduino:megaavr:uno2018 > brd-uno_wifi_r2.bat
:skip_arduino_megaavr

goto skip_adafruit_samd
arduino-cli core install --no-overwrite adafruit:samd ^
    --config-file .\config.yaml
echo set FQBN=adafruit:samd:adafruit_qtpy_m0 > brd-qtpy_m0.bat
::echo set FQBN=adafruit:samd:adafruit_feather_m4 > brd-feather_m4.bat
:skip_adafruit_samd

goto skip_seeeduino_samd
arduino-cli core install --no-overwrite Seeeduino:samd ^
    --config-file .\config.yaml
echo set FQBN=Seeeduino:samd:seeed_XIAO_m0 > brd-xiao.bat
:skip_seeeduino_samd

goto skip_teensy_avr
arduino-cli core install --no-overwrite teensy:avr ^
    --config-file .\config.yaml
echo set FQBN=teensy:avr:teensy40 > brd-teensy.bat
:skip_teensy_avr

::goto skip_rpi_pico
::arduino-cli core install --no-overwrite pico:rp2040 ^
::    --config-file .\config.yaml
echo set FQBN=pico:rp2040:rpipico:flash=2097152_0,freq=133,opt=Small,rtti=Disabled,dbgport=Disabled,dbglvl=None,usbstack=picosdk > brd-pico.bat
:skip_rpi_pico
