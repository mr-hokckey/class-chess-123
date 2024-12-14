#include "Chess.h"
#include "Evaluate.h"

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

Chess::Chess()
{
}

Chess::~Chess()
{
}

//
// make a chess piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    //
    // we want white to be at the bottom of the screen so we need to reverse the board
    //
    char piece[2];
    piece[1] = 0;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * (_gameOptions.rowY - y) + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            piece[0] = bitToPieceNotation(y,x);
            _grid[y][x].setNotation(piece);
        }
    }

    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    
    // FENtoBoard("5k2/8/8/8/8/8/8/4K2R w K - 0 1"); // white can castle
    // FENtoBoard("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"); // white can castle queen side
    // FENtoBoard("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1"); // white can castle both sides
    // FENtoBoard("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"); // white can promote to queen
    // FENtoBoard("4k3/1P6/8/8/8/8/K7/8 w - - 0 1"); // white can promote to queen

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }
}

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // can't move if it's not your turn
    if (bit.getOwner() != getCurrentPlayer()) {
        return false;
    }

    struct Move {
        int y;
        int x;
    };

    Move moves[16] = {
        { 1,  0 }, { 0,  1 }, {-1,  0 }, { 0, -1 }, // Horizontal moves
        { 1, -1 }, { 1,  1 }, {-1,  1 }, {-1, -1 }, // Diagonal moves
        { 2,  1 }, { 1,  2 }, {-1,  2 }, {-2,  1 }, // Knight moves
        {-2, -1 }, {-1, -2 }, { 1, -2 }, { 2, -1 },
    };

    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    int srcY = srcSquare.getRow();
    int srcX = srcSquare.getColumn();
    
    if (bit.gameTag() % 128 == Pawn) {
        int direction = (bit.gameTag() < 128) ? 1 : -1;

        // Check if the spot in front of it is empty
        int newY = srcY + direction;
        int newX = srcX;
        if (newY >= 0 && newY < 8 && _grid[newY][newX].empty()) {
            return true;
        }

        // Check if diagonals have a piece to be captured
        for (int i = 4; i < 6; i++) {
            newY = srcY + (moves[i].y * direction);
            newX = srcX + moves[i].x;
            if (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // Check if the other square contains an opposing piece
                if (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
                // AND check if the square is an enPassant square
                if (&_grid[newY][newX] == _enPassant) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == Knight) {
        int start = 8;
        int end = 16;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces. As long as one of these squares is a legal move, return true.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == Bishop) {
        int start = 4;
        int end = 8;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces. As long as one of these squares is a legal move, return true.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == Rook) {
        int start = 0;
        int end = 4;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces. As long as one of these squares is a legal move, return true.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == Queen) {
        int start = 0;
        int end = 8;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces. As long as one of these squares is a legal move, return true.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == King) {
        int start = 0;
        int end = 8;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces. As long as one of these squares is a legal move, return true.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    // There is no circumstance where a piece can move to a square that is occupied
    // by a friendly piece. So let's just get that check out of the way now.
    if (!dst.empty() && ((bit.gameTag() >= 128) == (dst.bit()->gameTag() >= 128))) {
        return false;
    }
    
    struct Move {
        int y;
        int x;
    };

    Move moves[16] = {
        { 1,  0 }, { 0,  1 }, {-1,  0 }, { 0, -1 }, // Horizontal moves
        { 1, -1 }, { 1,  1 }, {-1,  1 }, {-1, -1 }, // Diagonal moves
        { 2,  1 }, { 1,  2 }, {-1,  2 }, {-2,  1 }, // Knight moves
        {-2, -1 }, {-1, -2 }, { 1, -2 }, { 2, -1 },
    };

    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare& dstSquare = static_cast<ChessSquare&>(dst);

    int srcY = srcSquare.getRow();
    int srcX = srcSquare.getColumn();

    int dstY = dstSquare.getRow();
    int dstX = dstSquare.getColumn();

    if (bit.gameTag() % 128 == Pawn) {
        int direction = (bit.gameTag() < 128) ? 1 : -1;
        int startingPos = (direction == 1) ? 1 : 6;

        // If pawn is in starting position, check 2 spaces in front
        int newY = srcY + (2 * direction);
        if (srcY == startingPos) {
            if (newY == dstY && srcX == dstX && _grid[newY][srcX].empty()) {
                return true;
            }
        }

        // Check space directly in front
        newY -= direction;
        if (newY == dstY && srcX == dstX && _grid[newY][srcX].empty()) {
            return true;
        }

        // Check diagonals for pieces to capture
        int newX;
        for (int i = 4; i < 6; i++) {
            newX = srcX + moves[i].x;
            if (newY == dstY && newX == dstX) {
                // Check if the other square contains an opposing piece
                if (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
                // AND check if it's an en passant square.
                if (&_grid[newY][newX] == _enPassant) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == Knight) {
        int start = 8;
        int end = 16;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY == dstY && newX == dstX) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces. As long as one of these squares is a legal move, return true.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
    }
    else if (bit.gameTag() % 128 == Bishop) {
        int start = 4;
        int end = 8;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            while (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // If there's a friendly piece in the way, stop checking this direction
                if (!_grid[newY][newX].empty() && 
                    (bit.gameTag() >= 128) == (_grid[newY][newX].bit()->gameTag() >= 128)) {
                    break;
                }
                // If there's an enemy piece in the way, we're gonna break after checking this square
                bool enemyPiece = (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128)));
                
                if (newY == dstY && newX == dstX) {
                    return true;
                }
                if (enemyPiece) {
                    break;
                }
                newY += moves[i].y;
                newX += moves[i].x;
            }
        }
    }
    else if (bit.gameTag() % 128 == Rook) {
        int start = 0;
        int end = 4;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            while (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // If there's a friendly piece in the way, stop checking this direction
                if (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) == (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    break;
                }
                // If there's an enemy piece in the way, we're gonna break after checking this square
                bool enemyPiece = (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128)));
                
                if (newY == dstY && newX == dstX) {
                    return true;
                }
                if (enemyPiece) {
                    break;
                }
                newY += moves[i].y;
                newX += moves[i].x;
            }
        }
    }
    else if (bit.gameTag() % 128 == Queen) {
        int start = 0;
        int end = 8;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            while (newY >= 0 && newY < 8 && newX >= 0 && newX < 8) {
                // If there's a friendly piece in the way, stop checking this direction
                if (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) == (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    break;
                }
                // If there's an enemy piece in the way, we're gonna break after checking this square
                bool enemyPiece = (!_grid[newY][newX].empty() && ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128)));
                
                if (newY == dstY && newX == dstX) {
                    return true;
                }
                if (enemyPiece) {
                    break;
                }
                newY += moves[i].y;
                newX += moves[i].x;
            }
        }
    }
    else if (bit.gameTag() % 128 == King) {
        int start = 0;
        int end = 8;
        for (int i = start; i < end; i++) {
            int newY = srcY + moves[i].y;
            int newX = srcX + moves[i].x;
            if (newY == dstY && newX == dstX) {
                // Check if the other square is empty, or contains an opposing piece, by comparing the gameTags
                // of both pieces.
                if (_grid[newY][newX].empty() || ((bit.gameTag() >= 128) != (_grid[newY][newX].bit()->gameTag() >= 128))) {
                    return true;
                }
            }
        }
        // For each player...
        for (int playerNumber = 0; playerNumber < 2; playerNumber++) {
            // ...check king side castling
            if (_castling[playerNumber * 2] == true) {
                if (_grid[playerNumber*7][5].empty() &&
                    _grid[playerNumber*7][6].empty() &&
                    dstY == playerNumber*7 && dstX == 6) {
                    return true;
                }
            }
            // ... and check queen side castling
            if (_castling[playerNumber * 2 + 1] == true) {
                if (_grid[playerNumber*7][3].empty() &&
                    _grid[playerNumber*7][2].empty() &&
                    _grid[playerNumber*7][1].empty() &&
                    dstY == playerNumber*7 && dstX == 2) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    
    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare& dstSquare = static_cast<ChessSquare&>(dst);

    // There are a number of things to check for if the piece that was just moved is a pawn...
    if (bit.gameTag() % 128 == Pawn) {
        // reset halfmove clock
        _halfmoveClock = 0;

        // Check if an en passant was made, or if a new one is available
        int direction = (bit.gameTag() < 128) ? 1 : -1;
        checkEnPassant(bit, srcSquare, dstSquare, direction);

        // If a pawn just reached the other side of the board, it should be promoted to a queen.
        int farthestRank = (bit.gameTag() < 128) ? 7 : 0;
        checkPawnPromotion(bit, srcSquare, dstSquare, farthestRank);
    } 
    else if (_captured == &dst) {
        _halfmoveClock = 0;
        _captured = nullptr;
    }
    else { _halfmoveClock++; }

    // If King just moved 2 spaces, it just castled, so that needs to be updated.
    checkCastlingRights(bit, srcSquare, dstSquare);

    if (gameHasAI() && getCurrentPlayer() && getCurrentPlayer()->isAIPlayer()) 
    {
        updateAI();
        return;
    }
    
    endTurn();
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    // check to see if either player has won
    return nullptr;
}

bool Chess::checkForDraw()
{
    // check to see if the board is full
    return false;
}

//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit* bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() & 127];
    } else {
        notation = '0';
    }
    return notation;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            s += bitToPieceNotation(y, x);
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int index = y * _gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            } else {
                _grid[y][x].setBit(nullptr);
            }
        }
    }
}

//
// Set the board to the state shown by a FEN string.
//
void Chess::FENtoBoard(const std::string& fen) {
    std::string piecePlacement;
    std::string activeColor = "w"; 
    std::string castlingString = "KQkq"; 
    std::string enPassantString = "-";
    _halfmoveClock = 0;
    int fullmoveNumber = 1;
    std::istringstream iss(fen);

    iss >> piecePlacement >> activeColor >> castlingString >> enPassantString >> _halfmoveClock >> fullmoveNumber;

    _gameOptions.currentTurnNo = (fullmoveNumber - 1) * 2;
    if (activeColor == "b") { _gameOptions.currentTurnNo++; }


    for (int i = 0; i < 4; i++) {
        _castling[i] = false;
    }
    if (castlingString != "-") {
        for (char c : castlingString) {
            if      (c == 'K')  { _castling[0] = true; }
            else if (c == 'Q')  { _castling[1] = true; }
            else if (c == 'k')  { _castling[2] = true; }
            else if (c == 'q')  { _castling[3] = true; }
        }
    }

    if (enPassantString != "-") {
        std::string ranks = "12345678";
        std::string files = "abcdefgh";
        size_t rank = ranks.find(enPassantString[1]);
        size_t file = files.find(enPassantString[0]);
        _enPassant = &_grid[rank][file];
    }
    else { _enPassant = nullptr; }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };

    int y = 7;
    int x = 0;
    for (char c : piecePlacement) {
        if (std::isdigit(c)) {
            x += (c - '0');
        }
        else if (c == '/') {
            y--;
            x = 0;
        }
        else if (strchr(wpieces, c)) {
            ChessPiece piece = static_cast<ChessPiece>(strchr(wpieces, c) - wpieces);
            Bit* bit = PieceForPlayer(0, piece);
            bit->setPosition(_grid[y][x].getPosition());
            bit->setParent(&_grid[y][x]);
            bit->setGameTag(piece);
            _grid[y][x].setBit(bit);
            x++;
        }
        else if (strchr(bpieces, c)) {
            ChessPiece piece = static_cast<ChessPiece>(strchr(bpieces, c) - bpieces);
            Bit* bit = PieceForPlayer(1, piece);
            bit->setPosition(_grid[y][x].getPosition());
            bit->setParent(&_grid[y][x]);
            bit->setGameTag(piece + 128);
            _grid[y][x].setBit(bit);
            x++;
        }
    }
}

//
// boardToFEN - return FEN string of the current board. Mostly just using this to debug
//
std::string Chess::boardToFEN() {
    std::string s;
    for (int y = 7; y >= 0; y--) {
        int spaces = 0;
        for (int x = 0; x < 8; x++) {
            char c = bitToPieceNotation(y, x);
            if (c == '0') {
                spaces++;
                if (x == 7) {
                    s += std::to_string(spaces);
                }
            }
            else {
                if (spaces) {
                    s += std::to_string(spaces);
                    spaces = 0;
                }
                s += c;
            }
        }
        if (y) { s += '/'; }
    }

    std::string castlingString = "";
    for (int i = 0; i < 4; i++) {
        if (_castling[i]) { 
            castlingString += "KQkq"[i]; 
        }
    }
    if (castlingString.empty()) { castlingString = "-"; }

    std::string enPassantString;
    if (_enPassant) {
        enPassantString = std::string(1, "abcdefgh"[_enPassant->getColumn()]) + std::string(1, "12345678"[_enPassant->getRow()]);
    } else { enPassantString = "-"; }

    s = s + " " + (getCurrentPlayer()->playerNumber() == 0 ? "w" : "b");
    s = s + " " + castlingString;
    s = s + " " + enPassantString;
    s = s + " " + std::to_string(_halfmoveClock);
    s = s + " " + std::to_string((_gameOptions.currentTurnNo / 2) + 1);
    return s;
}

//
// This function is kind of weird because Game.cpp will call this method on a move even
// if the move is then immediately cancelled for being an illegal one. So I made a separate
// variable to store the BitHolder of the piece that was just "captured," and we'll compare
// it with dst in bitMovedFromTo().
// 
void Chess::pieceTaken(Bit* bit) {
    _captured = bit->getHolder();
}

//
// First check if an En Passant move was just made. Then check if an En Passant move is available.
//
void Chess::checkEnPassant(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare, int direction) {
    int srcY = srcSquare.getRow();
    int srcX = srcSquare.getColumn();
    int dstY = dstSquare.getRow();
    int dstX = dstSquare.getColumn();
    int enpY = _enPassant ? _enPassant->getRow() : -8;
    int enpX = _enPassant ? _enPassant->getColumn() : -8;
    int enemyPawn = bit.gameTag() == Pawn ? Pawn + 128 : Pawn;

    if ((dstY == enpY && dstX == enpX) && 
        (srcY == enpY - direction && (srcX == enpX + 1 || srcX == enpX - 1))) {
        _grid[enpY - direction][enpX].destroyBit();
        _halfmoveClock = 0;
    }
    _enPassant = nullptr;

    if (dstY - srcY == 2 * direction) {
        _enPassant = &_grid[dstY - direction][dstX];
    }
}

//
// If a pawn reaches the end of the board, it gets promoted to a queen.
//
void Chess::checkPawnPromotion(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare, int farthestRank) {
    if (dstSquare.getRow() == farthestRank) {
        int playerNumber = getCurrentPlayer()->playerNumber();
        int friendlyQueen = bit.gameTag() - Pawn + Queen;

        Bit* newBit = PieceForPlayer(playerNumber, Queen);
        newBit->setPosition(dstSquare.getPosition());
        newBit->setParent(&dstSquare);
        newBit->setGameTag(friendlyQueen);
        dstSquare.setBit(newBit);
    }    
}

//
// This is for keeping track of when castling rights disappear. Additionally, figuring out the place
// where the King can move to will be handled in canBitMoveFromTo, but this function will also move the
// rook when a castle is made.
//
void Chess::checkCastlingRights(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare) {
    
    if (bit.gameTag() == King) {
        if (srcSquare.getColumn() == 4 && dstSquare.getColumn() == 6) {
            _castling[0] = false;
            _castling[1] = false;
            
            _grid[0][7].destroyBit();
            
            Bit* newBit = PieceForPlayer(0, Rook);
            newBit->setPosition(_grid[0][5].getPosition());
            newBit->setParent(&_grid[0][5]);
            newBit->setGameTag(Rook);
            _grid[0][5].setBit(newBit);
        }
        else if (srcSquare.getColumn() == 4 && dstSquare.getColumn() == 2) {
            _castling[0] = false;
            _castling[1] = false;
            
            _grid[0][0].destroyBit();
            
            Bit* newBit = PieceForPlayer(0, Rook);
            newBit->setPosition(_grid[0][3].getPosition());
            newBit->setParent(&_grid[0][3]);
            newBit->setGameTag(Rook);
            _grid[0][3].setBit(newBit);
        } 
    }
    else if (bit.gameTag() == King + 128) {
        if (srcSquare.getColumn() == 4 && dstSquare.getColumn() == 6) {
            _castling[2] = false;
            _castling[3] = false;
            
            _grid[7][7].destroyBit();
            
            Bit* newBit = PieceForPlayer(1, Rook);
            newBit->setPosition(_grid[7][5].getPosition());
            newBit->setParent(&_grid[7][5]);
            newBit->setGameTag(Rook + 128);
            _grid[7][5].setBit(newBit);
        }
        else if (srcSquare.getColumn() == 4 && dstSquare.getColumn() == 2) {
            _castling[2] = false;
            _castling[3] = false;
            
            _grid[7][0].destroyBit();
            
            Bit* newBit = PieceForPlayer(1, Rook);
            newBit->setPosition(_grid[7][3].getPosition());
            newBit->setParent(&_grid[7][3]);
            newBit->setGameTag(Rook + 128);
            _grid[7][3].setBit(newBit);
        } 
    }

    if (_castling[0] == true) {
        _castling[0] =  !_grid[0][7].empty() &&
                         _grid[0][7].bit()->gameTag() == Rook &&
                        !_grid[0][4].empty() &&
                         _grid[0][4].bit()->gameTag() == King;
    }
    if (_castling[1] == true) {
        _castling[1] =  !_grid[0][0].empty() &&
                         _grid[0][0].bit()->gameTag() == Rook &&
                        !_grid[0][4].empty() &&
                         _grid[0][4].bit()->gameTag() == King;
    }
    if (_castling[2] == true) {
        _castling[2] =  !_grid[7][7].empty() &&
                         _grid[7][7].bit()->gameTag() == Rook + 128 &&
                        !_grid[7][4].empty() &&
                         _grid[7][4].bit()->gameTag() == King + 128;
    }
    if (_castling[3] == true) {
        _castling[3] =  !_grid[7][0].empty() &&
                         _grid[7][0].bit()->gameTag() == Rook + 128 &&
                        !_grid[7][4].empty() &&
                         _grid[7][4].bit()->gameTag() == King + 128;
    }
}

//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
    if (checkForWinner() || checkForDraw()) {
        endTurn();
        return;
    }

    int bestScore = -1000000;
    int bestMoveYsrc = 0;
    int bestMoveXsrc = 0;
    int bestMoveYdst = 0;
    int bestMoveXdst = 0;
    
    for (int i = 0; i < 64; i++) {
        if (_grid[i/8][i%8].bit() && canBitMoveFrom(*_grid[i/8][i%8].bit(), _grid[i/8][i%8])) {
            for (int j = 0; j < 64; j++) {
                if (canBitMoveFromTo(*_grid[i/8][i%8].bit(), _grid[i/8][i%8], _grid[j/8][j%8])) {
                    std::cout << "testing a move" << std::endl; 
                    std::string newState = sampleMove(*_grid[i/8][i%8].bit(), _grid[i/8][i%8], _grid[j/8][j%8]);
                    int score = -negamax(newState, 0, -1);
                    if (score > bestScore) {
                        bestScore = score;
                        bestMoveYsrc = i/8;
                        bestMoveXsrc = i%8;
                        bestMoveYdst = j/8;
                        bestMoveXdst = j%8;
                    }
                }
            }
        }
    }
    
    ChessPiece tag = static_cast<ChessPiece>(_grid[bestMoveYsrc][bestMoveXsrc].bit()->gameTag() % 128);
    _grid[0][0].destroyBit();
            
    Bit* newBit = PieceForPlayer(1, tag);
    newBit->setPosition(_grid[bestMoveYdst][bestMoveXdst].getPosition());
    newBit->setParent(&_grid[bestMoveYdst][bestMoveXdst]);
    newBit->setGameTag(tag);
    _grid[bestMoveYdst][bestMoveXdst].setBit(newBit);
    
    endTurn();
}

// This function does a few things, in this order:
// 1) temporarily moves a piece from one square to another.
// 2) generates a FEN string of this new board state.
// 3) 
std::string Chess::sampleMove(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare) {
    // Whose move is this?
    int playerMoving = bit.getOwner()->playerNumber();

    std::string current_state = boardToFEN();

    // Is a piece about to be captured?
    char capturedPiece = bitToPieceNotation(dstSquare.getRow(), dstSquare.getColumn());



    // Simulate the consequences of moving a piece. Much of this can be copypasted from bitMovedFromTo().

    if (bit.gameTag() % 128 == Pawn) {
        // reset halfmove clock
        _halfmoveClock = 0;

        // Check if an en passant was made, or if a new one is available
        int direction = (bit.gameTag() < 128) ? 1 : -1;
        checkEnPassant(bit, srcSquare, dstSquare, direction);

        // If a pawn just reached the other side of the board, it should be promoted to a queen.
        int farthestRank = (bit.gameTag() < 128) ? 7 : 0;
        checkPawnPromotion(bit, srcSquare, dstSquare, farthestRank);
    } 
    else if (capturedPiece != '0') {
        _halfmoveClock = 0;
    }
    else { _halfmoveClock++; }

    // If King just moved 2 spaces, it just castled, so that needs to be updated.
    checkCastlingRights(bit, srcSquare, dstSquare);

    // Now get a FEN string for this new state
    std::string new_state = boardToFEN();

    // Revert board state
    FENtoBoard(current_state);

    return new_state;
}

int Chess::negamax(const std::string state, int depth, int playerColor) {
    int thisScore = playerColor * evaluateBoard(state);

    if (depth > 3)      { return thisScore; }

    int bestScore = -1000000;
    
    for (int i = 0; i < 64; i++) {
        if (_grid[i/8][i%8].bit() && canBitMoveFrom(*_grid[i/8][i%8].bit(), _grid[i/8][i%8])) {
            for (int j = 0; j < 64; j++) {
                if (canBitMoveFromTo(*_grid[i/8][i%8].bit(), _grid[i/8][i%8], _grid[j/8][j%8])) {
                    std::string newState = sampleMove(*_grid[i/8][i%8].bit(), _grid[i/8][i%8], _grid[j/8][j%8]);
                    int score = -negamax(newState, depth + 1, -playerColor);
                    if (score > bestScore) {
                        bestScore = score;
                    }
                }
            }
        }
    }
    return bestScore;
}