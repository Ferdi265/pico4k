#!/bin/bash -e

SCRIPT_DIR=$(dirname "$(realpath "${BASH_SOURCE[0]}")")

cd "$SCRIPT_DIR"
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel .

cd "$SCRIPT_DIR/build"
make -j8
picotool load -v "proj/pico4k.bin" -t bin
picotool reboot
