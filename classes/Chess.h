#pragma once
#include "Game.h"
#include "ChessSquare.h"

const int pieceSize = 64;

enum ChessPiece {
    NoPiece = 0,
    Pawn = 1,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

//
// this is the AI class
// we use a small clone here so we can recursively call minimax
//
// struct ChessAI
// {
//     int evaluateBoard();
//     int evaluateBoardMinimax();
//     int minimax(TicTacToeAI* state, int depth, bool isMaximizingPlayer);
//     int negamax(TicTacToeAI* state, int depth, int playerColor);
//     int ownerAt(int index ) const;
//     int AICheckForWinner();
//     // new helper function
//     TicTacToeAI* clone();
// };

//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder& holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    // New FEN string methods + other stuff
    void        FENtoBoard(const std::string& fen);
    std::string boardToFEN();
    void        checkEnPassant(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare, int direction);
    void        checkPawnPromotion(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare, int farthestRank);
    void        checkCastlingRights(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare);
    void        pieceTaken(Bit* bit) override;

    // New AI-related functions:
    int         evaluateBoard(const std::string state);
    std::string sampleMove(Bit& bit, ChessSquare& srcSquare, ChessSquare& dstSquare);
    int         negamax(const std::string state, int depth, int playerColor);

    void        stopGame() override;
    BitHolder& getHolderAt(const int x, const int y) override { return _grid[y][x]; }

	void        updateAI() override;
    bool        gameHasAI() override { return true; }
private:
    Bit *       PieceForPlayer(const int playerNumber, ChessPiece piece);
    const char  bitToPieceNotation(int row, int column) const;

    ChessSquare     _grid[8][8];
    
    // New private variables
    BitHolder*      _captured;
    bool            _castling[4]; // booleans for K, Q, k, q, in that order
    ChessSquare*    _enPassant;
    int             _halfmoveClock;

    // New private variable to detect if anyone is in check.
    // 0 if player 0 is in check, 1 if player 1 is in check, -1 if no one is in check.
    // int _check;
};

