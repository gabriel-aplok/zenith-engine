# Zenith Engine

Zenith is being reset into a code-first engine with explicit backend abstractions for application flow, windowing, input, and rendering.

The current repository state is intentionally transitional:

- `Application`, `Window`, and `InputState` are moving toward backend-neutral APIs.
- GLFW is currently the default window backend.
- bgfx now handles rendering through `RenderContext` and `IRenderer` implementations under `src/render`.
- the runtime renderer is a minimal clear-screen path for now.
- `Engine` was removed and render orchestration now lives directly in the runtime application layer.
- `BakedMeshAsset` is the CPU-side mesh payload used across asset loading and future upload paths.
- baking and baked-asset loading stay under `src/asset`.

## Current Goal

The goal of this reset is to make the engine core independent from:

- GLFW
- backend-specific bootstrap
- any single native event system
- any renderer-specific bootstrap path

## Rebuild Document

The clean-slate rebuild plan lives here:

- [docs/backend-agnostic-rebuild.md](docs/backend-agnostic-rebuild.md)

That document defines:

- target layering
- core interfaces
- backend boundaries
- rebuild phases
- cleanup rules

## Build

Requirements:

- Visual Studio 2026 or newer with C++ tooling
- CMake 3.28+
- `VCPKG_ROOT` configured

Configure:

```powershell
cmake --preset windows-debug
```

Build:

```powershell
cmake --build --preset windows-debug
```

Run:

```powershell
.\out\build\windows-debug\Debug\zenith-game.exe
```

VS Code `F5` is configured to build and launch `zenith-game`.

## Current Dependencies

- `bgfx`
- `glfw3`
- `glm`
- `tinyobjloader`

## Status

This repository should now be treated as a reset-in-progress, not as a finished engine architecture.

Recent completed reset steps:

- backend-neutral `Application`, `Window`, and `InputState`
- GLFW backend isolation under `src/platform`
- `IRenderer` and `RenderContext` split under `src/render`
- bgfx renderer swap from the old graphics backend path
- `Engine` removal after it became redundant
- separation of asset baking from baked-asset loading
