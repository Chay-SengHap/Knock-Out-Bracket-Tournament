#include "player.hpp"
using namespace std;
class Tournament {
private:
    unique_ptr<Player> root;
    mt19937 rng;

public:
    Tournament() : rng(random_device{}()) {}

    // Generate a random integer in [min, max]
    int randomScore(int min, int max) {
        uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    // Shuffle a vector using Fisher-Yates
    template<typename T>
    void shuffleVector(vector<T>& vec) {
        for (size_t i = vec.size() - 1; i > 0; --i) {
            uniform_int_distribution<size_t> dist(0, i);
            size_t j = dist(rng);
            swap(vec[i], vec[j]);
        }
    }

    // Compute tree height from this node
    int height(const Player* node) const {
        if (!node) return -1;
        if (!node->left && !node->right) return 0; // leaf
        int hl = height(node->left.get());
        int hr = height(node->right.get());
        return max(hl, hr) + 1;
    }

    // Find a leaf node whose name matches (only search in leaves)
    Player* findLeaf(Player* root, const string& name) const {
        if (!root) return nullptr;

        // If this is a leaf (player), compare the name
        if (!root->left && !root->right) {
            if (root->name == name)
                return root;
            return nullptr;
        }

        // Otherwise search in left subtree, then right subtree
        Player* left = findLeaf(root->left.get(), name);
        if (left) return left;
        return findLeaf(root->right.get(), name);
    }

    // Find a match node by its matchId (search entire tree)
    Player* findMatchById(Player* root, int matchId) const {
        if (!root) return nullptr;
        if (root->matchId == matchId) return root;
        Player* left = findMatchById(root->left.get(), matchId);
        if (left) return left;
        return findMatchById(root->right.get(), matchId);
    }

    // Find LCA of two nodes a and b in the tree
    Player* lca(Player* root, Player* a, Player* b) const {
        // Base cases: empty tree or found one of the nodes
        if (!root || root == a || root == b) return root;

        // Recur into both sides
        Player* left  = lca(root->left.get(), a, b);
        Player* right = lca(root->right.get(), a, b);

        // If a is in left and b is in right (or vice versa), root is LCA
        if (left && right) return root;

        // Otherwise LCA is in the side that is non-nullptr
        return left ? left : right;
    }

    // Helper: collect all matchIds from first match to final
    bool collectTheoreticalPath(Player* node, const string& playerName,
                              vector<int>& path) const {
        if (!node) return false;

        // If this is a leaf: check if it's the player
        if (!node->left && !node->right) {
            return node->name == playerName;
        }

        // If player is found in left OR right subtree,
        // then this node's matchId is part of their ladder.
        if (collectTheoreticalPath(node->left.get(), playerName, path) ||
            collectTheoreticalPath(node->right.get(), playerName, path)) {
            path.push_back(node->matchId); // add current matchId (bottom -> top)
            return true;
        }
        return false;
    }

    // pathToFinal(player):
    // 1. Get theoretical ladder of matchIds.
    // 2. Walk through that ladder; stop at the first match where they lose.
    vector<int> pathToFinal(const string& playerName) const {
        vector<int> theoreticalPath;
        
        // Step 1: build theoretical path from tree topology
        if (!collectTheoreticalPath(root.get(), playerName, theoreticalPath)) {
            return {}; // player not found as leaf
        }

        // Step 2: filter using actual winners (stop at first loss)
        vector<int> actualPath;
        for (int matchId : theoreticalPath) {
            Player* m = findMatchById(root.get(), matchId);
            if (!m) continue;

            // Player plays in this match
            actualPath.push_back(matchId);

            // If they did not win this match, they are eliminated here
            if (m->name != playerName) {
                break;
            }
        }
        return actualPath;
    }

    // wouldMeet(p1, p2):
    // "If both keep winning, in which match and at which round do they meet?"
    pair<int, int> wouldMeet(const string& p1, const string& p2) const {
        // Find the leaf nodes for each player
        Player* n1 = findLeaf(root.get(), p1);
        Player* n2 = findLeaf(root.get(), p2);
        if (!n1 || !n2) return {-1, -1}; // at least one player not found

        // Find their LCA (the match where their paths cross)
        Player* m = lca(root.get(), n1, n2);
        if (!m || (!m->left && !m->right)) return {-1, -1}; // LCA is leaf (same player)

        return {m->matchId, height(m)};  // round: QF=1, SF=2, Final=3 in this full tree
    }

    // Sum all scores from matches where this name is the winner
    int getTotalScoreByName(const Player* root, const string& name) const {
        if (!root) return 0;

        int total = 0;

        // For internal nodes (matchId != 0), compare winner name
        if (root->matchId != 0 && root->name == name) {
            total += root->score;
        }

        // Recur down to both children
        total += getTotalScoreByName(root->left.get(), name);
        total += getTotalScoreByName(root->right.get(), name);

        return total;
    }

    // Find the first match node where this name is the winner
    Player* findMatchByName(Player* root, const string& name) const {
        if (!root) return nullptr;

        if (root->matchId != 0 && root->name == name) {
            return root;
        }

        Player* left = findMatchByName(root->left.get(), name);
        if (left) return left;

        return findMatchByName(root->right.get(), name);
    }

    // Print all player names (leaves)
    void printPlayers(const Player* root) const {
        if (!root) return;

        // Leaf → player
        if (!root->left && !root->right) {
            cout << "  Player: " << root->name << "\n";
            return;
        }

        // Otherwise, continue searching in both subtrees
        printPlayers(root->left.get());
        printPlayers(root->right.get());
    }

    // Print all matches that belong to a given "round"
    void printMatchesAtRound(const Player* root, int round) const {
        if (!root) return;
        if (!root->left && !root->right) return; // skip leaves

        int h = height(root);
        if (h == round) {
            // Since the tree is complete, left and right are never nullptr here
            cout << "  Match " << root->matchId << ": " 
                      << root->left->name << " vs " << root->right->name 
                      << " -> winner: " << (root->score > 0 ? root->name : "?")
                      << " (score " << root->score << ")\n";
        }

        printMatchesAtRound(root->left.get(), round);
        printMatchesAtRound(root->right.get(), round);
    }

    // Print whole bracket: players, then each round
    void printBracket() const {
        int maxRound = height(root.get()); // FINAL level = 3 in this 8-player tree

        cout << "\n=== ROUND 0: PLAYERS ===\n";
        printPlayers(root.get());

        for (int r = 1; r <= maxRound; ++r) {
            if (r == maxRound)
                cout << "\n=== FINAL (ROUND " << r << ") ===\n";
            else
                cout << "\n=== ROUND " << r << " ===\n";
            printMatchesAtRound(root.get(), r);
        }
    }

    // Play a single match: use left/right children, generate random scores,
    // decide the winner, and store winner name + score in this match node.
    void playMatchRandom(Player* match, const string& label) {
        int leftScore  = randomScore(1, 10);
        int rightScore = randomScore(1, 10);

        cout << label << ": " << match->left->name << " (" << leftScore 
                  << ") vs " << match->right->name << " (" << rightScore << ")\n";

        if (leftScore >= rightScore) {
            match->name = match->left->name;
            match->score = leftScore;
        } else {
            match->name = match->right->name;
            match->score = rightScore;
        }

        cout << "  Winner: " << match->name << " (score " << match->score << ")\n\n";
    }

    // Round 1: quarterfinals (4 matches: QF1..QF4)
    void playRound1() {
        Player* q1 = root->left->left.get();   // QF1
        Player* q2 = root->left->right.get();  // QF2
        Player* q3 = root->right->left.get();  // QF3
        Player* q4 = root->right->right.get(); // QF4

        cout << "\n=== ROUND 1: QUARTERFINALS ===\n";
        playMatchRandom(q1, "QF1");
        playMatchRandom(q2, "QF2");
        playMatchRandom(q3, "QF3");
        playMatchRandom(q4, "QF4");
    }

    // Round 2: semifinals (2 matches: SF1, SF2)
    void playRound2() {
        Player* s1 = root->left.get();   // SF1
        Player* s2 = root->right.get();  // SF2

        cout << "\n=== ROUND 2: SEMIFINALS ===\n";
        playMatchRandom(s1, "SF1");
        playMatchRandom(s2, "SF2");
    }

    // Round 3: final match (root)
    void playFinal() {
        cout << "\n=== FINAL ===\n";
        playMatchRandom(root.get(), "FINAL");
    }

    // Build the tournament structure
    void buildTournament() {
        // 1) Build match structure (internal nodes only, no players yet)

        // Root is FINAL (matchId = 7, winner not decided yet)
        root = Player::createNode(7, 0, "FINAL TBD");

        // Two children: semifinals SF1 and SF2
        root->left  = Player::createNode(5, 0, "SF1 TBD");
        root->right = Player::createNode(6, 0, "SF2 TBD");

        // Each semifinal has two children: quarterfinals QF1..QF4
        root->left->left   = Player::createNode(1, 0, "QF1 TBD");
        root->left->right  = Player::createNode(2, 0, "QF2 TBD");
        root->right->left  = Player::createNode(3, 0, "QF3 TBD");
        root->right->right = Player::createNode(4, 0, "QF4 TBD");

        // 2) Prepare 8 player names and randomize their order
        vector<string> names = {
            "Alice", "Bob", "Carol", "David",
            "Eva", "Frank", "Grace", "Henry"
        };

        shuffleVector(names);

        cout << "Random player order:\n";
        for (const auto& name : names) {
            cout << "  " << name << "\n";
        }

        // 3) Attach leaf players to the quarterfinals in random order
        root->left->left->left    = Player::createNode(0, 0, names[0]);
        root->left->left->right   = Player::createNode(0, 0, names[1]);
        root->left->right->left   = Player::createNode(0, 0, names[2]);
        root->left->right->right  = Player::createNode(0, 0, names[3]);
        root->right->left->left   = Player::createNode(0, 0, names[4]);
        root->right->left->right  = Player::createNode(0, 0, names[5]);
        root->right->right->left  = Player::createNode(0, 0, names[6]);
        root->right->right->right = Player::createNode(0, 0, names[7]);
    }

    // Run the complete tournament
    void runTournament() {
        buildTournament();
        playRound1();
        playRound2();
        playFinal();
        printBracket();
    }

    // Getters
    const Player* getRoot() const { return root.get(); }
    Player* getRoot() { return root.get(); }

    // No need for explicit freeTree - unique_ptr handles memory automatically
};