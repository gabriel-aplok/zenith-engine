#include "engine/application.hpp"
#include "engine/startup.hpp"
#include "components/mesh_filter.hpp"
#include "components/camera.hpp"
#include "components/mesh_renderer.hpp"
#include "log/log.hpp"
#include "resource/resource_manager.hpp"
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
            m_scene.setMeshMetadataProvider(&m_meshCache);
            m_resources.registerLoader<Render::MeshData>([](const std::string &path) -> std::shared_ptr<Render::MeshData> {
                if (path == "builtin://cube")
                {
                    return std::make_shared<Render::MeshData>(MeshBuilder::makeCube());
                }

                if (path == "builtin://pyramid")
                {
                    return std::make_shared<Render::MeshData>(MeshBuilder::makePyramid());
                }

                if (path == "builtin://plane")
                {
                    return std::make_shared<Render::MeshData>(MeshBuilder::makePlane());
                }

                return nullptr;
            });

            m_cubeMeshData = m_resources.load<Render::MeshData>("builtin://cube");
            if (!m_cubeMeshData)
            {
                Log::Error("Failed to load demo cube resource");
                requestQuit();
                return;
            }

            m_cubeMesh = m_meshCache.acquireRef("demo/cube", *m_cubeMeshData);
            if (!m_cubeMesh)
            {
                Log::Error("Failed to create demo meshes");
                requestQuit();
                return;
            }

            GameObject &cameraObject = m_scene.createGameObject("Main Camera");
            cameraObject.transform().setPosition(glm::vec3{0.0f, 2.5f, 6.0f});
            cameraObject.transform().lookAt(glm::vec3{0.0f, 0.0f, 0.0f});

            auto &camera = cameraObject.add_component<Components::Camera>();
            camera.setFieldOfView(45.0f);
            camera.setNearClipPlane(0.1f);
            camera.setFarClipPlane(100.0f);
            camera.setClearFlags(Components::Camera::ClearFlags::SolidColor);
            camera.setBackgroundColor(glm::vec4{0.08f, 0.09f, 0.11f, 1.0f});

            m_pivotObject = &m_scene.createGameObject("Pivot");
            m_pivotObject->transform().setPosition(glm::vec3{0.0f, 0.0f, 0.0f});

            GameObject &cube = m_scene.createGameObject("Cube");
            cube.setParent(m_pivotObject);
            cube.transform().setPosition(glm::vec3{1.75f, 0.0f, 0.0f});

            auto &filter = cube.add_component<Components::MeshFilter>();
            filter.setMesh(m_cubeMesh.handle());

            auto &renderer = cube.add_component<Components::MeshRenderer>();
            renderer.setMaterial({.tint = glm::vec4{0.9f, 1.0f, 1.0f, 1.0f}});
        }

        void onUpdate(float deltaTime) override
        {
            if (m_pivotObject != nullptr)
            {
                m_pivotObject->transform().rotateEulerDegrees(glm::vec3{0.0f, 45.0f * deltaTime, 0.0f});
            }

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
            if (!m_scene.buildRenderFrame(m_frame, m_renderContext->framebufferSize()))
            {
                m_frame.finalize();
                m_renderContext->endFrame();
                return;
            }

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
        std::unique_ptr<RenderContext> m_renderContext;
        std::unique_ptr<IRenderer> m_renderer;
        ResourceManager m_resources;
        Render::RenderMeshCache m_meshCache;
        ResourceHandle<Render::MeshData> m_cubeMeshData{};
        Render::MeshRef m_cubeMesh{};
        Scene m_scene{};
        RenderFrame m_frame{};
        GameObject *m_pivotObject = nullptr;
    };

} // namespace Zenith

int main(int argc, char **argv)
{
    Zenith::Log::Init();
    Zenith::GameApplication app{Zenith::parseApplicationConfig(argc, argv)};
    app.run();
    return 0;
}
