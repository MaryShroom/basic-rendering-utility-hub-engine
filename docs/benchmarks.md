# BruhEngine Benchmarks

Lab notebook for performance tests. Append new results, do not overwrite old ones.
Only compare runs that share the same window size, overdraw, blending and depth settings.

## 1. Test machines

### PC-A (Ubuntu)

- CPU: Intel Core i5-2320 @ 3.00 GHz (4 cores, no hyperthreading)
- GPU: AMD Radeon HD 5450 (Cedar), MSI, open-source `radeon`/r600 driver
- RAM: 6 GB
- OS: Ubuntu 24.04
- Kernel: 7.0.0-42-generic
- Mesa: 25.2.8 (reports OpenGL 4.6 core, GLSL 4.60; the engine targets 3.3 core)
- Compiler: g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0

### PC-B (CachyOS)

- CPU: AMD Ryzen 7 5700X3D (16) @ 4.15 GHz
- GPU: AMD Radeon RX 5700 XT [Discrete]
- RAM: 32 GB
- OS: CachyOS
- Mesa / driver: Mesa 26.2.4-arch3.1 (OpenGL 4.6 Core Profile)
- Compiler: g++ (GCC) 16.2.1 20260810

## 2. Build info (record for every result)

- Git commit hash
- Build type: Release for real benchmarks (Debug with ASan skews everything)
- SDL version: 2.32.10
- GL loader: glad (generated for 3.3 core)

## 3. Test setup (keep identical between comparable runs)

- Window size (fill cost scales with pixels; use a small window for risky tests) 700 by 700 resolutions
- Sprite count and sprite size
- Overdraw = total sprite area / window area (1.0 means each pixel drawn once on average)
- Gpix/s (derived) = overdraw x window width x window height x fps / 1e9
  - Lets you compare machines fairly. Only meaningful for fill-bound runs (GPU at 100%, frame time grows with overdraw).
  - Only as accurate as the overdraw figure; print the exact overdraw from code instead of estimating.
  - Counts pixels submitted, not pixels actually shaded (depth-rejected and discarded pixels still count).
  - Do not compare blended runs with opaque runs, or runs with different shaders or texture sizes.
  - Record the window size for every run; without it Gpix/s cannot be computed.
- Blending on/off, depth test on/off
- Vsync on/off (vsync off for benchmarks, otherwise fps is capped)
- Random seed and sprite movement (same every run)
- Run duration: 30-60 s, then read the graph (not the first seconds)
- Let the GPU cool down between full-load runs

## 4. Results

Frame time is the main number. Record the average and the worst frames, not just fps.
Fields marked (est.) were estimated from fps, not measured.

Window and Gpix/s are new columns. Gpix/s is derived (see section 3), not measured.
Runs from before these columns existed show n/a.

| Date | PC | Commit | Test | Window | Overdraw | Blend | Depth | Vsync | Avg ms | Worst ms | FPS | Gpix/s (derived) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-08 | A | n/a | 10k sprites, 0.1 size, 1 draw call | n/a | ~25x | on | on | off | ~8.5 (est.) | n/a | ~117 | n/a | 100% | ~33% | 88 C | Baseline. GPU-bound (fill rate). 10 minute run, too long; keep runs short. |
| 2026-11-08 | A | n/a | 10k sprites, small sprites, 1 draw call | n/a | low | on | on | off | ~3 | n/a | ~337 | n/a | n/a | n/a | n/a | Early run before the overdraw test; sprite count, not size, was the limit. |
| 2026-11-08 | A | n/a | 10k sprites, large sprites, 1 draw call | n/a | very high | on | on | off | ~320 | n/a | ~3 | n/a | n/a | n/a | n/a | Fill-rate wall. Desktop nearly froze. Do not repeat at this size. |
| 2026-11-08 | B | n/a | 60k Sprites (0.1x) | n/a | ~150x | on | on | on | 22.25 | 28.03 | 44.8 | n/a | 100% | 14% | 62 C | First test on CachyOS. Window size not recorded. |
| 2026-11-08 | B | 4dbb38d | 2k Sprites (0.1x) | 700x700 | ~5x | on | on | off | 1.33 | 8.08 | 749.5 | 1.836 | 100% | 11% | 60 C | 2000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. |
| 2026-11-08 | B | 4dbb38d | 4k Sprites (0.1x) | 700x700 | ~10x | on | on | off | 2.40 | 8.58 | 416.9 | 2.043 | 100% | 14% | 61 C | 4000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. |
| 2026-11-08 | B | 4dbb38d | 8k Sprites (0.1x) | 700x700 | ~20x | on | on | off | 4.38 | 10.63 | 228.3 | 2.237 | 100% | 17% | 60 C | 8000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. |
| 2026-11-08 | B | 4dbb38d | 10k Sprites (0.1x) | 700x700 | ~25x | on | on | off | 5.37 | 11.83 | 186.0 | 2.279 | 100% | 18% | 60 C | 10000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. |
| 2026-11-08 | B | 4dbb38d | 2k Sprites (0.1x), Blend off | 700x700 | ~5x | off | on | off | 1.17 | 7.67 | 854.9 | 2.094 | 100% | 12% | 60 C | 2000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. |
| 2026-11-08 | B | 4dbb38d | 4k Sprites (0.1x), Blend off | 700x700 | ~10x | off | on | off | 2.38 | 9.03 | 419.4 | 2.055 | 100% | 15% | 60 C | 4000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. |
| 2026-11-08 | A | n/a | 8k Sprites (0.1x), Blend off | 700x700 | ~20x | off | on | off | 4.45 | 10.57 | 224.6 | 2.201 | 100% | 17% | 60 C | 8000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. |
| 2026-11-08 | A | n/a | 10k Sprites (0.1x), Blend off | 700x700 | ~25x | off | on | off | 5.38 | 11.57 | 185.9 | 2.277 | 100% | 17% | 60 C | 10000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. |

## 5. Planned runs

- [ ] Overdraw 5x, 10x, 20x with blending on
- [ ] Same overdraw levels with blending off
- [ ] Opaque sprites with depth testing, drawn front to back
- [ ] Re-run baseline after: camera, culling, atlas, render passes
- [ ] Run the same tests on PC-B
- [ ] CPU-side stage timings (update, buffer fill, swap)
- [ ] Optional: GPU timer queries

## 6. Findings

- 10k sprites in one draw call is fine on this GPU; the batcher works.
- Cost is dominated by pixels shaded and blended (overdraw), not by sprite count.
- CPU load stays low (~33%) while the GPU is at 100%, so the engine is GPU-bound.
- Practical budget: aim for roughly 5x-10x average overdraw in real scenes; measure the real ceiling with the planned runs.
- 88 C under sustained load is hot. Keep stress runs short.

## 7. How to reproduce

- Build: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release` then `cmake --build build-release`
- Run with Mesa's overlay: `GALLIUM_HUD="fps,frametime" ./build-release/<executable>`
  (use `GALLIUM_HUD=help` to list the counters your driver exposes)
- Stress test controls: <fill in: keys for sprite count, sprite size, blending, vsync toggle, reset>
- Note: `GALLIUM_HUD` only works with Mesa Gallium drivers. It also costs a little frame time, so compare with it off for final numbers.

## 8. Rules of thumb

- Append rows, never edit old ones. Re-run the baseline after big engine changes to catch regressions.
- Change one variable at a time.
- If frame time grows with sprite count but not with size: CPU-bound. If it grows with size and overdraw: GPU fill-rate-bound.
- Stop a test as soon as frame time passes ~100 ms.

