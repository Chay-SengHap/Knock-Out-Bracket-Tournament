#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <utility>
using namespace std;

class Player {
public:
    int matchId;            // 0 for leaf players, >0 for matches (QF, SF, Final)
    int score;              // winner's score at this match (0 if not played yet)
    string name;            // player name or winner name at this node
    unique_ptr<Player> left;    // left child (player or match)
    unique_ptr<Player> right;   // right child (player or match)

    // Constructor
    Player(int matchId, int score, const string& name) 
        : matchId(matchId), score(score), name(name), left(nullptr), right(nullptr) {}

    // Factory method to create a new node
    static unique_ptr<Player> createNode(int matchId, int score, const string& name) {
        return make_unique<Player>(matchId, score, name);
    }
};