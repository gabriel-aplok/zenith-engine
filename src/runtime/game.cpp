#include "engine/application.hpp"
#include "engine/startup.hpp"
#include "log/log.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/mesh_builder.hpp"
#include "render/mesh_cache.hpp"
#include "render/render_context.hpp"
#include "render/render_submission.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Zenith {

class GameApplication final : public Application {
public:
    explicit GameApplication(const ApplicationConfig &config)
        : Application(config) {}

protected:
    void onInit() override {
        m_renderContext = createRenderContext(getWindow(), getConfig().debug);
        if (!m_renderContext) {
            Log::Error("Failed to create render context");
            requestQuit();
            return;
        }

        m_renderer = std::make_unique<Render::BgfxRenderer>();
        if (!m_renderer->initialize(*m_renderContext)) {
            Log::Error("Failed to initialize renderer");
            requestQuit();
            return;
        }

        m_meshCache.setUploader(m_renderer.get());
        m_pyramidMesh = m_meshCache.acquire("demo/pyramid", MeshBuilder::makePyramid());
        m_cubeMesh = m_meshCache.acquire("demo/cube", MeshBuilder::makeCube());
        if (m_pyramidMesh.id == 0 || m_cubeMesh.id == 0) {
            Log::Error("Failed to create demo meshes");
            requestQuit();
        }
    }

    void onRender() override {
        if (!m_renderer || !m_renderContext) {
            return;
        }

        m_renderContext->beginFrame();
        m_frame.commands.clear();
        m_frame.commands.reserve(2);
        m_frame.commands.setView(makeViewState());
        m_frame.commands.drawIndexed(m_pyramidMesh, 0, 0, glm::translate(glm::mat4{1.0f}, glm::vec3{-1.5f, 0.0f, 0.0f}));
        m_frame.commands.drawIndexed(m_cubeMesh, 0, 0, glm::translate(glm::mat4{1.0f}, glm::vec3{1.5f, 0.0f, 0.0f}));
        m_renderer->render(m_frame);
        m_renderContext->endFrame();
    }

    void onShutdown() override {
        if (m_renderer) {
            m_meshCache.clear();
            m_renderer->shutdown();
        }

        m_renderer.reset();
        m_renderContext.reset();
    }

private:
    Render::RenderViewState makeViewState() const {
        const glm::vec3 eye{0.0f, 2.5f, 6.0f};
        const glm::vec3 target{0.0f, 0.0f, 0.0f};
        const glm::vec3 up{0.0f, 1.0f, 0.0f};
        const glm::ivec2 size = m_renderContext ? m_renderContext->framebufferSize() : glm::ivec2{1, 1};
        const float aspect = size.y > 0 ? static_cast<float>(size.x) / static_cast<float>(size.y) : 1.0f;

        Render::RenderViewState viewState{};
        viewState.view = glm::lookAt(eye, target, up);
        viewState.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
        viewState.projection[1][1] *= -1.0f;
        return viewState;
    }

    std::unique_ptr<RenderContext> m_renderContext;
    std::unique_ptr<IRenderer> m_renderer;
    Render::RenderMeshCache m_meshCache;
    Render::MeshHandle m_pyramidMesh{};
    Render::MeshHandle m_cubeMesh{};
    RenderFrame m_frame{};
};

} // namespace Zenith

int main(int argc, char **argv) {
    Zenith::Log::Init();
    Zenith::GameApplication app{Zenith::parseApplicationConfig(argc, argv)};
    app.run();
    return 0;
}
