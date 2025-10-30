cmake_minimum_required(VERSION 3.28)

project(zenith-engine
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Zenith Game Engine"
)

# ---- Language Standards ----
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---- vcpkg Toolchain ----
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
else()
    message(FATAL_ERROR "VCPKG_ROOT environment variable not set. Please set it to your vcpkg installation directory.")
endif()

# ---- Find Packages ----
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Jolt CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# ---- Include Directories ----
set(ZENITH_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src # Yea, I will include the header files in the same location as the source code files.
)

# ---- Zenith Core Library ----
add_library(zenith-lib
    "src/engine/window.cpp"
    "src/engine/renderer.cpp"
    "src/engine/shader.cpp"
    "src/engine/application.cpp"
    "src/engine/scene.cpp"
    "src/engine/input_manager.cpp"
)

target_include_directories(zenith-lib
    PUBLIC ${ZENITH_INCLUDE_DIRS}
)

target_link_libraries(zenith-lib
    PUBLIC glad::glad
	PUBLIC glfw
    PUBLIC glm::glm
	PUBLIC imgui::imgui
	PUBLIC Jolt::Jolt
	PUBLIC lodepng
	PUBLIC nlohmann_json::nlohmann_json
    PUBLIC spdlog::spdlog
)

target_compile_definitions(zenith-lib PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_features(zenith-lib PUBLIC cxx_std_23)

# ---- Helper: Copy shaders to output directory ----
function(copy_shaders TARGET_NAME)
    set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shader")
    set(SHADER_DEST_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>/shader")

    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${SHADER_SOURCE_DIR}"
                "${SHADER_DEST_DIR}"
        COMMENT "Copying shaders to ${SHADER_DEST_DIR}"
        VERBATIM
    )
endfunction()

# ---- Zenith Editor Executable ----
add_executable(zenith-editor "src/editor/editor.cpp")

target_include_directories(zenith-editor PRIVATE ${ZENITH_INCLUDE_DIRS} )
target_link_libraries(zenith-editor PRIVATE zenith-lib)
copy_shaders(zenith-editor)

# ---- Zenith Game Executable ----
add_executable(zenith-game "src/runtime/game.cpp")

target_include_directories(zenith-game PRIVATE ${ZENITH_INCLUDE_DIRS} )
target_link_libraries(zenith-game PRIVATE zenith-lib)
copy_shaders(zenith-game)
