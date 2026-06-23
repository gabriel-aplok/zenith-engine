# Resource System Plan

This file tracks the first engine resource layer before more ECS work.

## Goals

- Keep resource loading separate from GPU upload.
- Keep the API close to Unity and Godot:
  - load by path
  - cache loaded resources
  - return typed handles/references
  - support in-memory and file-backed resources later
- Keep the first pass small and explicit.

## Phase 1: Core Resource Manager

Milestone:

- `ResourceManager`
- typed load cache
- typed resource handles
- loader registration by resource type

Notes:

- The manager should own resource lifetime at the CPU/object level only.
- Rendering systems can consume resources afterward, but should not own loading.

## Phase 2: Builtin and File Sources

Milestone:

- builtin resource registration
- file-backed resource loaders
- explicit resource keys/paths

Notes:

- Keep parsing isolated from cache ownership.
- Prefer simple paths over reflection or editor concepts.

## Phase 3: Runtime Integration

Milestone:

- scene uses resource handles for assets
- render cache consumes CPU resources
- runtime sample uses resource manager directly

Notes:

- This is the bridge point into the ECS/resource split.

## Phase 4: Asset Formats

Milestone:

- mesh source loading
- texture source loading
- baked resource loading
- binary/raw resource loading
- texture asset loading

Notes:

- Source parsing and baking stay below GPU upload.
- GPU upload remains renderer-specific.

## Current Status

- Phase 1 is implemented in a first pass.
- Builtin and file-backed mesh loading now both work through `ResourceManager`.
- Mesh parsing now lands in `MeshSource` first, with `Render::MeshData` treated as the renderer-facing conversion result.
- Resource registration is now centralized in `registerStandardResourceLoaders`.
- Text sources can now be loaded through the same typed cache path.
- Binary sources now use the same path, which is the right base layer for image decoding and other opaque assets.
- Image sources now exist as typed encoded data, ready for a real decoder once one is added.
- Texture assets now wrap encoded image sources as a runtime-facing type.
- Baked mesh assets now exist as a separate runtime-facing type with source provenance.
- The old duplicate `TextAsset` stub was removed so the resource layer only keeps one text-loading concept.
- The next resource step is image/texture-backed asset types, not more ECS work.
