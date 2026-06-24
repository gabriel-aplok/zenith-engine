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
#include "scene/scene_manager.hpp"
#include "systems/camera_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"
#include "systems/script_system.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <functional>

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

    class SceneSwitchBehaviour final : public ScriptBehaviour
    {
    public:
        using Callback = std::function<void()>;

        SceneSwitchBehaviour() = default;
        SceneSwitchBehaviour(Callback onSpacePressed, Callback onSceneEntered)
            : m_onSpacePressed(std::move(onSpacePressed)), m_onSceneEntered(std::move(onSceneEntered)) {}

        void onStart(GameObject &owner) override
        {
            if (m_onSceneEntered)
            {
                m_onSceneEntered();
            }
            (void)owner;
        }

        void onUpdate(GameObject &owner, float deltaTime) override
        {
            (void)deltaTime;
            auto *scene = owner.scene();
            if (!scene || !scene->input())
            {
                return;
            }

            if (scene->input()->isKeyPressed(KeyCode::Space) && m_onSpacePressed)
            {
                m_onSpacePressed();
            }
        }

    private:
        Callback m_onSpacePressed;
        Callback m_onSceneEntered;
    };

    namespace
    {
        void applyCubeMaterial(Components::MeshRenderer &renderer, const Render::TextureRef &primaryTexture, const Render::TextureRef &fallbackTexture, const glm::vec4 &tint)
        {
            Render::MaterialState material{};
            material.tint = tint;
            if (primaryTexture)
            {
                material.textureId = primaryTexture.handle().id;
            }
            else if (fallbackTexture)
            {
                material.textureId = fallbackTexture.handle().id;
            }
            renderer.setMaterial(material);
        }
    } // namespace

    class GameApplication final : public Application
    {
    public:
        explicit GameApplication(const ApplicationConfig &config)
            : Application(config) {}

    protected:
        std::unique_ptr<Scene> buildMainScene()
        {
            auto scene = std::make_unique<Scene>();

            GameObject &controller = scene->createGameObject("Scene Controller");
            auto &controllerScript = controller.add_component<Components::ScriptComponent>();
            controllerScript.setBehaviour<SceneSwitchBehaviour>(
                [this]()
                {
                    m_sceneManager.requestScene(buildCubeScene());
                },
                []() {});

            GameObject &cameraObject = scene->createGameObject("Main Camera");
            cameraObject.transform().setPosition(glm::vec3{-10.0f, 0.0f, 6.0f});
            cameraObject.transform().lookAt(glm::vec3{0.0f, 0.0f, 0.0f});

            auto &camera = cameraObject.add_component<Components::Camera>();
            camera.setFieldOfView(45.0f);
            camera.setNearClipPlane(0.1f);
            camera.setFarClipPlane(100.0f);
            camera.setClearFlags(Components::Camera::ClearFlags::SolidColor);
            camera.setBackgroundColor(glm::vec4{0.08f, 0.09f, 0.11f, 1.0f});

            GameObject &marker = scene->createGameObject("Pivot");
            marker.transform().setPosition(glm::vec3{0.0f, 0.0f, 0.0f});
            auto &script = marker.add_component<Components::ScriptComponent>();
            script.setBehaviour<DemoRotateBehaviour>();

            GameObject &cube = scene->createGameObject("Cube");
            cube.setParent(&marker);
            cube.transform().setPosition(glm::vec3{1.75f, 0.0f, 0.0f});
            auto &filter = cube.add_component<Components::MeshFilter>();
            filter.setMesh(m_cubeMesh.handle(), m_cubeMeshAsset->mesh.bounds);

            auto &renderer = cube.add_component<Components::MeshRenderer>();
            applyCubeMaterial(renderer, m_cubeTexture, m_checkerTexture, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

            return scene;
        }

        std::unique_ptr<Scene> buildCubeScene()
        {
            auto scene = std::make_unique<Scene>();

            GameObject &controller = scene->createGameObject("Scene Controller");
            auto &controllerScript = controller.add_component<Components::ScriptComponent>();
            controllerScript.setBehaviour<SceneSwitchBehaviour>(
                [this]()
                {
                    m_sceneManager.requestScene(buildMainScene());
                },
                []() {});

            GameObject &cameraObject = scene->createGameObject("Cube Camera");
            cameraObject.transform().setPosition(glm::vec3{-15.0f, 10.0f, 16.0f});
            cameraObject.transform().lookAt(glm::vec3{0.0f, 0.0f, 0.0f});

            auto &camera = cameraObject.add_component<Components::Camera>();
            camera.setFieldOfView(50.0f);
            camera.setNearClipPlane(0.1f);
            camera.setFarClipPlane(200.0f);
            camera.setClearFlags(Components::Camera::ClearFlags::SolidColor);
            camera.setBackgroundColor(glm::vec4{0.05f, 0.05f, 0.08f, 1.0f});

            for (int x = -3; x <= 3; ++x)
            {
                for (int y = -2; y <= 2; ++y)
                {
                    for (int z = -3; z <= 3; ++z)
                    {
                        GameObject &cube = scene->createGameObject("Cube");
                        cube.transform().setPosition(glm::vec3{static_cast<float>(x) * 2.5f, static_cast<float>(y) * 2.0f, static_cast<float>(z) * 2.5f});
                        auto &filter = cube.add_component<Components::MeshFilter>();
                        filter.setMesh(m_cubeMesh.handle(), m_cubeMeshAsset->mesh.bounds);

                        auto &renderer = cube.add_component<Components::MeshRenderer>();
                        applyCubeMaterial(renderer, m_cubeTexture, m_checkerTexture, glm::vec4{0.35f + 0.1f * static_cast<float>((x + 3) % 4), 0.45f + 0.1f * static_cast<float>((y + 2) % 3), 0.65f + 0.05f * static_cast<float>((z + 3) % 4), 1.0f});
                    }
                }
            }

            return scene;
        }

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
            registerStandardResourceLoaders(m_resources);

            m_vertexShaderSource = m_resources.load<TextSource>("resources/shaders/mesh_vs.sc");
            if (m_vertexShaderSource)
            {
                Log::Info("Loaded vertex shader source: {} bytes", m_vertexShaderSource->data().size());
            }

            m_cubeMeshAsset = m_resources.load<BakedMeshAsset>("resources/models/m1014.obj");
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

            m_cubeTextureAsset = m_resources.load<TextureAsset>("resources/models/m1014.png");
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

            m_sceneManager.addSystem(std::make_unique<TransformSystem>());
            m_sceneManager.addSystem(std::make_unique<ScriptSystem>());
            m_sceneManager.addSystem(std::make_unique<CameraSystem>());
            m_sceneManager.addSystem(std::make_unique<RenderSystem>());
            m_sceneManager.setScene(buildMainScene());
            if (!m_sceneManager.currentScene())
            {
                Log::Error("Failed to create initial scene");
                requestQuit();
                return;
            }
        }

        void onUpdate(float deltaTime) override
        {
            if (auto *scene = m_sceneManager.currentScene())
            {
                scene->setInputState(&getInput());
            }

            m_sceneManager.update(deltaTime);
        }

        void onRender() override
        {
            if (!m_renderer || !m_renderContext)
            {
                return;
            }

            m_renderContext->beginFrame();
            m_frame.begin();
            if (auto *scene = m_sceneManager.currentScene())
            {
                scene->setFramebufferSize(m_renderContext->framebufferSize());
                scene->setInputState(&getInput());
            }
            m_sceneManager.render(m_frame);
            m_frame.finalize();
            m_renderer->render(m_frame);
            m_renderContext->endFrame();
        }

        void onShutdown() override
        {
            m_sceneManager.clear();
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
        SceneManager m_sceneManager{};
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
