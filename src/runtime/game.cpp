#include "engine/application.hpp"
#include "engine/startup.hpp"
#include "components/mesh_filter.hpp"
#include "components/camera.hpp"
#include "components/mesh_renderer.hpp"
#include "components/script_component.hpp"
#include "components/script_behaviour.hpp"
#include "log/log.hpp"
#include "resource/baked_mesh_asset.hpp"
#include "resource/image_source.hpp"
#include "resource/resource_loaders.hpp"
#include "resource/resource_manager.hpp"
#include "resource/text_source.hpp"
#include "resource/texture_asset.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/mesh_cache.hpp"
#include "render/texture_cache.hpp"
#include "render/render_context.hpp"
#include "scene/scene.hpp"
#include "systems/camera_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"
#include "systems/script_system.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace Zenith
{
    class DemoRotateBehaviour final : public ScriptBehaviour
    {
    public:
        void onUpdate(GameObject &owner, float deltaTime) override
        {
            owner.transform().rotateEulerDegrees(glm::vec3{0.0f, 45.0f * deltaTime, 0.0f});
        }
    };

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
            m_textureCache.setUploader(m_renderer.get());
            m_scene.setMeshMetadataProvider(&m_meshCache);
            m_scene.addSystem(std::make_unique<TransformSystem>());
            m_scene.addSystem(std::make_unique<ScriptSystem>());
            m_scene.addSystem(std::make_unique<CameraSystem>());
            m_scene.addSystem(std::make_unique<RenderSystem>());
            registerStandardResourceLoaders(m_resources);

            m_vertexShaderSource = m_resources.load<TextSource>("resources/shaders/mesh_vs.sc");
            if (m_vertexShaderSource)
            {
                Log::Info("Loaded vertex shader source: {} bytes", m_vertexShaderSource->data().size());
            }

            m_cubeMeshAsset = m_resources.load<BakedMeshAsset>("resources/models/obj/cube.obj");
            if (!m_cubeMeshAsset)
            {
                Log::Error("Failed to load file-backed cube resource; falling back to builtin");
                m_cubeMeshAsset = m_resources.load<BakedMeshAsset>("builtin://cube");
                if (!m_cubeMeshAsset)
                {
                    Log::Error("Failed to load demo cube resource");
                    requestQuit();
                    return;
                }
            }

            m_cubeMesh = m_meshCache.acquireRef("demo/cube", m_cubeMeshAsset->mesh);
            if (!m_cubeMesh)
            {
                Log::Error("Failed to create demo meshes");
                requestQuit();
                return;
            }

            m_cubeTextureAsset = m_resources.load<TextureAsset>("resources/models/obj/cube_diffuse.png");
            if (m_cubeTextureAsset)
            {
                m_cubeTexture = m_textureCache.acquireRef("demo/cube_diffuse", m_cubeTextureAsset->source.data());
            }

            if (!m_cubeTexture)
            {
                ImageSourceData checker{};
                checker.width = 2;
                checker.height = 2;
                checker.format = "rgba8";
                checker.pixels = {
                    255,
                    255,
                    255,
                    255,
                    32,
                    32,
                    32,
                    255,
                    32,
                    32,
                    32,
                    255,
                    255,
                    255,
                    255,
                    255,
                };
                m_checkerTexture = m_textureCache.acquireRef("demo/checker", checker);
            }

            GameObject &cameraObject = m_scene.createGameObject("Main Camera");
            cameraObject.transform().setPosition(glm::vec3{-10.0f, 0.0f, 6.0f});
            cameraObject.transform().lookAt(glm::vec3{0.0f, 0.0f, 0.0f});

            auto &camera = cameraObject.add_component<Components::Camera>();
            camera.setFieldOfView(45.0f);
            camera.setNearClipPlane(0.1f);
            camera.setFarClipPlane(100.0f);
            camera.setClearFlags(Components::Camera::ClearFlags::SolidColor);
            camera.setBackgroundColor(glm::vec4{0.08f, 0.09f, 0.11f, 1.0f});

            m_pivotObject = &m_scene.createGameObject("Pivot");
            m_pivotObject->transform().setPosition(glm::vec3{0.0f, 0.0f, 0.0f});
            auto &script = m_pivotObject->add_component<Components::ScriptComponent>();
            script.setBehaviour<DemoRotateBehaviour>();

            GameObject &cube = m_scene.createGameObject("Cube");
            cube.setParent(m_pivotObject);
            cube.transform().setPosition(glm::vec3{1.75f, 0.0f, 0.0f});

            auto &filter = cube.add_component<Components::MeshFilter>();
            filter.setMesh(m_cubeMesh.handle());

            auto &renderer = cube.add_component<Components::MeshRenderer>();
            Render::MaterialState material{};
            material.tint = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
            if (m_cubeTexture)
            {
                material.textureId = m_cubeTexture.handle().id;
            }
            else if (m_checkerTexture)
            {
                material.textureId = m_checkerTexture.handle().id;
            }
            renderer.setMaterial(material);
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
            m_scene.setFramebufferSize(m_renderContext->framebufferSize());
            m_scene.render(m_frame);
            m_frame.finalize();
            m_renderer->render(m_frame);
            m_renderContext->endFrame();
        }

        void onShutdown() override
        {
            m_scene.clear();
            m_cubeMesh.reset();
            m_cubeTexture.reset();
            m_checkerTexture.reset();
            if (m_renderer)
            {
                m_meshCache.clear();
                m_textureCache.clear();
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
        Render::TextureCache m_textureCache;
        ResourceHandle<TextSource> m_vertexShaderSource{};
        ResourceHandle<BakedMeshAsset> m_cubeMeshAsset{};
        ResourceHandle<TextureAsset> m_cubeTextureAsset{};
        Render::MeshRef m_cubeMesh{};
        Render::TextureRef m_cubeTexture{};
        Render::TextureRef m_checkerTexture{};
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
