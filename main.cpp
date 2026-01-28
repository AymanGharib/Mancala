// main.cpp
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "core/Window.h"
#include "Rendering/Camera.h"
#include "Rendering/shader.h"
#include "Scene/GameObject.h"
#include "Game/MancalaGame.h"
#include "Interaction/ObjectPicker.h"
#include "Rendering/RenderModeManager.h"
#include "Rendering/TextureManager.h"
#include "Game/ThemeManager.h"

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

// ===== CONFIGURATION =====
constexpr int   WINDOW_WIDTH      = 1280;
constexpr int   WINDOW_HEIGHT     = 720;
constexpr float CAMERA_PAN_SPEED  = 0.05f;

// ===== GLOBAL STATE =====
struct AppState {
    MancalaGame* game = nullptr;

    RenderModeManager& renderMode   = RenderModeManager::getInstance();
    ThemeManager&      themeManager = ThemeManager::getInstance();
    TextureManager&    textureMgr   = TextureManager::getInstance();

    glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};

    // Hover state (optional)
    GameObject* hoveredObject = nullptr;

    // UI toggles
    bool showHelp  = true;
    bool showStats = true;

    // Lighting toggle
    bool lightsEnabled = true;

    // timing
    float deltaTime  = 0.0f;
    float lastFrame  = 0.0f;

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
static void applyThemeToGame(AppState& state);
static void drawImGuiHUD(AppState& state);

// Re-apply theme to existing objects (board + pits + seeds)
static void applyThemeToGame(AppState& state) {
    auto objects = state.game->getAllObjects();
    int seedIdx = 0;

    // Board
    if (!objects.empty() && objects[0]) {
        state.themeManager.applyThemeToBoard(objects[0]);
    }

    // Pits + Seeds
    const auto& pits = state.game->getPits();

    for (const auto& pit : pits) {
        if (pit.pitObject) {
            state.themeManager.applyThemeToPit(pit.pitObject, pit.index);
        }
    }

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
        config.width       = WINDOW_WIDTH;
        config.height      = WINDOW_HEIGHT;
        config.title       = "Mancala 3D - Interactive Game";
        config.msaaSamples = 4;
        config.vsync       = true;

        Window window(config);

        // Keep viewport correct if resized
        window.setFramebufferSizeCallback([&](int w, int h) {
            glViewport(0, 0, w, h);
        });

        // ===== ImGui init =====
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window.getGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 330");

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

        // Apply initial theme once
        applyThemeToGame(state);

        // Print controls (console)
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
        std::cout << "  L                  : Toggle lighting\n";
        std::cout << "  H                  : Toggle help\n";
        std::cout << "  F                  : Toggle stats\n";
        std::cout << "  ESC                : Exit\n";
        std::cout << "=========================================\n\n";

        // Main loop
        while (!window.shouldClose()) {
            // delta time
            float currentFrame = static_cast<float>(glfwGetTime());
            state.deltaTime = currentFrame - state.lastFrame;
            state.lastFrame = currentFrame;

            window.pollEvents();

            // Input
            processInput(window, state);

            // Update camera from Window orbit controls
            float yaw      = window.getYaw();
            float pitch    = window.getPitch();
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

            // Mouse picking & click-to-play (respects ImGui capture)
            handleMousePicking(window, camera, state);

            // Render 3D
            glClearColor(0.10f, 0.10f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();

            // matrices
            shader.setMat4("view", camera.getViewMatrix());
            shader.setMat4("projection", camera.getProjectionMatrix());
            shader.setVec3("viewPos", camera.getPosition());

            // textures usage depends on render mode
            shader.setBool("useTextures", state.renderMode.shouldUseTextures());

            // lighting toggle
            // IMPORTANT: your phong.fs must have:
            //   uniform bool lightingEnabled;
            shader.setBool("lightingEnabled", state.lightsEnabled);

            if (state.lightsEnabled) {
                shader.setInt("numLights", static_cast<int>(state.lights.size()));
                for (size_t i = 0; i < state.lights.size(); ++i) {
                    std::string base = "lights[" + std::to_string(i) + "]";
                    shader.setVec3(base + ".position", state.lights[i].position);
                    shader.setVec3(base + ".color", state.lights[i].color);
                    shader.setFloat(base + ".intensity", state.lights[i].intensity);
                }
            } else {
                shader.setInt("numLights", 0);
            }

            // draw objects
            renderScene(shader, state.game->getAllObjects(), state);

            // ===== ImGui frame =====
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            drawImGuiHUD(state);

            ImGui::Render();

            // Ensure UI renders on top
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            window.swapBuffers();
        }

        // ===== ImGui shutdown =====
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // cleanup
        delete state.game;
        state.textureMgr.cleanup();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return -1;
    }
}

// ===== IMPLEMENTATION =====

static void setupLights(AppState& state) {
    state.lights.clear();

    AppState::Light mainLight;
    mainLight.position  = glm::vec3(5.0f, 8.0f, 5.0f);
    mainLight.color     = glm::vec3(1.0f, 0.95f, 0.85f);
    mainLight.intensity = 1.0f;
    state.lights.push_back(mainLight);

    AppState::Light fillLight;
    fillLight.position  = glm::vec3(-4.0f, 6.0f, 3.0f);
    fillLight.color     = glm::vec3(0.6f, 0.7f, 0.8f);
    fillLight.intensity = 0.5f;
    state.lights.push_back(fillLight);

    AppState::Light backLight;
    backLight.position  = glm::vec3(0.0f, 4.0f, -6.0f);
    backLight.color     = glm::vec3(0.8f, 0.8f, 1.0f);
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
    static bool rWas=false, mWas=false, tWas=false, hWas=false, fWas=false, lWas=false;

    bool r = window.isKeyPressed(GLFW_KEY_R);
    if (r && !rWas) {
        state.game->reset();
        applyThemeToGame(state);
        std::cout << "[Game] Reset!\n";
    }
    rWas = r;

    bool m = window.isKeyPressed(GLFW_KEY_M);
    if (m && !mWas) {
        state.renderMode.cycleMode();
        std::cout << "[Display] Mode: " << state.renderMode.getModeName() << "\n";
    }
    mWas = m;

    bool t = window.isKeyPressed(GLFW_KEY_T);
    if (t && !tWas) {
        int nextTheme = (state.themeManager.getCurrentThemeIndex() + 1) % state.themeManager.getThemeCount();
        state.themeManager.setTheme(nextTheme);
        applyThemeToGame(state);
        std::cout << "[Theme] Changed to: " << state.themeManager.getCurrentTheme().name << "\n";
    }
    tWas = t;

    bool l = window.isKeyPressed(GLFW_KEY_L);
    if (l && !lWas) {
        state.lightsEnabled = !state.lightsEnabled;
        std::cout << "[Lighting] " << (state.lightsEnabled ? "ON" : "OFF") << "\n";
    }
    lWas = l;

    bool h = window.isKeyPressed(GLFW_KEY_H);
    if (h && !hWas) state.showHelp = !state.showHelp;
    hWas = h;

    bool f = window.isKeyPressed(GLFW_KEY_F);
    if (f && !fWas) state.showStats = !state.showStats;
    fWas = f;
}

static void handleMousePicking(Window& window, Camera& camera, AppState& state) {
    static bool leftWasDown = false;
    bool leftDown = glfwGetMouseButton(window.getGLFWWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (ImGui::GetIO().WantCaptureMouse) {
        leftWasDown = leftDown;
        return;
    }

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

    // pick ONLY pits
    std::vector<GameObject*> pickables;
    const auto& pits = state.game->getPits();
    pickables.reserve(pits.size());
    for (const auto& p : pits) {
        if (p.pitObject) pickables.push_back(p.pitObject);
    }

    ObjectPicker::RayHit hit = ObjectPicker::pickObject(ray, pickables);
    state.hoveredObject = hit.hit ? hit.object : nullptr;

    // click-to-play
    if (leftDown && !leftWasDown) {
        if (state.hoveredObject && !state.game->isAnimating()) {
            for (size_t i = 0; i < pits.size(); ++i) {
                if (pits[i].pitObject == state.hoveredObject) {
                    int pitIndex = static_cast<int>(i);
                    if (state.game->isValidMove(pitIndex)) {
                        state.game->executeMove(pitIndex);
                    }
                    break;
                }
            }
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

static void drawImGuiHUD(AppState& state) {
    // Score window
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin("Score", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    int p1 = state.game->getStoreCount(MancalaGame::Player::PLAYER_ONE);
    int p2 = state.game->getStoreCount(MancalaGame::Player::PLAYER_TWO);
    auto cp = state.game->getCurrentPlayer();

    ImGui::Text("Current: %s",
        (cp == MancalaGame::Player::PLAYER_ONE) ? "Player 1 (Bottom)" : "Player 2 (Top)");
    ImGui::Separator();
    ImGui::Text("Store P1: %d", p1);
    ImGui::Text("Store P2: %d", p2);

    ImGui::Separator();
    ImGui::Text("Lighting: %s", state.lightsEnabled ? "ON" : "OFF");

    if (state.game->isGameOver()) {
        ImGui::Separator();
        auto gs = state.game->getGameState();
        if (gs == MancalaGame::GameState::PLAYER_ONE_WON) ImGui::Text("Winner: Player 1");
        else if (gs == MancalaGame::GameState::PLAYER_TWO_WON) ImGui::Text("Winner: Player 2");
        else ImGui::Text("Result: Draw");
    }

    ImGui::End();

    // Help window
    if (state.showHelp) {
        ImGui::SetNextWindowPos(ImVec2(10, 140), ImGuiCond_Always);
        ImGui::Begin("Help", &state.showHelp, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("RMB Drag : Orbit camera");
        ImGui::Text("Wheel    : Zoom");
        ImGui::Text("W/A/S/D/Q/E : Pan");
        ImGui::Separator();
        ImGui::Text("LMB      : Play (select pit)");
        ImGui::Text("R        : Reset");
        ImGui::Text("T        : Theme");
        ImGui::Text("M        : Render mode");
        ImGui::Text("L        : Toggle lighting");
        ImGui::Text("H        : Toggle help");
        ImGui::Text("F        : Toggle stats");
        ImGui::End();
    }

    // Stats window
    if (state.showStats) {
        ImGui::SetNextWindowPos(ImVec2(10, 330), ImGuiCond_Always);
        ImGui::Begin("Stats", &state.showStats, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::End();
    }
}
