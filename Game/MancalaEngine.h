#pragma once

#include <vector>
#include <functional>

/**
 * @class MancalaEngine
 * @brief Implémentation complète des règles du jeu Mancala
 * 
 * Variante : Kalah (6 pits par joueur, capture sur dernier pit vide)
 * 
 * Layout du plateau :
 *     [13] [12][11][10][ 9][ 8][ 7]   ← Joueur 2
 *          [ 0][ 1][ 2][ 3][ 4][ 5]  [6]
 *          ↑ Joueur 1                  ↑ Store P1
 *     Store P2
 * 
 * Indices :
 * - 0-5   : Pits Joueur 1
 * - 6     : Store Joueur 1
 * - 7-12  : Pits Joueur 2
 * - 13    : Store Joueur 2
 */
class MancalaEngine {
public:
    // ===== TYPES =====
    
    enum class GameState {
        WaitingPlayer1,
        WaitingPlayer2,
        Animating,
        GameOver
    };
    
    enum class Player {
        Player1 = 1,
        Player2 = 2
    };
    
    struct MoveResult {
        bool valid;
        bool extraTurn;      // Dernière graine dans store → rejouer
        bool capture;        // Capture effectuée
        int capturedSeeds;
        std::vector<int> path; // Chemin des graines pour animation
    };

    // ===== CONSTRUCTION =====
    
    MancalaEngine(int seedsPerPit = 4)
        : m_seedsPerPit(seedsPerPit)
        , m_currentPlayer(Player::Player1)
        , m_state(GameState::WaitingPlayer1)
    {
        reset();
    }

    // ===== MÉTHODES PUBLIQUES =====
    
    /**
     * @brief Réinitialise le plateau
     */
    void reset() {
        // Remplir pits
        for (int i = 0; i < 6; i++) {
            m_board[i] = m_seedsPerPit;      // Joueur 1
            m_board[i + 7] = m_seedsPerPit;  // Joueur 2
        }
        
        // Vider stores
        m_board[6] = 0;   // Store P1
        m_board[13] = 0;  // Store P2
        
        m_currentPlayer = Player::Player1;
        m_state = GameState::WaitingPlayer1;
        m_moveHistory.clear();
    }
    
    /**
     * @brief Vérifie si un coup est valide
     */
    bool isValidMove(int pitIndex) const {
        if (m_state == GameState::Animating || m_state == GameState::GameOver) {
            return false;
        }
        
        // Vérifier que c'est bien un pit du joueur actuel
        if (m_currentPlayer == Player::Player1) {
            if (pitIndex < 0 || pitIndex > 5) return false;
        } else {
            if (pitIndex < 7 || pitIndex > 12) return false;
        }
        
        // Vérifier que le pit n'est pas vide
        return m_board[pitIndex] > 0;
    }
    
    /**
     * @brief Effectue un coup
     * @return Résultat du coup (validité, extra turn, capture...)
     */
    MoveResult makeMove(int startPit) {
        MoveResult result;
        result.valid = false;
        result.extraTurn = false;
        result.capture = false;
        result.capturedSeeds = 0;
        
        if (!isValidMove(startPit)) {
            return result;
        }
        
        result.valid = true;
        
        // Prendre les graines du pit
        int seeds = m_board[startPit];
        m_board[startPit] = 0;
        
        int currentPit = startPit;
        int opponentStore = (m_currentPlayer == Player::Player1) ? 13 : 6;
        
        // Distribution des graines
        while (seeds > 0) {
            currentPit = (currentPit + 1) % 14;
            
            // Skip opponent's store
            if (currentPit == opponentStore) {
                continue;
            }
            
            m_board[currentPit]++;
            result.path.push_back(currentPit);
            seeds--;
        }
        
        int playerStore = (m_currentPlayer == Player::Player1) ? 6 : 13;
        
        // Règle 1 : Extra turn si dernière graine dans store
        if (currentPit == playerStore) {
            result.extraTurn = true;
        }
        
        // Règle 2 : Capture
        if (!result.extraTurn && isPlayerPit(currentPit, m_currentPlayer)) {
            // Dernière graine dans pit vide du joueur
            if (m_board[currentPit] == 1) {
                int oppositePit = getOppositePit(currentPit);
                
                if (m_board[oppositePit] > 0) {
                    // Capturer les graines opposées + celle déposée
                    int captured = m_board[oppositePit] + m_board[currentPit];
                    m_board[playerStore] += captured;
                    m_board[oppositePit] = 0;
                    m_board[currentPit] = 0;
                    
                    result.capture = true;
                    result.capturedSeeds = captured;
                }
            }
        }
        
        // Enregistrer coup
        m_moveHistory.push_back(startPit);
        
        // Vérifier fin de partie
        if (isGameOver()) {
            collectRemainingSeeds();
            m_state = GameState::GameOver;
        } else if (!result.extraTurn) {
            switchPlayer();
        }
        
        return result;
    }
    
    /**
     * @brief Vérifie si la partie est terminée
     */
    bool isGameOver() const {
        // Partie terminée si un joueur n'a plus de graines
        bool p1Empty = true;
        for (int i = 0; i < 6; i++) {
            if (m_board[i] > 0) {
                p1Empty = false;
                break;
            }
        }
        
        bool p2Empty = true;
        for (int i = 7; i < 13; i++) {
            if (m_board[i] > 0) {
                p2Empty = false;
                break;
            }
        }
        
        return p1Empty || p2Empty;
    }
    
    /**
     * @brief Retourne le gagnant (0 = égalité, 1 = P1, 2 = P2)
     */
    int getWinner() const {
        if (!isGameOver()) return 0;
        
        int score1 = m_board[6];
        int score2 = m_board[13];
        
        if (score1 > score2) return 1;
        if (score2 > score1) return 2;
        return 0; // Égalité
    }
    
    // ===== GETTERS =====
    
    int getSeedCount(int pitIndex) const {
        return m_board[pitIndex];
    }
    
    Player getCurrentPlayer() const {
        return m_currentPlayer;
    }
    
    GameState getState() const {
        return m_state;
    }
    
    int getPlayer1Score() const { return m_board[6]; }
    int getPlayer2Score() const { return m_board[13]; }
    
    const std::vector<int>& getMoveHistory() const {
        return m_moveHistory;
    }
    
    /**
     * @brief Obtient tous les coups valides pour le joueur actuel
     */
    std::vector<int> getValidMoves() const {
        std::vector<int> validMoves;
        
        if (m_currentPlayer == Player::Player1) {
            for (int i = 0; i < 6; i++) {
                if (m_board[i] > 0) validMoves.push_back(i);
            }
        } else {
            for (int i = 7; i < 13; i++) {
                if (m_board[i] > 0) validMoves.push_back(i);
            }
        }
        
        return validMoves;
    }

private:
    /**
     * @brief Change de joueur
     */
    void switchPlayer() {
        m_currentPlayer = (m_currentPlayer == Player::Player1) 
                          ? Player::Player2 
                          : Player::Player1;
        
        m_state = (m_currentPlayer == Player::Player1) 
                  ? GameState::WaitingPlayer1 
                  : GameState::WaitingPlayer2;
    }
    
    /**
     * @brief Vérifie si un pit appartient au joueur
     */
    bool isPlayerPit(int pit, Player player) const {
        if (player == Player::Player1) {
            return pit >= 0 && pit <= 5;
        } else {
            return pit >= 7 && pit <= 12;
        }
    }
    
    /**
     * @brief Obtient le pit opposé
     */
    int getOppositePit(int pit) const {
        // Pit 0 ↔ 12, Pit 1 ↔ 11, ..., Pit 5 ↔ 7
        return 12 - pit;
    }
    
    /**
     * @brief Collecte les graines restantes à la fin
     */
    void collectRemainingSeeds() {
        // P1 récupère ses graines
        for (int i = 0; i < 6; i++) {
            m_board[6] += m_board[i];
            m_board[i] = 0;
        }
        
        // P2 récupère ses graines
        for (int i = 7; i < 13; i++) {
            m_board[13] += m_board[i];
            m_board[i] = 0;
        }
    }

    // ===== DONNÉES =====
    
    int m_board[14];                  // État du plateau
    int m_seedsPerPit;                // Graines initiales par pit
    Player m_currentPlayer;           // Joueur actuel
    GameState m_state;                // État du jeu
    std::vector<int> m_moveHistory;   // Historique des coups
};