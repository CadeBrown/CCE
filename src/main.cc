/* main.cc - Main binary f
 *
 * @author: Cade Brown <cade@cade.site>
 */

#include <cce.hh>

using namespace cce;


// Splits 'line' into arguments (on spaces), populating 'args'
static void splitargs(const string& line, vector<string>& args) {
    args.clear();

    // Spluts on spaces
    stringstream ss(line);
    string part;
    while (getline(ss, part, ' ')) {
        args.push_back(part);
    }
}




// Accept UCI commands and feed them to 'eng'
static void do_uci(Engine& eng) {
    string line;
    vector<string> args;

    // send some information
    cout << "id name cce 0.1" << endl;
    cout << "id author Cade Brown" << endl;

    cout << "uciok" << endl;

    while (getline(cin, line)) {
        splitargs(line, args);
        if (args.size() == 0) continue;

        // Handle UCI command here
        if (args[0] == "debug") {
            if (args.size() == 2) {
                // Ignore for now, but in the future enable debugging
            } else {
                cerr << "Command 'debug' expected 2 arguments " << endl;
            }
        } else if (args[0] == "uci") {
            // Ignore, as we're always UCI
        } else if (args[0] == "quit") {
            // Quit the entire program
            return;
        } else if (args[0] == "isready") {
            // Just a check-up, always return 'readyok'
            cout << "readyok" << endl;
        } else if (args[0] == "setoption") {
            // Ignore for now
            // Sets an option in a dictionary
        } else if (args[0] == "register") {
            // Ignore for now
        } else if (args[0] == "ucinewgame") {
            // Ignore for now
        } else if (args[0] == "position") {
            if (args.size() < 2) {
                cerr << "Command 'position' expected 2 arguments or more" << endl;
            } else {
                string fen = "";
                if (args[1] == "startpos") {
                    // We need to start from initial position
                    fen = FEN_START;
                } else if (args[1] == "fen") {
                    if (args.size() < 3) {
                        cerr << "Command 'position fen' expected at least 3 arguments giving FEN string" << endl;
                    } else {
                        // Initialize from a FEN string (from remaining arguments)
                        for (int i = 2; i < args.size(); ++i) {
                            if (i > 2) fen.push_back(' ');
                            fen += args[i];
                        }
                    }
                } else {
                    cerr << "Command 'position' expected second argument to be 'startpos' or 'fen'" << endl;
                }
                if (fen.size() > 0) {
                    // Was successful, now set the engine to analyze this position
                    State s = State::from_FEN(fen);

                    // Set state for the engine
                    eng.setstate(s);
                }
            }

        } else if (args[0] == "go") {
            // Start computing
            eng.go();
            
            eng.stop();
            // Print out best move (we need to lock it to avoid undefined behaviour)
            eng.lock.lock();

            cout << "bestmove " << eng.best_move.LAN() << endl;
            eng.lock.unlock();


        } else if (args[0] == "stop") {
            // Stop computing
            eng.stop();

            // Print out best move (we need to lock it to avoid undefined behaviour)
            eng.lock.lock();
            //cout << "bestmove " << eng.best_move.LAN() << endl;
            eng.lock.unlock();

        } else {
            cerr << "Unknown command: '" << args[0] << "'" << endl;
        }

    }
}


// Perf test

static size_t perft(const State& s, int dep=0) {
    if (dep <= 0) {
        return 1;
    }

    size_t res = 0;

    // Get all available moves
    vector<cce::move> moves;
    s.getmoves(moves);

    for (int i = 0; i < moves.size(); ++i) {
        State ns = s;
        ns.apply(moves[i]);
        res += perft(ns, dep-1);
    }

    return res;
}

int main(int argc, char** argv) {

    srand(time(NULL));

    // Create engine
    Engine eng;
    State s = State::from_FEN(FEN_START);


    //cout << perft(s, 1) << endl;
    //cout << perft(s, 2) << endl;
    //cout << perft(s, 3) << endl;
    cout << perft(s, 4) << endl;
    //cout << perft(s, 5) << endl;
    //cout << perft(s, 6) << endl;

    /*
    State s = State::from_FEN("r2k1bnr/pp3ppp/5q2/1NpP4/2nPQ3/2P5/PP3PPP/RNB1R1K1 w - - 0 13");

    pair<cce::move, eval> bm = eng.findbest1(s);
    cout << "BM: " << bm.first.LAN() << " EV: " << bm.second.getstr() << endl;
    */
    //do_uci(eng);

}
