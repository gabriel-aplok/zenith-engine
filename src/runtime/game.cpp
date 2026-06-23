#include "engine/application.hpp"
#include "engine/startup.hpp"
#include "components/mesh_filter.hpp"
#include "components/mesh_renderer.hpp"
#include "log/log.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/mesh_builder.hpp"
#include "render/mesh_cache.hpp"
#include "render/render_context.hpp"
#include "render/render_submission.hpp"
#include "scene/scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Zenith
{

    class GameApplication final : public Application
    {
    public:
        explicit GameApplication(const ApplicationConfig &config)
            : Application(config) {}

    protected:
        void onInit() override
        {
            m_renderContext = createRenderContext(getWindow(), getConfig().debug);
            if (!m_renderContext)
            {
                Log::Error("Failed to create render context");
                requestQuit();
                return;
            }

            m_renderer = std::make_unique<Render::BgfxRenderer>();
            if (!m_renderer->initialize(*m_renderContext))
            {
                Log::Error("Failed to initialize renderer");
                requestQuit();
                return;
            }

            m_meshCache.setUploader(m_renderer.get());
            m_cubeMesh = m_meshCache.acquireRef("demo/cube", MeshBuilder::makeCube());
            if (!m_cubeMesh)
            {
                Log::Error("Failed to create demo meshes");
                requestQuit();
                return;
            }

            GameObject &cube = m_scene.createGameObject("Cube");
            cube.transform().setPosition(glm::vec3{0.0f, 0.0f, 0.0f});

            auto &filter = cube.add_component<Components::MeshFilter>();
            filter.setMesh(m_cubeMesh.handle());

            auto &renderer = cube.add_component<Components::MeshRenderer>();
            renderer.setMaterial({.tint = glm::vec4{0.9f, 1.0f, 1.0f, 1.0f}});
        }

        void onUpdate(float deltaTime) override
        {
            m_scene.update(deltaTime);
        }

        void onRender() override
        {
            if (!m_renderer || !m_renderContext)
            {
                return;
            }

            m_renderContext->beginFrame();
            m_frame.begin();
            m_frame.setView(makeViewState());
            m_scene.render(m_frame);
            m_frame.finalize();
            m_renderer->render(m_frame);
            m_renderContext->endFrame();
        }

        void onShutdown() override
        {
            m_scene.clear();
            m_cubeMesh.reset();
            if (m_renderer)
            {
                m_meshCache.clear();
                m_renderer->shutdown();
            }

            m_renderer.reset();
            m_renderContext.reset();
        }

    private:
        Render::RenderViewState makeViewState() const
        {
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
        Render::MeshRef m_cubeMesh{};
        Scene m_scene{};
        RenderFrame m_frame{};
    };

} // namespace Zenith

int main(int argc, char **argv)
{
    Zenith::Log::Init();
    Zenith::GameApplication app{Zenith::parseApplicationConfig(argc, argv)};
    app.run();
    return 0;
}
