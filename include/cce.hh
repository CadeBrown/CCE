/* cce.hh - cce (Cade's Chess Engine) header
 *
 * 
 * 
 * @author: Cade Brown <cade@cade.site>
 */

// C standard
#include <assert.h>
#include <stdint.h>

// Stanard library
#include <iostream>
#include <sstream>
#include <cmath>

// Multithreading support
#include <mutex>
#include <thread>


// STL
#include <string>
#include <algorithm>
#include <vector>

// Use C++ standard libary without 'std::' prefix
using namespace std;


namespace cce {


/* Constants */

// Color enumeration
enum Color {
    // W: White
    WHITE    = 0,
    // B: Black
    BLACK    = 1,
};

// Piece enumeration
enum Piece {

    // K: King
    K        = 0,
    // Q: Queen
    Q        = 1,
    // B: Bishop
    B        = 2,
    // N: Knight (since K is used for King)
    N        = 3,
    // R: Rook
    R        = 4,
    // P: Pawn
    P        = 5,

};

// Number of colors
#define N_COLORS 2

// Number of pieces
#define N_PIECES 6

// FEN for the starting position
#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

/* Util Macros */

// Creates a position from the board, from a rank (_i) and file (_j)
// NOTE: These are 0-indexed! For example, the square 'a1' is TILE(0, 0)
// Examples:
// b3 -> TILE(b->1, 3->2) = TILE(1, 2) 
#define TILE(_i, _j) ((_i) + 8 * (_j))

// Un-does the 'TILE' macro on '_val', and decomposes it into the '_i' and '_j' variables
#define UNTILE(_i, _j, _val) do { \
    int val_ = _val; \
    (_i) = val_ % 8; \
    (_j) = val_ / 8; \
} while (0)


/* Types */


// cce::bb - Bitboard integer type
//
// A bitboard is a collection of 64 bits, one corresponding to each tile
//   on the board. They typically answer a question like "is a piece on tile X?",
//   and can be combined with bitwise operators
//
// Use 'ONEHOT(_i)' to create a bitboard with a single bit set
//
typedef uint64_t bb;

// Creates a bitmask from a single bit, _i, as a 1 bit, the rest being zeros
#define ONEHOT(_i) (1ULL << (_i))


/* Utilities */

// Compute a list of the tiles in a bitboard, returning the number, and storing in 'pos'
// NOTE: 'pos' should be able to hold '64' integers
int bbtiles(bb v, int pos[64]);

// Returns a string representing the algebraic name for a tile
const string& tile_name(int tile);

// Returns a string representing a specific color and piece
// White are uppercase, black are lowercase
const string& cp_name(Color c, Piece p);


// cce::move - Simple move structure, just containing the from and to 
//
//
struct move {

    // Tile being moved from
    int from;

    // Tile being moved to
    int to;

    move(int from_=-1, int to_=-1) : from(from_), to(to_) {}

    // Returns whether the move is unintialized or out of range

    bool isbad() const { return from < 0 || to < 0; }

    // Return long algebraic notation
    string LAN() const { return isbad() ? "0000" : tile_name(from) + tile_name(to); }
};

// cce::State - Chess board state
//
// This is like the board, except it also keeps bits storing
//   castling rights, en-passant, etc
//
struct State {
    
    // Bitboard telling which color occupies which tile
    bb color[N_COLORS];

    // Bitboard array (indexed by 'Piece::*' enum members) telling which
    //   pieces are located where on the board
    bb piece[N_PIECES];

    // Which color is about to move?
    Color tomove;

    // Castling rights, whether W==white, B==black, K==Kingside, Q==Queenside
    // i.e. 'c_WK' tells whether the White king can castle Kingside
    bool c_WK, c_WQ;
    bool c_BK, c_BQ;

    // En-passant target square (i.e. the square on which it could be captured)
    // Or, -1 if there is no such square
    // This should be reset every half-move
    int ep;

    // Half-moves since last capture or pawn advance
    // Used for the fifty-move rule
    int hmclock;

    // The number of full-moves, starting at 0, and incremented after black's move
    int fullmove;

    State() {
        for (int i = 0; i < N_COLORS; ++i) {
            color[i] = 0;
        }
        for (int i = 0; i < N_PIECES; ++i) {
            piece[i] = 0;
        }
        tomove = Color::WHITE;
        c_WK = c_WQ = c_BK = c_BQ = true;
        ep = -1;
        hmclock = 0;
        fullmove = 0;
    }

    // Create a new state from FEN notation
    // SEE: https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
    static State from_FEN(const string& fen);

    // Convert to FEN notation
    // SEE: https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
    string to_FEN() const;

    // Gets a list of valid moves (from the 'tomove's players perspective), populating 'res'
    // Clears 'res' first
    // If 'ignorepins==true', then generate moves ignoring pinned pieces
    // If 'ignorecastling==true', then generate moves ignoring castling
    void getmoves(vector<move>& res, bool ignorepins=false, bool ignorecastling=false) const;

    // Queries a tile on the board, and returns whether it is occupied
    // If it was occupied, sets 'c' and 'p' to the color and piece that occupied
    //   it, respectively
    bool query(int tile, Color& c, Piece& p) const {
        bb m = ONEHOT(tile);
        for (int i = 0; i < N_PIECES; ++i) {
            if (piece[i] & m) {
                // Found
                c = (color[Color::WHITE] & m) ? Color::WHITE : Color::BLACK;
                p = Piece(i);
                return true;
            }
        }
        // Not found
        return false;
    }

    // Apply a move to a state
    void apply(const move& mv) {
        // Get masks
        bb mf = ONEHOT(mv.from), mt = ONEHOT(mv.to);
        Color other = tomove == Color::WHITE ? Color::BLACK : Color::WHITE;

        int i, p;
        for (p = 0; p < N_PIECES; ++p) {
            if (piece[p] & mf) {
                // Found piece moving from
                break;
            }
        }
        // Assert we found a piece from the place that is moving!
        //assert(p < N_PIECES);
        if (p >= N_PIECES) {
            cout << "IN " << to_FEN() << " MOVE " << mv.LAN() << endl;
        }

        // Remove where it was moving from
        color[tomove] &= ~mf;
        color[other] &= ~mf;

        // Now, add it where it is moving to, and clear opposite color tile if it was present
        color[tomove] |= mt;
        color[other] &= ~mt;


        // Remove piece where it was moving from
        piece[p] &= ~mf;

        // Remove from all pieces
        for (i = 0; i < N_PIECES; ++i) {
            piece[i] &= ~mt;
        }

        // Add it back where it is moving to
        piece[p] |= mt;


        // Handle castling
        if (tomove == Color::WHITE) {
            if (mv.from == TILE(4, 0)) {
                if (mv.to == TILE(6, 0) && c_WK) {
                    // White kingside
                    bb rf = ONEHOT(TILE(7, 0));
                    bb rt = ONEHOT(TILE(5, 0));
                    color[Color::WHITE] &= ~rf;
                    color[Color::WHITE] |= rt;
                    piece[Piece::R] &= ~rf;
                    piece[Piece::R] |= rt;
                } else if (mv.to == TILE(2, 0) && c_WQ) {
                    // White queenside
                    bb rf = ONEHOT(TILE(0, 0));
                    bb rt = ONEHOT(TILE(3, 0));
                    color[Color::WHITE] &= ~rf;
                    color[Color::WHITE] |= rt;
                    piece[Piece::R] &= ~rf;
                    piece[Piece::R] |= rt;
                }
                c_WK = c_WQ = false;
            }
            if (mv.from == TILE(0, 0)) {
                c_WQ = false;
            } else if (mv.from == TILE(0, 7)) {
                c_WK = false;
            }
        } else {
            if (mv.from == TILE(4, 7)) {
                if (mv.to == TILE(6, 7) && c_BK) {
                    // Black kingside
                    bb rf = ONEHOT(TILE(7, 7));
                    bb rt = ONEHOT(TILE(5, 7));
                    color[Color::WHITE] &= ~rf;
                    color[Color::WHITE] |= rt;
                    piece[Piece::R] &= ~rf;
                    piece[Piece::R] |= rt;
                    c_WK = false;
                } else if (mv.to == TILE(2, 7) && c_BQ) {
                    // Black queenside
                    bb rf = ONEHOT(TILE(0, 7));
                    bb rt = ONEHOT(TILE(3, 7));
                    color[Color::WHITE] &= ~rf;
                    color[Color::WHITE] |= rt;
                    piece[Piece::R] &= ~rf;
                    piece[Piece::R] |= rt;
                }

                c_BK = c_BQ = false;
            }

            if (mv.from == TILE(7, 0)) {
                c_BQ = false;
            } else if (mv.from == TILE(7, 7)) {
                c_BK = false;
            }
        }



        // TODO: Handle en passant
        ep = -1;

        // Increment half move clock (TODO: check if it should be reset)
        hmclock++;

        // Now, increment state variables
        if (tomove == Color::WHITE) {
            // White
            tomove = Color::BLACK;
        } else {
            // Black
            fullmove++;
            tomove = Color::WHITE;
        }

    }

    // Returns whether the tile 'tile' is being attacked by the color about to move
    bool is_attacked(int tile) const;

    // Calculates whether the state represents a finished game, either by stalemate or checkmate (or draw
    //   due to repetition)
    // Stores status the winner, +1==white, 0==draw, -1==black
    bool is_done(int& status) const;

};

// cce::eval - Chess position evaluation
//
//
struct eval {

    // Score, in pawns, for white
    //   if > 0, then position is better for white
    //   if < 0, then position is better for black
    // If 'score==INFINITY' or 'score==-INFINITY', there is a forced checkmate in 'matein' moves
    float score;

    // Number of moves until checkmate (assuming best play)
    // Only used if 'score == INFINITY' (checkmate for white) or 'score == -INFINITY' (checkmate for black)
    int matein;

    // Contructor
    eval(float score_=0.0f, int matein_=-1) : score(score_), matein(matein_) {}

    // Return a forced draw
    static eval draw() {
        return eval(NAN);
    }

    // Returns whether the evaluation is a forced draw
    bool isdraw() const { return isnan(score); }

    // Returns whether the evaluation is a forced checkmate
    bool ismate() const { return isinf(score); }

    // Return evaluation as a string
    string getstr() const {
        if (isdraw()) {
            return "DRAW";
        } else if (ismate()) {
            if (score >= 0) {
                // + for white
                return "M+" + to_string(matein);
            } else {
                // - for black
                return "M-" + to_string(matein);
            }
        } else {
            // Use snprintf so we can specify that only 2 decimal digits should be printed
            char tmp[64];
            snprintf(tmp, sizeof(tmp) - 1, "%+.2f", score);
            return (string)tmp;
        }
    }

    // Comparator
    //   if > 0, then b is better for white than a
    //   if = 0, then a is the same as b
    //   if < 0, then b is worse for white than a
    static int cmp(const eval& a, const eval& b) {
        if (a.score > b.score) {
            return +1;
        } else if (a.score < b.score) {
            return -1;
        } else {
            if (a.ismate() && b.ismate()) {
                
                if (a.score > 0) {
                    // The checkmate is for white
                    // Choose faster checkmate
                    if (a.matein < b.matein) {
                        return +1;
                    } else {
                        return -1;
                    }
                } else {
                    // The checkmate is for black
                    // Choose slower checkmate
                    if (a.matein > b.matein) {
                        return +1;
                    } else {
                        return -1;
                    }
                }
            } else {
                // Draw
                return 0;
            }
        }
    }

};



// cce::Engine - Chess engine implementation
//
//
struct Engine {

    // Lock required to read/write variables on this engine
    mutex lock;

    // Computing thread which runs the number crunching
    thread thd_compute;

    // The current best move for the starting position
    // NOTE: Check 'isbad()' to see if it is uninitialized
    move best_move;

    // The evaluation for 'best_move'
    eval best_ev;

    // Current state the engine is analyzing
    State state;

    // Set the current state the engine should analyze
    void setstate(const State& state_);

    // Start computing the current position
    void go();

    // Stop computing the current position
    void stop();


    // Static evaluation method, which does not recurse or check move combinations
    eval eval_static(const State& s);

    // Find the best move, by using the evaluation function with a single move depth
    pair<move, eval> findbest1(const State& s);

    // Find the best move with a given depth, brute force search
    pair<move, eval> findbestN(const State& s, int dep=1);

};



}
