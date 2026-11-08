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

- Window size (fill cost scales with pixels; use a small window for risky tests)
- Sprite count and sprite size
- Overdraw = total sprite area / window area (1.0 means each pixel drawn once on average)
- Blending on/off, depth test on/off
- Vsync on/off (vsync off for benchmarks, otherwise fps is capped)
- Random seed and sprite movement (same every run)
- Run duration: 30-60 s, then read the graph (not the first seconds)
- Let the GPU cool down between full-load runs

## 4. Results

Frame time is the main number. Record the average and the worst frames, not just fps.
Fields marked (est.) were estimated from fps, not measured.

| Date | PC | Commit | Test | Overdraw | Blend | Depth | Vsync | Avg ms | Worst ms | FPS | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-08 | A | n/a | 10k sprites, 0.1 size, 1 draw call | ~25x | on | on | off | ~8.5 (est.) | n/a | ~117 | 100% | ~33% | 88 C | Baseline. GPU-bound (fill rate). 10 minute run, too long; keep runs short. |
| 2026-11-08 | A | n/a | 10k sprites, small sprites, 1 draw call | low | on | on | off | ~3 | n/a | ~337 | n/a | n/a | n/a | Early run before the overdraw test; sprite count, not size, was the limit. |
| 2026-11-08 | A | n/a | 10k sprites, large sprites, 1 draw call | very high | on | on | off | ~320 | n/a | ~3 | n/a | n/a | n/a | Fill-rate wall. Desktop nearly froze. Do not repeat at this size. |
| 2026-11-08 | B | n/a | 60k Sprites (0.1x) | ~150x | on | on | on | 22.25 | 28.03 | 44.8 | 100% | 14% | 62 C | First test on CachyOS. |


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

