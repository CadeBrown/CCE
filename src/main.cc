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


#define FILEDEBUG(...) do { \
    FILE* fp = fopen("/home/cade/projects/cce/e.log", "a"); \
    fprintf(fp, __VA_ARGS__); \
    fclose(fp); \
} while (0)

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
            cout << "bestmove " << eng.best_move.LAN() << endl;
            eng.lock.unlock();

        } else {
            cerr << "Unknown command: '" << args[0] << "'" << endl;
        }

    }
}

int main(int argc, char** argv) {

    // Create engine
    Engine eng;
    do_uci(eng);

}
