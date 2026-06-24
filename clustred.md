Use a clustered forward renderer with explicit passes.

**Target Architecture**

- `Scene` builds visible draw data.
- `RenderSystem` produces one or more `RenderView`s.
- `RenderGraph` or `FrameGraph` schedules passes.
- `IRenderer` executes the graph, not individual scene logic.
- bgfx stays the backend, but it only sees pass execution.

**Why this is the best fit**

- Better than pure forward for many dynamic lights.
- Lighter than full deferred for open-world scenes.
- Shadows stay separate and budgeted.
- Post-process becomes additive, not structural.

**What to build first**

1. `RenderFrame`

- Holds camera/view state.
- Holds visible draw list.
- Holds light list.
- Holds pass requests.

2. `RenderPass`

- Each pass declares:
    - inputs
    - outputs
    - clear/load/store behavior
    - whether it is compute or raster
- Example passes:
    - `ShadowMapPass`
    - `ClusterBuildPass`
    - `LightCullingPass`
    - `SceneLightingPass`
    - `PostProcessTonemapPass`
    - `DebugOverlayPass`

3. `RenderTarget`

- Abstract color/depth textures and framebuffer attachments.
- Needed for:
    - shadow maps
    - G-buffer if you ever need it
    - bloom chain
    - temporal effects

4. `LightData`

- Keep light data CPU-side in a flat array.
- Each frame, upload only active lights.
- Clustered lighting uses this to cull per screen cluster.

5. `ShadowBudget`

- A policy object that decides:
    - which lights cast shadows
    - resolution per light
    - update frequency
    - cascade count for directional light
- This is critical for open-world performance.

**Recommended render flow**

- Pass 1: depth or shadow rendering
- Pass 2: cluster generation
- Pass 3: light culling into clusters
- Pass 4: opaque scene lighting
- Pass 5: transparents
- Pass 6: optional post-process
- Pass 7: UI/debug overlay

**What to avoid**

- Do not port the full `16-shadowmaps` sample structure.
- Do not make post-process a special case inside `BgfxRenderer::render()`.
- Do not default to deferred rendering unless the game truly needs many complex materials and lights per pixel.

**How `Cluster` maps to Zenith**

- `ClusteredRenderer` in the sample is the right model for lighting scalability.
- The important idea is the compute-driven cluster/light grid, not its app framework.
- For Zenith, that becomes:
    - cluster build compute pass
    - light culling compute pass
    - forward clustered fragment shader

**How `16-shadowmaps` maps to Zenith**

- Use it for:
    - cascaded shadow maps
    - shadow atlas layout ideas
    - PCF/VSM/ESM reference
- Do not use it as the engine’s render architecture.

**Practical recommendation for Unturned-like games**

- Default renderer: clustered forward
- Shadowing:
    - one cascaded directional shadow system
    - a small number of important local shadows
- Post-process:
    - tone mapping
    - exposure
    - optional bloom/fog
- World scaling:
    - chunk streaming
    - aggressive frustum culling
    - LODs
    - object pooling
    - visibility/occlusion later

**If I were implementing Zenith next**

- First: introduce `RenderGraph` + `RenderTargetHandle`.
- Second: move scene drawing into a `SceneLightingPass`.
- Third: add clustered light culling.
- Fourth: add cascaded directional shadows.
- Fifth: add post-process as optional passes.

If you want, I can sketch the exact C++ interfaces for:

- `RenderGraph`
- `RenderPass`
- `RenderTargetHandle`
- `LightData`
- `ShadowBudget`

and show how they fit into your current `IRenderer` and `RenderFrame`.
