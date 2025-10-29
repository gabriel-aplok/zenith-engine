#include "engine/Application.hpp"
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace Zenith {

    Application::Application() {
        Window::Config config;
        config.title = "Zenith Engine";
        config.width = 1600;
        config.height = 900;
        config.vsync = true;

        m_window = std::make_unique<Window>(config);
        m_window->setResizeCallback([this](int w, int h) {
            // Optional: forward to derived class
            });

        // Setup Dear ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window->getHandle(), true);
        ImGui_ImplOpenGL3_Init("#version 460");

        m_lastTime = Clock::now();
    }

    void Application::run() {
        onInit();

        while (!m_window->shouldClose() && m_running) {
            auto now = Clock::now();
            float deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
            m_lastTime = now;

            glfwPollEvents();

            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            onUpdate(deltaTime);
            onGui();

            // Rendering
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            onRender();

            // Render ImGui
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            m_window->swapBuffers();
        }

        onShutdown();

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

} // namespace Zenith