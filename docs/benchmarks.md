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

### PC-A Results
| Date | PC | Commit | Test | Window | Overdraw | Blend | Depth | Vsync | Avg ms | Worst ms | FPS | Gpix/s (derived) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-08 | A | n/a | 10k sprites, 0.1 size, 1 draw call | n/a | ~25x | on | on | off | ~8.5 (est.) | n/a | ~117 | n/a | 100% | ~33% | 88 C | Baseline. GPU-bound (fill rate). 10 minute run, too long; keep runs short. |
| 2026-11-08 | A | n/a | 10k sprites, small sprites, 1 draw call | n/a | low | on | on | off | ~3 | n/a | ~337 | n/a | n/a | n/a | n/a | Early run before the overdraw test; sprite count, not size, was the limit. |
| 2026-11-08 | A | n/a | 10k sprites, large sprites, 1 draw call | n/a | very high | on | on | off | ~320 | n/a | ~3 | n/a | n/a | n/a | n/a | Fill-rate wall. Desktop nearly froze. Do not repeat at this size. |

### PC-B Results
| Date | PC | Commit | Test | Window | Overdraw | Blend | Depth | Vsync | Avg ms | Worst ms | FPS | Gpix/s (derived) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-08 | B | n/a | 60k Sprites (0.1x) | n/a | ~150x | on | on | on | 22.25 | 28.03 | 44.8 | n/a | 100% | 14% | 62 C | First test on CachyOS. Window size not recorded. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 2k Sprites (0.1x) | 700x700 | ~5x | on | on | off | 1.33 | 8.08 | 749.5 | 1.836 | 100% | 11% | 60 C | 2000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 4k Sprites (0.1x) | 700x700 | ~10x | on | on | off | 2.40 | 8.58 | 416.9 | 2.043 | 100% | 14% | 61 C | 4000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 8k Sprites (0.1x) | 700x700 | ~20x | on | on | off | 4.38 | 10.63 | 228.3 | 2.237 | 100% | 17% | 60 C | 8000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 10k Sprites (0.1x) | 700x700 | ~25x | on | on | off | 5.37 | 11.83 | 186.0 | 2.279 | 100% | 18% | 60 C | 10000 particles, with particle size of 0.1 by 0.1 in NDC space. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 2k Sprites (0.1x), Blend off | 700x700 | ~5x | off | on | off | 1.17 | 7.67 | 854.9 | 2.094 | 100% | 12% | 60 C | 2000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 4k Sprites (0.1x), Blend off | 700x700 | ~10x | off | on | off | 2.38 | 9.03 | 419.4 | 2.055 | 100% | 15% | 60 C | 4000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 8k Sprites (0.1x), Blend off | 700x700 | ~20x | off | on | off | 4.45 | 10.57 | 224.6 | 2.201 | 100% | 17% | 60 C | 8000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-08 | B | 4dbb38d | 10k Sprites (0.1x), Blend off | 700x700 | ~25x | off | on | off | 5.38 | 11.57 | 185.9 | 2.277 | 100% | 17% | 60 C | 10000 particles, with particle size of 0.1 by 0.1 in NDC space. Blend off. 1 minute run. (Invalid run due to misread GPU load) |
| 2026-11-09 | B | c157439 | 100000 Sprites (1.0x) | 700x700 | ~25000x | on | on | off | 20.55 | 24.36 | 47.9 | 586.694 | 1% | 24% | 73 C | 100k particles. Particle size of 1.0 by 1.0 in NDC space. Blend On. 1 minute run. Release build. Baseline for PC-B. |
| 2026-11-09 | B | e615b36 | 2000 Sprites (0.1x) | 700x700 | ~5x | on | on | off | 0.87 | 2.82 | 1144.6 | 2.804 | 14% | 19% | 61 C | 2k particles. Particle size of 0.1 by 0.1 in NDC space. Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 4000 Sprites (0.1x) | 700x700 | ~10x | on | on | off | 1.59 | 3.67 | 626.3 | 3.069 | 11% | 23% | 61 C | 4k particles. Particle size of 0.1 by 0.1 in NDC space. Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 8000 Sprites (0.1x) | 700x700 | ~20x | on | on | off | 3.04 | 5.30 | 326.6 | 3.201 | 13% | 26% | 61 C | 8k particles. Particle size of 0.1 by 0.1 in NDC space. Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 2000 Sprites (0.1x) | 700x700 | ~5x | off | on | off | 0.87 | 2.78 | 1131.6 | 2.772 | 12% | 19% | 61 C | 2k particles. Particle size of 0.1 by 0.1 in NDC space. Blend Off. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 4000 Sprites (0.1x) | 700x700 | ~10x | off | on | off | 1.58 | 3.63 | 629.5 | 3.085 | 13% | 22% | 61 C | 4k particles. Particle size of 0.1 by 0.1 in NDC space. Blend Off. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 8000 Sprites (0.1x) | 700x700 | ~20x | off | on | off | 3.03 | 5.34 | 328.6 | 3.220 | 12% | 23% | 62 C | 8k particles. Particle size of 0.1 by 0.1 in NDC space. Blend Off. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (0.1x) | 700x700 | ~25x | on | on | off | 3.76 | 6.17 | 262.2 | 3.212 | 11% | 24% | 61 C | 10k particles. Particle size of 0.1 by 0.1 in NDC space. Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (0.25x) | 700x700 | ~156x | on | on | off | 3.76 | 6.17 | 264.6 | 20.260 | 10% | 26% | 62 C | 10k particles. Particle size of 0.25 by 0.25 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (0.5x) | 700x700 | ~625x | on | on | off | 3.77 | 6.70 | 263.5 | 80.711 | 15% | 28% | 62 C | 10k particles. Particle size of 0.5 by 0.5 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (1.0x) | 700x700 | ~2500x | on | on | off | 3.76 | 6.63 | 262.5 | 321.528 | 50% | 22% | 65 C | 10k particles. Particle size of 1.0 by 1.0 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (2.0x) | 700x700 | ~10000x | on | on | off | 3.78 | 7.39 | 261.8 | 1282.677 | 77% | 25% | 73 C | 10k particles. Particle size of 2.0 by 2.0 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (4.0x) | 700x700 | ~40000x | on | on | off | 3.76 | 8.67 | 264.6 | 5185.417 | 80% | 26% | 75 C | 10k particles. Particle size of 4.0 by 4.0 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build. |
| 2026-11-09 | B | e615b36 | 10000 Sprites (8.0x) | 700x700 | ~160000x | on | on | off | 3.95 | 6.91 | 250.4 | 19632.299 | 74% | 20% | 73 C | 10k particles. Particle size of 8.0 by 8.0 in NDC space (fixed count, varying size). Blend On. 1 minute run. Release build. |

## 5. Planned runs

- [ ] Overdraw 5x, 10x, 20x with blending on (PC-B done)
- [ ] Same overdraw levels with blending off (PC-B done)
- [ ] Opaque sprites with depth testing, drawn front to back
- [ ] Re-run baseline after: camera, culling, atlas, render passes
- [ ] Run the same tests on PC-B
- [ ] CPU-side stage timings (update, buffer fill, swap)
- [ ] Optional: GPU timer queries
- [ ] Sweep 1 (fixed count, varying size) on PC-B (done), then PC-A
- [ ] Sweep 2 (fixed overdraw, varying count) on PC-B, then PC-A

### Test design: per-sprite cost vs per-pixel cost

Why: the runs so far change sprite count and overdraw together (same sprite size), so they
cannot say whether frame time is limited by sprites (vertex work, buffer fill, driver
overhead) or by pixels (fill rate, blending). These two sweeps separate the two.

Fixed settings for every run in both sweeps:

- Window 700x700, same random seed and sprite movement, vsync off
- Release build, same commit on both PCs (record the hash in every row)
- 60 s per run; exclude a warm-up of about 2 s from the worst-frame figure
- Record the exact overdraw printed from code, not an estimate
- Record Blend and Depth state explicitly (blend on unless stated)
- Label rows `S1-<size>` and `S2-<count>` in the Test column so sweeps are easy to find
- Let the GPU cool between runs; keep the ~100 ms cut-off active

Overdraw = count x (size / 2)^2, with size in NDC units and the window spanning 2 units.

**Sweep 1: fixed count, varying size** (is cost per pixel?)

| Run | Count | Size (NDC) | Overdraw |
|---|---|---|---|
| S1-0.05 | 10,000 | 0.05 | ~6.25x |
| S1-0.1 | 10,000 | 0.1 | ~25x (matches existing runs) |
| S1-0.2 | 10,000 | 0.2 | ~100x |

- Frame time barely changes with size: limited per sprite (CPU, vertex, driver side).
- Frame time scales with area: limited per pixel (fill rate, blending).
- PC-A caution: start with 0.05 and 0.1, and run 0.2 only if frame times stay well under
  100 ms, since the HD 5450 may hit the fill-rate wall (it nearly froze the desktop before).

**Sweep 2: fixed overdraw (~25x), varying count** (is cost per sprite?)

| Run | Count | Size (NDC) | Overdraw |
|---|---|---|---|
| S2-2500 | 2,500 | 0.2 | ~25x |
| S2-10000 | 10,000 | 0.1 | ~25x (same as S1-0.1, reuse the row) |
| S2-40000 | 40,000 | 0.05 | ~25x |
| S2-160000 | 160,000 | 0.025 | ~25x (optional; needs 16 batches of 10k) |

- Frame time stays flat as count grows: pixel-bound.
- Frame time grows with count: per-sprite cost dominates.

**Optional extras**

- Repeat S1-0.2 with blending off, to see where blending starts to matter.
- Show the draw-call count in the title bar to confirm batching (count / 10,000, rounded up).
- Compare Gpix/s across the sweeps. It should stay roughly constant only for runs that are pixel-bound.

Expected outcome: the two PCs may disagree. PC-B looks per-sprite-limited so far (about 2 Gpix/s,
far below what its GPU can fill), while PC-A is more likely to be pixel-bound.

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

