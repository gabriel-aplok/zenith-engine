# ECS / Scene Plan

This file tracks the next layer after the backend-agnostic renderer:
`Scene`, `GameObject`, `Transform`, and a small Unity-style component model.

## Goals

- Keep the runtime simple.
- Keep rendering backend-agnostic.
- Keep the first pass readable and explicit.
- Avoid a full ECS framework until the engine proves the shape.

## Phase 1: Core Scene Model

Milestone:

- `Scene`
- `GameObject`
- `Transform`
- `Component`
- `MeshFilter`
- `MeshRenderer`

Rules:

- `Scene` owns `GameObject` instances.
- `GameObject` owns components.
- Every `GameObject` has one automatic `Transform`.
- `MeshFilter` stores a render mesh handle.
- `MeshRenderer` submits render intent through the existing render submission layer.

## Phase 2: Scene Hierarchy

Milestone:

- Parent / child transforms
- Local vs world transform propagation
- Optional reparenting without breaking component ownership

Notes:

- Keep this strictly transform-level.
- Do not add physics or editor features here.

## Phase 3: Camera and Visibility

Milestone:

- `Camera` component
- Scene render pass selection
- Frustum culling hooks
- View / projection data coming from scene state instead of runtime glue

Notes:

- This should still submit into the same render command layer.
- Do not couple the scene to bgfx.

## Phase 4: Shared Data and Asset Hooks

Milestone:

- Mesh references from source/baked assets
- Material data abstraction
- Optional shared component data caches

Notes:

- Keep source parsing, baking, and GPU upload separate.
- The scene should only hold runtime references.

## Phase 5: Behaviour and Input

Milestone:

- `ScriptBehaviour`
- Input-driven component updates
- Simple lifecycle hooks

Notes:

- No reflection.
- No editor serialization yet.

## Phase 6: Physics Primitives

Milestone:

- `Rigidbody`
- `Collider`
- Transform synchronization

Notes:

- Keep physics optional and isolated.
- No tight renderer coupling.

## Phase 7: Serialization and Editor Prep

Milestone:

- Save/load scene graph
- Stable identifiers for objects/components
- Editor-facing metadata only if needed later

Notes:

- Do not add editor workflows until the runtime model is stable.

## Current Status

- Phase 1 is implemented in a first pass.
- The current runtime uses one scene with a mesh-driven GameObject.
- The next prerequisite is a basic resource system so future ECS work can load assets through one path.
