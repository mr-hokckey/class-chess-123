#include "Chess.h"

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
    // Made this array so we can iterate through it for the back ranks.
    ChessPiece backRankPieces[8] = { Rook, Knight, Bishop, Queen, King, Bishop, Knight, Rook };
    
    for (int x = 0; x < _gameOptions.rowX; x++) {
        Bit* bit = PieceForPlayer(1, backRankPieces[x]);
        bit->setPosition(_grid[7][x].getPosition());
        bit->setParent(&_grid[7][x]);
        bit->setGameTag(backRankPieces[x] + 128); // For black pieces, set game tag to enum + 128
        _grid[7][x].setBit(bit);
    }

    for (int x = 0; x < _gameOptions.rowX; x++) {
        Bit* bit = PieceForPlayer(1, Pawn);
        bit->setPosition(_grid[6][x].getPosition());
        bit->setParent(&_grid[6][x]);
        bit->setGameTag(Pawn + 128); // For black pieces, set game tag to enum + 128 
        _grid[6][x].setBit(bit);
    }

    for (int x = 0; x < _gameOptions.rowX; x++) {
        Bit* bit = PieceForPlayer(0, Pawn);
        bit->setPosition(_grid[1][x].getPosition());
        bit->setParent(&_grid[1][x]);
        bit->setGameTag(Pawn);
        _grid[1][x].setBit(bit);
    }

    for (int x = 0; x < _gameOptions.rowX; x++) {
        Bit* bit = PieceForPlayer(0, backRankPieces[x]);
        bit->setPosition(_grid[0][x].getPosition());
        bit->setParent(&_grid[0][x]);
        bit->setGameTag(backRankPieces[x]);
        _grid[0][x].setBit(bit);
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
    // if (bit.getOwner() != getCurrentPlayer()) {
    //     return false;
    // }

    // ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    // int y = srcSquare.getRow();
    // int x = srcSquare.getColumn();
    // int pieceTag = bit.gameTag();


    // bool canBitMove = true;
    // // Pawns cannot move if they're at the end of the board,
    // // or if they're blocked by a piece in front of it and there's nothing to capture on the diagonals.
    // if (pieceTag % 128 == Pawn) {
    //     int direction = pieceTag < 128 ? 1 : -1;
    //     if (y + direction >= 8 || y + direction < 0) {
    //         canBitMove = false;
    //     }
    //     if (!_grid[y + direction][x].empty()) {
    //         canBitMove = false;
    //     }
    // }
    // else if (bit.gameTag() % 128 == Knight) {

    // }
    // else if (bit.gameTag() % 128 == Bishop) {

    // }
    // else if (bit.gameTag() % 128 == Rook) {

    // }
    // else if (bit.gameTag() % 128 == Queen) {

    // }
    // else if (bit.gameTag() % 128 == King) {
        
    // }

    // return canBitMove;

    // Above was the code I started working on, but realized halfway through that
    // it was a nightmare to write, and isn't the established correct way of doing
    // things. So I copypasted this code from Discord and started working on the
    // generated moves lists.
    
    // ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    // bool canMove = false;
    // for (auto move : _moves) {
    //     if (move.from == srcSquare.getSquareIndex()) {
    //         canMove = true;
    //         for (int y = 0; y < _gameOptions.rowY; y++) {
    //             for (int x = 0; x < _gameOptions.rowX; x++) {
    //                 ChessSquare& dstSquare = _grid[y][x];
    //                 if (move.to == dstSquare.getSquareIndex()) {
    //                     dstSquare.setMoveHighlighted(true);
    //                 }
    //             }
    //         }
    //     }
    // }
    // return canMove;
    return true;
}

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    return true;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    endTurn();
    generateMoves();
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

// Here are the helper functions I'm writing:

void Chess::generateMoves() {
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int piece = _grid[y][x].gameTag();
        }
    }
}


//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
}

