#include "systems/camera_system.hpp"

#include "components/camera.hpp"
#include "scene/scene.hpp"

namespace Zenith
{
    namespace
    {
        Render::RenderClearState buildClearState(const Components::Camera &camera)
        {
            Render::RenderClearState clear{};
            clear.color = camera.backgroundColor();

            switch (camera.clearFlags())
            {
            case Components::Camera::ClearFlags::Skybox:
            case Components::Camera::ClearFlags::SolidColor:
                clear.flags = 0x01 | 0x02;
                break;
            case Components::Camera::ClearFlags::DepthOnly:
                clear.flags = 0x02;
                break;
            case Components::Camera::ClearFlags::Nothing:
            default:
                clear.flags = 0x00;
                break;
            }

            return clear;
        }
    } // namespace

    void CameraSystem::render(Scene &scene, RenderFrame &frame)
    {
        auto *camera = scene.findCamera();
        if (!camera)
        {
            return;
        }

        for (auto &object : scene.gameObjects())
        {
            auto *objectCamera = object->scene() ? object->scene()->tryGetComponent<Components::Camera>(*object) : object->get_component<Components::Camera>();
            if (objectCamera == camera)
            {
                frame.clear = buildClearState(*camera);
                const glm::mat4 cameraWorld = object->transform().localToWorld();
                camera->buildViewState(cameraWorld, scene.framebufferSize(), frame.view);
                frame.commands.setView(frame.view);
                return;
            }
        }
    }
} // namespace Zenith
