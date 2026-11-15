# BruhEngine TODO

2D engine in C++, SDL2 (2.32.10) + OpenGL 3.3 core + glad.
Target hardware: i5-2320, Radeon HD 5450 (Cedar), 6 GB RAM.

## Baseline (benchmark notes)

- 10,000 sprites, 35 world units (0.1 NDC at 700x700 before commit 2bb33f7), ~25x overdraw, 1 draw call
- Vsync off: ~117 fps, GPU load 100%, CPU ~33%, 88 C after 10 minutes (measured before 2bb33f7)
- Conclusion: GPU-bound (fill rate and blending), not CPU-bound
- Keep stress runs short (30-60 s). Do not run long at 100% GPU load.

## Benchmarking (finish quickly, then move on)

- [x] Save baseline numbers in `docs/benchmarks.md`
- [x] Test overdraw levels (5x, 10x, 20x), blending on and off, note frame time for each
- [x] Add automatic cut-off if a frame exceeds ~100 ms
- [x] Use a small window for future stress tests
- [x] Use the same seed and sprite movement every run so results compare

## Next up (in order)

- [x] 1. Orthographic camera: world units, pan and zoom, correct aspect ratio, viewport update on resize
- [x] Fix startup overdraw calculation to use the camera view rect (not NDC [-1, 1])
- [x] Rename or document `--pixels` as world units (equals screen pixels only at window height 700, zoom 1.0)
- [ ] 2. Culling: skip sprites outside the camera view (overdraw figure must then count visible sprites only)
- [ ] 3. Texture atlas and sprite sheets (add padding to avoid edge bleeding)
- [ ] 4. Input layer: actions instead of raw keys, Escape to quit, gamepad support
- [ ] 5. Render passes: opaque first (blending off, depth on), blended second, sorted by layer and texture

## Later

- [ ] 6. Dear ImGui as a debug-only overlay (count its draw calls separately from sprites)
- [ ] 7. Entity and component storage (plain arrays per component)
- [ ] 8. Collision: AABB, then circles, then a spatial grid
- [ ] 9. Audio with miniaudio
- [ ] 10. Asset manager, tilemaps, animation, scene/level format
- [ ] Optional: GPU timer queries (compare CPU time vs GPU time)
- [ ] Optional: lower-resolution offscreen render target, scaled up

## Housekeeping (anytime)

- [ ] CMake presets for debug and release builds
- [ ] Confirm warnings (-Wall -Wextra) and ASan/UBSan are Debug-only, on own targets only
- [ ] Build and run on the CachyOS PC, note any differences
- [ ] README with build steps for Ubuntu and CachyOS
- [ ] Commit small and often

## Design rules to remember

- Stay on OpenGL 3.3 core (glad generated for 3.3) even though Mesa reports 4.6
- Overdraw is the budget: aim for ~5x-10x average in real scenes
- Draw opaque sprites without blending; blend only what needs it
- No buffer or texture creation in the per-frame path
- Keep SDL calls inside a small platform layer (easier SDL3 migration later)
- Sprite sizes and positions are in world units; the camera converts to pixels. Never mix NDC and world units
