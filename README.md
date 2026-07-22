# ♟ Chess — Premium Edition

A full chess game written in C++ with a polished **Raylib GUI** and a classic
**terminal** version that share the exact same engine. It has legal-move
validation, castling, en passant, promotion, undo/redo, save/load, and a
Minimax AI with three difficulty levels.

![themes](assets/pieces/wN.png)

---

## ✨ Features

- **Full chess rules** — legal move generation, check / checkmate / stalemate,
  castling, **en passant**, and pawn **promotion** (choose Q/R/B/N).
- **Play modes** — Player vs Player, or Player vs Bot.
- **Minimax AI** — Easy (random), Medium (greedy), Hard (minimax + alpha–beta).
- **Premium GUI (Raylib)**
  - 3 switchable themes: **Emerald Glass**, **Wood Classic**, **Midnight Neon**
  - Sound effects (move, capture, check, castle, promote, game-over, UI click)
  - **Fullscreen** toggle (`F11` or the on-screen button) with responsive layout
  - **Board flip** — view from either side (auto-flips when you play Black)
  - **Captured-pieces tray** with live material advantage (`+N`)
  - **Optional chess clock** — Off / 3 / 5 / 10 minutes per side
  - Undo / Redo, Save / Load
- **Terminal version** — the original text UI, still fully playable.

---

## 🛠 Build & Run

### Requirements
- `g++` (C++17)
- **raylib** (only needed for the GUI). Install:
  - Arch/Manjaro: `sudo pacman -S raylib`
  - Ubuntu/Debian: `sudo apt install libraylib-dev` *(or build from source)*
  - From source: <https://github.com/raysan5/raylib>

### One command
```bash
./build.sh          # build the GUI      -> ./chess
./build.sh cli      # build terminal     -> ./chess-cli
./build.sh all      # build both
./build.sh run      # build the GUI and launch it
./build.sh clean    # remove the binaries
```

Then run:
```bash
./chess             # GUI  (run from THIS folder so it finds assets/)
./chess-cli         # terminal version
```

> ⚠️ **Important:** always run from the project folder. The GUI loads
> `assets/…` using **relative paths**, so launching from elsewhere shows
> letter-fallback pieces and no sound.

### Why a build script instead of `g++ *.cpp`?
`chess_gui.cpp` **`#include`s** `engine.cpp` to reuse the whole engine. If you
compile both files together (`g++ chess_gui.cpp engine.cpp`), `engine.cpp` gets
compiled twice and you get **duplicate-symbol / "multiple definition of main"**
errors. `build.sh` always compiles the correct single file. The GUI defines
`CHESS_GUI_BUILD` so the engine's terminal `main()` is switched off in that build.

---

## 🎮 Controls (GUI)

| Action | How |
|---|---|
| Select / move a piece | Left click the piece, then the destination |
| See legal moves | Dots = quiet moves, rings = captures |
| Fullscreen | `F11` or the `[ ]` button |
| Flip board | **Flip** button (auto-flips when you play Black) |
| Change theme | **Theme** button |
| Undo / Redo | panel buttons |
| Save / Load | **Save** button in game; **Load Saved Game** in the menu |
| Timer | choose on the main menu before starting |

---

## 📁 Project layout

```
Final Project Chess/
├── build.sh            # build helper (GUI + terminal)
├── chess_gui.cpp       # Raylib GUI  (includes engine.cpp)
├── engine.cpp          # game logic + AI + terminal main()  (shared core)
├── newgame.txt         # starting board layout (loaded at new game)
├── Save.txt            # runtime save (git-ignored)
├── logic.md            # how the engine & AI work
├── README.md
└── assets/
    ├── pieces/         # wK.png … bP.png  (+ svg sources)
    ├── fonts/          # heading.ttf, ui.ttf
    └── sounds/         # *.wav  (+ gen_sfx.py that generates them)
```

### Regenerating the sound effects
The SFX are synthesized offline with Python (standard library only):
```bash
cd assets/sounds && python3 gen_sfx.py
```

---

## 📝 Notes
- `engine.cpp` and `chess_gui.cpp` share one board representation and one set of
  rules — the GUI never re-implements chess logic, it just draws it.
- See **[logic.md](logic.md)** for the design of move generation, check
  detection, and the Minimax AI.
