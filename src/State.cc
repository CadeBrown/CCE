/* State.cc - Implementation of 'cce::State'
 *
 * @author: Cade Brown <cade@cade.site>
 */

#include <cce.hh>

namespace cce {

State State::from_FEN(const string& fen) {
    State r;
    for (int i = 0; i < N_COLORS; ++i) {
        r.color[i] = 0;
    }
    for (int i = 0; i < N_PIECES; ++i) {
        r.piece[i] = 0;
    }

    // Position in the string
    int pos = 0;

    // Input pieces on the board
    for (int j = 7; j >= 0; --j) {

        // File position
        int i = 0;

        // Keep going until '/' is hit (rank seperator), or we run out of input, or go out
        //   of range
        while (pos < fen.size() && fen[pos] != '/' && i < 8) {
            char chr = fen[pos];
            pos++;

            Color c;
            Piece p;

            // Check whether empty spaces, or a piece located on the relevant square
            if ('0' <= chr && chr <= '9') {
                // Skip these squares
                i += chr - '0';
                continue;

            } else if (chr == 'K') {
                c = Color::WHITE;
                p = Piece::K;   
            } else if (chr == 'Q') {
                c = Color::WHITE;
                p = Piece::Q;
            } else if (chr == 'B') {
                c = Color::WHITE;
                p = Piece::B;
            } else if (chr == 'N') {
                c = Color::WHITE;
                p = Piece::N;
            } else if (chr == 'R') {
                c = Color::WHITE;
                p = Piece::R;
            } else if (chr == 'P') {
                c = Color::WHITE;
                p = Piece::P;
            } else if (chr == 'k') {
                c = Color::BLACK;
                p = Piece::K;   
            } else if (chr == 'q') {
                c = Color::BLACK;
                p = Piece::Q;
            } else if (chr == 'b') {
                c = Color::BLACK;
                p = Piece::B;
            } else if (chr == 'n') {
                c = Color::BLACK;
                p = Piece::N;
            } else if (chr == 'r') {
                c = Color::BLACK;
                p = Piece::R;
            } else if (chr == 'p') {
                c = Color::BLACK;
                p = Piece::P;
            }
            
            // Create mask of the position where we should insert the piece
            bb m = ONEHOT(TILE(i, j));
            // Set color
            r.color[c] |= m;
            // Make sure the correct bit is set for this piece
            r.piece[p] |= m;
            
            // Advance position
            i++;
        }

        // Skip seperator
        if (j >= 0 + 1) {
            assert(fen[pos] == '/');
            pos++;
        }
    }

    // Parse active color
    assert(fen[pos] == ' ');
    pos++;

    if (fen[pos] == 'w') {
        r.tomove = Color::WHITE;
    } else if (fen[pos] == 'b') {
        r.tomove = Color::BLACK;
    } else {
        // Error, ignore for now
    }
    pos++;

    // Parse castling availability
    assert(fen[pos] == ' ');
    pos++;
    r.c_WK = r.c_WQ = r.c_BK = r.c_BQ = false;

    while (pos < fen.size() && fen[pos] != ' ') {
        char chr = fen[pos];
        pos++;
        if (chr == '-') {
            break;
        } else if (chr == 'K') {
            r.c_WK = true;
        } else if (chr == 'Q') {
            r.c_WQ = true;
        } else if (chr == 'k') {
            r.c_BK = true;
        } else if (chr == 'q') {
            r.c_BQ = true;
        } else {
            // Error, ignore for now
        }
    }

    // Parse en passant target square
    assert(fen[pos] == ' ');
    pos++;

    if (fen[pos] == '-') {
        // No en-passant square
        r.ep = -1;
        pos++;
    } else {
        // Get position
        // Right now, assumes input is correct
        char c0 = fen[pos];
        pos++;
        char c1 = fen[pos];
        pos++;

        // Construct from offsets
        r.ep = TILE(c0 - 'a', c1 - '1');
    }

    // Parse half move clock
    assert(fen[pos] == ' ');
    pos++;

    // Get integer portion
    int spos = pos;
    int slen = 0;
    while (spos + slen < fen.size() && fen[spos + slen] != ' ') {
        slen++;
        pos++;
    }
    r.hmclock = stoi(fen.substr(spos, slen));

    assert(fen[pos] == ' ');
    pos++;

    // Parse full move number
    // Subtract one due to 0-based indexing
    r.fullmove = stoi(fen.substr(pos)) - 1;

    return r;
}

string State::to_FEN() const {
    string r;

    // Output pieces on the board, rank-by-rank
    for (int j = 7; j >= 0; --j) {
        if (j < 7) r += '/';
        // Keep track of empty squares, and we put a digit representing how many
        //   are empty
        int empty = 0;

        // For query output
        Color c;
        Piece p;

        for (int i = 0; i < 8; ++i) {
            if (query(TILE(i, j), c, p)) {
                // Found piece on this tile

                // Dump seperator, if there were empty tiles
                if (empty > 0) r += to_string(empty);
                empty = 0;

                // Now, output the color and piece name
                r += cp_name(c, p);
            } else {
                // Empty tile
                empty++;
            }
        }
        // Dump seperator, if there were empty tiles
        if (empty > 0) r += to_string(empty);
    }

    // Output active color
    r += ' ';
    if (tomove == Color::WHITE) {
        r += 'w';
    } else {
        r += 'b';
    }

    // Castling rights
    r += ' ';
    if (c_WK || c_WQ || c_BK || c_BQ) {
        if (c_WK) r += 'K';
        if (c_WQ) r += 'Q';
        if (c_BK) r += 'k';
        if (c_BQ) r += 'q';
    } else {
        r += '-';
    }

    // En-passant target square
    r += ' ';
    if (ep < 0) {
        r += '-';
    } else {
        r += tile_name(ep);
    }

    // Half-move clock
    r += ' ';
    r += to_string(hmclock);


    // Full-move number
    r += ' ';

    // +1, since it is 1 indexed
    r += to_string(fullmove + 1);


    return r;
}

// Internal method to determine whether `mv` is a valid move
bool isvalid(const State& s, const move& mv) {
    if (mv.isbad()) {
        // Out of range moves are never good
        return false;
    }

    bb mf = ONEHOT(mv.from), mt = ONEHOT(mv.to);
    bool issame = (s.color[s.tomove] & mf) != 0 && (s.color[s.tomove] & mt) != 0;
    if (issame) {
        // Can't move where your piece is
        return false;
    }

    // Create a new state
    State ns = s;

    // Apply move
    ns.apply(mv);

    // Now, determine if it was valid
    return true;
}


void State::getmoves(vector<move>& res) const {
    res.clear();

    // Positions of various pieces
    int ntiles;
    int tiles[64];

    // Get the color mask to modify pieces with
    bb cmask = color[tomove];

    // Try and add '_mv', by checking if it is legal
    #define TRYADD(...) do { \
        move mv_ = __VA_ARGS__; \
        if (isvalid(*this, mv_)) { \
            res.push_back(mv_); \
        } \
    } while (0)

    int from;
    int i, j;

    /* Generate king moves */
    ntiles = bbtiles(piece[Piece::K] & cmask, tiles);
    assert(ntiles == 1); // Must have exactly 1 king!

    from = tiles[0];
    UNTILE(i, j, from);
    
    if (i >= 1 && j >= 1) TRYADD({from, TILE(i-1, j-1)});
    if (i >= 1) TRYADD({from, TILE(i-1, j)});
    if (i >= 1 && j <= 6) TRYADD({from, TILE(i-1, j+1)});
    
    if (j >= 1) TRYADD({from, TILE(i, j-1)});
    if (j <= 6) TRYADD({from, TILE(i, j+1)});
    
    if (i <= 6 && j >= 1) TRYADD({from, TILE(i+1, j-1)});
    if (i <= 6) TRYADD({from, TILE(i+1, j)});
    if (i <= 6 && j <= 6)TRYADD({from, TILE(i+1, j+1)});

    /* Generate queen moves */

    /* Generate bishop moves */

    /* Generate knight moves */
    ntiles = bbtiles(piece[Piece::N] & cmask, tiles);
    for (int k = 0; k < ntiles; ++k) {
        from = tiles[k];
        UNTILE(i, j, from);

        // Consider all tiles (+-1, +-2) and (+-2, +-1) away, if they are in range
        if (i <= 6 && j <= 5) TRYADD({from, TILE(i+1, j+2)});
        if (i >= 1 && j <= 5) TRYADD({from, TILE(i-1, j+2)});
        if (i <= 6 && j >= 2) TRYADD({from, TILE(i+1, j-2)});
        if (i >= 1 && j >= 2) TRYADD({from, TILE(i-1, j-2)});

        if (i <= 5 && j <= 6) TRYADD({from, TILE(i+2, j+1)});
        if (i >= 2 && j <= 6) TRYADD({from, TILE(i-2, j+1)});
        if (i <= 5 && j >= 1) TRYADD({from, TILE(i+2, j-1)});
        if (i >= 2 && j >= 1) TRYADD({from, TILE(i-2, j-1)});
    }

    /* Generate rook moves */

    /* Generate pawn moves */
    ntiles = bbtiles(piece[Piece::P] & cmask, tiles);
    for (int k = 0; k < ntiles; ++k) {
        from = tiles[k];
        UNTILE(i, j, from);

        if (tomove == Color::WHITE) {
            // White pawns move up
            if (j == 1) {
                // Can move 2 tiles
                TRYADD({from, TILE(i, j+2)});
            }
            TRYADD({from, TILE(i, j+1)});
        } else {
            // Black pawns move down
            if (j == 6) {
                // Can move 2 tiles
                TRYADD({from, TILE(i, j-2)});
            }
            TRYADD({from, TILE(i, j-1)});
        }
    }

}


bool State::is_attacked(int tile, Color by) const {

    return false;
}



}
