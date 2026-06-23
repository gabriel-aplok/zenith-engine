# Backend-Agnostic Rebuild Plan

This repository is being reset toward a code-first engine core with explicit abstractions between:

- application lifecycle
- windowing and event pumping
- input collection
- graphics backend bootstrap
- renderer implementation

The goal is to keep the engine core independent from GLFW, bgfx internals, or any single platform API.

## 1. Principles

1. The engine layer must not include backend headers such as `GLFW/glfw3.h` or `bgfx/bgfx.h`.
2. Platform code lives behind interfaces in `src/platform`.
3. Renderer code lives behind interfaces in `src/render` or an equivalent backend-facing module.
4. The application loop owns orchestration only.
5. Input is an engine-owned state object updated by platform events.
6. All backend-specific bootstrap must happen in backend implementations, not in `Application`.
7. The runtime target should stay runnable at every phase, even if features are stubbed.

## 2. Target Layering

Recommended structure:

```text
src/
  core/
    application.*
    time.*
    log.*
    service_locator.*          # optional
  platform/
    window_backend.hpp
    input_codes.hpp
    glfw/
      glfw_window_backend.*
  render/
    irenderer.hpp
    render_context.hpp
    bgfx_renderer.*
    bgfx_render_context.*
  runtime/
    game.cpp
```

Current repo already has the beginning of this split for `Application`, `Window`, `WindowBackend`, `InputState`, `RenderContext`, and `IRenderer`. The renderer implementation is now a minimal bgfx clear path.

## 3. Core Interfaces

### 3.1 Application

`Application` should only:

- create services from config
- advance time
- begin input frame
- poll window events
- call `onUpdate`
- call `onRender`
- request present
- react to high-level window events

`Application` should not:

- call backend APIs directly
- know how contexts are created
- know how swapchains/framebuffers are managed

### 3.2 Window Backend

`WindowBackend` owns:

- native window creation
- OS event pumping
- cursor mode
- native handle access
- backend-specific present behavior

`Window` is only a thin engine wrapper around `WindowBackend` plus engine-owned `InputState`.

### 3.3 Input

`InputState` should remain backend-neutral and contain:

- key/button states
- mouse position and delta
- scroll delta
- focus state

Keep input codes engine-owned. Backend code maps native codes into engine enums.

### 3.4 Render Context

Next abstraction to add:

```cpp
class RenderContext {
public:
    virtual ~RenderContext() = default;
    virtual GraphicsApi api() const = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void resize(int width, int height) = 0;
};
```

This separates context/bootstrap from the game-facing renderer.

### 3.5 Renderer

Recommended shape:

```cpp
class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual void initialize(const RendererConfig&) = 0;
    virtual void shutdown() = 0;
    virtual void resize(int width, int height) = 0;
    virtual void render(const RenderFrame&) = 0;
};
```

bgfx then becomes one implementation, not the engine definition of rendering.

## 4. Rebuild Order

Use this order to redo the engine cleanly.

### Phase 1: Stable Core

Keep or build:

- `Application`
- `Window`
- `WindowBackend`
- `InputState`
- logging
- basic timing

Exit criteria:

- window opens
- close event works
- resize events work
- mouse and keyboard states update correctly
- no engine header includes GLFW directly

### Phase 2: Render Bootstrap Split

Add:

- `RenderContext`
- `bgfx` context backend
- `Renderer` interface

Move from window/backend into render context:

- bgfx initialization
- platform handle bootstrap
- default clear/view setup

Exit criteria:

- `Application` only asks for a render context and renderer
- GLFW backend creates a window but does not define rendering policy

### Phase 3: Minimal Renderer

Rebuild a minimal renderer first:

- clear screen
- viewport resize
- basic shader compilation
- single triangle or mesh draw

Do not port the full GPU-driven renderer immediately. Rebuild from the smallest verified unit.

Exit criteria:

- one render pass
- explicit resize handling
- renderer independent from scene complexity

### Phase 4: Submission API

Introduce a neutral frame submission layer:

- mesh handles
- per-instance transforms
- per-instance material or color data
- flat draw submission lists

Do not rebuild a large scene graph until the renderer actually needs one. Grow from flat submission data first.

### Phase 5: Advanced Renderer Recovery

Only after the simple path is solid:

- reintroduce culling
- reintroduce indirect draw command generation
- reintroduce transparency sorting
- reintroduce Hi-Z and depth pre-pass
- reintroduce profiling and debug overlays

## 5. Cleanup Rules

When resetting files, follow these rules:

1. Delete stale code instead of leaving unused branches.
2. Keep interfaces small and explicit.
3. Do not let `core` depend on `platform` implementation details.
4. Do not let submission data models depend on backend internals.
5. Prefer compile-safe stubs over large unfinished systems.
6. Rebuild features incrementally behind interfaces.

## 6. Current Repository Guidance

Completed cleanup passes:

1. Rendering files were moved out of `src/engine` into a dedicated render module.
2. `IRenderer` and `RenderContext` were introduced.
3. The renderer was switched to bgfx.
4. `Engine` was removed after becoming redundant with `Application` plus render abstractions.
5. The old asset pipeline was removed from the codebase.
6. Shader sources now live under `resources/shaders`, with generated headers emitted into the build tree.
7. `RenderMeshCache` now sits between CPU mesh data and GPU upload so the renderer only exposes an uploader interface.
8. `RenderCommandList` is the stable frame submission buffer and already supports multiple `DrawIndexed` entries.

Recommended next cleanup passes:

1. Keep CPU-side mesh generation and file loading separate from GPU upload.
2. Add batching or instancing only if the renderer needs it.
3. Move any remaining generic engine primitives into narrower modules if they stop being runtime-facing.
4. Rewrite `README.md` whenever the architecture changes so it matches the code.

## 7. Definition of Done

This reset is complete when:

- engine-facing headers are backend-neutral
- backend implementations are isolated under `src/platform` and backend-specific render folders
- the runtime builds and runs with one backend
- adding a second backend does not require changing `Application`, `Window`, or `InputState`
- the simple renderer path is stable before advanced rendering features return
