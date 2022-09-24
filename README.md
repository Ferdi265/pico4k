# PICO4K

The first 4K intro for the Raspberry Pi Pico.  
Wild compo entry released at Function 2022.

[![screenshot](https://content.pouet.net/files/screenshots/00092/00092223.png)](https://youtu.be/p3JvfWUPL2M)

- [youtube](https://youtu.be/p3JvfWUPL2M)
- [pouet.net](https://www.pouet.net/prod.php?which=92223)

This release comes with a new and shiny packer for Raspberry Pi Pico, see
`PICOPACK.md`.

## Platform

Raspberry Pi Pico - 264 KB RAM, 2 MB flash, 125+ MHz CPU (easily overclockable
to 250 MHz and more)

The [Pico Demo Base](https://shop.pimoroni.com/products/pimoroni-pico-vga-demo-base)
seems to be the standard way to do demo stuff on the Pico, it conveniently has
everything you need (VGA, PWM Audio, a few buttons). KiCad schematics are
[available](https://datasheets.raspberrypi.org/rp2040/VGA-KiCAD.zip) to build
your own since ordering it is expensive.

This intro only uses the passive parts of the Pico Demo Base, everything else
is unused.  
Used:
- the VGA "DAC" resistor ladder
- the PWM Audio resistor-capacitor signal path

## Building and Flashing

Dependencies:

- CMake
- arm-none-eabi-gcc (version 12 or later required! very scary constexpr magic required)
- [pico-sdk](https://github.com/raspberrypi/pico-sdk) (run `git submodule update --init`)
- [exomizer](https://bitbucket.org/magli143/exomizer) (designed for exomizer 3.1.1, shipped in `tools/exomizer/`)

Compile with:

```
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel .
cd build && make
```

Flash via any of these methods:

- `picotool load build/proj/pico4k.bin -t bin`
- `picotool load build/proj/pico4k.uf2`
- drag `pico4k.uf2` onto the Pico's virtual flash drive in bootloader mode

## Pinout for manual Setup

If you don't have a Pico Demo Board or just want to feel the joy of having a
mess of wires on a breadboard that works, here are the detailed pinouts:

- VGA Red: GPIO 0-4 (0 is LSB)
- not connected: GPIO 5
- VGA Green: GPIO 6-10 (6 is LSB)
- VGA Blue: GPIO 11-15 (11 is LSB)
- VGA HSync: GPIO 16
- VGA VSync: GPIO 17
- not connected: GPIO 18-26
- Audio Right: GPIO 27
- Audio Left: GPIO 28
