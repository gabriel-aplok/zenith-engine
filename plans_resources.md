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

Notes:

- Source parsing and baking stay below GPU upload.
- GPU upload remains renderer-specific.

## Current Status

- Phase 1 is implemented in a first pass.
- Builtin and file-backed mesh loading now both work through `ResourceManager`.
- The next resource step is texture/file-backed asset types, not more ECS work.
