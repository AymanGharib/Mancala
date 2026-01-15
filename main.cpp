#include "core/Window.h"
#include "Rendering/Camera.h"
#include "Rendering/shader.h"
#include "Rendering/Material.h"
#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include "Game/MancalaGame.h"
#include "Interaction/ObjectPicker.h"
#include "Rendering/TextureManager.h"
#include "Rendering/RenderModeManager.h"
#include "Game/ThemeManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// ===== CONFIGURATION =====
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr float CAMERA_ORBIT_SENSITIVITY = 0.15f;
constexpr float CAMERA_PAN_SPEED = 0.05f;
constexpr float CAMERA_ZOOM_SPEED = 0.5f;

// ===== GLOBAL STATE =====
struct AppState {
    // Managers
    MancalaGame* game = nullptr;
    RenderModeManager& renderMode = RenderModeManager::getInstance();
    ThemeManager& themeManager = ThemeManager::getInstance();
    TextureManager& textureManager = TextureManager::getInstance();
    
    // Camera
    glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};
    
    // Input
    GameObject* selectedObject = nullptr;
    GameObject* hoveredObject = nullptr;
    bool isDragging = false;
    glm::vec3 dragOffset{0.0f};
    
    // Lighting
    struct Light {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };
    std::vector<Light> lights;
    
    // UI State
    bool showHelp = true;
    bool showStats = true;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
};

// ===== FORWARD DECLARATIONS =====
void processInput(Window& window, AppState& state);
void handleMousePicking(Window& window, Camera& camera, AppState& state);
void renderScene(Shader& shader, const std::vector<GameObject*>& objects, AppState& state);
void renderUI(AppState& state);
void setupLights(AppState& state);

// ===== MAIN =====
int main() {
    try {
        // Create window
        Window::Config config;
        config.width = WINDOW_WIDTH;
        config.height = WINDOW_HEIGHT;
        config.title = "Mancala 3D - Interactive Game";
        config.msaaSamples = 4;
        config.vsync = true;
        Window window(config);

        // Setup camera
        int width, height;
        window.getFramebufferSize(width, height);
        Camera camera(
            glm::vec3(0, 6, 10),     // Position
            glm::vec3(0, 0, 0),      // Look at
            45.0f,                    // FOV
            static_cast<float>(width) / height,
            0.1f,                     // Near plane
            100.0f                    // Far plane
        );

        // Load shader
        Shader shader("Shaders/phong.vs", "Shaders/phong.fs");

        // Initialize game state
        AppState state;
        state.game = new MancalaGame();
        state.game->initialize();
        
        // Setup lighting (multiple sources as required)
        setupLights(state);

        // Load textures (optional)
        // GLuint woodTexture = state.textureManager.loadTexture("textures/wood.jpg", true);
        // GLuint stoneTexture = state.textureManager.loadTexture("textures/stone.jpg", true);

        // Print controls
        std::cout << "\n=============== MANCALA 3D ===============" << std::endl;
        std::cout << "=== CAMERA CONTROLS ===" << std::endl;
        std::cout << "  Right Mouse + Drag : Orbit camera" << std::endl;
        std::cout << "  Mouse Wheel        : Zoom in/out" << std::endl;
        std::cout << "  W/A/S/D/Q/E       : Pan camera" << std::endl;
        std::cout << "\n=== GAME CONTROLS ===" << std::endl;
        std::cout << "  Left Click         : Select pit & play" << std::endl;
        std::cout << "  R                  : Reset game" << std::endl;
        std::cout << "\n=== DISPLAY CONTROLS ===" << std::endl;
        std::cout << "  M                  : Cycle render mode" << std::endl;
        std::cout << "  T                  : Change theme" << std::endl;
        std::cout << "  H                  : Toggle help" << std::endl;
        std::cout << "  F                  : Toggle stats" << std::endl;
        std::cout << "  ESC                : Exit" << std::endl;
        std::cout << "=========================================\n" << std::endl;

        // Main loop
        while (!window.shouldClose()) {
            // Delta time
            float currentFrame = glfwGetTime();
            state.deltaTime = currentFrame - state.lastFrame;
            state.lastFrame = currentFrame;

            window.pollEvents();
            processInput(window, state);

            // Update camera from window orbit controls
            float yaw = window.getYaw();
            float pitch = window.getPitch();
            float distance = window.getDistance();

            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            
            glm::vec3 cameraPos = state.cameraTarget - glm::normalize(direction) * distance;
            camera.setPosition(cameraPos);
            camera.lookAt(state.cameraTarget);

            // Update game animations
            if (state.game->isAnimating()) {
                state.game->updateAnimation(state.deltaTime);
            }

            // Mouse picking (object selection)
            handleMousePicking(window, camera, state);

            // === RENDERING ===
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            
            // Set camera matrices
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 projection = camera.getProjectionMatrix();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("viewPos", camera.getPosition());

            // Set lighting (multiple lights)
            shader.setInt("numLights", static_cast<int>(state.lights.size()));
            for (size_t i = 0; i < state.lights.size(); ++i) {
                std::string base = "lights[" + std::to_string(i) + "]";
                shader.setVec3(base + ".position", state.lights[i].position);
                shader.setVec3(base + ".color", state.lights[i].color);
                shader.setFloat(base + ".intensity", state.lights[i].intensity);
            }

            // Set render mode flags
            shader.setBool("useTextures", state.renderMode.shouldUseTextures());

            // Render scene
            auto objects = state.game->getAllObjects();
            renderScene(shader, objects, state);

            // Render UI/HUD
            renderUI(state);

            window.swapBuffers();
        }

        // Cleanup
        delete state.game;
        state.textureManager.cleanup();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return -1;
    }
}

// ===== IMPLEMENTATION =====

void setupLights(AppState& state) {
    // Main light (key light)
    AppState::Light mainLight;
    mainLight.position = glm::vec3(5.0f, 8.0f, 5.0f);
    mainLight.color = glm::vec3(1.0f, 0.95f, 0.85f);
    mainLight.intensity = 1.0f;
    state.lights.push_back(mainLight);

    // Fill light
    AppState::Light fillLight;
    fillLight.position = glm::vec3(-4.0f, 6.0f, 3.0f);
    fillLight.color = glm::vec3(0.6f, 0.7f, 0.8f);
    fillLight.intensity = 0.5f;
    state.lights.push_back(fillLight);

    // Back light
    AppState::Light backLight;
    backLight.position = glm::vec3(0.0f, 4.0f, -6.0f);
    backLight.color = glm::vec3(0.8f, 0.8f, 1.0f);
    backLight.intensity = 0.3f;
    state.lights.push_back(backLight);
}

void processInput(Window& window, AppState& state) {
    // Camera panning
    if (window.isKeyPressed(GLFW_KEY_W)) state.cameraTarget.z -= CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_S)) state.cameraTarget.z += CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_A)) state.cameraTarget.x -= CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_D)) state.cameraTarget.x += CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_Q)) state.cameraTarget.y -= CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_E)) state.cameraTarget.y += CAMERA_PAN_SPEED;

    // Game controls (use key callbacks for single press)
    static bool rPressed = false;
    if (window.isKeyPressed(GLFW_KEY_R)) {
        if (!rPressed) {
            state.game->reset();
            std::cout << "[Game] Reset!" << std::endl;
            rPressed = true;
        }
    } else {
        rPressed = false;
    }

    // Display mode
    static bool mPressed = false;
    if (window.isKeyPressed(GLFW_KEY_M)) {
        if (!mPressed) {
            state.renderMode.cycleMode();
            std::cout << "[Display] Mode: " << state.renderMode.getModeName() << std::endl;
            mPressed = true;
        }
    } else {
        mPressed = false;
    }

    // Theme switching
    static bool tPressed = false;
    if (window.isKeyPressed(GLFW_KEY_T)) {
        if (!tPressed) {
            int nextTheme = (state.themeManager.getCurrentThemeIndex() + 1) 
                          % state.themeManager.getThemeCount();
            state.themeManager.setTheme(nextTheme);
            
            // Reapply theme to all objects
            state.game->updateSeedPositions();  // This will also update materials
            
            std::cout << "[Theme] Changed to: " 
                      << state.themeManager.getCurrentTheme().name << std::endl;
            tPressed = true;
        }
    } else {
        tPressed = false;
    }

    // Toggle help
    static bool hPressed = false;
    if (window.isKeyPressed(GLFW_KEY_H)) {
        if (!hPressed) {
            state.showHelp = !state.showHelp;
            hPressed = true;
        }
    } else {
        hPressed = false;
    }

    // Toggle stats
    static bool fPressed = false;
    if (window.isKeyPressed(GLFW_KEY_F)) {
        if (!fPressed) {
            state.showStats = !state.showStats;
            fPressed = true;
        }
    } else {
        fPressed = false;
    }
}

void handleMousePicking(Window& window, Camera& camera, AppState& state) {
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window.getGLFWWindow(), &mouseX, &mouseY);
    
    int width, height;
    window.getFramebufferSize(width, height);

    // Create ray from mouse position
    ObjectPicker::Ray ray = ObjectPicker::screenToWorldRay(
        static_cast<float>(mouseX),
        static_cast<float>(mouseY),
        width,
        height,
        camera
    );

    // Get all pickable objects (pits)
    auto objects = state.game->getAllObjects();
    
    // Find hovered object
    ObjectPicker::RayHit hit = ObjectPicker::pickObject(ray, objects);
    state.hoveredObject = hit.hit ? hit.object : nullptr;

    // Handle left click (pit selection)
    static bool leftMousePressed = false;
    bool leftMouseDown = glfwGetMouseButton(window.getGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    
    if (leftMouseDown && !leftMousePressed) {
        if (state.hoveredObject && !state.game->isAnimating()) {
            // Find which pit was clicked
            const auto& pits = state.game->getPits();
            for (size_t i = 0; i < pits.size(); ++i) {
                if (pits[i].pitObject == state.hoveredObject) {
                    // Try to play this pit
                    if (state.game->isValidMove(i)) {
                        state.game->executeMove(i);
                        std::cout << "[Game] Pit " << i << " selected" << std::endl;
                    } else {
                        std::cout << "[Game] Invalid move!" << std::endl;
                    }
                    break;
                }
            }
        }
        leftMousePressed = true;
    } else if (!leftMouseDown) {
        leftMousePressed = false;
    }

    // Visual feedback for hover (change color slightly)
    // This could be done by modifying material or using shader uniform
}

void renderScene(Shader& shader, const std::vector<GameObject*>& objects, AppState& state) {
    auto mode = state.renderMode.getCurrentMode();

    if (mode == RenderModeManager::Mode::SHADED_WIRE) {
        // Two-pass rendering: solid + wireframe overlay
        
        // Pass 1: Solid rendering
        for (auto* obj : objects) {
            if (obj && obj->isVisible()) {
                obj->render(shader);
            }
        }
        
        // Pass 2: Wireframe overlay
        state.renderMode.enableWireframeOverlay();
        shader.setVec3("wireframeColor", glm::vec3(0.0f, 0.0f, 0.0f));
        
        for (auto* obj : objects) {
            if (obj && obj->isVisible()) {
                obj->render(shader);
            }
        }
        
        state.renderMode.disableWireframeOverlay();
    } else {
        // Single pass rendering
        for (auto* obj : objects) {
            if (obj && obj->isVisible()) {
                obj->render(shader);
            }
        }
    }
}

void renderUI(AppState& state) {
    // This would use a text rendering system (e.g., FreeType)
    // For now, output to console on state changes
    
    static int lastPlayer = -1;
    int currentPlayer = static_cast<int>(state.game->getCurrentPlayer());
    
    if (currentPlayer != lastPlayer) {
        std::cout << "\n--- CURRENT PLAYER: " 
                  << (currentPlayer == 0 ? "Player 1 (Bottom)" : "Player 2 (Top)") 
                  << " ---" << std::endl;
        
        std::cout << "Store 1: " << state.game->getStoreCount(MancalaGame::Player::PLAYER_ONE)
                  << " | Store 2: " << state.game->getStoreCount(MancalaGame::Player::PLAYER_TWO)
                  << std::endl;
        
        lastPlayer = currentPlayer;
    }

    // Check win condition
    if (state.game->isGameOver()) {
        static bool winAnnounced = false;
        if (!winAnnounced) {
            auto gameState = state.game->getGameState();
            if (gameState == MancalaGame::GameState::PLAYER_ONE_WON) {
                std::cout << "\n*** PLAYER 1 WINS! ***\n" << std::endl;
            } else if (gameState == MancalaGame::GameState::PLAYER_TWO_WON) {
                std::cout << "\n*** PLAYER 2 WINS! ***\n" << std::endl;
            } else {
                std::cout << "\n*** DRAW! ***\n" << std::endl;
            }
            winAnnounced = true;
        }
    }
}