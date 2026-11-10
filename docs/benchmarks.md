# BruhEngine Benchmarks

Lab notebook for performance tests. Append new results; superseded or invalid runs are listed in
section 4.3 with the reason they were dropped.
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
- GPU load: not available (the `radeon` driver has no `gpu_busy_percent`; `GALLIUM_HUD=gpu-load` shows nothing either)

### PC-B (CachyOS)

- CPU: AMD Ryzen 7 5700X3D (16) @ 4.15 GHz
- GPU: AMD Radeon RX 5700 XT [Discrete]
- RAM: 32 GB
- OS: CachyOS
- Mesa / driver: Mesa 26.2.4-arch3.1 (OpenGL 4.6 Core Profile)
- Compiler: g++ (GCC) 16.2.1 20260810

## 2. Build info (record for every result)

- Git commit hash (a `-dirty` suffix means uncommitted changes; rerun clean before trusting a row)
- Build type: Release for real benchmarks (Debug with ASan skews everything)
- SDL version: 2.32.10
- GL loader: glad (generated for 3.3 core)

## 3. Test setup (keep identical between comparable runs)

- Window size (fill cost scales with pixels; use a small window for risky tests) 700 by 700 resolutions
- Sprite count and sprite size
- Overdraw = total sprite area, clipped to the window, / window area (1.0 means each pixel drawn once on average)
  - The code computes this once at startup from the real sprite geometry (centered pivot) and prints it in the report.
- Gpix/s (derived) = overdraw x window width x window height x fps / 1e9
  - Lets you compare machines fairly. Only meaningful for fill-bound runs (frame time grows with overdraw).
  - Only as accurate as the overdraw figure; print the exact overdraw from code instead of estimating.
  - Counts pixels submitted, not pixels actually shaded (depth-rejected and discarded pixels still count).
  - Do not compare blended runs with opaque runs, or runs with different shaders or texture sizes.
  - Record the window size for every run; without it Gpix/s cannot be computed.
- Blending on/off, depth test on/off
- Vsync on/off (vsync off for benchmarks, otherwise fps is capped)
- Random seed and sprite movement (same every run)
- Run duration: 60 s after a 2 s warm-up (the engine aborts and marks the run INCOMPLETE if a frame exceeds 100 ms)
- Let the GPU cool down between full-load runs

## 4. Results

Frame time is the main number. Record the average and the worst frames, not just fps.
Fields marked (est.) were estimated, not measured. Gpix/s is derived (see section 3), not measured.
`n/a` means the value was not recorded or is known to be wrong (the Notes column says which).

### 4.1 PC-A results

Window 700x700, vsync off, unless stated.

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (derived) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-10 | 9bb71d0-dirty | 10k sprites, 64 px | ~79.8x | on | off | 35.97 | 52.72 | 27.8 | 1.092 | N/A | 17% | 75 C | 60 s, Release. Run-average 1.088 Gpix/s. 1,668 frames. GPU load N/A (radeon driver). Rerun on a clean commit before relying on it. |
| 2026-11-10 | not recorded | 10k sprites, 512 px | ~3,570x (analytic est.) | on | off | n/a | 1,281 (cut-off frame) | n/a | n/a | N/A | n/a | n/a | Aborted by the 100 ms cut-off before any frame was recorded, so no report. The single slow frame implies roughly 1.4 Gpix/s (rough; the 64 px run measured 1.09). Treat as "exceeds cut-off". |

Early informal runs (before benchmark mode; window size, commit and CPU/GPU load not recorded):

| Date | Test | Overdraw | Blend | Depth | Avg ms | FPS | Temp | Notes |
|---|---|---|---|---|---|---|---|---|
| 2026-11-08 | 10k sprites, 0.1 size, 1 draw call | ~25x | on | on | ~8.5 (est.) | ~117 | 88 C | Baseline. 10 minute run, too long. Its GPU 100% / CPU ~33% readings came from fallback values that were later removed, so they are not listed. |
| 2026-11-08 | 10k sprites, small sprites, 1 draw call | low | on | on | ~3 | ~337 | n/a | Early run before the overdraw test; sprite count, not size, was the limit. |
| 2026-11-08 | 10k sprites, large sprites, 1 draw call | very high | on | on | ~320 | ~3 | n/a | Fill-rate wall. Desktop nearly froze. Do not repeat at this size. |

### 4.2 PC-B results

Window 700x700, vsync off, 60 s, Release build, unless stated.

Overdraw note: runs up to 0.1 NDC list the nominal (unclipped) overdraw, so the real clipped value is
about 5% lower and Gpix/s about 5% too high. From 0.25 NDC upward the old overdraw formula was wrong
(unclipped, and sprites were bottom-left anchored), so Overdraw and Gpix/s are n/a. Frame times are valid.

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (derived) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-09 | c157439 | 100k sprites, 1.0 NDC | n/a | on | on | 20.55 | 24.36 | 47.9 | n/a | 1% (suspect) | 24% | 73 C | Early baseline. Window 700x700. GPU load 1% looks like a misread. |
| 2026-11-09 | e615b36 | 2k sprites, 0.1 NDC | ~5x (nominal) | on | on | 0.87 | 2.82 | 1144.6 | 2.804 | 14% | 19% | 61 C | |
| 2026-11-09 | e615b36 | 4k sprites, 0.1 NDC | ~10x (nominal) | on | on | 1.59 | 3.67 | 626.3 | 3.069 | 11% | 23% | 61 C | |
| 2026-11-09 | e615b36 | 8k sprites, 0.1 NDC | ~20x (nominal) | on | on | 3.04 | 5.30 | 326.6 | 3.201 | 13% | 26% | 61 C | |
| 2026-11-09 | e615b36 | 10k sprites, 0.1 NDC | ~25x (nominal) | on | on | 3.76 | 6.17 | 262.2 | 3.212 | 11% | 24% | 61 C | |
| 2026-11-09 | e615b36 | 2k sprites, 0.1 NDC, blend off | ~5x (nominal) | off | on | 0.87 | 2.78 | 1131.6 | 2.772 | 12% | 19% | 61 C | |
| 2026-11-09 | e615b36 | 4k sprites, 0.1 NDC, blend off | ~10x (nominal) | off | on | 1.58 | 3.63 | 629.5 | 3.085 | 13% | 22% | 61 C | |
| 2026-11-09 | e615b36 | 8k sprites, 0.1 NDC, blend off | ~20x (nominal) | off | on | 3.03 | 5.34 | 328.6 | 3.220 | 12% | 23% | 62 C | |
| 2026-11-09 | e615b36 | 10k sprites, 0.25 NDC | n/a | on | on | 3.76 | 6.17 | 264.6 | n/a | 10% | 26% | 62 C | Fixed count, varying size. |
| 2026-11-09 | e615b36 | 10k sprites, 0.5 NDC | n/a | on | on | 3.77 | 6.70 | 263.5 | n/a | 15% | 28% | 62 C | Fixed count, varying size. |
| 2026-11-09 | e615b36 | 10k sprites, 1.0 NDC | n/a | on | on | 3.76 | 6.63 | 262.5 | n/a | 50% | 22% | 65 C | Fixed count, varying size. |
| 2026-11-09 | e615b36 | 10k sprites, 2.0 NDC | n/a | on | on | 3.78 | 7.39 | 261.8 | n/a | 77% | 25% | 73 C | Fixed count, varying size. |
| 2026-11-09 | e615b36 | 10k sprites, 4.0 NDC | n/a | on | on | 3.76 | 8.67 | 264.6 | n/a | 80% | 26% | 75 C | Fixed count, varying size. |
| 2026-11-09 | e615b36 | 10k sprites, 8.0 NDC | n/a | on | on | 3.95 | 6.91 | 250.4 | n/a | 74% | 20% | 73 C | Fixed count, varying size. |

Reading the e615b36 size sweep: frame time stays at ~3.76 ms while GPU load climbs from 10% to 80%.
These runs had depth test on and every sprite at the same depth, so early depth rejection probably
discarded most of the covered pixels (hypothesis, not yet confirmed). Rerun the sweep with depth off
on the current commit to measure real fill cost.

### 4.3 Dropped runs

Removed from the tables above. The frame times were real, but the rows were unusable.

| Date | PC | Commit | What | Why dropped |
|---|---|---|---|---|
| 2026-11-08 | B | n/a | 60k sprites, 0.1 NDC | GPU load misread; window size and commit not recorded. |
| 2026-11-08 | B | 4dbb38d | 2k, 4k, 8k, 10k sprites at 0.1 NDC, blend on and off (8 runs) | GPU load misread (fallback value). The 2k, 4k, 8k blend-on/off runs were redone on e615b36 and the 10k blend-on run too. **The 10k blend-off run has no replacement yet.** |

## 5. Planned runs

- [ ] Overdraw 5x, 10x, 20x with blending on (PC-B done)
- [ ] Same overdraw levels with blending off (PC-B done, except 10k)
- [ ] 10k sprites at 0.1 NDC with blend off on PC-B (dropped run, no replacement)
- [ ] Rerun the size sweep on the current commit with depth off (PC-B), then PC-A
- [ ] Rerun the PC-A 64 px run on a clean (non-dirty) commit
- [ ] Opaque sprites with depth testing, drawn front to back
- [ ] Re-run baseline after: camera, culling, atlas, render passes
- [ ] Run the same tests on PC-B
- [ ] CPU-side stage timings (update, buffer fill, swap)
- [ ] Optional: GPU timer queries
- [ ] Sweep 1 (fixed count, varying size) on PC-B (done for depth on; redo with depth off), then PC-A
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

Nominal overdraw (unclipped) = count x (size / 2)^2, with size in NDC units and the window spanning 2 units.
The code prints the clipped value, which is lower (about 5% at 0.1 NDC, much lower for large sprites).

**Sweep 1: fixed count, varying size** (is cost per pixel?)

| Run | Count | Size (NDC) | Nominal overdraw |
|---|---|---|---|
| S1-0.05 | 10,000 | 0.05 | ~6.25x |
| S1-0.1 | 10,000 | 0.1 | ~25x (matches existing runs) |
| S1-0.2 | 10,000 | 0.2 | ~100x |

- Frame time barely changes with size: limited per sprite (CPU, vertex, driver side).
- Frame time scales with area: limited per pixel (fill rate, blending).
- PC-A caution: the HD 5450 measured about 1.09 Gpix/s (64 px run), so a 100 ms frame allows
  roughly 220x overdraw at 700x700. With 10k sprites that means sizes up to about 96 px. 128 px is
  predicted to exceed the cut-off and 512 px did (1,281 ms).

**Sweep 2: fixed overdraw (~25x), varying count** (is cost per sprite?)

| Run | Count | Size (NDC) | Nominal overdraw |
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

Expected outcome: the two PCs may disagree. PC-B looks per-sprite-limited so far (about 3 Gpix/s,
far below what its GPU can fill), while PC-A is more likely to be pixel-bound.

## 6. Findings

- 10k sprites in one draw call is fine on both GPUs; the batcher works.
- PC-A (HD 5450), 64 px run: 36 ms frames, ~1.09 Gpix/s, CPU load 17%. This looks GPU-bound, but there
  is no GPU load counter on the radeon driver to confirm it. Confirm by halving sprite count or window
  area and checking that frame time roughly halves.
- PC-B, depth on: frame time stays flat across a 25x to 160,000x nominal size sweep, so those runs do
  not measure fill rate (see the note under 4.2).
- Practical budget: aim for roughly 5x-10x average overdraw in real scenes; measure the real ceiling with the planned runs.
- PC-A reached 75 C in the 64 px run (88 C was recorded in an earlier informal run). Keep stress runs short.

## 7. How to reproduce

- Build: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release` then `cmake --build build-release`
- Run with Mesa's overlay: `GALLIUM_HUD="fps,frametime" ./build-release/<executable>`
  (use `GALLIUM_HUD=help` to list the counters your driver exposes)
- Stress test controls: V toggles vsync, H toggles the HUD, D toggles depth test. Sprite count, size, blend and depth start state are set in main.cpp.
- Note: `GALLIUM_HUD` only works with Mesa Gallium drivers. It also costs a little frame time, so compare with it off for final numbers.

## 8. Rules of thumb

- Append rows. Do not edit old results; move a bad run to section 4.3 and say why. Re-run the baseline after big engine changes to catch regressions.
- Change one variable at a time.
- If frame time grows with sprite count but not with size: CPU-bound. If it grows with size and overdraw: GPU fill-rate-bound.
- Stop a test as soon as frame time passes ~100 ms.
