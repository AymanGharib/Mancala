#include "core/Window.h"
#include "Rendering/Camera.h"
#include "Rendering/shader.h"
#include "Rendering/Material.h"
#include "Scene/GameObject.h"
#include "Scene/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

int main() {
    try {
        // Create window
        Window::Config config;
        config.width = 1280;
        config.height = 720;
        config.title = "Mancala 3D - Interactive";
        Window window(config);

        // Setup camera
        int width, height;
        window.getFramebufferSize(width, height);
        Camera camera(glm::vec3(0, 5, 8), glm::vec3(0, 0, 0), 45.0f, 
                     static_cast<float>(width) / height, 0.1f, 100.0f);

        // Load shader
        Shader shader("Shaders/phong.vs", "Shaders/phong.fs");

        // Create game objects
        std::vector<GameObject*> objects;
        
        // Board (flat platform)
        GameObject* board = new GameObject();
        Mesh* boardMesh = new Mesh(Mesh::createCube());
        board->setMesh(boardMesh);
        board->getTransform().setScale(glm::vec3(8.0f, 0.2f, 3.0f));
        
        Material boardMat;
        boardMat.ambient = glm::vec3(0.4f, 0.3f, 0.2f);
        boardMat.diffuse = glm::vec3(0.6f, 0.4f, 0.2f);
        boardMat.specular = glm::vec3(0.3f);
        boardMat.shininess = 32.0f;
        board->setMaterial(boardMat);
        objects.push_back(board);

        // Create some spheres (seeds)
        for (int i = 0; i < 4; i++) {
            GameObject* sphere = new GameObject();
            Mesh* sphereMesh = new Mesh(Mesh::createSphere(0.3f, 16));
            sphere->setMesh(sphereMesh);
            
            float x = -3.0f + i * 2.0f;
            sphere->getTransform().setPosition(glm::vec3(x, 0.5f, 0));
            
            Material mat;
            mat.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
            mat.diffuse = glm::vec3(0.3f + i * 0.15f, 0.6f - i * 0.1f, 0.8f);
            mat.specular = glm::vec3(0.8f);
            mat.shininess = 64.0f;
            sphere->setMaterial(mat);
            
            objects.push_back(sphere);
        }

        // Lighting
        glm::vec3 lightPos(5.0f, 8.0f, 5.0f);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

        std::cout << "\n=== CONTROLS ===" << std::endl;
        std::cout << "Right Mouse Button + Drag: Orbit camera" << std::endl;
        std::cout << "Mouse Wheel: Zoom in/out" << std::endl;
        std::cout << "WASD/QE: Pan camera" << std::endl;
        std::cout << "ESC: Exit\n" << std::endl;

        // Camera target (orbit center)
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        const float panSpeed = 0.05f;

        // Main loop
        while (!window.shouldClose()) {
            window.pollEvents();

            // === UPDATE CAMERA FROM WINDOW STATE ===
            float yaw = window.getYaw();
            float pitch = window.getPitch();
            float distance = window.getDistance();

            // Optional: WASD panning
            if (window.isKeyPressed(GLFW_KEY_W)) cameraTarget.z -= panSpeed;
            if (window.isKeyPressed(GLFW_KEY_S)) cameraTarget.z += panSpeed;
            if (window.isKeyPressed(GLFW_KEY_A)) cameraTarget.x -= panSpeed;
            if (window.isKeyPressed(GLFW_KEY_D)) cameraTarget.x += panSpeed;
            if (window.isKeyPressed(GLFW_KEY_Q)) cameraTarget.y -= panSpeed;
            if (window.isKeyPressed(GLFW_KEY_E)) cameraTarget.y += panSpeed;

            // Calculate camera position from orbit parameters
            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = sin(glm::radians(pitch));
            direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            
            glm::vec3 cameraPos = cameraTarget - glm::normalize(direction) * distance;
            
            camera.setPosition(cameraPos);
            camera.lookAt(cameraTarget);

            // Rendering
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            
            // Set matrices
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 projection = camera.getProjectionMatrix();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);

            // Set lighting uniforms
            shader.setVec3("viewPos", camera.getPosition());
            shader.setVec3("lightPos", lightPos);
            shader.setVec3("lightColor", lightColor);

            // Render all objects
            for (auto* obj : objects) {
                obj->render(shader);
            }

            window.swapBuffers();
        }

        // Cleanup
        for (auto* obj : objects) {
            delete obj;
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return -1;
    }
}