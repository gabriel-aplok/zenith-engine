#include "engine/application.hpp"
#include "engine/startup.hpp"
#include "components/mesh_filter.hpp"
#include "components/camera.hpp"
#include "components/mesh_renderer.hpp"
#include "components/script_component.hpp"
#include "components/script_behaviour.hpp"
#include "log/log.hpp"
#include "resource/resource.hpp"
#include "resource/resource_manager.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/render_context.hpp"
#include "scene/scene.hpp"
#include "scene/scene_manager.hpp"
#include "systems/camera_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"
#include "systems/script_system.hpp"

#include <functional>

namespace Zenith
{
    class DemoRotateBehaviour final : public ScriptBehaviour
    {
    public:
        void onUpdate(GameObject& owner, float deltaTime) override
        {
            owner.transform().rotateEulerDegrees(Vector3{ 0.0f, 45.0f * deltaTime, 0.0f });
        }
    };

    class SceneSwitchBehaviour final : public ScriptBehaviour
    {
    public:
        using Callback = std::function<void()>;

        SceneSwitchBehaviour() = default;
        SceneSwitchBehaviour(Callback onSpacePressed, Callback onSceneEntered)
            : m_onSpacePressed(std::move(onSpacePressed)), m_onSceneEntered(std::move(onSceneEntered)) {
        }

        void onStart(GameObject& owner) override
        {
            if (m_onSceneEntered)
                m_onSceneEntered();

            (void)owner;
        }

        void onUpdate(GameObject& owner, float deltaTime) override
        {
            (void)deltaTime;
            auto* scene = owner.scene();
            if (!scene || !scene->input())
                return;

            if (scene->input()->isKeyPressed(KeyCode::Space) && m_onSpacePressed)
                m_onSpacePressed();

        }

    private:
        Callback m_onSpacePressed;
        Callback m_onSceneEntered;
    };

    namespace
    {
        void applyCubeMaterial(Components::MeshRenderer& renderer, Render::TextureHandle primaryTexture, Render::TextureHandle fallbackTexture, const Vector4& tint)
        {
            Render::MaterialState material{};
            material.tint = tint;

            if (primaryTexture.id != 0)
                material.textureId = primaryTexture.id;
            else if (fallbackTexture.id != 0)
                material.textureId = fallbackTexture.id;

            renderer.setMaterial(material);
        }
    } // namespace

    class GameApplication final : public Application
    {
    public:
        explicit GameApplication(const ApplicationConfig& config)
            : Application(config) {
        }

    protected:
        std::unique_ptr<Scene> buildMainScene()
        {
            auto scene = std::make_unique<Scene>();

            GameObject& controller = scene->createGameObject("Scene Controller");
            auto& controllerScript = controller.add_component<Components::ScriptComponent>();
            controllerScript.setBehaviour<SceneSwitchBehaviour>(
                [this]()
                {
                    m_sceneManager.requestScene(buildCubeScene());
                },
                []() {});

            GameObject& cameraObject = scene->createGameObject("Main Camera");
            cameraObject.transform().setPosition(Vector3{ -10.0f, 0.0f, 6.0f });
            cameraObject.transform().lookAt(Vector3{ 0.0f, 0.0f, 0.0f });

            auto& camera = cameraObject.add_component<Components::Camera>();
            camera.setFieldOfView(45.0f);
            camera.setNearClipPlane(0.1f);
            camera.setFarClipPlane(100.0f);
            camera.setClearFlags(Components::Camera::ClearFlags::SolidColor);
            camera.setBackgroundColor(Vector4{ 0.08f, 0.09f, 0.11f, 1.0f });

            GameObject& marker = scene->createGameObject("Pivot");
            marker.transform().setPosition(Vector3{ 0.0f, 0.0f, 0.0f });
            auto& script = marker.add_component<Components::ScriptComponent>();
            script.setBehaviour<DemoRotateBehaviour>();

            GameObject& cube = scene->createGameObject("Cube");
            cube.setParent(&marker);
            cube.transform().setPosition(Vector3{ 1.75f, 0.0f, 0.0f });
            auto& filter = cube.add_component<Components::MeshFilter>();
            filter.setMesh(m_cubeMesh, m_cubeMeshAsset->meshes.front().mesh.bounds);

            auto& renderer = cube.add_component<Components::MeshRenderer>();
            applyCubeMaterial(renderer, m_cubeTexture, m_checkerTexture, Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });

            GameObject& car = scene->createGameObject("car");
            car.setParent(&marker);
            car.transform().setPosition(Vector3{ 1.75f, 0.0f, 3.0f });
            auto& filter2 = car.add_component<Components::MeshFilter>();
            filter2.setMesh(m_cubeMesh, m_cubeMeshAsset->meshes.front().mesh.bounds);

            auto& renderer2 = car.add_component<Components::MeshRenderer>();
            applyCubeMaterial(renderer2, m_cubeTexture, m_checkerTexture, Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });

            return scene;
        }

        std::unique_ptr<Scene> buildCubeScene()
        {
            auto scene = std::make_unique<Scene>();

            GameObject& controller = scene->createGameObject("Scene Controller");
            auto& controllerScript = controller.add_component<Components::ScriptComponent>();
            controllerScript.setBehaviour<SceneSwitchBehaviour>(
                [this]()
                {
                    m_sceneManager.requestScene(buildMainScene());
                },
                []() {});

            GameObject& cameraObject = scene->createGameObject("Cube Camera");
            cameraObject.transform().setPosition(Vector3{ -15.0f, 10.0f, 16.0f });
            cameraObject.transform().lookAt(Vector3{ 0.0f, 0.0f, 0.0f });

            auto& camera = cameraObject.add_component<Components::Camera>();
            camera.setFieldOfView(50.0f);
            camera.setNearClipPlane(0.1f);
            camera.setFarClipPlane(200.0f);
            camera.setClearFlags(Components::Camera::ClearFlags::SolidColor);
            camera.setBackgroundColor(Vector4{ 0.05f, 0.05f, 0.08f, 1.0f });

            for (int x = -3; x <= 3; ++x)
            {
                for (int y = -2; y <= 2; ++y)
                {
                    for (int z = -3; z <= 3; ++z)
                    {
                        GameObject& cube = scene->createGameObject("Cube");
                        cube.transform().setPosition(Vector3{ static_cast<float>(x) * 2.5f, static_cast<float>(y) * 2.0f, static_cast<float>(z) * 2.5f });
                        auto& filter = cube.add_component<Components::MeshFilter>();
                        filter.setMesh(m_cubeMesh, m_cubeMeshAsset->meshes.front().mesh.bounds);

                        auto& renderer = cube.add_component<Components::MeshRenderer>();
                        applyCubeMaterial(renderer, m_cubeTexture, m_checkerTexture, Vector4{ 0.35f + 0.1f * static_cast<float>((x + 3) % 4), 0.45f + 0.1f * static_cast<float>((y + 2) % 3), 0.65f + 0.05f * static_cast<float>((z + 3) % 4), 1.0f });
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

            m_resources.setRenderer(m_renderer.get());
            m_resources.importAllStaleAssets();

            m_vertexShaderSource = m_resources.load<ShaderResource>("res://shaders/mesh_vs.sc");
            if (m_vertexShaderSource)
                Log::Info("Loaded vertex shader source: {} bytes", m_vertexShaderSource->source.size());

            m_cubeMeshAsset = m_resources.load<ModelResource>("builtin://cube");
            if (!m_cubeMeshAsset)
            {
                Log::Error("Failed to load demo cube resource");
                requestQuit();
                return;
            }

            if (m_cubeMeshAsset->meshes.empty() || m_cubeMeshAsset->meshes.front().handle.id == 0)
            {
                Log::Error("Failed to create demo meshes");
                requestQuit();
                return;
            }

            m_cubeMesh = m_cubeMeshAsset->meshes.front().handle;

            m_cubeTextureAsset = m_resources.load<TextureResource>("res://textures/texel_checker.png");
            if (!m_cubeTextureAsset || m_cubeTextureAsset->handle.id == 0)
            {
                auto checkerResource = m_resources.load<TextureResource>("builtin://checker");
                if (checkerResource)
                    m_checkerTexture = checkerResource->handle;
            }

            m_cubeTexture = m_cubeTextureAsset ? m_cubeTextureAsset->handle : Render::TextureHandle{};
            if (m_cubeTexture.id == 0 && m_checkerTexture.id != 0)
            {
                m_cubeTexture = m_checkerTexture;
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
            if (auto* scene = m_sceneManager.currentScene())
                scene->setInputState(&getInput());

            m_sceneManager.update(deltaTime);
        }

        void onRender() override
        {
            if (!m_renderer || !m_renderContext)
                return;

            m_renderContext->beginFrame();
            m_frame.begin();
            if (auto* scene = m_sceneManager.currentScene())
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
            m_cubeMesh = {};
            m_cubeTexture = {};
            m_checkerTexture = {};
            if (m_renderer)
            {
                m_renderer->shutdown();
            }

            m_renderer.reset();
            m_renderContext.reset();
        }

    private:
        std::unique_ptr<RenderContext> m_renderContext;
        std::unique_ptr<IRenderer> m_renderer;
        ResourceManager m_resources;
        std::shared_ptr<ShaderResource> m_vertexShaderSource{};
        std::shared_ptr<ModelResource> m_cubeMeshAsset{};
        std::shared_ptr<TextureResource> m_cubeTextureAsset{};
        Render::MeshHandle m_cubeMesh{};
        Render::TextureHandle m_cubeTexture{};
        Render::TextureHandle m_checkerTexture{};
        SceneManager m_sceneManager{};
        RenderFrame m_frame{};
    };

} // namespace Zenith

int main(int argc, char** argv)
{
    Zenith::Log::Init();
    Zenith::GameApplication app{ Zenith::parseApplicationConfig(argc, argv) };
    app.run();
    return 0;
}
