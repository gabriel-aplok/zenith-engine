cmake_minimum_required(VERSION 3.28)

project(zenith-engine
	VERSION 0.1.0
	DESCRIPTION "Zenith Game Engine"
	LANGUAGES
		C
		CXX
)

# ============================================================
# Global project settings
# ============================================================

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ============================================================
# Dependencies
# ============================================================

find_package(bgfx CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(RGFW_INCLUDE_DIRS "RGFW.h")

set(ZENITH_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# headers are currently colocated with source files.
set(ZENITH_INCLUDE_DIRS
	"${ZENITH_SOURCE_DIR}"
	"${RGFW_INCLUDE_DIRS}"
	${Stb_INCLUDE_DIR}
)

set(ZENITH_SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders")
set(ZENITH_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(ZENITH_GENERATED_SHADER_DIR "${ZENITH_GENERATED_DIR}/render/shaders")

set(ZENITH_SHADER_OUTPUTS
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_vs_dx11.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_fs_dx11.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_vs_glsl430.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_fs_glsl430.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_vs_gles300.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_fs_gles300.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_vs_spirv.bin.h"
	"${ZENITH_GENERATED_SHADER_DIR}/mesh_fs_spirv.bin.h"
)

add_custom_command(
	OUTPUT ${ZENITH_SHADER_OUTPUTS}
	COMMAND ${CMAKE_COMMAND} -E make_directory "${ZENITH_GENERATED_SHADER_DIR}"
	COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -File
		"${CMAKE_CURRENT_SOURCE_DIR}/tools/build_bgfx_shaders.ps1" -ShaderDir
		"${ZENITH_SHADER_SOURCE_DIR}" -OutputDir "${ZENITH_GENERATED_SHADER_DIR}"
	DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/tools/build_bgfx_shaders.ps1"
		"${ZENITH_SHADER_SOURCE_DIR}/mesh_vs.sc"
		"${ZENITH_SHADER_SOURCE_DIR}/mesh_fs.sc"
		"${ZENITH_SHADER_SOURCE_DIR}/varying.def.sc"
	VERBATIM
)

add_custom_target(bgfx-shaders DEPENDS ${ZENITH_SHADER_OUTPUTS})

function(zenith_configure_app TARGET_NAME)
	target_include_directories(${TARGET_NAME}
		PRIVATE ${ZENITH_INCLUDE_DIRS}
	)

	target_link_libraries(${TARGET_NAME}
		PRIVATE zenith-lib
	)

	set_target_properties(${TARGET_NAME}
		PROPERTIES FOLDER "Zenith/Apps"
	)

	add_custom_command(
		TARGET ${TARGET_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${CMAKE_CURRENT_SOURCE_DIR}/resources"
			"$<TARGET_FILE_DIR:${TARGET_NAME}>/resources"
		COMMENT "Copying resources for ${TARGET_NAME}"
	)
endfunction()

# ============================================================
# Zenith Core Library
# ============================================================

add_library(zenith-lib
	"src/engine/window.cpp"
	"src/engine/startup.cpp"
	"src/platform/rgfw_window_backend.cpp"
	"src/engine/application.cpp"
	"src/render/bgfx_debug.cpp"
	"src/render/bgfx_render_context.cpp"
	"src/render/bgfx_renderer.cpp"
	"src/render/render_commands.cpp"
	"src/render/mesh_cache.cpp"
	"src/render/texture_cache.cpp"
	"src/render/mesh_builder.cpp"
	"src/log/log.cpp"
	"src/engine/input.cpp"
	"src/resource/resource_loaders.cpp"
	"src/resource/obj_mesh_loader.cpp"
	"src/scene/component.cpp"
	"src/scene/game_object.cpp"
	"src/scene/transform.cpp"
	"src/scene/scene.cpp"
	"src/scene/scene_manager.cpp"
	"src/scene/system_registry.cpp"
	"src/components/camera.cpp"
	"src/components/mesh_filter.cpp"
	"src/components/mesh_renderer.cpp"
	"src/components/script_component.cpp"
	"src/systems/script_system.cpp"
	"src/systems/render_system.cpp"
	"src/systems/transform_system.cpp"
	"src/systems/camera_system.cpp"
)

target_include_directories(zenith-lib
	PUBLIC ${ZENITH_INCLUDE_DIRS}
	PRIVATE "${ZENITH_GENERATED_DIR}"
)

target_link_libraries(zenith-lib
	PUBLIC
		glm::glm
		bgfx::bx
		bgfx::bgfx
		bgfx::bimg
		bgfx::bimg_decode
)

add_dependencies(zenith-lib bgfx-shaders)

set_target_properties(zenith-lib
	PROPERTIES
		FOLDER "Zenith/Core"
		OUTPUT_NAME "zenith"
)

# ============================================================
# Zenith Game Runtime
# ============================================================

add_executable(zenith-game
	"src/runtime/game.cpp"
)

zenith_configure_app(zenith-game)
