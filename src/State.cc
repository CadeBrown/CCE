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
bool isvalid(const State& s, const move& mv, bool ignorepins) {
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

    // At this point, just return early
    if (ignorepins) return true; 

    // Create a new state
    State ns = s;


    // Apply move
    ns.apply(mv);
    assert(ns.tomove != s.tomove);

    // Get king position and see if it is attacked

    // Positions of various pieces
    int ntiles;
    int tiles[64];

    /* Generate king */
    ntiles = bbtiles(ns.piece[Piece::K] & ns.color[s.tomove], tiles);
    assert(ntiles == 1); // Must have exactly 1 king!

    // Now, switch back
    if (ns.is_attacked(tiles[0])) {
        // King cannot be attacked!
        return false;
    }


    // Now, determine if it was valid
    return true;
}


bool State::is_attacked(int tile) const {

    // Super inefficient! But more correct... We can speed it up later
    vector<move> moves;
    getmoves(moves, true, true);

    int i;
    for (i = 0; i < moves.size(); ++i) {
        if (moves[i].to == tile) return true;
    }

    return false;
}

bool State::is_done(int& status) const {
    vector<move> moves;
    getmoves(moves);
    if (moves.size() == 0) {

        // Get king's position
        int ntiles;
        int tiles[64];
        ntiles = bbtiles(piece[Piece::K] & color[tomove], tiles);
        assert(ntiles == 1); // Must have exactly 1 king!

        // Check state from opposite perspective
        State ns = *this;
        ns.tomove = tomove == Color::WHITE ? Color::BLACK : Color::WHITE;

        if (ns.is_attacked(tiles[0])) {
            // Checkmate, the king is attacked and there are no legal moves
            status = tomove == Color::WHITE ? -1 : +1;
            return true;
        } else {
            // Stalemate, the king is not attacked and there are no legal moves
            status = 0;
            return true;
        }
    } else {

        // Game is not over, there are still legal moves
        status = 0;
        return false;
    }
}

void State::getmoves(vector<move>& res, bool ignorepins, bool ignorecastling) const {
    res.clear();

    // Positions of various pieces
    int ntiles;
    int tiles[64];

    // Get the color mask to modify pieces with
    bb cmask = color[tomove], omask = color[tomove == Color::WHITE ? Color::BLACK : Color::WHITE];

    // Try and add '_mv', by checking if it is legal
    #define TRYADD(...) do { \
        move mv_ = __VA_ARGS__; \
        if (isvalid(*this, mv_, ignorepins)) { \
            res.push_back(mv_); \
        } \
    } while (0)

    int from, to;
    int i, j;
    bb m;

    /* Generate king moves */
    ntiles = bbtiles(piece[Piece::K] & cmask, tiles);
    if (ntiles != 1) {
        return;
    }
    assert(ntiles == 1); // Must have exactly 1 king!

    int kingpos = from = tiles[0];
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
    ntiles = bbtiles(piece[Piece::Q] & cmask, tiles);
    for (int k = 0; k < ntiles; ++k) {
        from = tiles[k];
        UNTILE(i, j, from);

        // Add [i+n, j+n]
        for (int n = 1; i + n < 8 && j + n < 8; ++n) {
            to = TILE(i+n, j+n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        // Add [i+n, j-n]
        for (int n = 1; i + n < 8 && j - n >= 0; ++n) {
            to = TILE(i+n, j-n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        // Add [i-n, j+n]
        for (int n = 1; i - n >= 0 && j + n < 8; ++n) {
            to = TILE(i-n, j+n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        // Add [i-n, j-n]
        for (int n = 1; i - n >= 0 && j - n >= 0; ++n) {
            to = TILE(i-n, j-n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }


        // Add [*, j]
        for (int ti = i-1; ti >= 0; --ti) {
            to = TILE(ti, j);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        for (int ti = i+1; ti < 8; ++ti) {
            to = TILE(ti, j);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }

        // Add [i, *]
        for (int tj = j-1; tj >=0; --tj) {
            to = TILE(i, tj);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        for (int tj = j+1; tj < 8; ++tj) {
            to = TILE(i, tj);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
    }

    /* Generate bishop moves */
    ntiles = bbtiles(piece[Piece::B] & cmask, tiles);
    for (int k = 0; k < ntiles; ++k) {
        from = tiles[k];
        UNTILE(i, j, from);

        // Add [i+n, j+n]
        for (int n = 1; i + n < 8 && j + n < 8; ++n) {
            to = TILE(i+n, j+n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        // Add [i+n, j-n]
        for (int n = 1; i + n < 8 && j - n >= 0; ++n) {
            to = TILE(i+n, j-n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        // Add [i-n, j+n]
        for (int n = 1; i - n >= 0 && j + n < 8; ++n) {
            to = TILE(i-n, j+n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        // Add [i-n, j-n]
        for (int n = 1; i - n >= 0 && j - n >= 0; ++n) {
            to = TILE(i-n, j-n);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
    }

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
        if (i >= 2 && j >= 1) {
            TRYADD({from, TILE(i-2, j-1)});
        }
    }

    /* Generate rook moves */
    ntiles = bbtiles(piece[Piece::R] & cmask, tiles);
    for (int k = 0; k < ntiles; ++k) {
        from = tiles[k];
        UNTILE(i, j, from);

        // Add [*, j]
        for (int ti = i-1; ti >= 0; --ti) {
            to = TILE(ti, j);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        for (int ti = i+1; ti < 8; ++ti) {
            to = TILE(ti, j);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }

        // Add [i, *]
        for (int tj = j-1; tj >=0; --tj) {
            to = TILE(i, tj);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
        for (int tj = j+1; tj < 8; ++tj) {
            to = TILE(i, tj);
            m = ONEHOT(to);
            if (cmask & m) {
                // Can't do our own pieces
                break;
            } else if (omask & m) {
                // Can capture, but go no further
                TRYADD({from, to});
                break;
            }
            TRYADD({from, to});
        }
    }


    /* Generate pawn moves */
    ntiles = bbtiles(piece[Piece::P] & cmask, tiles);
    for (int k = 0; k < ntiles; ++k) {
        from = tiles[k];
        UNTILE(i, j, from);

        if (tomove == Color::WHITE) {
            // White pawns move up

            to = TILE(i, j+1);
            m = ONEHOT(to);
            if (!((cmask & m) || (omask & m))) {
                TRYADD({from, to});

                if (j == 1) {
                    // Can move 2 tiles
                    to = TILE(i, j+2);
                    m = ONEHOT(to);
                    if (!((cmask & m) || (omask & m))) {
                        TRYADD({from, to});
                    }
                }
            }

            // Handle diagonal captures
            if (j <= 6) {
                if (i <= 6) {
                    to = TILE(i+1, j+1);
                    m = ONEHOT(to);
                    // Handle en-passant capture as well, with 'ep==to'
                    if ((omask & m) || ep == to) {
                        TRYADD({from, to});
                    }
                }
                if (i >= 1) {
                    to = TILE(i-1, j+1);
                    m = ONEHOT(to);
                    if ((omask & m) || ep == to) {
                        TRYADD({from, to});
                    }
                }
            }

        } else {
            // Black pawns move down
            to = TILE(i, j-1);
            m = ONEHOT(to);
            if (!((cmask & m) || (omask & m))) {
                TRYADD({from, to});

                if (j == 6) {
                    // Can move 2 tiles
                    to = TILE(i, j-2);
                    m = ONEHOT(to);
                    if (!((cmask & m) || (omask & m))) {
                        TRYADD({from, to});
                    }
                }
            }

            // Handle diagonal captures
            if (j >= 1) {
                if (i <= 6) {
                    to = TILE(i+1, j-1);
                    m = ONEHOT(to);
                    if ((omask & m) || ep == to) {
                        TRYADD({from, to});
                    }
                }
                if (i >= 1) {
                    to = TILE(i-1, j-1);
                    m = ONEHOT(to);
                    if ((omask & m) || ep == to) {
                        TRYADD({from, to});
                    }
                }
            }
        }
    }

    /* Generate castling moves */
    if (!ignorecastling) {
            
        if (tomove == Color::WHITE && (c_WK || c_WQ)) {

            // White can castle either king or queen or both
            // First, let's calculate moves that black has to see if they are attacking the squares we are going through
            // We do this by duplicating the state but chaning the 'tomove' color
            State ts = *this;
            ts.tomove = Color::BLACK;
            vector<move> bmv;
            ts.getmoves(bmv, true, true);


            if (c_WK) {
                // Three tiles that must be passed through not in check
                int t0 = TILE(4, 0), t1 = TILE(5, 0), t2 = TILE(6, 0);

                if ((ONEHOT(t0) | ONEHOT(t1) | ONEHOT(t2)) & (omask | cmask)) {
                    // Can't castle, people in the way
                } else {
                    bool good = true;
                    for (int k = 0; k < bmv.size(); ++k) {
                        int dest = bmv[k].to;
                        if (dest == t0 || dest == t1 || dest == t2) {
                            good = false;
                            break;
                        }
                    }

                    if (good) {
                        // Just add, since we already checked whether we were attacked
                        res.push_back({ TILE(4, 0), TILE(6, 0) });
                    }

                }
            }

            if (c_WQ) {
                // Three tiles that must be passed through not in check
                int t0 = TILE(4, 0), t1 = TILE(3, 0), t2 = TILE(2, 0);
                if ((ONEHOT(t0) | ONEHOT(t1) | ONEHOT(t2)) & (omask | cmask)) {
                    // Can't castle, people in the way
                } else {
                    bool good = true;
                    for (int k = 0; k < bmv.size(); ++k) {
                        int dest = bmv[k].to;
                        if (dest == t0 || dest == t1 || dest == t2) {
                            good = false;
                            break;
                        }
                    }

                    if (good) {
                        // Just add, since we already checked whether we were attacked
                        res.push_back({ TILE(4, 0), TILE(2, 0) });
                    }
                }
            }
        } else if (tomove == Color::BLACK && (c_BK || c_WQ)) {

            // Black can castle either king or queen or both
            // First, let's calculate moves that white has to see if they are attacking the squares we are going through
            // We do this by duplicating the state but chaning the 'tomove' color
            State ts = *this;
            ts.tomove = Color::WHITE;
            vector<move> bmv;
            ts.getmoves(bmv, true, true);

            if (c_BK) {
                // Three tiles that must be passed through not in check
                int t0 = TILE(4, 7), t1 = TILE(5, 7), t2 = TILE(6, 7);
                if ((ONEHOT(t0) | ONEHOT(t1) | ONEHOT(t2)) & (omask | cmask)) {
                    // Can't castle, people in the way
                } else {
                    bool good = true;
                    for (int k = 0; k < bmv.size(); ++k) {
                        int dest = bmv[k].to;
                        if (dest == t0 || dest == t1 || dest == t2) {
                            good = false;
                            break;
                        }
                    }

                    if (good) {
                        // Just add, since we already checked whether we were attacked
                        res.push_back({ TILE(4, 7), TILE(6, 7) });
                    }
                }
            }
            if (c_BQ) {
                // Three tiles that must be passed through not in check
                int t0 = TILE(4, 7), t1 = TILE(3, 7), t2 = TILE(2, 7);
                if ((ONEHOT(t0) | ONEHOT(t1) | ONEHOT(t2)) & (omask | cmask)) {
                    // Can't castle, people in the way
                } else {
                    bool good = true;
                    for (int k = 0; k < bmv.size(); ++k) {
                        int dest = bmv[k].to;
                        if (dest == t0 || dest == t1 || dest == t2) {
                            good = false;
                            break;
                        }
                    }

                    if (good) {
                        // Just add, since we already checked whether we were attacked
                        res.push_back({ TILE(4, 7), TILE(2, 7) });
                    }
                }
            }
        }
    }

}


}
