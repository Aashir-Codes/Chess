// =============================================================
//  chess_gui.cpp  —  Raylib GUI  (Premium themes, SFX, fullscreen,
//                     board flip, captured tray, optional timer)
//
//  BUILD:  ./build.sh          (or see the exact g++ line inside it)
//  Single command:
//    g++ chess_gui.cpp -o chess -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
//
//  This file reuses the whole game engine by #including engine.cpp.
//  We #define CHESS_GUI_BUILD first so engine.cpp's terminal main()
//  is switched off (otherwise there would be two main() functions).
//
//  CONTROLS:
//    Left click        select / move
//    F11  or  [ ]      toggle fullscreen
//    Theme button      cycle Emerald / Wood / Neon
//    Flip button       flip the board view
// =============================================================

#define CHESS_GUI_BUILD
#include "raylib.h"
#include "engine.cpp"
#include <cstring>
#include <cmath>

// =============================================================
//  RESPONSIVE LAYOUT
// =============================================================
struct Layout {
    int square, boardPx, margin, labelW;
    int boardX, boardY;
    int panelX, panelW;
    int winW, winH;
    int fontSize;
    int screenW, screenH;
};
static Layout L;

static int contentWidth()  { return L.labelW + L.margin + L.boardPx + L.margin + L.panelW + L.margin/2; }
static int contentHeight() { return L.labelW + L.margin + L.boardPx + L.labelW + L.margin; }

static void recomputeDerived()
{
    L.boardPx  = L.square * 8;
    L.margin   = L.square / 3;
    L.labelW   = L.square / 4;
    L.panelW   = (int)(L.square * 3.2f);
    L.fontSize = L.square / 6;
    if (L.fontSize < 14) L.fontSize = 14;
}

void pickSquareSize(int availW, int availH)
{
    int boardH = (int)(availH * 0.82f);
    L.square   = (boardH / 8) & ~1;
    if (L.square < 40)  L.square = 40;
    if (L.square > 130) L.square = 130;
    recomputeDerived();
    while (contentWidth() > availW && L.square > 40) {
        L.square -= 2;
        recomputeDerived();
    }
}

void layoutCentered(int screenW, int screenH)
{
    L.screenW = screenW;
    L.screenH = screenH;
    int cw = contentWidth();
    int ch = contentHeight();
    int offX = (screenW - cw) / 2; if (offX < 0) offX = 0;
    int offY = (screenH - ch) / 2; if (offY < 0) offY = 0;
    L.boardX = offX + L.labelW + L.margin;
    L.boardY = offY + L.labelW + L.margin;
    L.panelX = L.boardX + L.boardPx + L.margin;
    L.winW   = cw;
    L.winH   = ch;
}

// =============================================================
//  THEMES  (Emerald Glass default; Wood Classic; Midnight Neon)
// =============================================================
struct Theme {
    const char* name;
    Color bgTop, bgBot;
    Color lightSq, darkSq, boardEdge;
    Color panelBg, panelBorder, glass;
    Color btnIdle, btnHov, btnAct, btnBorder;
    Color txtMain, txtDim, accent;
    Color sel, dot, capture, checkCol, lastMove, glow;
    Color whitePiece, blackPiece;
};

static const Theme THEMES[3] = {
    {   "Emerald",
        { 18, 26, 24, 255 }, { 8, 14, 12, 255 },
        { 234, 233, 210, 255 }, { 74, 120, 86, 255 }, { 30, 44, 38, 255 },
        { 22, 30, 27, 235 }, { 60, 90, 74, 255 }, { 255, 255, 255, 16 },
        { 34, 48, 42, 255 }, { 52, 74, 62, 255 }, { 76, 168, 112, 255 }, { 70, 110, 88, 255 },
        { 235, 240, 236, 255 }, { 140, 160, 148, 255 }, { 88, 214, 141, 255 },
        { 246, 214, 92, 130 }, { 96, 210, 130, 210 }, { 214, 92, 84, 190 },
        { 220, 60, 55, 150 }, { 246, 214, 92, 90 }, { 88, 214, 141, 120 },
        { 248, 248, 244, 255 }, { 26, 30, 28, 255 },
    },
    {   "Wood",
        { 46, 34, 24, 255 }, { 24, 17, 11, 255 },
        { 240, 217, 181, 255 }, { 156, 104, 66, 255 }, { 60, 40, 26, 255 },
        { 40, 28, 20, 255 }, { 96, 70, 46, 255 }, { 255, 240, 210, 18 },
        { 62, 44, 30, 255 }, { 88, 62, 40, 255 }, { 176, 130, 74, 255 }, { 110, 80, 52, 255 },
        { 244, 232, 214, 255 }, { 176, 150, 120, 255 }, { 214, 170, 96, 255 },
        { 246, 210, 110, 140 }, { 120, 90, 60, 210 }, { 200, 90, 70, 190 },
        { 210, 70, 60, 150 }, { 246, 210, 110, 95 }, { 214, 170, 96, 110 },
        { 250, 244, 232, 255 }, { 40, 26, 16, 255 },
    },
    {   "Neon",
        { 14, 16, 30, 255 }, { 4, 5, 12, 255 },
        { 60, 66, 104, 255 }, { 30, 34, 60, 255 }, { 18, 20, 40, 255 },
        { 16, 18, 34, 235 }, { 60, 70, 140, 255 }, { 120, 160, 255, 20 },
        { 26, 30, 56, 255 }, { 40, 48, 88, 255 }, { 70, 120, 240, 255 }, { 60, 80, 170, 255 },
        { 228, 234, 255, 255 }, { 130, 140, 190, 255 }, { 90, 200, 255, 255 },
        { 90, 220, 255, 130 }, { 90, 210, 255, 210 }, { 255, 90, 150, 190 },
        { 255, 70, 120, 160 }, { 90, 200, 255, 90 }, { 90, 220, 255, 130 },
        { 240, 244, 255, 255 }, { 20, 24, 44, 255 },
    },
};
static Theme T = THEMES[0];

// =============================================================
//  ASSETS: piece textures, fonts, sounds
// =============================================================
static Texture2D tex[128] = {};
static Font fontHeading, fontUI;

enum { SFX_MOVE, SFX_CAPTURE, SFX_CHECK, SFX_CASTLE, SFX_PROMOTE, SFX_OVER, SFX_CLICK, SFX_COUNT };
static Sound sfx[SFX_COUNT];
static bool  audioReady = false;

void loadTextures()
{
    const char* names[12] = {"wK","wQ","wR","wB","wN","wP","bK","bQ","bR","bB","bN","bP"};
    const char  chars[12] = {'K','Q','R','B','N','P','k','q','r','b','n','p'};
    char path[64];
    for (int i = 0; i < 12; i++) {
        snprintf(path, sizeof(path), "assets/pieces/%s.png", names[i]);
        if (FileExists(path)) {
            tex[(int)chars[i]] = LoadTexture(path);
            SetTextureFilter(tex[(int)chars[i]], TEXTURE_FILTER_BILINEAR);
        }
    }
}
void unloadTextures()
{
    const char* chars = "KQRBNPkqrbnp";
    for (int i = 0; chars[i]; i++)
        if (tex[(int)chars[i]].id > 0) UnloadTexture(tex[(int)chars[i]]);
}

void loadFonts()
{
    fontHeading = FileExists("assets/fonts/heading.ttf")
                ? LoadFontEx("assets/fonts/heading.ttf", 96, 0, 250) : GetFontDefault();
    fontUI      = FileExists("assets/fonts/ui.ttf")
                ? LoadFontEx("assets/fonts/ui.ttf", 64, 0, 250) : GetFontDefault();
    SetTextureFilter(fontHeading.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fontUI.texture, TEXTURE_FILTER_BILINEAR);
}
void unloadFonts()
{
    if (fontHeading.texture.id != GetFontDefault().texture.id) UnloadFont(fontHeading);
    if (fontUI.texture.id != GetFontDefault().texture.id)      UnloadFont(fontUI);
}

void loadSounds()
{
    const char* files[SFX_COUNT] = {
        "assets/sounds/move.wav",   "assets/sounds/capture.wav",
        "assets/sounds/check.wav",  "assets/sounds/castle.wav",
        "assets/sounds/promote.wav","assets/sounds/gameover.wav",
        "assets/sounds/click.wav",
    };
    for (int i = 0; i < SFX_COUNT; i++)
        if (FileExists(files[i])) sfx[i] = LoadSound(files[i]);
}
void unloadSounds()
{
    for (int i = 0; i < SFX_COUNT; i++)
        if (sfx[i].frameCount > 0) UnloadSound(sfx[i]);
}
void playSfx(int i)
{
    if (audioReady && i >= 0 && i < SFX_COUNT && sfx[i].frameCount > 0) PlaySound(sfx[i]);
}
void playResultSound(bool capture, bool castle, bool promote)
{
    if (checkmate() || stalemate()) { playSfx(SFX_OVER);    return; }
    if (check())                    { playSfx(SFX_CHECK);   return; }
    if (promote)                    { playSfx(SFX_PROMOTE); return; }
    if (castle)                     { playSfx(SFX_CASTLE);  return; }
    if (capture)                    { playSfx(SFX_CAPTURE); return; }
    playSfx(SFX_MOVE);
}

// =============================================================
//  TEXT + SHAPE HELPERS
// =============================================================
void gtext(const char* s, int x, int y, int size, Color c, bool heading = false)
{
    DrawTextEx(heading ? fontHeading : fontUI, s, { (float)x, (float)y }, (float)size, 1.0f, c);
}
float gtextW(const char* s, int size, bool heading = false)
{
    return MeasureTextEx(heading ? fontHeading : fontUI, s, (float)size, 1.0f).x;
}
void gtextCentered(const char* s, int cx, int y, int size, Color c, bool heading = false)
{
    gtext(s, cx - (int)(gtextW(s, size, heading) / 2), y, size, c, heading);
}
void glassCard(Rectangle r, float round, Color fill, Color border)
{
    DrawRectangleRounded(r, round, 12, fill);
    Rectangle sheen = { r.x, r.y, r.width, r.height * 0.34f };
    DrawRectangleRounded(sheen, round, 12, T.glass);
    DrawRectangleRoundedLinesEx(r, round, 12, 1.5f, border);
}

// =============================================================
//  BOARD ORIENTATION (flip)
//  g_flip=true  → black at the bottom.  squareRect/pixelToSquare
//  translate between board coords and on-screen view coords, so
//  the rest of the code keeps using plain board coordinates.
// =============================================================
static bool g_flip = false;

Rectangle squareRect(int row, int col)
{
    int vr = g_flip ? 7 - row : row;
    int vc = g_flip ? 7 - col : col;
    return { (float)(L.boardX + vc * L.square),
             (float)(L.boardY + vr * L.square),
             (float)L.square, (float)L.square };
}
bool pixelToSquare(int px, int py, int& row, int& col)
{
    int bx = px - L.boardX;
    int by = py - L.boardY;
    if (bx < 0 || by < 0 || bx >= L.boardPx || by >= L.boardPx) return false;
    int vc = bx / L.square;
    int vr = by / L.square;
    row = g_flip ? 7 - vr : vr;
    col = g_flip ? 7 - vc : vc;
    return true;
}

// =============================================================
//  DRAW PIECE  (with soft drop shadow)
// =============================================================
void drawPiece(char piece, int row, int col)
{
    if (piece == '.') return;
    Rectangle r  = squareRect(row, col);
    Texture2D t  = tex[(int)piece];
    if (t.id > 0) {
        float pad  = L.square * 0.07f;
        float size = L.square - pad * 2;
        float off  = L.square * 0.04f;
        DrawTexturePro(t, { 0,0,(float)t.width,(float)t.height },
            { r.x + pad + off, r.y + pad + off*1.4f, size, size }, { 0,0 }, 0.0f, { 0,0,0,90 });
        DrawTexturePro(t, { 0,0,(float)t.width,(float)t.height },
            { r.x + pad, r.y + pad, size, size }, { 0,0 }, 0.0f, WHITE);
    } else {
        Color c  = isupper(piece) ? T.whitePiece : T.blackPiece;
        int   fs = (int)(L.square * 0.5f);
        char  buf[2] = { piece, 0 };
        int   tx = (int)(r.x + L.square/2 - gtextW(buf, fs, true)/2);
        int   ty = (int)(r.y + L.square/2 - fs/2);
        gtext(buf, tx+2, ty+2, fs, { 0,0,0,120 }, true);
        gtext(buf, tx,   ty,   fs, c, true);
    }
}

// Draw one piece image scaled to fit a box (used by tray + promo popup)
void drawPieceIcon(char piece, float x, float y, float w, float h)
{
    Texture2D t = tex[(int)piece];
    if (t.id > 0)
        DrawTexturePro(t, { 0,0,(float)t.width,(float)t.height }, { x, y, w, h }, { 0,0 }, 0, WHITE);
    else {
        char buf[2] = { piece, 0 };
        gtext(buf, (int)(x + w/2 - gtextW(buf,(int)h,true)/2), (int)y, (int)h,
              isupper(piece) ? T.whitePiece : T.blackPiece, true);
    }
}

// =============================================================
//  BUTTON
// =============================================================
bool drawButton(const char* label, int x, int y, int w, int h, bool active = false, int fontSize = -1)
{
    Rectangle r    = { (float)x, (float)y, (float)w, (float)h };
    Vector2 mouse  = GetMousePosition();
    bool hover     = CheckCollisionPointRec(mouse, r);
    bool clicked   = hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    float round = 0.28f;
    if (hover) {
        Rectangle g = { r.x - 3, r.y - 3, r.width + 6, r.height + 6 };
        DrawRectangleRounded(g, round, 10, { T.accent.r, T.accent.g, T.accent.b, 40 });
    }
    Color face = active ? T.btnAct : (hover ? T.btnHov : T.btnIdle);
    DrawRectangleRounded(r, round, 10, face);
    DrawRectangleRoundedLinesEx(r, round, 10, 1.5f, active ? T.accent : T.btnBorder);

    int fs = (fontSize > 0) ? fontSize : L.fontSize;
    int pad = (int)(h * 0.30f);
    while (fs > 8 && gtextW(label, fs) > w - pad) fs--;   // shrink so text never spills
    Color tc = active ? Color{ 12, 16, 14, 255 } : T.txtMain;
    gtextCentered(label, x + w/2, y + (h - fs)/2, fs, tc);

    if (clicked) playSfx(SFX_CLICK);
    return clicked;
}
bool drawIconButton(const char* glyph, int x, int y, int s, bool active = false)
{
    return drawButton(glyph, x, y, s, s, active, (int)(s * 0.5f));
}

// =============================================================
//  GUI STATE
// =============================================================
struct GuiState {
    enum Screen { MENU, GAME, RESULT } screen = MENU;
    enum Mode   { PVP,  PVB  }        mode   = PVP;

    bool hasSel     = false;
    int  selRow     = -1, selCol = -1;
    Move legalMoves[256];
    int  legalCount = 0;

    int  lastSR = -1, lastSC = -1, lastDR = -1, lastDC = -1;

    bool humanIsWhite = true;
    int  difficulty   = 2;

    bool awaitingPromo = false;
    int  promoRow = -1, promoCol = -1;
    char promoPiece = 'P';

    const char* resultMsg = "";

    float botDelay = 0.0f;
    bool  botMoved = false;

    int   themeIdx   = 0;
    bool  fullscreen = false;
    bool  flip       = false;

    // ---- timer ----
    int   timerMode  = 0;         // 0=Off, 1=3min, 2=5min, 3=10min
    bool  timerOn    = false;
    float whiteClock = 0, blackClock = 0;
};

static const int TIMER_MINUTES[4] = { 0, 3, 5, 10 };
static const char* TIMER_LABELS[4] = { "Off", "3 min", "5 min", "10 min" };

// =============================================================
//  LEGAL MOVE CACHE
// =============================================================
void rebuildLegal(GuiState& g)
{
    g.legalCount = 0;
    if (!g.hasSel) return;
    Move all[256];
    int n = generateMoves(all, 256);
    for (int i = 0; i < n; i++)
        if (all[i].s_r == g.selRow && all[i].s_c == g.selCol)
            g.legalMoves[g.legalCount++] = all[i];
}
bool isLegalDest(const GuiState& g, int row, int col)
{
    for (int i = 0; i < g.legalCount; i++)
        if (g.legalMoves[i].d_r == row && g.legalMoves[i].d_c == col) return true;
    return false;
}

// =============================================================
//  INIT GAME
// =============================================================
void startGame(GuiState& g)
{
    FileToArray();
    check_king_White(); check_king_Black();
    turn = 1; Count = 0; undo = 0;
    Stalemate = 0; Exit = 0;
    epRow = -1; epCol = -1;
    for (int i = 0; i < 4; i++) rook[i] = 0;
    for (int i = 0; i < 2; i++) king[i] = 0;
    History();

    g.hasSel = false; g.legalCount = 0;
    g.awaitingPromo = false; g.resultMsg = "";
    g.lastSR = g.lastSC = g.lastDR = g.lastDC = -1;
    g.botDelay = 0.0f; g.botMoved = false;

    // auto-flip so the human's pieces sit at the bottom
    g.flip = (g.mode == GuiState::PVB && !g.humanIsWhite);

    // timer
    g.timerOn = (g.timerMode > 0);
    g.whiteClock = g.blackClock = TIMER_MINUTES[g.timerMode] * 60.0f;

    g.screen = GuiState::GAME;
}

// =============================================================
//  GAME OVER (mate / stalemate)
// =============================================================
void checkGameOver(GuiState& g)
{
    if (checkmate()) {
        g.resultMsg = (turn == 1) ? "Black wins by checkmate!" : "White wins by checkmate!";
        g.screen = GuiState::RESULT;
    } else if (stalemate()) {
        g.resultMsg = "Stalemate - it's a draw!";
        g.screen = GuiState::RESULT;
    }
}
// Flag on time-out.
void checkTimeout(GuiState& g)
{
    if (!g.timerOn) return;
    if (g.whiteClock <= 0) {
        g.whiteClock = 0; g.resultMsg = "Black wins on time!";
        g.screen = GuiState::RESULT; playSfx(SFX_OVER);
    } else if (g.blackClock <= 0) {
        g.blackClock = 0; g.resultMsg = "White wins on time!";
        g.screen = GuiState::RESULT; playSfx(SFX_OVER);
    }
}

// =============================================================
//  APPLY MOVES
// =============================================================
void applyHumanMove(GuiState& g, int dr, int dc)
{
    char piece = board[g.selRow][g.selCol];
    bool isPromo = ((piece == 'P' && dr == 0) || (piece == 'p' && dr == 7));
    if (isPromo) {
        g.awaitingPromo = true;
        g.promoRow = dr; g.promoCol = dc; g.promoPiece = piece;
        return;
    }
    bool isPawn  = (piece == 'P' || piece == 'p');
    bool capture = (board[dr][dc] != '.') || (isPawn && dc != g.selCol && board[dr][dc] == '.');
    bool castle  = (piece == 'K' || piece == 'k') && abs(dc - g.selCol) == 2;

    History();
    makeMove({ g.selRow, g.selCol, dr, dc }, false);
    check_king_White(); check_king_Black();
    turn = !turn;
    History();

    playResultSound(capture, castle, false);
    g.lastSR = g.selRow; g.lastSC = g.selCol;
    g.lastDR = dr;       g.lastDC = dc;
    g.hasSel = false; g.legalCount = 0;
    g.botDelay = 0.35f;
}
void applyPromotion(GuiState& g, char chosen)
{
    bool capture = (board[g.promoRow][g.promoCol] != '.');
    History();
    // autoPromoteQueen=true so makeMove does NOT open the terminal promo()
    // prompt and block on cin. We overwrite with the GUI choice next line.
    makeMove({ g.selRow, g.selCol, g.promoRow, g.promoCol }, true);
    board[g.promoRow][g.promoCol] = chosen;
    check_king_White(); check_king_Black();
    turn = !turn;
    History();

    playResultSound(capture, false, true);
    g.lastSR = g.selRow;   g.lastSC = g.selCol;
    g.lastDR = g.promoRow; g.lastDC = g.promoCol;
    g.hasSel = false; g.legalCount = 0;
    g.awaitingPromo = false;
    g.botDelay = 0.35f;
}
void doBotTurn(GuiState& g)
{
    Move m;
    if      (g.difficulty == 1) m = pickRandomMove();
    else if (g.difficulty == 2) m = pickGreedyMove();
    else                        m = pickBestMoveMinimax(3);

    char piece   = board[m.s_r][m.s_c];
    bool isPawn  = (piece == 'P' || piece == 'p');
    bool capture = (board[m.d_r][m.d_c] != '.') || (isPawn && m.d_c != m.s_c && board[m.d_r][m.d_c] == '.');
    bool castle  = (piece == 'K' || piece == 'k') && abs(m.d_c - m.s_c) == 2;
    bool promote = (piece == 'P' && m.d_r == 0) || (piece == 'p' && m.d_r == 7);

    History();
    makeMove(m, true);
    check_king_White(); check_king_Black();
    turn = !turn;
    History();

    playResultSound(capture, castle, promote);
    g.lastSR = m.s_r; g.lastSC = m.s_c;
    g.lastDR = m.d_r; g.lastDC = m.d_c;
    g.botMoved = true;
}

// =============================================================
//  HANDLE BOARD CLICK
// =============================================================
void handleClick(GuiState& g, int row, int col)
{
    if (g.awaitingPromo) return;
    if (!g.hasSel) {
        if (board[row][col] == '.') return;
        if (check_BlackWhite(row, col) != (int)turn) return;
        g.hasSel = true; g.selRow = row; g.selCol = col;
        rebuildLegal(g);
        return;
    }
    if (board[row][col] != '.' && check_BlackWhite(row, col) == (int)turn) {
        g.selRow = row; g.selCol = col;
        rebuildLegal(g);
        return;
    }
    if (!isLegalDest(g, row, col)) { g.hasSel = false; g.legalCount = 0; return; }
    applyHumanMove(g, row, col);
    checkGameOver(g);
}

// =============================================================
//  CAPTURED PIECES  (computed from the board vs the full starting set)
// =============================================================
static int startCount(char p)
{
    char l = tolower(p);
    if (l == 'p') return 8;
    if (l == 'q' || l == 'k') return 1;
    return 2;  // rook / knight / bishop
}

// Draw a row of the pieces a side has captured. `blackPieces=true` shows the
// black men White has taken. Returns the material advantage of that side.
void drawCapturedTray(int x, int y, int w, int iconH, const int cnt[128], bool blackPieces)
{
    const char whiteOrder[5] = { 'Q','R','B','N','P' };
    const char blackOrder[5] = { 'q','r','b','n','p' };
    const char* order = blackPieces ? blackOrder : whiteOrder;

    float cx = (float)x;
    float step = iconH * 0.60f;
    for (int k = 0; k < 5; k++) {
        char pc = order[k];
        int missing = startCount(pc) - cnt[(int)pc];
        for (int m = 0; m < missing && cx < x + w - iconH; m++) {
            drawPieceIcon(pc, cx, (float)y, (float)iconH, (float)iconH);
            cx += step;
        }
    }
}

// =============================================================
//  DRAW BOARD
// =============================================================
void drawBoard(const GuiState& g)
{
    Rectangle frame = { (float)(L.boardX - L.margin*0.5f), (float)(L.boardY - L.margin*0.5f),
                        (float)(L.boardPx + L.margin), (float)(L.boardPx + L.margin) };
    DrawRectangleRounded({ frame.x+4, frame.y+6, frame.width, frame.height }, 0.06f, 8, { 0,0,0,120 });
    DrawRectangleRounded(frame, 0.06f, 8, T.boardEdge);

    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++) {
            Rectangle r = squareRect(row, col);
            DrawRectangleRec(r, ((row + col) % 2 == 0) ? T.lightSq : T.darkSq);
            if ((row == g.lastSR && col == g.lastSC) || (row == g.lastDR && col == g.lastDC))
                DrawRectangleRec(r, T.lastMove);
        }

    if (check()) {
        int kr = (turn == 1) ? K_r : k_r;
        int kc = (turn == 1) ? K_c : k_c;
        Rectangle r = squareRect(kr, kc);
        DrawRectangleRec(r, T.checkCol);
        DrawCircleV({ r.x + L.square/2.0f, r.y + L.square/2.0f },
                    L.square * 0.62f, { T.checkCol.r, T.checkCol.g, T.checkCol.b, 60 });
    }
    if (g.hasSel) {
        Rectangle r = squareRect(g.selRow, g.selCol);
        DrawRectangleRec(r, T.sel);
        DrawRectangleLinesEx(r, 2.0f, T.accent);
    }
    for (int i = 0; i < g.legalCount; i++) {
        int dr = g.legalMoves[i].d_r, dc = g.legalMoves[i].d_c;
        Rectangle r = squareRect(dr, dc);
        float cx = r.x + L.square/2.0f, cy = r.y + L.square/2.0f;
        if (board[dr][dc] != '.')
            DrawRing({ cx, cy }, L.square/2.0f - 6, L.square/2.0f - 1, 0, 360, 40, T.capture);
        else
            DrawCircle((int)cx, (int)cy, L.square / 7.0f, T.dot);
    }
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            drawPiece(board[row][col], row, col);

    // rank + file labels (respect flip)
    int lfs = L.fontSize - 2;
    for (int i = 0; i < 8; i++) {
        int br = g_flip ? 7 - i : i;                 // board row shown at view row i
        gtext(TextFormat("%d", 8 - br),
              L.boardX - L.labelW, L.boardY + i * L.square + L.square/2 - lfs/2, lfs, T.txtDim);
    }
    const char* files = "abcdefgh";
    for (int i = 0; i < 8; i++) {
        int bc = g_flip ? 7 - i : i;
        char buf[2] = { files[bc], 0 };
        gtext(buf, L.boardX + i * L.square + L.square/2 - (int)(gtextW(buf, lfs)/2),
              L.boardY + L.boardPx + 6, lfs, T.txtDim);
    }
}

// =============================================================
//  SIDE PANEL  → returns an action char
// =============================================================
char drawPanel(GuiState& g)
{
    Rectangle panel = { (float)L.panelX, (float)(L.boardY - L.margin*0.5f),
                        (float)L.panelW, (float)(L.boardPx + L.margin) };
    glassCard(panel, 0.05f, T.panelBg, T.panelBorder);

    int px  = L.panelX + L.square / 3;
    int py  = (int)panel.y + L.margin / 2;
    int bw  = L.panelW - 2 * (L.square / 3);
    int bh  = (int)(L.square * 0.46f);
    int gap = bh / 5;
    int fs  = L.fontSize;
    char action = 0;

    // Title + fullscreen icon
    int titleFs = (int)(fs * 1.6f);
    gtext("CHESS", px, py, titleFs, T.accent, true);
    int iconS = (int)(fs * 1.5f);
    if (drawIconButton(g.fullscreen ? "x" : "[]",
                       (int)(panel.x + panel.width) - iconS - L.square/3, py, iconS)) action = 'f';
    py += titleFs + gap;

    // ---- captured material for both sides ----
    int cnt[128] = {0};
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (board[i][j] != '.') cnt[(int)board[i][j]]++;

    int matW = 0, matB = 0;
    const char up[5] = { 'Q','R','B','N','P' };
    for (int k = 0; k < 5; k++) {
        matW += cnt[(int)up[k]]        * pieceValue(up[k]);
        matB += cnt[(int)tolower(up[k])] * pieceValue(up[k]);
    }
    int matDiff = matW - matB;
    int trayH = (int)(fs * 1.3f);

    // top tray = pieces WHITE captured (black men), with white's advantage
    gtext("Captured", px, py, fs - 3, T.txtDim); py += fs - 1;
    drawCapturedTray(px, py, bw - fs, trayH, cnt, true);
    if (matDiff > 0) gtext(TextFormat("+%d", matDiff), px + bw - fs*2, py + 2, fs, T.txtMain);
    py += trayH + 2;
    drawCapturedTray(px, py, bw - fs, trayH, cnt, false);
    if (matDiff < 0) gtext(TextFormat("+%d", -matDiff), px + bw - fs*2, py + 2, fs, T.txtMain);
    py += trayH + gap;

    // ---- clocks (only when timer is on) ----
    if (g.timerOn) {
        int cw = (bw - gap) / 2;
        int ch = (int)(bh * 0.9f);
        // white clock
        bool wActive = (turn == 1);
        DrawRectangleRounded({ (float)px, (float)py, (float)cw, (float)ch }, 0.3f, 8,
                             wActive ? T.btnHov : T.btnIdle);
        gtextCentered(TextFormat("%d:%02d", (int)ceilf(g.whiteClock)/60, (int)ceilf(g.whiteClock)%60),
                      px + cw/2, py + (ch-fs)/2, fs, wActive ? T.accent : T.txtMain);
        // black clock
        int bx = px + cw + gap;
        bool bActive = (turn == 0);
        DrawRectangleRounded({ (float)bx, (float)py, (float)cw, (float)ch }, 0.3f, 8,
                             bActive ? T.btnHov : T.btnIdle);
        gtextCentered(TextFormat("%d:%02d", (int)ceilf(g.blackClock)/60, (int)ceilf(g.blackClock)%60),
                      bx + cw/2, py + (ch-fs)/2, fs, bActive ? T.accent : T.txtMain);
        py += ch + gap;
    }

    // ---- turn + check ----
    const char* turnStr = (turn == 1) ? "White to move" : "Black to move";
    DrawCircle(px + 6, py + fs/2, fs*0.32f, (turn == 1) ? Color{240,240,240,255} : Color{40,40,48,255});
    gtext(turnStr, px + (int)(fs*0.9f), py, fs, T.txtMain);
    if (check()) gtext("CHECK!", px + bw - (int)gtextW("CHECK!",fs) , py, fs, { 255, 110, 110, 255 });
    py += fs + gap;

    // ---- buttons (two per row where sensible) ----
    int half = (bw - gap) / 2;
    if (drawButton("Undo", px, py, half, bh)) action = 'u';
    if (drawButton("Redo", px + half + gap, py, half, bh)) action = 'r';
    py += bh + gap;
    if (drawButton("Save", px, py, half, bh)) action = 's';
    if (drawButton("Flip", px + half + gap, py, half, bh, g.flip)) action = 'x';
    py += bh + gap;
    if (drawButton(TextFormat("Theme: %s", T.name), px, py, bw, bh)) action = 't';
    py += bh + gap;
    if (drawButton("Main Menu", px, py, bw, bh)) action = 'm';
    py += bh + gap;

    if (g.mode == GuiState::PVB) {
        const char* diffs[4] = { "", "Easy", "Medium", "Hard" };
        gtext(TextFormat("Bot: %s   You: %s", diffs[g.difficulty],
                         g.humanIsWhite ? "White" : "Black"),
              px, py, fs - 2, T.txtDim);
    }
    return action;
}

// =============================================================
//  PROMOTION POPUP
// =============================================================
char drawPromoPopup(bool isWhite)
{
    int bw  = (int)(L.square * 1.0f);
    int bh  = (int)(L.square * 0.9f);
    int pad = 14;
    int pw  = 4 * bw + 5 * pad;
    int ph  = bh + 74;
    int px  = L.boardX + (L.boardPx - pw) / 2;
    int py  = L.boardY + (L.boardPx - ph) / 2;

    DrawRectangle(0, 0, L.screenW, L.screenH, { 0, 0, 0, 120 });
    Rectangle box = { (float)px, (float)py, (float)pw, (float)ph };
    DrawRectangleRounded({ box.x+5, box.y+7, box.width, box.height }, 0.14f, 10, { 0,0,0,140 });
    glassCard(box, 0.14f, T.panelBg, T.accent);
    gtextCentered("Promote your pawn", px + pw/2, py + 14, L.fontSize, T.txtMain);

    const char* labels[4] = { "Queen", "Rook", "Bishop", "Knight" };
    const char  wc[4] = { 'Q','R','B','N' };
    const char  bc[4] = { 'q','r','b','n' };

    int bY = py + 50;
    for (int i = 0; i < 4; i++) {
        int bX = px + pad + i * (bw + pad);
        bool clicked = drawButton("", bX, bY, bw, bh);
        char glyph = isWhite ? wc[i] : bc[i];
        drawPieceIcon(glyph, bX + bw*0.19f, (float)(bY + 4), bh*0.62f, bh*0.62f);
        gtextCentered(labels[i], bX + bw/2, bY + bh - L.fontSize, L.fontSize - 2, T.txtDim);
        if (clicked) return glyph;
    }
    return 0;
}

// =============================================================
//  BACKGROUND
// =============================================================
void drawBackground()
{
    DrawRectangleGradientV(0, 0, L.screenW, L.screenH, T.bgTop, T.bgBot);
    // vignette
    int vw = L.screenW / 4, vh = L.screenH / 4;
    DrawRectangleGradientH(0, 0, vw, L.screenH, { 0,0,0,120 }, { 0,0,0,0 });
    DrawRectangleGradientH(L.screenW - vw, 0, vw, L.screenH, { 0,0,0,0 }, { 0,0,0,120 });
    DrawRectangleGradientV(0, 0, L.screenW, vh, { 0,0,0,110 }, { 0,0,0,0 });
    DrawRectangleGradientV(0, L.screenH - vh, L.screenW, vh, { 0,0,0,0 }, { 0,0,0,110 });
}

// =============================================================
//  MAIN MENU
// =============================================================
void drawMenu(GuiState& g)
{
    drawBackground();
    int cx  = L.screenW / 2;
    int bw  = (int)(L.square * 3.8f);
    int bh  = (int)(L.square * 0.58f);
    int gap = bh / 3;
    int titleFs = (int)(L.square * 0.85f);
    int subFs   = L.fontSize;

    int topY = L.screenH/2 - (int)(L.square * 3.6f);
    if (topY < L.margin) topY = L.margin;

    gtextCentered("CHESS", cx+2, topY+2, titleFs, { 0,0,0,120 }, true);
    gtextCentered("CHESS", cx, topY, titleFs, T.accent, true);
    gtextCentered("Premium Edition   |   Minimax AI   |   3 Difficulties",
                  cx, topY + titleFs + 6, subFs, T.txtDim);

    int by = topY + titleFs + subFs + gap * 3;

    if (drawButton("New Game  (Player vs Player)", cx - bw/2, by, bw, bh))
    { g.mode = GuiState::PVP; startGame(g); }
    by += bh + gap;
    if (drawButton("Play vs Bot", cx - bw/2, by, bw, bh))
    { g.mode = GuiState::PVB; startGame(g); }
    by += bh + gap;
    if (drawButton("Load Saved Game", cx - bw/2, by, bw, bh)) {
        saveload();
        check_king_White(); check_king_Black();
        Count=0; undo=0; epRow=-1; epCol=-1;
        for(int i=0;i<4;i++) rook[i]=0;
        for(int i=0;i<2;i++) king[i]=0;
        Stalemate=0; History();
        g.hasSel=false; g.legalCount=0;
        g.awaitingPromo=false; g.resultMsg="";
        g.lastSR=g.lastSC=g.lastDR=g.lastDC=-1;
        g.timerOn=(g.timerMode>0);
        g.whiteClock=g.blackClock=TIMER_MINUTES[g.timerMode]*60.0f;
        g.flip=(g.mode==GuiState::PVB && !g.humanIsWhite);
        g.screen=GuiState::GAME;
    }
    by += bh + gap * 2;

    // settings row 1: Side / Diff / Theme
    int sw = (bw - gap*2) / 3;
    const char* diffs[4] = { "", "Easy", "Medium", "Hard" };
    if (drawButton(g.humanIsWhite ? "Side: White" : "Side: Black", cx - bw/2, by, sw, bh))
        g.humanIsWhite = !g.humanIsWhite;
    if (drawButton(TextFormat("Diff: %s", diffs[g.difficulty]), cx - bw/2 + sw + gap, by, sw, bh))
        g.difficulty = (g.difficulty % 3) + 1;
    if (drawButton(TextFormat("Theme: %s", T.name), cx - bw/2 + 2*(sw + gap), by, sw, bh)) {
        g.themeIdx = (g.themeIdx + 1) % 3; T = THEMES[g.themeIdx];
    }
    by += bh + gap;

    // settings row 2: Timer (full width)
    if (drawButton(TextFormat("Timer: %s   (click to change)", TIMER_LABELS[g.timerMode]),
                   cx - bw/2, by, bw, bh))
        g.timerMode = (g.timerMode + 1) % 4;
    by += bh + gap * 2;

    gtextCentered("F11 or the fullscreen button toggles fullscreen",
                  cx, by, subFs - 2, T.txtDim);
}

// =============================================================
//  RESULT SCREEN
// =============================================================
void drawResult(GuiState& g)
{
    drawBackground();
    drawBoard(g);
    DrawRectangle(0, 0, L.screenW, L.screenH, { 0, 0, 0, 180 });
    int cx  = L.screenW / 2;
    int rfs = (int)(L.square * 0.5f);
    gtextCentered(g.resultMsg, cx, L.screenH/2 - rfs*2, rfs, WHITE, true);
    int bw = (int)(L.square * 3.0f);
    int bh = (int)(L.square * 0.6f);
    if (drawButton("Back to Main Menu", cx - bw/2, L.screenH/2 + rfs/2, bw, bh))
        g.screen = GuiState::MENU;
}

// =============================================================
//  FULLSCREEN TOGGLE
// =============================================================
static int g_windowedAvailW = 1280;
static int g_windowedAvailH = 800;

void toggleFullscreen(GuiState& g)
{
    if (!g.fullscreen) {
        int mw = GetMonitorWidth(0), mh = GetMonitorHeight(0);
        if (!IsWindowFullscreen()) { SetWindowSize(mw, mh); ToggleFullscreen(); }
        pickSquareSize(mw, mh);
        layoutCentered(mw, mh);
        g.fullscreen = true;
    } else {
        if (IsWindowFullscreen()) ToggleFullscreen();
        pickSquareSize(g_windowedAvailW, g_windowedAvailH);
        int cw = contentWidth(), ch = contentHeight();
        SetWindowSize(cw, ch);
        layoutCentered(cw, ch);
        int mw = GetMonitorWidth(0), mh = GetMonitorHeight(0);
        SetWindowPosition((mw - cw) / 2, (mh - ch) / 2);
        g.fullscreen = false;
    }
}

// =============================================================
//  MAIN
// =============================================================
int main()
{
    srand((unsigned)time(0));
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(800, 600, "Chess");

    int mw = GetMonitorWidth(0), mh = GetMonitorHeight(0);
    g_windowedAvailW = (int)(mw * 0.95f);
    g_windowedAvailH = (int)(mh * 0.90f);
    pickSquareSize(g_windowedAvailW, g_windowedAvailH);
    int cw = contentWidth(), ch = contentHeight();
    SetWindowSize(cw, ch);
    SetWindowTitle("Chess - Premium");
    SetWindowPosition((mw - cw) / 2, (mh - ch) / 2);
    layoutCentered(cw, ch);

    InitAudioDevice();
    audioReady = IsAudioDeviceReady();
    SetTargetFPS(60);
    loadTextures();
    loadFonts();
    if (audioReady) loadSounds();

    GuiState g;
    T = THEMES[g.themeIdx];

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (IsKeyPressed(KEY_F11)) toggleFullscreen(g);

        BeginDrawing();
        drawBackground();

        if (g.screen == GuiState::MENU)   { drawMenu(g);   EndDrawing(); continue; }
        if (g.screen == GuiState::RESULT) { drawResult(g); EndDrawing(); continue; }

        // ── IN GAME ─────────────────────────────────────────────
        g_flip = g.flip;

        bool humanTurn = (g.mode == GuiState::PVP) ||
                         (turn == 1 &&  g.humanIsWhite) ||
                         (turn == 0 && !g.humanIsWhite);

        // countdown for the side to move
        if (g.timerOn && !g.awaitingPromo) {
            if (turn == 1) g.whiteClock -= dt; else g.blackClock -= dt;
            checkTimeout(g);
        }
        if (g.screen == GuiState::RESULT) { drawResult(g); EndDrawing(); continue; }

        // bot move (short natural delay)
        if (!humanTurn && !g.awaitingPromo) {
            if (g.botDelay > 0.0f)      g.botDelay -= dt;
            else if (!g.botMoved)     { doBotTurn(g); checkGameOver(g); }
            else                        g.botMoved = false;
        } else { g.botDelay = 0.0f; g.botMoved = false; }

        drawBoard(g);

        char action = drawPanel(g);
        if (action == 'u') { Undo(); check_king_White(); check_king_Black(); g.hasSel=false; g.legalCount=0; }
        if (action == 'r') { Redo(); check_king_White(); check_king_Black(); g.hasSel=false; g.legalCount=0; }
        if (action == 's') { save(); }
        if (action == 'm') { g.screen = GuiState::MENU; }
        if (action == 'f') { toggleFullscreen(g); }
        if (action == 't') { g.themeIdx = (g.themeIdx + 1) % 3; T = THEMES[g.themeIdx]; }
        if (action == 'x') { g.flip = !g.flip; g_flip = g.flip; }

        if (humanTurn && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int mr = 0, mc = 0;
            if (pixelToSquare((int)GetMousePosition().x, (int)GetMousePosition().y, mr, mc))
                handleClick(g, mr, mc);
        }

        if (g.awaitingPromo) {
            char chosen = drawPromoPopup(g.promoPiece == 'P');
            if (chosen) { applyPromotion(g, chosen); checkGameOver(g); }
        }
        if (humanTurn && !g.awaitingPromo) checkGameOver(g);

        EndDrawing();
    }

    if (audioReady) { unloadSounds(); CloseAudioDevice(); }
    unloadFonts();
    unloadTextures();
    CloseWindow();
    return 0;
}
