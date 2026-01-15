#include "MancalaGame.h"
#include "ThemeManager.h"
#include <glm/gtc/constants.hpp>

#include <iostream>

MancalaGame::MancalaGame() 
    : m_currentPlayer(Player::PLAYER_ONE),
      m_gameState(GameState::PLAYING),
      m_isAnimating(false),
      m_animationProgress(0.0f),
      m_board(nullptr) {
}

MancalaGame::~MancalaGame() {
    // Cleanup all game objects
    delete m_board;
    for (auto& pit : m_pits) {
        delete pit.pitObject;
        for (auto* seed : pit.seeds) {
            delete seed;
        }
    }
}

void MancalaGame::initialize() {
    createBoard();
    createPits();
    createSeeds();
    updateSeedPositions();
}

void MancalaGame::createBoard() {
    m_board = new GameObject();
    Mesh* boardMesh = new Mesh(Mesh::createCube());
    m_board->setMesh(boardMesh);
    m_board->getTransform().setScale(glm::vec3(10.0f, 0.3f, 4.0f));
    m_board->getTransform().setPosition(glm::vec3(0.0f, -0.15f, 0.0f));
    
    ThemeManager::getInstance().applyThemeToBoard(m_board);
}

void MancalaGame::createPits() {
    m_pits.resize(14);  // 6 + 1 + 6 + 1
    
    // Player 1 pits (0-5) - Bottom row
    for (int i = 0; i < PITS_PER_PLAYER; ++i) {
        Pit& pit = m_pits[i];
        pit.index = i;
        pit.isStore = false;
        pit.owner = Player::PLAYER_ONE;
        
        // Position from right to left (traditional mancala layout)
        float x = 3.0f - i * PIT_SPACING;
        float z = -1.0f;
        pit.basePosition = glm::vec3(x, 0.0f, z);
        
        // Create pit visual (cylinder for now, can be custom mesh)
        pit.pitObject = new GameObject();
        Mesh* pitMesh = new Mesh(Mesh::createCube());  // Use cylinder if available
        pit.pitObject->setMesh(pitMesh);
        pit.pitObject->getTransform().setPosition(pit.basePosition);
        pit.pitObject->getTransform().setScale(glm::vec3(0.8f, 0.3f, 0.8f));
        
        ThemeManager::getInstance().applyThemeToPit(pit.pitObject, i);
    }
    
    // Player 1 store (6)
    {
        Pit& pit = m_pits[6];
        pit.index = 6;
        pit.isStore = true;
        pit.owner = Player::PLAYER_ONE;
        pit.basePosition = glm::vec3(-4.5f, 0.0f, 0.0f);
        
        pit.pitObject = new GameObject();
        Mesh* storeMesh = new Mesh(Mesh::createCube());
        pit.pitObject->setMesh(storeMesh);
        pit.pitObject->getTransform().setPosition(pit.basePosition);
        pit.pitObject->getTransform().setScale(glm::vec3(1.0f, 0.5f, 1.5f));
        
        ThemeManager::getInstance().applyThemeToPit(pit.pitObject, 6);
    }
    
    // Player 2 pits (7-12) - Top row
    for (int i = 0; i < PITS_PER_PLAYER; ++i) {
        int pitIdx = 7 + i;
        Pit& pit = m_pits[pitIdx];
        pit.index = pitIdx;
        pit.isStore = false;
        pit.owner = Player::PLAYER_TWO;
        
        // Position from left to right (opposite of player 1)
        float x = -3.0f + i * PIT_SPACING;
        float z = 1.0f;
        pit.basePosition = glm::vec3(x, 0.0f, z);
        
        pit.pitObject = new GameObject();
        Mesh* pitMesh = new Mesh(Mesh::createCube());
        pit.pitObject->setMesh(pitMesh);
        pit.pitObject->getTransform().setPosition(pit.basePosition);
        pit.pitObject->getTransform().setScale(glm::vec3(0.8f, 0.3f, 0.8f));
        
        ThemeManager::getInstance().applyThemeToPit(pit.pitObject, pitIdx);
    }
    
    // Player 2 store (13)
    {
        Pit& pit = m_pits[13];
        pit.index = 13;
        pit.isStore = true;
        pit.owner = Player::PLAYER_TWO;
        pit.basePosition = glm::vec3(4.5f, 0.0f, 0.0f);
        
        pit.pitObject = new GameObject();
        Mesh* storeMesh = new Mesh(Mesh::createCube());
        pit.pitObject->setMesh(storeMesh);
        pit.pitObject->getTransform().setPosition(pit.basePosition);
        pit.pitObject->getTransform().setScale(glm::vec3(1.0f, 0.5f, 1.5f));
        
        ThemeManager::getInstance().applyThemeToPit(pit.pitObject, 13);
    }
}

void MancalaGame::createSeeds() {
    int seedIdx = 0;
    
    // Create 4 seeds per regular pit (not stores)
    for (int i = 0; i < 14; ++i) {
        if (m_pits[i].isStore) continue;  // Stores start empty
        
        for (int j = 0; j < INITIAL_SEEDS_PER_PIT; ++j) {
            GameObject* seed = new GameObject();
            Mesh* seedMesh = new Mesh(Mesh::createSphere(SEED_RADIUS, 16));
            seed->setMesh(seedMesh);
            
            ThemeManager::getInstance().applyThemeToSeed(seed, seedIdx++);
            
            m_pits[i].seeds.push_back(seed);
        }
    }
}

void MancalaGame::updateSeedPositions() {
    for (auto& pit : m_pits) {
        stackSeedsInPit(pit.index);
    }
}

void MancalaGame::stackSeedsInPit(int pitIndex) {
    Pit& pit = m_pits[pitIndex];
    int numSeeds = pit.seeds.size();
    
    if (numSeeds == 0) return;
    
    // Arrange seeds in a circular/spiral pattern
    float radius = 0.3f;
    float height = 0.2f;
    
    for (int i = 0; i < numSeeds; ++i) {
        float angle = (i * 2.0f * glm::pi<float>()) / std::min(numSeeds, 8);
        float layer = i / 8;
        
        glm::vec3 pos = pit.basePosition;
        pos.x += radius * cos(angle) * (1.0f - layer * 0.1f);
        pos.z += radius * sin(angle) * (1.0f - layer * 0.1f);
        pos.y = height + layer * SEED_RADIUS * 2.5f;
        
        pit.seeds[i]->getTransform().setPosition(pos);
    }
}

bool MancalaGame::isValidMove(int pitIndex) const {
    if (m_isAnimating) return false;
    if (pitIndex < 0 || pitIndex >= 14) return false;
    
    const Pit& pit = m_pits[pitIndex];
    
    // Can't select stores
    if (pit.isStore) return false;
    
    // Can only select your own pits
    if (pit.owner != m_currentPlayer) return false;
    
    // Pit must have seeds
    if (pit.seeds.empty()) return false;
    
    return true;
}

void MancalaGame::executeMove(int pitIndex) {
    if (!isValidMove(pitIndex)) return;
    
    // TODO: Implement seed distribution with animation
    // For now, simplified version:
    
    Pit& sourcePit = m_pits[pitIndex];
    int seedsToDistribute = sourcePit.seeds.size();
    std::vector<GameObject*> seedsToMove = sourcePit.seeds;
    sourcePit.seeds.clear();
    
    int currentPit = pitIndex;
    int lastPit = -1;
    
    // Distribute seeds counter-clockwise
    for (int i = 0; i < seedsToDistribute; ++i) {
        currentPit = (currentPit + 1) % 14;
        
        // Skip opponent's store
        int opponentStore = (m_currentPlayer == Player::PLAYER_ONE) ? 13 : 6;
        if (currentPit == opponentStore) {
            currentPit = (currentPit + 1) % 14;
        }
        
        m_pits[currentPit].seeds.push_back(seedsToMove[i]);
        lastPit = currentPit;
    }
    
    updateSeedPositions();
    
    // Check for extra turn (landed in own store)
    int myStore = (m_currentPlayer == Player::PLAYER_ONE) ? 6 : 13;
    bool extraTurn = (lastPit == myStore);
    
    // Check for capture
    if (!extraTurn && !m_pits[lastPit].isStore && 
        m_pits[lastPit].owner == m_currentPlayer && 
        m_pits[lastPit].seeds.size() == 1) {
        checkCapture(lastPit);
    }
    
    // Switch player if no extra turn
    if (!extraTurn) {
        switchPlayer();
    }
    
    // Check win condition
    checkWinCondition();
}

void MancalaGame::checkCapture(int lastPitIndex) {
    // Capture logic: if last seed lands in empty pit on your side,
    // capture that seed + all seeds from opposite pit
    int oppositePit = getOppositePitIndex(lastPitIndex);
    
    if (oppositePit != -1 && !m_pits[oppositePit].seeds.empty()) {
        int myStore = (m_currentPlayer == Player::PLAYER_ONE) ? 6 : 13;
        
        // Move all seeds to your store
        m_pits[myStore].seeds.insert(
            m_pits[myStore].seeds.end(),
            m_pits[lastPitIndex].seeds.begin(),
            m_pits[lastPitIndex].seeds.end()
        );
        m_pits[lastPitIndex].seeds.clear();
        
        m_pits[myStore].seeds.insert(
            m_pits[myStore].seeds.end(),
            m_pits[oppositePit].seeds.begin(),
            m_pits[oppositePit].seeds.end()
        );
        m_pits[oppositePit].seeds.clear();
        
        updateSeedPositions();
    }
}

int MancalaGame::getOppositePitIndex(int pitIndex) const {
    // Player 1 pits (0-5) opposite to Player 2 pits (7-12)
    if (pitIndex >= 0 && pitIndex <= 5) {
        return 12 - pitIndex;
    } else if (pitIndex >= 7 && pitIndex <= 12) {
        return 12 - (pitIndex - 7);
    }
    return -1;  // Stores have no opposite
}

void MancalaGame::checkWinCondition() {
    // Game ends when one side has no seeds left
    bool p1Empty = true;
    bool p2Empty = true;
    
    for (int i = 0; i < 6; ++i) {
        if (!m_pits[i].seeds.empty()) p1Empty = false;
    }
    
    for (int i = 7; i < 13; ++i) {
        if (!m_pits[i].seeds.empty()) p2Empty = false;
    }
    
    if (p1Empty || p2Empty) {
        // Move remaining seeds to respective stores
        // ... (implementation)
        
        // Compare store counts
        int p1Score = getStoreCount(Player::PLAYER_ONE);
        int p2Score = getStoreCount(Player::PLAYER_TWO);
        
        if (p1Score > p2Score) {
            m_gameState = GameState::PLAYER_ONE_WON;
        } else if (p2Score > p1Score) {
            m_gameState = GameState::PLAYER_TWO_WON;
        } else {
            m_gameState = GameState::DRAW;
        }
    }
}

void MancalaGame::switchPlayer() {
    m_currentPlayer = (m_currentPlayer == Player::PLAYER_ONE) 
                    ? Player::PLAYER_TWO 
                    : Player::PLAYER_ONE;
}

void MancalaGame::reset() {
    // Clear all seeds
    for (auto& pit : m_pits) {
        for (auto* seed : pit.seeds) {
            delete seed;
        }
        pit.seeds.clear();
    }
    
    // Recreate seeds
    createSeeds();
    updateSeedPositions();
    
    // Reset game state
    m_currentPlayer = Player::PLAYER_ONE;
    m_gameState = GameState::PLAYING;
    m_isAnimating = false;
}

std::vector<GameObject*> MancalaGame::getAllObjects() const {
    std::vector<GameObject*> objects;
    
    objects.push_back(m_board);
    
    for (const auto& pit : m_pits) {
        objects.push_back(pit.pitObject);
        for (auto* seed : pit.seeds) {
            objects.push_back(seed);
        }
    }
    
    return objects;
}

void MancalaGame::updateAnimation(float deltaTime) {
    // TODO: Implement smooth seed movement animation
    m_isAnimating = false;
}