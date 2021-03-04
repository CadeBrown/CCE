/* Engine.cc - Implementation of 'cce::Engine'
 *
 * @author: Cade Brown <cade@cade.site>
 */

#include <cce.hh>

namespace cce {

void Engine::setstate(const State& state_) {
    lock.lock();

    state = state_;

    // Initialize to bad moves
    best_move = move();
    best_ev = eval(0.0f);

    lock.unlock();
}

void Engine::go() {
    lock.lock();
    
    // Search for best move
    best_move = findbestN(state, 2).first;

    //for (int i = 0; i < moves.size(); ++i) {
    //    cout << moves[i].LAN() << endl;
    //}

    lock.unlock();
}

void Engine::stop() {
    lock.lock();

    // TODO: Kill thread here
    
    // For now, just output 'e2e4'
    //best_move = move(TILE(4, 1), TILE(4, 3));
    //best_ev = eval(1.0f);

    lock.unlock();
}

// Scores for each piece
#define SCORE_Q (9.0)
#define SCORE_B (3.15)
#define SCORE_N (3.0)
#define SCORE_R (5.0)
#define SCORE_P (1.0)


// Scores for castling rights
#define SCORE_CK (0.4)
#define SCORE_CQ (0.3)

// Score for having the next move
#define SCORE_TOMOVE (0.15)

// Score per available move
#define SCORE_PERMOVE (0.1)

// Score for checking the enemy king
#define SCORE_CHECK (0.5)

// Constant for having any piece in a position
#define ADD_INPOS (0.13)

// Multiplier for having a piece in a position
#define MULT_INPOS (0.03)


// Moving to a position multiplier
#define MULT_TOPOS (0.08)

// Database of center values
static float db_centerval[64] = {
     0.33, 0.40, 0.46, 0.49, 0.49, 0.46, 0.40, 0.33 ,
     0.40, 0.49, 0.59, 0.65, 0.65, 0.59, 0.49, 0.40 ,
     0.46, 0.59, 0.73, 0.83, 0.83, 0.73, 0.59, 0.46 ,
     0.49, 0.65, 0.83, 0.96, 0.96, 0.83, 0.65, 0.49 ,
     0.49, 0.65, 0.83, 0.96, 0.96, 0.83, 0.65, 0.49 ,
     0.46, 0.59, 0.73, 0.83, 0.83, 0.73, 0.59, 0.46 ,
     0.40, 0.49, 0.59, 0.65, 0.65, 0.59, 0.49, 0.40 ,
     0.33, 0.40, 0.46, 0.49, 0.49, 0.46, 0.40, 0.33 ,
};


// Attack and defense score for a list of moves
static float my_adscore(const Engine& eng, const State& s, const vector<move>& moves) {
    
    // Count number of legal moves to a given square
    int numto[64];
    for (int i = 0; i < 64; ++i) numto[i] = 0;

    float res = 0.0f;

    // Add up moves to the center (which is being able to "defend" that square)
    for (int i = 0; i < moves.size(); ++i) {
        // Compute extra score based on number of defenders
        // 0.3f is just a magic constant... expirement with this!
        float numbonus = numto[moves[i].to] * 0.3f;

        // Add the center value and the bonus
        res += MULT_TOPOS * db_centerval[moves[i].to] * (1 + numbonus);
        numto[moves[i].to]++;
    }

    // Also, bonus points of the enemy king is attacked
    Color other = s.tomove == Color::WHITE ? Color::BLACK : Color::WHITE;

    int ntiles;
    int tiles[64];
    ntiles = bbtiles(s.piece[Piece::K] & s.color[other], tiles);
    assert(ntiles == 1); // Must have one king!

    if (s.is_attacked(tiles[0])) {
        // Other king is attacked
        res += SCORE_CHECK;
    }

    return res;
}

// Calculate a score for a particular color
static float my_score(const Engine& eng, const State& s, Color c) {
    // Positions of each piece
    int ntiles;
    int tiles[64];

    // Total material score for this color
    float mat = 0.0f;

    // Attacking/defending score
    float ads = 0.0f;

    // Positional score
    float pos = 0.0f;

    ntiles = bbtiles(s.color[c] & s.piece[Piece::Q], tiles);
    for (int i = 0; i < ntiles; ++i) {
        mat += SCORE_Q;
        pos += (SCORE_Q * MULT_INPOS + ADD_INPOS) * db_centerval[tiles[i]];
    }

    ntiles = bbtiles(s.color[c] & s.piece[Piece::B], tiles);
    for (int i = 0; i < ntiles; ++i) {
        mat += SCORE_B;
        pos += (SCORE_B * MULT_INPOS + ADD_INPOS) * db_centerval[tiles[i]];
    }

    ntiles = bbtiles(s.color[c] & s.piece[Piece::N], tiles);
    for (int i = 0; i < ntiles; ++i) {
        mat += SCORE_N;
        pos += (SCORE_N * MULT_INPOS + ADD_INPOS) * db_centerval[tiles[i]];
    }

    ntiles = bbtiles(s.color[c] & s.piece[Piece::R], tiles);
    for (int i = 0; i < ntiles; ++i) {
        mat += SCORE_R;
        pos += (SCORE_R * MULT_INPOS + ADD_INPOS) * db_centerval[tiles[i]];
    }

    ntiles = bbtiles(s.color[c] & s.piece[Piece::P], tiles);
    for (int i = 0; i < ntiles; ++i) {
        mat += SCORE_P;
        pos += (SCORE_P * MULT_INPOS + ADD_INPOS) * db_centerval[tiles[i]];
    }

    // Castling rights
    if (c == Color::WHITE) {
        if (s.c_WK) mat += SCORE_CK;
        if (s.c_WQ) mat += SCORE_CQ;
    } else {
        if (s.c_BK) mat += SCORE_CK;
        if (s.c_BQ) mat += SCORE_CQ;
    }


    if (s.tomove == c) {
        vector<move> moves;
        s.getmoves(moves);

        pos += SCORE_PERMOVE * moves.size();

        // Compute attacking and defining score
        ads += my_adscore(eng, s, moves);

    } else {
        // Generate a list of moves if the tomove was different
        State ns = s;
        ns.tomove = c;

        vector<move> moves;
        ns.getmoves(moves);

        pos += SCORE_PERMOVE * moves.size();

        // Compute attacking and defining score
        ads += my_adscore(eng, ns, moves);
    }

    // Misc. score
    float misc = 0.0f;
    if (s.tomove == c) {
        misc += SCORE_TOMOVE;
    }

    // Sum all parts of the score
    return mat + pos + ads + misc;
}

eval Engine::eval_static(const State& s) {

    // Check if the game is over
    int status;
    if (s.is_done(status)) {
        if (status == 0) {
            // Draw
            return eval::draw();
        } else {
            // Checkmate (in zero)
            return eval(status * INFINITY, 0);
        }
    }

    // Calculate score for each side
    float sW = my_score(*this, s, Color::WHITE), sB = my_score(*this, s, Color::BLACK);

    // Return the difference of the scores, so >0 means white is winning
    return eval(sW - sB);
}

#define FILEDEBUG(...) do { \
    FILE* fp = fopen("/home/cade/projects/cce/e.log", "a"); \
    fprintf(fp, __VA_ARGS__); \
    fclose(fp); \
} while (0)

pair<move, eval> Engine::findbest1(const State& s) {
    // Find legal moves
    vector<move> moves;
    s.getmoves(moves);
    // Return NULL move
    if (moves.size() == 0) return {move(), eval()};

    // Best index
    int bi = -1;
    eval be = eval();
    for (int i = 0; i < moves.size(); ++i) {

        // Try applying the move
        State ns = s;
        ns.apply(moves[i]);
        eval ev = eval_static(ns);
        if (bi < 0) {
            bi = i;
            be = ev;
        } else if (s.tomove == Color::WHITE) {
            if (eval::cmp(ev, be) > 0) {
                bi = i;
                be = ev;
            }
        } else if (s.tomove == Color::BLACK) {
            if (eval::cmp(ev, be) < 0) {
                bi = i;
                be = ev;
            }
        }
    }

    return {moves[bi], be};
}

pair<move, eval> Engine::findbestN(const State& s, int dep) {
    // Base case to end recursion
    if (dep <= 1) return findbest1(s);

    // Otherwise, let's search through all possible moves
    vector<move> moves;
    s.getmoves(moves);
    if (moves.size() == 0) {
        // Need to handle ended games
        int status;
        s.is_done(status);

        return pair<move, eval>(move(), eval(INFINITY * status, 0));
    }

    // Best index
    int bi = -1;
    eval be = eval();
    for (int i = 0; i < moves.size(); ++i) {
        State ns = s;
        ns.apply(moves[i]);

        // Find best move in new position
        pair<move, eval> bm = findbestN(ns, dep-1);
        if (bi < 0) {
            bi = i;
            be = bm.second;
        } else if (s.tomove == Color::WHITE) {
            if (eval::cmp(bm.second, be) > 0) {
                bi = i;
                be = bm.second;
            }
        } else if (s.tomove == Color::BLACK) {
            if (eval::cmp(bm.second, be) < 0) {
                bi = i;
                be = bm.second;
            }
        }
    }
    
    return {moves[bi], be};
}


}
