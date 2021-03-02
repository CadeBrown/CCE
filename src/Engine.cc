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
    
    // TODO: Launch thread to do logic here
//    cout << "info depth 1 seldepth 1 multipv 1 score cp 114 nodes 20 nps 20000 tbhits 0 time 1 pv e2e3" << endl;

    vector<move> moves;
    state.getmoves(moves);

    // Pick random move
    best_move = moves[rand() % moves.size()];

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

}
