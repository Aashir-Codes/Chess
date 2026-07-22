# Engine & Game Logic

This document explains how the chess engine in **`engine.cpp`** works and how
the GUI in **`chess_gui.cpp`** builds on top of it. The GUI never re-implements
any chess rule — it reuses these functions and only handles drawing and input.

---

## 1. Board representation

```cpp
char board[8][8];   // 'P','N','B','R','Q','K' = white,  lower = black,  '.' = empty
bool turn;          // 1 = White to move, 0 = Black to move
```

- Row `0` is the top (Black's back rank), row `7` is the bottom (White's back rank).
- The starting position is read from `newgame.txt` by `FileToArray()`.
- King locations are cached in `K_r,K_c` (white) and `k_r,k_c` (black) and kept
  up to date by `check_king_White()` / `check_king_Black()`.

Colour of a square is decided by ASCII range in `check_BlackWhite(r,c)`:
`1` = white piece, `0` = black piece, `-1` = empty.

### Special-move state
```cpp
bool rook[4];   // has each corner rook moved?  (a1,h1,a8,h8)
bool king[2];   // has each king moved?          (used for castling rights)
int  epRow, epCol;   // square a pawn may capture "en passant" onto (-1 = none)
```

---

## 2. Move legality

Each piece type has a boolean function that answers *“can this piece move from
`(s_r,s_c)` to `(d_r,d_c)` on the current board?”*: `Pawn()`, `Knight()`,
`Bishop()`, `Rook()`, `Queen()`, `King()`. `checkpieces()` dispatches to the
right one based on the piece on the source square.

- **Pawn** handles the double first step, diagonal captures, and en passant
  (using `epRow/epCol`). The pawn's colour is read from the piece itself, not
  from `turn`, so the same function is reused during check detection of enemy
  pawns.
- **King** also encodes **castling**: if the king and the chosen rook haven't
  moved, the squares between are empty, and the king isn't passing through or
  landing on an attacked square (verified with `premove`), the castle is legal.
- **Queen** is simply `Bishop() || Rook()`.

These functions only check *geometry + path*. They do **not** check whether the
move leaves your own king in check — that's done separately.

---

## 3. Check, and "does this move expose my king?"

```cpp
bool check();                                   // is the side-to-move king attacked?
bool premove(int Sr, int Sc, int Dr, int Dc);   // would THIS move leave my king in check?
```

- `check()` scans every enemy piece and asks whether it can legally reach the
  friendly king's square. If any can, the king is in check.
- `premove()` **simulates** the move on the real board (temporarily moving the
  piece, handling en passant, updating the cached king position), runs the
  attack scan, then **restores** everything. It returns `true` if the move would
  be illegal (king left in check). This is the guard used before every real move.

---

## 4. Generating all legal moves

```cpp
int generateMoves(Move list[], int maxMoves);
```

Brute force but correct: for every friendly piece, for every target square, keep
the move if `checkpieces()` says the geometry is legal **and** `premove()` says
it doesn't expose the king. Returns the count.

`check_legal_movews()` is a lighter variant that just answers *“does the side to
move have ANY legal move?”* — the basis for mate/stalemate:

```cpp
bool checkmate();   //  check()  && no legal moves
bool stalemate();   // !check()  && no legal moves
```

---

## 5. Making a move

```cpp
void makeMove(Move m, bool autoPromoteQueen = false);
```

Applies a move to the board and updates all special-move state:
- moves the rook when castling,
- removes the passed pawn on en passant,
- sets `epRow/epCol` when a pawn double-steps,
- updates castling rights (`rook[]`, `king[]`) and the cached king position,
- handles **promotion**. When `autoPromoteQueen` is `true` it silently promotes
  to a queen; when `false` the *terminal* version prompts on `cin`.

> **GUI promotion:** the GUI shows its own popup, so it calls
> `makeMove(..., true)` (auto-queen, no `cin` blocking) and then overwrites the
> destination square with the piece the player actually picked. Calling it with
> `false` from the GUI would block the program on terminal input — that was a
> real bug that froze the game on promotion.

---

## 6. History: undo / redo / save

```cpp
char history[500][8][8];  bool Turn[500];  int Count, undo;
```

`History()` snapshots the board + whose turn it is after every move. `Undo()`
and `Redo()` step the `Count` cursor through those snapshots. `save()` /
`saveload()` write/read `Save.txt`.

---

## 7. The AI

All in `engine.cpp`, driven by a static board evaluation:

```cpp
int pieceValue(char);   // P=1 N=3 B=3 R=5 Q=9 K=1000
int evaluate();         // sum(white) - sum(black)   (positive favours White)
```

| Difficulty | Function | Idea |
|---|---|---|
| Easy | `pickRandomMove()` | random legal move |
| Medium | `pickGreedyMove()` | the move with the best *immediate* evaluation |
| Hard | `pickBestMoveMinimax(3)` | **Minimax** depth 3 with **alpha–beta pruning** |

`minimax()` recurses with `saveState()` / `restoreState()` around each trial
move (a full snapshot of board + flags + king positions + `turn`), so search
never corrupts the live game state. White maximizes, Black minimizes; terminal
nodes return `evaluate()`, mate returns ±100000, stalemate returns 0.

---

## 8. GUI architecture (`chess_gui.cpp`)

The GUI is a single Raylib loop with three screens: **MENU → GAME → RESULT**.
It keeps its own presentation state in `GuiState` (selection, last move, theme,
flip, timer, …) but all *rules* come from the engine above.

Key pieces:

- **Layout** — square size is derived from the monitor/window size, then the
  whole board+panel block is centered (`pickSquareSize` + `layoutCentered`), so
  it scales on any screen and in fullscreen.
- **Board flip** — a single `g_flip` flag. `squareRect()` and `pixelToSquare()`
  translate between **board coordinates** and **on-screen view coordinates**, so
  every other function keeps using plain board coords and flipping is free.
- **Captured tray** — counts each piece on the board and subtracts from the full
  starting set (`startCount`) to know what's been captured; material advantage is
  `Σwhite − Σblack` using `pieceValue`.
- **Timer** — per-side countdown; the side-to-move's clock decreases each frame,
  and hitting `0:00` ends the game on time.
- **Audio/themes/fonts** — loaded from `assets/`, with graceful fallback if a
  file is missing (silent SFX, default font, letter pieces).

The important design rule: **`chess_gui.cpp` draws; `engine.cpp` decides.**
