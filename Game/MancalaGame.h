#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Scene/GameObject.h"

/**
 * @brief Règles du Mancala:
 * - 2 joueurs, 6 pits chacun + 1 store (mancala)
 * - Chaque pit commence avec 4 graines
 * - Tour: Prendre toutes les graines d'un pit et les distribuer dans le sens horaire
 * - Si dernière graine tombe dans votre store: rejouer
 * - Si dernière graine tombe dans pit vide de votre côté: capturer
 * - Gagner: Avoir le plus de graines dans son store à la fin
 */

class MancalaGame {
public:
    enum class Player {
        PLAYER_ONE = 0,  // Bottom side
        PLAYER_TWO = 1   // Top side
    };

    enum class GameState {
        PLAYING,
        PLAYER_ONE_WON,
        PLAYER_TWO_WON,
        DRAW
    };

    struct Pit {
        GameObject* pitObject;           // Visual representation
        std::vector<GameObject*> seeds;  // Seed objects in this pit
        glm::vec3 basePosition;          // Pit center position
        int index;                       // 0-13 (0-5: P1, 6: Store1, 7-12: P2, 13: Store2)
        bool isStore;                    // Is this a store/mancala?
        Player owner;                    // Which player owns this pit
    };

    MancalaGame();
    ~MancalaGame();

    // Game setup
    void initialize();
    void reset();

    // Game actions
    bool selectPit(int pitIndex);           // Player selects a pit
    void executeMove(int pitIndex);         // Execute the move (distribute seeds)
    bool isValidMove(int pitIndex) const;   // Check if move is legal
    
    // Game state queries
    Player getCurrentPlayer() const { return m_currentPlayer; }
    GameState getGameState() const { return m_gameState; }
    int getSeedCount(int pitIndex) const;
    bool isGameOver() const;
    
    // Visual updates
    void updateSeedPositions();             // Arrange seeds visually in pits
    std::vector<GameObject*> getAllObjects() const;
    
    // Getters
    const std::vector<Pit>& getPits() const { return m_pits; }
    Pit* getPitByIndex(int index);
    int getStoreCount(Player player) const;
    
    // Animation state
    bool isAnimating() const { return m_isAnimating; }
    void updateAnimation(float deltaTime);

private:
    // Internal helpers
    void createBoard();
    void createPits();
    void createSeeds();
    void distributeSeedsAnimation(int startPitIndex);
    void checkCapture(int lastPitIndex);
    void checkWinCondition();
    void switchPlayer();
    int getOppositePitIndex(int pitIndex) const;
    
    // Seed positioning helpers
    glm::vec3 calculateSeedPosition(int pitIndex, int seedIndexInPit);
    void stackSeedsInPit(int pitIndex);

    // Game data
    std::vector<Pit> m_pits;              // 14 pits total (6+1+6+1)
    GameObject* m_board;                   // The wooden board
    Player m_currentPlayer;
    GameState m_gameState;
    
    // Animation system
    bool m_isAnimating;
    std::vector<GameObject*> m_animatingSeeds;
    std::vector<glm::vec3> m_seedTargets;
    float m_animationProgress;
    
    // Configuration
    static constexpr int PITS_PER_PLAYER = 6;
    static constexpr int INITIAL_SEEDS_PER_PIT = 4;
    static constexpr float PIT_SPACING = 1.2f;
    static constexpr float STORE_SPACING = 1.5f;
    static constexpr float SEED_RADIUS = 0.15f;
};

// Implementation details
inline int MancalaGame::getSeedCount(int pitIndex) const {
    if (pitIndex < 0 || pitIndex >= m_pits.size()) return 0;
    return m_pits[pitIndex].seeds.size();
}

inline int MancalaGame::getStoreCount(Player player) const {
    int storeIndex = (player == Player::PLAYER_ONE) ? 6 : 13;
    return getSeedCount(storeIndex);
}

inline bool MancalaGame::isGameOver() const {
    return m_gameState != GameState::PLAYING;
}