#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <utility>
#include "tournament.hpp"
using namespace std;




int main() {
    Tournament tournament;

    // Run the tournament
    tournament.runTournament();

    // Example: show Alice's path to the final (only matches she actually played)
    auto path = tournament.pathToFinal("Alice");
    cout << "\nPath to final for Alice (stop at first loss): ";
    if (path.empty()) {
        cout << "(no matches or player not found)\n";
    } else {
        for (size_t i = 0; i < path.size(); ++i) {
            cout << "Match " << path[i] 
                      << (i == path.size() - 1 ? "" : " -> ");
        }
        cout << "\n";
    }

    // Example: if both keep winning, when would Alice and Grace meet?
    auto [meetMatchId, meetRound] = tournament.wouldMeet("Alice", "Grace");
    if (meetMatchId != -1) {
        cout << "\nAlice and Grace would meet at match " << meetMatchId 
                  << " in round " << meetRound << " (if both keep winning).\n";
    } else {
        cout << "\nAlice and Grace would never meet in this bracket.\n";
    }

    // Example: total score and first match that Alice won
    int totalAlice = tournament.getTotalScoreByName(tournament.getRoot(), "Alice");
    cout << "\nTotal score for Alice (matches she won): " << totalAlice << "\n";

    Player* m = tournament.findMatchByName(tournament.getRoot(), "Alice");
    if (m) {
        cout << "First match Alice won: match " << m->matchId 
                  << " with score " << m->score << "\n";
    } else {
        cout << "Alice did not win any match.\n";
    }

    cout << "\nChampion: " << tournament.getRoot()->name 
              << " with score " << tournament.getRoot()->score << "\n";

    return 0;
}