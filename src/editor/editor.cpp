#include "engine/application.hpp"
#include <imgui.h>
#include <iostream>

class EditorApp : public Zenith::Application {
protected:
    void onGui() override {
        ImGui::ShowDemoWindow();

        ImGui::Begin("Zenith Editor");
        ImGui::Text("Hello, Engine!");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        if (ImGui::Button("Exit")) {
            getWindow().setShouldClose(true);
        }
        ImGui::End();
    }
};

int main() {
    try {
        EditorApp app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}