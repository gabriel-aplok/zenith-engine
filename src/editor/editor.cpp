#include "engine/application.hpp"
#include "engine/input_manager.hpp"
#include "engine/renderer.hpp"
#include "engine/shader.hpp"
#include "engine/scene.hpp"
#include <imgui.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Zenith {

    class EditorApp : public Application {
        Renderer m_renderer;
        Shader m_shader{ "shader/triangle.vert", "shader/triangle.frag" };
        Scene m_scene;

        void onInit() override {
            // Triangle
            std::vector<Zenith::Vertex> vertices = {
             { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, // Top - Red
             { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, // Right - Green
             { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }  // Left - Blue
            };
            std::vector<uint32_t> indices = { 0, 1, 2 };

            m_renderer.beginScene();
            m_renderer.submit(vertices, indices, m_shader);
            m_renderer.endScene();

            spdlog::info("Triangle submitted successfully!");
        }

        void onRender() override {
            m_shader.bind();
            m_renderer.draw();
        }

        void onGui() override
        {
            ImGui::ShowDemoWindow();

            ImGui::Begin("Zenith Editor");
            ImGui::Text("Hello, Engine!");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            if (ImGui::Button("Exit"))
            {
                getWindow().setShouldClose(true);
            }
            ImGui::End();

            auto& input = InputManager::get();

            ImGui::Begin("Controls");
            if (ImGui::Button("Reload Shader")) {
                spdlog::warn("Shader reload not implemented yet!");
            }
            ImGui::Text("Mouse Delta: %.2f %.2f",
                input.getMouseDelta().x,
                input.getMouseDelta().y);
            ImGui::End();

            if (input.isKeyJustPressed(GLFW_KEY_E)) {
                spdlog::info("Key 'E' pressed -> shutting down...");
                getWindow().setShouldClose(true);
            }
        }
    };

} // namespace Zenith

int main() {
    // ---------- spdlog setup ----------
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    try {
        spdlog::info("Starting Zenith Editor application...");

        Zenith::EditorApp app;
        app.run();

        spdlog::info("Application terminated gracefully.");
    }
    catch (const std::exception& e) {
        spdlog::error("Fatal exception: {}", e.what());
        return -1;
    }
    catch (...) {
        spdlog::critical("Unknown exception caught!");
        return -1;
    }

    return 0;
}