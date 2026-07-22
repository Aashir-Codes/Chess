#!/usr/bin/env bash
# =============================================================
#  build.sh  —  one-command build for the chess project
#
#  Usage:
#    ./build.sh          build the GUI  ->  ./chess
#    ./build.sh cli      build terminal ->  ./chess-cli
#    ./build.sh all      build both
#    ./build.sh run      build the GUI and launch it
#    ./build.sh clean    remove the built binaries
#
#  WHY A SCRIPT?  chess_gui.cpp already #includes engine.cpp, so you must
#  compile ONLY chess_gui.cpp. Doing "g++ chess_gui.cpp engine.cpp" compiles
#  engine.cpp twice and fails with duplicate-symbol errors. This script uses
#  the correct single-file command every time.
# =============================================================
set -e
cd "$(dirname "$0")"

GUI_SRC="chess_gui.cpp"
CLI_SRC="engine.cpp"
GUI_OUT="chess"
CLI_OUT="chess-cli"

# raylib + the system libraries it needs on Linux
RAYLIB_LIBS="-lraylib -lGL -lm -lpthread -ldl -lrt -lX11"
CXXFLAGS="-O2 -std=c++17 -Wall -Wno-unused-variable -Wno-unused-but-set-variable"

check_raylib() {
    if ! echo '#include "raylib.h"
int main(){return 0;}' | g++ -x c++ - $RAYLIB_LIBS -o /tmp/_raylib_check 2>/dev/null; then
        echo "ERROR: raylib not found or not linkable."
        echo "  Install it, e.g.:"
        echo "    Arch/Manjaro : sudo pacman -S raylib"
        echo "    Ubuntu/Debian: sudo apt install libraylib-dev   (or build from source)"
        echo "    From source  : https://github.com/raysan5/raylib"
        exit 1
    fi
    rm -f /tmp/_raylib_check
}

build_gui() {
    check_raylib
    echo ">> Building GUI ($GUI_OUT) ..."
    g++ $CXXFLAGS "$GUI_SRC" -o "$GUI_OUT" $RAYLIB_LIBS
    echo "   done -> ./$GUI_OUT"
}

build_cli() {
    echo ">> Building terminal version ($CLI_OUT) ..."
    g++ $CXXFLAGS "$CLI_SRC" -o "$CLI_OUT"
    echo "   done -> ./$CLI_OUT"
}

case "${1:-gui}" in
    gui|"")   build_gui ;;
    cli)      build_cli ;;
    all)      build_gui; build_cli ;;
    run)      build_gui; echo ">> Launching ..."; ./"$GUI_OUT" ;;
    clean)    rm -f "$GUI_OUT" "$CLI_OUT"; echo "cleaned." ;;
    *)        echo "Unknown target '$1'. Use: gui | cli | all | run | clean"; exit 1 ;;
esac
