#include "core/Window.h"
#include "Rendering/Camera.h"
#include "Rendering/shader.h"
#include "Scene/GameObject.h"
#include "Game/MancalaGame.h"
#include "Interaction/ObjectPicker.h"
#include "Rendering/RenderModeManager.h"
#include "Rendering/TextureManager.h"
#include "Game/ThemeManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

// ===== CONFIGURATION =====
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;

constexpr float CAMERA_PAN_SPEED = 0.05f;

// ===== GLOBAL STATE =====
struct AppState {
    MancalaGame* game = nullptr;
    RenderModeManager& renderMode = RenderModeManager::getInstance();
    ThemeManager& themeManager = ThemeManager::getInstance();
    TextureManager& textureManager = TextureManager::getInstance();

    glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};

    GameObject* hoveredObject = nullptr;

    // UI toggles
    bool showHelp = true;
    bool showStats = true;

    // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // simple lights
    struct Light {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };
    std::vector<Light> lights;
};

// ===== Forward decl =====
static void setupLights(AppState& state);
static void processInput(Window& window, AppState& state);
static void handleMousePicking(Window& window, Camera& camera, AppState& state);
static void renderScene(Shader& shader, const std::vector<GameObject*>& objects, AppState& state);
static void renderUI(AppState& state);

// Re-apply theme to existing objects (board + pits + seeds)
static void applyThemeToGame(AppState& state) {
    auto objects = state.game->getAllObjects();
    int seedIdx = 0;

    // Apply theme to board/pits/seeds by type inference:
    // - board: first object (as in your getAllObjects() implementation)
    // - pits: pit objects from getPits()
    // - seeds: everything else that is not board/pit
    // If your getAllObjects order differs, this still works because pits are explicit.

    // Board (if exists)
    if (!objects.empty() && objects[0]) {
        state.themeManager.applyThemeToBoard(objects[0]);
    }

    // Pits
    const auto& pits = state.game->getPits();
    for (const auto& pit : pits) {
        if (pit.pitObject) {
            state.themeManager.applyThemeToPit(pit.pitObject, pit.index);
        }
    }

    // Seeds: iterate pits' seeds (more reliable than guessing from objects list)
    for (const auto& pit : pits) {
        for (auto* seed : pit.seeds) {
            if (seed) state.themeManager.applyThemeToSeed(seed, seedIdx++);
        }
    }
}

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
        int fbW, fbH;
        window.getFramebufferSize(fbW, fbH);
        Camera camera(
            glm::vec3(0, 6, 10),
            glm::vec3(0, 0, 0),
            45.0f,
            static_cast<float>(fbW) / static_cast<float>(fbH),
            0.1f,
            100.0f
        );

        // Load shader
        Shader shader("Shaders/phong.vs", "Shaders/phong.fs");

        // Init app state
        AppState state;
        state.game = new MancalaGame();
        state.game->initialize();
        setupLights(state);

        // Apply initial theme once (ensures materials match current theme)
        applyThemeToGame(state);

        // Print controls
        std::cout << "\n=============== MANCALA 3D ===============\n";
        std::cout << "=== CAMERA CONTROLS ===\n";
        std::cout << "  Right Mouse + Drag : Orbit camera\n";
        std::cout << "  Mouse Wheel        : Zoom in/out\n";
        std::cout << "  W/A/S/D/Q/E       : Pan camera\n\n";
        std::cout << "=== GAME CONTROLS ===\n";
        std::cout << "  Left Click         : Select pit & play\n";
        std::cout << "  R                  : Reset game\n\n";
        std::cout << "=== DISPLAY CONTROLS ===\n";
        std::cout << "  M                  : Cycle render mode\n";
        std::cout << "  T                  : Change theme\n";
        std::cout << "  H                  : Toggle help\n";
        std::cout << "  F                  : Toggle stats\n";
        std::cout << "  ESC                : Exit\n";
        std::cout << "=========================================\n\n";

        // Main loop
        while (!window.shouldClose()) {
            // delta time
            float currentFrame = glfwGetTime();
            state.deltaTime = currentFrame - state.lastFrame;
            state.lastFrame = currentFrame;

            window.pollEvents();

            // Input
            processInput(window, state);

            // Update camera from Window orbit controls
            float yaw = window.getYaw();
            float pitch = window.getPitch();
            float distance = window.getDistance();

            glm::vec3 direction;
            direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
            direction.y = std::sin(glm::radians(pitch));
            direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

            glm::vec3 cameraPos = state.cameraTarget - glm::normalize(direction) * distance;
            camera.setPosition(cameraPos);
            camera.lookAt(state.cameraTarget);

            // Update game animation
            if (state.game->isAnimating()) {
                state.game->updateAnimation(state.deltaTime);
            }

            // Mouse picking & click-to-play
            handleMousePicking(window, camera, state);

            // Render
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();

            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 projection = camera.getProjectionMatrix();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("viewPos", camera.getPosition());

            // lights
            shader.setInt("numLights", static_cast<int>(state.lights.size()));
            for (size_t i = 0; i < state.lights.size(); ++i) {
                std::string base = "lights[" + std::to_string(i) + "]";
                shader.setVec3(base + ".position", state.lights[i].position);
                shader.setVec3(base + ".color", state.lights[i].color);
                shader.setFloat(base + ".intensity", state.lights[i].intensity);
            }

            shader.setBool("useTextures", state.renderMode.shouldUseTextures());

            auto objects = state.game->getAllObjects();
            renderScene(shader, objects, state);

            renderUI(state);

            window.swapBuffers();
        }

        // cleanup
        delete state.game;
        state.textureManager.cleanup();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return -1;
    }
}

// ===== IMPLEMENTATION =====

static void setupLights(AppState& state) {
    AppState::Light mainLight;
    mainLight.position = glm::vec3(5.0f, 8.0f, 5.0f);
    mainLight.color = glm::vec3(1.0f, 0.95f, 0.85f);
    mainLight.intensity = 1.0f;
    state.lights.push_back(mainLight);

    AppState::Light fillLight;
    fillLight.position = glm::vec3(-4.0f, 6.0f, 3.0f);
    fillLight.color = glm::vec3(0.6f, 0.7f, 0.8f);
    fillLight.intensity = 0.5f;
    state.lights.push_back(fillLight);

    AppState::Light backLight;
    backLight.position = glm::vec3(0.0f, 4.0f, -6.0f);
    backLight.color = glm::vec3(0.8f, 0.8f, 1.0f);
    backLight.intensity = 0.3f;
    state.lights.push_back(backLight);
}

static void processInput(Window& window, AppState& state) {
    // Continuous camera pan
    if (window.isKeyPressed(GLFW_KEY_W)) state.cameraTarget.z -= CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_S)) state.cameraTarget.z += CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_A)) state.cameraTarget.x -= CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_D)) state.cameraTarget.x += CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_Q)) state.cameraTarget.y -= CAMERA_PAN_SPEED;
    if (window.isKeyPressed(GLFW_KEY_E)) state.cameraTarget.y += CAMERA_PAN_SPEED;

    // One-press actions (debounced)
    static bool rWas=false, mWas=false, tWas=false, hWas=false, fWas=false;

    bool r = window.isKeyPressed(GLFW_KEY_R);
    if (r && !rWas) { state.game->reset(); applyThemeToGame(state); std::cout << "[Game] Reset!\n"; }
    rWas = r;

    bool m = window.isKeyPressed(GLFW_KEY_M);
    if (m && !mWas) { state.renderMode.cycleMode(); std::cout << "[Display] Mode: " << state.renderMode.getModeName() << "\n"; }
    mWas = m;

    bool t = window.isKeyPressed(GLFW_KEY_T);
    if (t && !tWas) {
        int nextTheme = (state.themeManager.getCurrentThemeIndex() + 1) % state.themeManager.getThemeCount();
        state.themeManager.setTheme(nextTheme);
        applyThemeToGame(state);
        std::cout << "[Theme] Changed to: " << state.themeManager.getCurrentTheme().name << "\n";
    }
    tWas = t;

    bool h = window.isKeyPressed(GLFW_KEY_H);
    if (h && !hWas) { state.showHelp = !state.showHelp; std::cout << "[UI] Help: " << (state.showHelp ? "ON" : "OFF") << "\n"; }
    hWas = h;

    bool f = window.isKeyPressed(GLFW_KEY_F);
    if (f && !fWas) { state.showStats = !state.showStats; std::cout << "[UI] Stats: " << (state.showStats ? "ON" : "OFF") << "\n"; }
    fWas = f;
}

static void handleMousePicking(Window& window, Camera& camera, AppState& state) {
    // mouse pos
    double mouseX, mouseY;
    glfwGetCursorPos(window.getGLFWWindow(), &mouseX, &mouseY);

    int width, height;
    window.getFramebufferSize(width, height);

    // ray
    ObjectPicker::Ray ray = ObjectPicker::screenToWorldRay(
        static_cast<float>(mouseX),
        static_cast<float>(mouseY),
        width,
        height,
        camera
    );

    // IMPORTANT: pick ONLY pits (not board, not seeds)
    std::vector<GameObject*> pickables;
    const auto& pits = state.game->getPits();
    pickables.reserve(pits.size());
    for (const auto& p : pits) {
        if (p.pitObject) pickables.push_back(p.pitObject);
    }

    ObjectPicker::RayHit hit = ObjectPicker::pickObject(ray, pickables);
    state.hoveredObject = hit.hit ? hit.object : nullptr;

    // one-click logic
    static bool leftWasDown = false;
    bool leftDown = glfwGetMouseButton(window.getGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (leftDown && !leftWasDown) {
        if (state.hoveredObject && !state.game->isAnimating()) {
            // find pit index
            for (size_t i = 0; i < pits.size(); ++i) {
                if (pits[i].pitObject == state.hoveredObject) {
                    int pitIndex = static_cast<int>(i);
                    if (state.game->isValidMove(pitIndex)) {
                        state.game->executeMove(pitIndex);
                        std::cout << "[Game] Pit " << pitIndex << " selected\n";
                    } else {
                        std::cout << "[Game] Invalid move!\n";
                    }
                    break;
                }
            }
        } else {
            std::cout << "[Pick] No pit under cursor\n";
        }
    }

    leftWasDown = leftDown;
}

static void renderScene(Shader& shader, const std::vector<GameObject*>& objects, AppState& state) {
    auto mode = state.renderMode.getCurrentMode();

    if (mode == RenderModeManager::Mode::SHADED_WIRE) {
        // solid pass
        for (auto* obj : objects) {
            if (obj && obj->isVisible()) obj->render(shader);
        }

        // wire overlay pass
        state.renderMode.enableWireframeOverlay();
        shader.setVec3("wireframeColor", glm::vec3(0.0f, 0.0f, 0.0f));
        for (auto* obj : objects) {
            if (obj && obj->isVisible()) obj->render(shader);
        }
        state.renderMode.disableWireframeOverlay();
    } else {
        for (auto* obj : objects) {
            if (obj && obj->isVisible()) obj->render(shader);
        }
    }
}

static void renderUI(AppState& state) {
    // prints player change + stores
    static int lastPlayer = -1;
    int currentPlayer = static_cast<int>(state.game->getCurrentPlayer());

    if (currentPlayer != lastPlayer) {
        std::cout << "\n--- CURRENT PLAYER: "
                  << (currentPlayer == 0 ? "Player 1 (Bottom)" : "Player 2 (Top)")
                  << " ---\n";

        std::cout << "Store 1: " << state.game->getStoreCount(MancalaGame::Player::PLAYER_ONE)
                  << " | Store 2: " << state.game->getStoreCount(MancalaGame::Player::PLAYER_TWO)
                  << "\n";

        lastPlayer = currentPlayer;
    }

    if (state.game->isGameOver()) {
        static bool winAnnounced = false;
        if (!winAnnounced) {
            auto gameState = state.game->getGameState();
            if (gameState == MancalaGame::GameState::PLAYER_ONE_WON) {
                std::cout << "\n*** PLAYER 1 WINS! ***\n\n";
            } else if (gameState == MancalaGame::GameState::PLAYER_TWO_WON) {
                std::cout << "\n*** PLAYER 2 WINS! ***\n\n";
            } else {
                std::cout << "\n*** DRAW! ***\n\n";
            }
            winAnnounced = true;
        }
    }
}
