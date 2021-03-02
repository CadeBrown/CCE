/* util.cc - Implementation of various utility functions
 *
 * @author: Cade Brown <cade@cade.site>
 */

#include <cce.hh>

namespace cce {

// Internal array of square names
static string i_tile_names[] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};


// Internal array of color-piece names
static string i_cp_names[][N_PIECES] = {
    {"K", "Q", "B", "N", "R", "P"},
    {"k", "q", "b", "n", "r", "p"},
};

const string& tile_name(int tile) {
    assert(0 <= tile && tile < 64);
    return i_tile_names[tile];
}

const string& cp_name(Color c, Piece p) {
    assert(0 <= c < 2);
    assert(0 <= p < N_PIECES);
    return i_cp_names[c][p];
}

int bbtiles(bb v, int pos[64]) {
    if (!v) return 0;

    int r = 0;
    // TODO: Use some primitives. For now, we can try this
    bb m = 1, i = 0;
    while (m) {
        if (m & v) {
            pos[r++] = i;
        }

        i++;

        // Move bit up one
        m <<= 1;
    }

    return r;
}


}
