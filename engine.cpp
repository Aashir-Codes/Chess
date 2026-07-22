#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

// So the game builds on both Windows and Linux
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// On Linux there is no Sleep(), so I make one that works the same way (in ms)
#ifndef _WIN32
void Sleep(int ms) { usleep(ms * 1000); }
#endif

// Global For Ease

char board[8][8] = {};
bool turn = 0;
bool checkflag = 0;
bool flagcheck = 0;
bool Stalemate = 0;
int d_r = 0;
int d_c = 0;
int s_r = 0;
int s_c = 0;
//<================================>

int k_r = 0;
int k_c = 0;
int K_r = 0;
int K_c = 0;

bool rook[4] = {0};
bool king[2] = {0};

//<===============================================>
char history[500][8][8];
int Count = 0;
bool Turn[500] = {};
int undo = 0;
bool Exit = 0;

int epRow = -1; // row of the square a pawn can capture "en passant" onto
int epCol = -1; // -1 means en passant is not available right now

//<===============================================>
//         AI move representation
//<===============================================>
struct Move {
  int s_r, s_c, d_r, d_c;
};
struct GameState {
  char board[8][8];
  bool rook[4];
  bool king[2];
  int K_r, K_c, k_r, k_c;
  int epRow, epCol;
  bool turn;
};

//<===============================================>
//            Function Protototypes
//<===============================================>

void save();
void check_king_White();
void check_king_Black();
void FileToArray();
void DisplayBoard();
void NewGame();
void input();
void LoadGame();
void PlayvsBot();
int check_BlackWhite(int a, int b);
void inputdr();
void MovePiece();
bool Rook();
bool King();
bool Queen();
bool Knight();
bool Bishop();
bool Pawn();
int _max(int a = 0, int b = 0);
int _min(int a = 0, int b = 0);
int Abs(int a);
void swap(char &a, char &b);
bool checkpieces();
bool check();
void color(int k);
void clearScreen();
bool premove(int S_r, int S_c, int D_r, int D_c);
void display_moves();
void promo();
bool check_legal_movews();
bool checkmate();
bool stalemate();
void Result();
void Undo();
void Redo();
void History();
void menu();
void saveload();

//<===============================================>
//            AI Function Prototypes
//<===============================================>
void makeMove(Move m, bool autoPromoteQueen = false);
int generateMoves(Move list[], int maxMoves);
void saveState(GameState &st);
void restoreState(const GameState &st);
int pieceValue(char c);
int evaluate();
Move pickRandomMove();
Move pickGreedyMove();
int minimax(int depth, int alpha, int beta);
Move pickBestMoveMinimax(int depth);

//<===============================================>
//                    Main (terminal version)
//
//  This is the console entry point. The GUI (chess_gui.cpp) reuses every
//  function in this file by #including it, so it #defines CHESS_GUI_BUILD
//  first to switch OFF this main() and avoid two main() functions.
//
//  Build terminal version:  g++ engine.cpp -o chess-cli
//  Build GUI version:       see build.sh  (defines CHESS_GUI_BUILD)
//<===============================================>

#ifndef CHESS_GUI_BUILD
int main() {

  srand(
      (unsigned)time(0)); // seed once so the bot's random moves differ each run

  char choice = 0;

  do {
    cout << "<=================================>" << endl;
    cout << "            MAIN  MENU                " << endl;
    cout << "<=================================>" << endl;
    cout << "1. Start New Game" << endl;
    cout << "2. Load Game" << endl;
    cout << "3. Play vs Bot " << endl;
    cout << "4. Exit" << endl;
    cout << "Enter your choice: ";
    cin >> choice;
    cout << "<=================================>" << endl;

    switch (choice) {
    case '1':
      NewGame();
      Sleep(5000);
      break;
    case '2':
      LoadGame();
      Sleep(5000);
      break;
    case '3':
      PlayvsBot();
      break;
    case '4':
      cout << "<===============================>" << endl;
      cout << "       Exiting the Game...        " << endl;
      cout << "<===============================>" << endl;

    default:
      cout << "   Invalid Choice." << endl;
      break;
    }

  } while (choice != '4');

  return 0;
}
#endif // CHESS_GUI_BUILD

//<===============================================>
//                  Function
//<==============================================
void promo() {
  cout << "<=================================>" << endl;
  cout << "            PROMOTION                " << endl;
  cout << "<=================================>" << endl;
  cout << "1. Queen" << endl;
  cout << "2. Knight" << endl;
  cout << "3. Rook " << endl;
  cout << "4. Bischop" << endl;
  cout << "Enter your choice: ";
}
void display_moves() {
  clearScreen();
  unsigned char vertical = 186;

  cout << "                             Turn: ";
  if (turn == 1)
    cout << "white\n";
  else
    cout << "Black\n";

  cout << "                       1   2   3   4   5   6   7   8\n";

  for (int i = 0; i < 8; i++) {
    cout << "\n                     +---+---+---+---+---+---+---+---+\n";
    cout << "                  " << i + 1 << "  " << vertical << " ";

    for (int j = 0; j < 8; j++) {
      bool isMove = false;

      d_r = i;
      d_c = j;

      switch (board[s_r][s_c]) {
      case 'P':
      case 'p':
        isMove = Pawn();
        break;
      case 'Q':
      case 'q':
        isMove = Queen();
        break;
      case 'R':
      case 'r':
        isMove = Rook();
        break;
      case 'B':
      case 'b':
        isMove = Bishop();
        break;
      case 'N':
      case 'n':
        isMove = Knight();
        break;
      case 'K':
      case 'k':
        isMove = King();
        break;
      }
      if (i == s_r && j == s_c)
        isMove = false;
      if (isMove && board[i][j] != '.') {
        if (isupper(board[s_r][s_c]) == isupper(board[i][j]))
          isMove = false;
      }

      if (s_r == i && s_c == j)
        color(15);
      else if (isMove)
        color(11);
      else
        color(7);

      if (board[i][j] != '.') {

        cout << board[i][j];
        color(15);
        cout << " ";

        cout << vertical << " ";
      } else {

        cout << ". ";
        color(15);
        cout << vertical << " ";
      }
    }
  }

  cout << "\n                     +---+---+---+---+---+---+---+---+\n";
  cout << "                       1   2   3   4   5   6   7   8\n";
}
void check_king_White() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == 'K') {
        K_r = i;
        K_c = j;
        return;
      }
    }
  }
}
void check_king_Black() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == 'k') {
        k_r = i;
        k_c = j;
        return;
      }
    }
  }
}
bool checkpieces() {
  char piece = board[s_r][s_c];
  switch (piece) {

  case 'B':
  case 'b':
    return Bishop();

  case 'Q':
  case 'q':
    return Queen();

  case 'P':
  case 'p':
    return Pawn();

  case 'R':
  case 'r':
    return Rook();
  case 'N':
  case 'n':
    return Knight();

  case 'K':
  case 'k':
    return King();
  }

  return false;
}
void MovePiece() {

  clearScreen();
  DisplayBoard();

  bool flag = 0;
  do {
    input();
    display_moves();
    inputdr();

    if (!checkpieces()) {

      cout << "There is no Valid Move" << endl;
      continue;
    }
    if ((premove(s_r, s_c, d_r, d_c))) {
      cout << "Illegal move! Your king would be in check!" << endl;
      continue;
    }

    makeMove(Move{s_r, s_c, d_r, d_c}, false);
    flag = 1;

  } while (flag != 1);

  check_king_White();
  check_king_Black();
}
bool Pawn() {
  // Decide the pawn's colour from the piece itself, not the global turn.
  // This way it is also correct when we test an enemy pawn during check
  // detection.
  bool white = (board[s_r][s_c] == 'P');

  if (white) {
    if (s_r == 6 && d_r == 4 && s_c == d_c && board[5][d_c] == '.' &&
        board[4][d_c] == '.')
      return true;
    if (board[d_r][d_c] != '.' && check_BlackWhite(d_r, d_c) == 0) {
      if ((d_r == s_r - 1 && d_c == s_c - 1) ||
          (d_r == s_r - 1 && d_c == s_c + 1))
        return true;
    }
    // en passant: capturing diagonally onto the tracked empty square
    if (d_r == s_r - 1 && (d_c == s_c - 1 || d_c == s_c + 1) && d_r == epRow &&
        d_c == epCol)
      return true;
    if ((d_r == s_r - 1 && d_c == s_c) && board[d_r][d_c] == '.')
      return true;
  } else {
    if (s_r == 1 && d_r == 3 && s_c == d_c && board[2][d_c] == '.' &&
        board[3][d_c] == '.')
      return true;
    if (board[d_r][d_c] != '.' && check_BlackWhite(d_r, d_c) == 1) {
      if ((d_r == s_r + 1 && d_c == s_c - 1) ||
          (d_r == s_r + 1 && d_c == s_c + 1))
        return true;
    }
    // en passant: capturing diagonally onto the tracked empty square
    if (d_r == s_r + 1 && (d_c == s_c - 1 || d_c == s_c + 1) && d_r == epRow &&
        d_c == epCol)
      return true;
    if ((d_r == s_r + 1 && d_c == s_c) && board[d_r][d_c] == '.')
      return true;
  }
  return false;
}
bool Rook() {
  if (s_r == d_r) {
    for (int i = _min(s_c, d_c) + 1; i < _max(s_c, d_c); i++) {
      if (board[s_r][i] != '.') {

        return false;
      }
    }
    return true;
  }
  if (d_c == s_c) {
    for (int i = _min(s_r, d_r) + 1; i < _max(s_r, d_r); i++) {
      if (board[i][s_c] != '.') {

        return false;
      }
    }
    return true;
  }
  return false;
}
bool King() {

  if (s_r == d_r && (d_c == s_c + 1 || d_c == s_c - 1))
    return true;
  if (s_c == d_c && (d_r == s_r + 1 || d_r == s_r - 1))
    return true;
  if ((d_r == s_r + 1 && d_c == s_c - 1) || (d_r == s_r - 1 && d_c == s_c + 1))
    return true;
  if ((d_r == s_r - 1 && d_c == s_c - 1) || (d_r == s_r + 1 && d_c == s_c + 1))
    return true;

  int row = (turn == 1) ? 7 : 0;

  bool k_m = (turn == 1) ? king[1] : king[0];

  if ((k_m == 0) && s_r == row && d_r == row && Abs(s_c - d_c) == 2)

  {
    if (check())
      return false;
    if (d_c == 6) {
      bool r_m = (turn == 1) ? rook[3] : rook[1];
      char cornerRook = (turn == 1) ? 'R' : 'r';
      if (r_m == 0 && board[row][7] == cornerRook && board[row][5] == '.' &&
          board[row][6] == '.') {

        if (!premove(s_r, s_c, row, 5))
          return true;
      }
    }
    if (d_c == 2) {
      bool r_m = (turn == 1) ? rook[2] : rook[0];
      char cornerRook = (turn == 1) ? 'R' : 'r';
      if (r_m == 0 && board[row][0] == cornerRook && board[row][1] == '.' &&
          board[row][2] == '.' && board[row][3] == '.') {

        if (!premove(s_r, s_c, row, 3))
          return true;
      }
    }
  }

  return false;
}
bool Knight() {
  if (((Abs(d_r - s_r) == 2 && Abs(d_c - s_c) == 1)) ||
      (Abs(d_r - s_r) == 1 && Abs(d_c - s_c) == 2))
    return true;

  return false;
}
bool Bishop() {
  if (Abs(d_r - s_r) != Abs(d_c - s_c))
    return false;

  if (s_r > d_r && s_c > d_c) {
    for (int i = 1; i < Abs(d_r - s_r); i++) {
      if (board[s_r - i][s_c - i] != '.') {

        return false;
      }
    }
    return true;
  }
  if (s_r > d_r && s_c < d_c) {
    for (int i = 1; i < Abs(d_r - s_r); i++) {
      if (board[s_r - i][s_c + i] != '.') {

        return false;
      }
    }
    return true;
  }
  if (s_r < d_r && s_c > d_c) {
    for (int i = 1; i < Abs(d_r - s_r); i++) {
      if (board[s_r + i][s_c - i] != '.') {

        return false;
      }
    }
    return true;
  }
  if (s_r < d_r && s_c < d_c) {

    for (int i = 1; i < Abs(d_r - s_r); i++) {
      if (board[s_r + i][s_c + i] != '.') {

        return false;
      }
    }
    return true;
  }

  return false;
}
bool Queen() {

  if (Bishop() || Rook())
    return true;
  return false;
}
void FileToArray() {

  ifstream Rdr("newgame.txt");
  if (!Rdr) {
    cout << "The File is not opened!" << endl;
    return;
  }

  for (int i = 0; i < 8; i++) {

    for (int j = 0; j < 8; j++) {

      Rdr >> board[i][j];
    }
  }
}
void DisplayBoard() {
  unsigned char vertical = 186;

  color(15);

  cout << "                             Turn: ";
  if (turn == 1)
    cout << "white" << endl;
  else
    cout << "Black\n";

  cout << "                     " << "  1   2   3   4   5   6   7   8" << endl;

  for (int i = 0; i < 8; i++) {
    cout << "                    ";

    cout << endl;
    cout << "                     +---+---+---+---+---+---+---+---+" << endl;
    cout << "                  ";
    cout << i + 1;
    cout << "  ";
    cout << vertical << " ";
    for (int j = 0; j < 8; j++) {

      if (board[i][j] != '.') {

        cout << board[i][j];
        cout << " ";

        cout << vertical << " ";
      } else {

        cout << ". ";

        cout << vertical << " ";
      }
      color(15);
    }
  }
  color(15);
  cout << "\n                     +---+---+---+---+---+---+---+---+" << endl;
  cout << "                     " << "  1   2   3   4   5   6   7   8" << endl;
}
void NewGame() {
  clearScreen();
  Exit = 0;
  FileToArray();
  check_king_White();
  check_king_Black();
  turn = 1;
  Count = 0;
  undo = 0;
  epRow = -1;
  epCol = -1;
  History();

  do {
    clearScreen();

    DisplayBoard();
    menu();
    if (Exit == 1)
      return;
    MovePiece();
    turn = !turn;
    History();
    if (checkmate() || stalemate())
      break;

  } while (true);

  Result();
}
void input() {
  bool flag = false;
  char m, n;
  do {

    cout << "Enter the coordinates of the piece you want to move (row and "
            "column): ";

    cin >> m >> n;
    if (m < '1' || m > '8' || n < '1' || n > '8') {
      cout << "\nInvalid input. Please enter numbers between 1 and 8." << endl;
      flag = 1;
      continue;
    }

  } while (flag != 0);
  s_r = m - '0';
  s_r--;
  s_c = n - '0';
  s_c--;
  if (board[s_r][s_c] == '.') {
    cout << "The Selected Postion is Empty" << endl;
    cout << "        Enter Again" << endl;
    input();
  }
  if (check_BlackWhite(s_r, s_c) != turn) {
    cout << "The Selected Piece is not yours" << endl;
    cout << "        Enter Again" << endl;
    input();
  }
}
void inputdr() {
  bool flag = false;
  char m, n;
  do {

    cout << "Enter the coordinates where you want to move the selected piece "
            "(row and column): ";

    cin >> m >> n;
    if (m < '1' || m > '8' || n < '1' || n > '8') {
      cout << "\nInvalid input. Please enter numbers between 1 and 8." << endl;
      flag = 1;
      continue;
    }

  } while (flag != 0);
  d_r = m - '0';
  d_r--;
  d_c = n - '0';
  d_c--;

  if (check_BlackWhite(d_r, d_c) == turn) {
    cout << "\nThe Selected  Destination contain  your piece" << endl;
    cout << "            Select Again" << endl;
    input();
    inputdr();
  }
}
int check_BlackWhite(int a, int b) {
  int check = board[a][b];
  if (check <= 90 && check >= 65)
    return 1;
  if (check >= 97 && check <= 122)
    return 0;
  else
    return -1;
}
void LoadGame() {
  clearScreen();
  Exit = 0;
  saveload();
  check_king_White();
  check_king_Black();
  Count = 0;
  undo = 0;
  epRow = -1;
  epCol = -1;
  History();

  do {
    clearScreen();

    DisplayBoard();
    menu();
    if (Exit == 1)
      return;
    MovePiece();
    turn = !turn;
    History();
    if (checkmate() || stalemate())
      break;

  } while (true);

  Result();
}
//<===============================================>
//                  AI
//<===============================================>
void makeMove(Move m, bool autoPromoteQueen) {
  s_r = m.s_r;
  s_c = m.s_c;
  d_r = m.d_r;
  d_c = m.d_c;

  char piece = board[s_r][s_c];
  bool isPawnPiece = (piece == 'P' || piece == 'p');
  bool wasEnPassant = isPawnPiece && s_c != d_c && board[d_r][d_c] == '.';
  bool wasDoubleStep = isPawnPiece && Abs(d_r - s_r) == 2;

  if (piece == 'R' || piece == 'r') {
    if (s_r == 0 && s_c == 0)
      rook[0] = 1;
    if (s_r == 0 && s_c == 7)
      rook[1] = 1;
    if (s_r == 7 && s_c == 0)
      rook[2] = 1;
    if (s_r == 7 && s_c == 7)
      rook[3] = 1;
  }

  if (piece == 'K' || piece == 'k') {
    if (Abs(d_c - s_c) == 2) // castling also moves the rook
    {
      if (d_c == 2) {
        board[d_r][3] = board[d_r][0];
        board[d_r][0] = '.';
      } else if (d_c == 6) {
        board[d_r][5] = board[d_r][7];
        board[d_r][7] = '.';
      }
    }
    if (piece == 'k')
      king[0] = 1;
    else
      king[1] = 1;
  }

  if (wasEnPassant)
    board[s_r][d_c] = '.'; // remove the pawn that got passed

  board[d_r][d_c] = board[s_r][s_c];
  board[s_r][s_c] = '.';

  if ((piece == 'P' && d_r == 0) || (piece == 'p' && d_r == 7)) {
    if (autoPromoteQueen) {
      board[d_r][d_c] = (piece == 'P') ? 'Q' : 'q';
    } else {
      int choice = 0;
      bool flag1 = 0;
      promo();
      do {
        cin >> choice;
        cout << "<=================================>" << endl;
        switch (choice) {
        case 1:
          board[d_r][d_c] = (piece == 'P') ? 'Q' : 'q';
          flag1 = 1;
          break;
        case 2:
          board[d_r][d_c] = (piece == 'P') ? 'N' : 'n';
          flag1 = 1;
          break;
        case 3:
          board[d_r][d_c] = (piece == 'P') ? 'R' : 'r';
          flag1 = 1;
          break;
        case 4:
          board[d_r][d_c] = (piece == 'P') ? 'B' : 'b';
          flag1 = 1;
          break;
        default:
          cout << "Invalid Choice!" << endl;
          break;
        }
      } while (flag1 != 1);
    }
  }

  if (piece == 'K') {
    K_r = d_r;
    K_c = d_c;
  } else if (piece == 'k') {
    k_r = d_r;
    k_c = d_c;
  }

  if (wasDoubleStep) {
    epRow = (s_r + d_r) / 2;
    epCol = d_c;
  } else {
    epRow = -1;
    epCol = -1;
  }
}
int generateMoves(Move list[], int maxMoves) {
  int count = 0;
  int temp_sr = s_r, temp_sc = s_c, temp_dr = d_r, temp_dc = d_c;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {

      if (board[i][j] == '.' || check_BlackWhite(i, j) != turn)
        continue;
      s_r = i;
      s_c = j;

      for (int a = 0; a < 8; a++) {
        for (int b = 0; b < 8; b++) {

          if (check_BlackWhite(a, b) == turn)
            continue;
          d_r = a;
          d_c = b;

          if (!checkpieces())
            continue;
          if (premove(s_r, s_c, d_r, d_c))
            continue;

          if (count < maxMoves) {
            list[count].s_r = s_r;
            list[count].s_c = s_c;
            list[count].d_r = d_r;
            list[count].d_c = d_c;
            count++;
          }
        }
      }
    }
  }

  s_r = temp_sr;
  s_c = temp_sc;
  d_r = temp_dr;
  d_c = temp_dc;
  return count;
}
void saveState(GameState &st) {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      st.board[i][j] = board[i][j];

  for (int i = 0; i < 4; i++)
    st.rook[i] = rook[i];
  for (int i = 0; i < 2; i++)
    st.king[i] = king[i];

  st.K_r = K_r;
  st.K_c = K_c;
  st.k_r = k_r;
  st.k_c = k_c;
  st.epRow = epRow;
  st.epCol = epCol;
  st.turn = turn;
}
void restoreState(const GameState &st) {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      board[i][j] = st.board[i][j];

  for (int i = 0; i < 4; i++)
    rook[i] = st.rook[i];
  for (int i = 0; i < 2; i++)
    king[i] = st.king[i];

  K_r = st.K_r;
  K_c = st.K_c;
  k_r = st.k_r;
  k_c = st.k_c;
  epRow = st.epRow;
  epCol = st.epCol;
  turn = st.turn;
}
int pieceValue(char c) {
  switch (tolower(c)) {
  case 'p':
    return 1;
  case 'n':
    return 3;
  case 'b':
    return 3;
  case 'r':
    return 5;
  case 'q':
    return 9;
  case 'k':
    return 1000;
  }
  return 0;
}
int evaluate() {
  int score = 0;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == '.')
        continue;
      int value = pieceValue(board[i][j]);
      if (isupper(board[i][j]))
        score += value;
      else
        score -= value;
    }
  }
  return score;
}
Move pickRandomMove() {
  Move list[500];
  int n = generateMoves(list, 500);
  int pick = rand() % n;
  return list[pick];
}
Move pickGreedyMove() {
  Move list[500];
  int n = generateMoves(list, 500);

  int bestIndex = 0;
  int bestScore = (turn == 1) ? -100000 : 100000;

  for (int i = 0; i < n; i++) {
    GameState st;
    saveState(st);
    makeMove(list[i], true);
    int score = evaluate();
    restoreState(st);

    if (turn == 1 && score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
    if (turn == 0 && score < bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  return list[bestIndex];
}
int minimax(int depth, int alpha, int beta) {
  Move list[500];
  int n = generateMoves(list, 500);

  if (n == 0) {
    if (check())
      return (turn == 1) ? -100000 : 100000; // side to move is checkmated
    return 0;                                // stalemate
  }
  if (depth == 0)
    return evaluate();

  bool maximizing = (turn == 1);
  int best = maximizing ? -100000 : 100000;

  for (int i = 0; i < n; i++) {
    GameState st;
    saveState(st);
    makeMove(list[i], true);
    turn = !turn;

    int score = minimax(depth - 1, alpha, beta);
    restoreState(st);

    if (maximizing) {
      if (score > best)
        best = score;
      if (best > alpha)
        alpha = best;
    } else {
      if (score < best)
        best = score;
      if (best < beta)
        beta = best;
    }
    if (beta <= alpha)
      break; // alpha-beta pruning
  }

  return best;
}
Move pickBestMoveMinimax(int depth) {
  Move list[500];
  int n = generateMoves(list, 500);

  bool maximizing = (turn == 1);
  int bestIndex = 0;
  int bestScore = maximizing ? -100000 : 100000;

  for (int i = 0; i < n; i++) {
    GameState st;
    saveState(st);
    makeMove(list[i], true);
    turn = !turn;

    int score = minimax(depth - 1, -100000, 100000);
    restoreState(st);

    if (maximizing && score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
    if (!maximizing && score < bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  return list[bestIndex];
}
void PlayvsBot() {
  clearScreen();
  Exit = 0;
  FileToArray();
  check_king_White();
  check_king_Black();
  turn = 1;
  Count = 0;
  undo = 0;
  epRow = -1;
  epCol = -1;
  History();

  char colorChoice = 0;
  cout << "<=================================>" << endl;
  cout << "            PLAY VS BOT               " << endl;
  cout << "<=================================>" << endl;
  cout << "1. Play as White" << endl;
  cout << "2. Play as Black" << endl;
  cout << "Enter your choice: ";
  cin >> colorChoice;
  bool humanIsWhite = (colorChoice != '2');

  int difficulty = 0;
  cout << "<=================================>" << endl;
  cout << "1. Easy" << endl;
  cout << "2. Medium" << endl;
  cout << "3. Hard" << endl;
  cout << "Enter your choice: ";
  cin >> difficulty;
  if (difficulty < 1 || difficulty > 3)
    difficulty = 2;

  do {
    clearScreen();
    DisplayBoard();

    bool humanTurn =
        (turn == 1 && humanIsWhite) || (turn == 0 && !humanIsWhite);

    if (humanTurn) {
      menu();
      if (Exit == 1)
        return;
      MovePiece();
    } else {
      cout << "\nBot is thinking..." << endl;
      Move botMove;
      if (difficulty == 1)
        botMove = pickRandomMove();
      else if (difficulty == 2)
        botMove = pickGreedyMove();
      else
        botMove = pickBestMoveMinimax(3);

      makeMove(botMove, true);
      check_king_White();
      check_king_Black();
      Sleep(800);
    }

    turn = !turn;
    History();
    if (checkmate() || stalemate())
      break;

  } while (true);

  Result();
}
int _max(int a, int b) {
  if (a > b)
    return a;
  else
    return b;
}
int _min(int a, int b) {
  if (a < b)
    return a;
  else
    return b;
}
int Abs(int a) {
  if (a < 0)
    return -a;
  return a;
}
void swap(char &a, char &b) {
  char temp = a;
  a = b;
  b = temp;
}
void color(int k) {
#ifdef _WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hConsole, k);
#else
  // On Linux map the few Windows colour codes I use to ANSI colours
  if (k == 11)
    cout << "\033[96m"; // highlighted moves (cyan)
  else
    cout << "\033[0m"; // normal text
#endif
}
void clearScreen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}
void save() {
  ofstream Wrt("Save.txt");

  Wrt << turn;
  Wrt << "\n";

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      Wrt << board[i][j];
      Wrt << " ";
    }
    Wrt << endl;
  }

  Wrt.close();
}
void saveload() {
  ifstream Wrt("Save.txt");

  Wrt >> turn;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      Wrt >> board[i][j];
      Wrt.ignore();
    }
    Wrt.ignore();
  }

  Wrt.close();
}
bool premove(int S_r, int S_c, int D_r, int D_c) {
  char savepiece = board[D_r][D_c];
  int oldk_r = k_r, oldk_c = k_c;
  int oldK_r = K_r, oldK_c = K_c;

  // en passant: the captured pawn sits beside the source square, not on the
  // destination square, so pull it off the board for this simulation too
  bool epCapture = (board[S_r][S_c] == 'P' || board[S_r][S_c] == 'p') &&
                   S_c != D_c && savepiece == '.';
  char epSavedPawn = epCapture ? board[S_r][D_c] : '.';
  if (epCapture)
    board[S_r][D_c] = '.';

  board[D_r][D_c] = board[S_r][S_c];
  board[S_r][S_c] = '.';

  if (board[D_r][D_c] == 'K') {
    K_r = D_r;
    K_c = D_c;
  } else if (board[D_r][D_c] == 'k') {
    k_r = D_r;
    k_c = D_c;
  }

  int temp_sr = s_r, temp_sc = s_c;
  int temp_dr = d_r, temp_dc = d_c;

  bool is_check = false;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {

      if (board[i][j] == '.') {
        continue;
      }

      int pieceOwner = check_BlackWhite(i, j);
      if (pieceOwner == turn) {

        continue;
      }

      if (pieceOwner == -1) {

        continue;
      }

      s_r = i;
      s_c = j;

      if (turn == 1) {

        d_r = K_r;
        d_c = K_c;
      } else {

        d_r = k_r;
        d_c = k_c;
      }

      if (checkpieces()) {
        is_check = true;
        break;
      }
    }
    if (is_check)
      break;
  }

  board[S_r][S_c] = board[D_r][D_c];
  board[D_r][D_c] = savepiece;
  if (epCapture)
    board[S_r][D_c] = epSavedPawn;
  K_r = oldK_r;
  K_c = oldK_c;
  k_r = oldk_r;
  k_c = oldk_c;
  s_r = temp_sr;
  s_c = temp_sc;
  d_r = temp_dr;
  d_c = temp_dc;

  return is_check;
}
bool check() {
  int temp_sr = s_r, temp_sc = s_c;
  int temp_dr = d_r, temp_dc = d_c;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (board[i][j] == '.') {
        continue;
      }

      int pieceOwner = check_BlackWhite(i, j);

      if (turn == 1) {

        if (pieceOwner != 0)
          continue;

        s_r = i;
        s_c = j;
        d_r = K_r;
        d_c = K_c;

        if (checkpieces()) {
          s_r = temp_sr;
          s_c = temp_sc;
          d_r = temp_dr;
          d_c = temp_dc;
          return true;
        }
      } else {

        if (pieceOwner != 1)
          continue;

        s_r = i;
        s_c = j;
        d_r = k_r;
        d_c = k_c;

        if (checkpieces()) {
          s_r = temp_sr;
          s_c = temp_sc;
          d_r = temp_dr;
          d_c = temp_dc;
          return true;
        }
      }
    }
  }

  s_r = temp_sr;
  s_c = temp_sc;
  d_r = temp_dr;
  d_c = temp_dc;
  return false;
}
bool check_legal_movews() {

  int temp_sr = s_r, temp_sc = s_c;
  int temp_dr = d_r, temp_dc = d_c;
  for (int i = 0; i < 8; i++) {

    for (int j = 0; j < 8; j++) {

      if (board[i][j] == '.' || check_BlackWhite(i, j) != turn)
        continue;
      s_r = i;
      s_c = j;

      for (int l = 0; l < 8; l++) {
        for (int k = 0; k < 8; k++) {

          d_r = l;
          d_c = k;

          if (check_BlackWhite(d_r, d_c) == turn)
            continue;
          if (!checkpieces())
            continue;
          if (!premove(s_r, s_c, d_r,
                       d_c)) { // basically premoved chesck that player move was
                               // causing the king to check
            s_r = temp_sr;
            s_c = temp_sc;
            d_r = temp_dr;
            d_c = temp_dc;
            return true;
          }
        }
      }
    }
  }
  s_r = temp_sr;
  s_c = temp_sc;
  d_r = temp_dr;
  d_c = temp_dc;
  return false;
}
bool checkmate() {

  if (check() && !check_legal_movews()) {
    return true;
  }

  return false;
}
bool stalemate() {
  if (!check() && !check_legal_movews()) {
    Stalemate = 1;
    return true;
  }
  return false;
}
void Result() {
  if (Stalemate == 1) {
    cout << "\n<=====================================>" << endl;
    cout << "          It is a Draw" << endl;
    cout << "<=====================================>" << endl;

  } else {
    if (turn == 1) {
      cout << "\n<=====================================>" << endl;
      cout << "          Black is The Winner" << endl;
      cout << "<=====================================>" << endl;
    } else {

      cout << "\n<=====================================>" << endl;
      cout << "          White is The Winner" << endl;
      cout << "<=====================================>" << endl;
    }
  }
}
void History() {
  if (Count >= 500)
    return; // don't run past the history array
  Turn[Count] = turn;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      history[Count][i][j] = board[i][j];
    }
  }
  Count++;
  undo = 0; // making a new move clears the redo branch
}
void Undo() {
  if (Count <= 1) { // need at least one move to take back
    cout << "   No undo Available" << endl;
    return;
  }
  Count--; // step back to the previous position
  turn = Turn[Count - 1];

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      board[i][j] = history[Count - 1][i][j];
    }
  }

  check_king_White();
  check_king_Black();
  undo++;
}
void Redo() {
  if (undo == 0) {
    cout << "   No redo Available" << endl;
    return;
  }

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      board[i][j] = history[Count][i][j];
    }
  }
  turn = Turn[Count];

  check_king_White();
  check_king_Black();
  Count++;
  undo--;
}
void menu() {
  bool flag = 0;
  int choice = 0;
  do {

    cout << "\n<=======================================>" << endl;
    cout << "1.Undo" << endl;
    cout << "2.Redo" << endl;
    cout << "3.Save && Exit" << endl;
    cout << "4.Continue" << endl;
    cout << "<=======================================>" << endl;

    cout << "Choice: ";
    cin >> choice;

    switch (choice) {
    case 1:
      Undo();
      Sleep(3000);
      clearScreen();
      DisplayBoard();

      break;
    case 2:
      Redo();
      Sleep(3000);
      clearScreen();
      DisplayBoard();
      break;
    case 3:
      save();
      flag = 1;
      Exit = 1;

      break;
    case 4:
      flag = 1;
      break;
    default:
      cout << "Invalid Choice Entered" << endl;
      break;
    }
  } while (flag != 1);
}
