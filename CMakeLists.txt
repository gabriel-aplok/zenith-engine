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

find_package(glfw3 CONFIG REQUIRED)
find_package(bgfx CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set(ZENITH_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# headers are currently colocated with source files.
set(ZENITH_INCLUDE_DIRS
	"${ZENITH_SOURCE_DIR}"
)

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
endfunction()

# ============================================================
# Zenith Core Library
# ============================================================

add_library(zenith-lib
	"src/engine/window.cpp"
	"src/engine/startup.cpp"
	"src/platform/glfw_window_backend.cpp"
	"src/engine/application.cpp"
	"src/render/bgfx_debug.cpp"
	"src/render/bgfx_render_context.cpp"
	"src/render/bgfx_renderer.cpp"
	"src/log/log.cpp"
	"src/engine/input.cpp"
)

target_include_directories(zenith-lib
	PUBLIC ${ZENITH_INCLUDE_DIRS}
)

target_link_libraries(zenith-lib
	PUBLIC
		glfw
		glm::glm
		bgfx::bx
		bgfx::bgfx
		bgfx::bimg
		bgfx::bimg_decode
)

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
