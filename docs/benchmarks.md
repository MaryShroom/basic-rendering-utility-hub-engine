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
  - The report prints two figures: "Derived Throughput" (mean of per-frame rates) and "Run-Average Gpix/s"
    (total pixels submitted / total time). Record the run-average. The derived figure is inflated by frame-time
    jitter (up to ~9% at high fps), so it is only listed in older rows.
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

Window 700x700, vsync off, depth off, 60 s after a 2 s warm-up, Release build, unless stated.
Sprite size is given in pixels (35 px = 0.1 NDC at 700x700). Overdraw is the "Expected Overdraw" line of the report.
Gpix/s here is the run-average figure. GPU load is N/A on this machine (radeon driver).
Rows marked `898acb7-dirty` were built from 898acb7 with only `particleCount` and `spritePixelWH` edited (per the author; not checked against a diff).
Rows marked `ef2068b-dirty` are assumed to differ from ef2068b only in those constants and the blend default (not checked against a diff).

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (run-avg) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-10 | 898acb7-dirty | 2k sprites, 35 px | ~4.9x | on | off | 4.08 | 7.36 | 245.0 | 0.586 | N/A | 18% | 71 C | 14,702 frames. Derived 0.615. |
| 2026-11-10 | 898acb7-dirty | 4k sprites, 35 px | ~9.7x | on | off | 6.32 | 9.48 | 158.1 | 0.755 | N/A | 15% | 74 C | 9,486 frames. Derived 0.759. |
| 2026-11-10 | 898acb7-dirty | 8k sprites, 35 px | ~19.5x | on | off | 10.84 | 13.88 | 92.3 | 0.882 | N/A | 12% | 75 C | 5,535 frames. Derived 0.883. |
| 2026-11-10 | 898acb7-dirty | 10k sprites, 35 px | ~24.4x | on | off | 12.55 | 15.74 | 79.7 | 0.953 | N/A | 12% | 77 C | 4,782 frames. Derived 0.954. |
| 2026-11-10 | 898acb7-dirty | 2k sprites, 35 px, blend off | ~4.9x | off | off | 3.12 | 6.46 | 320.0 | 0.765 | N/A | 21% | 73 C | 19,201 frames. Derived 0.831 (jitter-inflated). |
| 2026-11-10 | 898acb7-dirty | 4k sprites, 35 px, blend off | ~9.7x | off | off | 4.58 | 16.29 | 218.2 | 1.041 | N/A | 23% | 75 C | 13,092 frames. Worst frame is a one-off hitch (3.6x the average); do not quote it. |
| 2026-11-10 | 898acb7-dirty | 8k sprites, 35 px, blend off | ~19.5x | off | off | 7.33 | 10.91 | 136.4 | 1.305 | N/A | 16% | 77 C | 8,185 frames. |
| 2026-11-10 | 898acb7-dirty | 10k sprites, 35 px, blend off | ~24.4x | off | off | 7.78 | 10.61 | 128.6 | 1.538 | N/A | 16% | 82 C | 7,715 frames. Only +0.45 ms over 8k, flatter than the earlier steps. Rechecked (see Repeat runs): 10k 7.76 ms and 8k 6.96 ms, so most of the flatness was noise. |
| 2026-11-10 | 898acb7 | 10k sprites, 64 px | ~79.8x | on | off | 34.37 | 40.80 | 29.1 | 1.138 | N/A | 13% | 73 C | 1,745 frames. Clean commit. Replaces the earlier dirty 64 px run (see 4.3). |
| 2026-11-10 | not recorded | 10k sprites, 512 px | ~3,570x (analytic est.) | on | off | n/a | 1,281 (cut-off frame) | n/a | n/a | N/A | n/a | n/a | Aborted by the 100 ms cut-off before any frame was recorded, so no report. The single slow frame implies roughly 1.4 Gpix/s (rough; the clean 64 px run measured 1.14). Treat as "exceeds cut-off". |

#### Size sweep (S1): 10k sprites, blend on, depth off

Cooldown of 5 minutes between runs. The 35 px point is the S2-10000 blend-on row below (12.51 ms). The 512 px row further down is the other end of this sweep.

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (run-avg) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-10 | ef2068b-dirty | S1-64: 10k sprites, 64 px | ~79.8x | on | off | 34.20 | 40.82 | 29.2 | 1.144 | N/A | 8% | 72 C | 1,754 frames. Matches the clean 898acb7 run (34.37 ms). |
| 2026-11-10 | ef2068b-dirty | S1-96: 10k sprites, 96 px | ~175.4x | on | off | 70.86 | 75.15 | 14.1 | 1.213 | N/A | 5% | 72 C | 846 frames. Predicted 70 to 80 ms. |

#### Size sweep (S1), blend off: 10k sprites, depth off

The 35 px point is the S2-10000 blend-off row below (7.76 ms). Predicted before the runs: 18 to 20 ms (64 px) and 36 to 40 ms (96 px).

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (run-avg) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-10 | f2144dd | S1-64: 10k sprites, 64 px, blend off | ~79.8x | off | off | 19.96 | 22.70 | 50.1 | 1.961 | N/A | 10% | 74 C | 3,006 frames. Clean commit. Inside the predicted range. |
| 2026-11-10 | f2144dd | S1-96: 10k sprites, 96 px, blend off | ~175.4x | off | off | 40.53 | 42.58 | 24.7 | 2.121 | N/A | 7% | 75 C | 1,480 frames. About 1% above the predicted range. |

#### Same-overdraw sweep (S2): about 24.4x, depth off

Sprite count rises 17x while the overdraw stays the same. Cooldown of 2 minutes between runs. Draw calls = ceil(count / 10,000).

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (run-avg) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-10 | ef2068b-dirty | S2-2500: 2,500 sprites, 71 px | ~24.4x | on | off | 11.88 | 15.84 | 84.1 | 1.006 | N/A | 11% | 71 C | 5,048 frames. 1 draw call. |
| 2026-11-10 | ef2068b-dirty | S2-5000: 5,000 sprites, 50 px | ~24.6x | on | off | 12.62 | 15.11 | 79.2 | 0.954 | N/A | 12% | 72 C | 4,753 frames. 1 draw call. |
| 2026-11-10 | ef2068b-dirty | S2-10000: 10,000 sprites, 35 px | ~24.4x | on | off | 12.51 | 14.56 | 79.9 | 0.955 | N/A | 13% | 73 C | 4,794 frames. 1 draw call. Repeat of the 898acb7-dirty run (12.55 ms). |
| 2026-11-10 | ef2068b-dirty | S2-19460: 19,460 sprites, 25 px | ~24.4x | on | off | 13.87 | 16.55 | 72.1 | 0.862 | N/A | 14% | 72 C | 4,325 frames. 2 draw calls. |
| 2026-11-10 | ef2068b-dirty | S2-41843: 41,843 sprites, 17 px | ~24.4x | on | off | 15.59 | 17.27 | 64.1 | 0.767 | N/A | 18% | 73 C | 3,848 frames. 5 draw calls. |
| 2026-11-10 | ef2068b-dirty | S2-2500: 2,500 sprites, 71 px, blend off | ~24.4x | off | off | 7.54 | 13.51 | 132.6 | 1.586 | N/A | 15% | 73 C | 7,957 frames. Derived 1.614 (jitter-inflated); worst frame 1.8x the average. |
| 2026-11-10 | ef2068b-dirty | S2-5000: 5,000 sprites, 50 px, blend off | ~24.6x | off | off | 7.83 | 10.50 | 127.8 | 1.538 | N/A | 15% | 74 C | 7,666 frames. |
| 2026-11-10 | ef2068b | S2-10000: 10,000 sprites, 35 px, blend off | ~24.4x | off | off | 7.76 | 10.58 | 128.9 | 1.542 | N/A | 18% | 77 C | 7,736 frames. Clean commit. |
| 2026-11-10 | ef2068b-dirty | S2-19460: 19,460 sprites, 25 px, blend off | ~24.4x | off | off | 8.57 | 11.50 | 116.7 | 1.396 | N/A | 18% | 75 C | 7,004 frames. |
| 2026-11-10 | ef2068b-dirty | S2-41843: 41,843 sprites, 17 px, blend off | ~24.4x | off | off | 9.13 | 11.41 | 109.6 | 1.309 | N/A | 26% | 75 C | 6,574 frames. |

#### Repeat runs (reproducibility)

Rechecks of the blend-off 8k and 10k runs after the flat 8k-to-10k step in the first set. 10k was run first, then 8k after a 5 minute cooldown.

| Date | Commit | Test | Overdraw | Blend | Depth | Avg ms | Worst ms | FPS | Gpix/s (run-avg) | GPU load | CPU load | Temp | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2026-11-10 | ef2068b | 10k sprites, 35 px, blend off | ~24.4x | off | off | 7.76 | 10.44 | 128.9 | 1.541 | N/A | 17% | 76 C | 7,734 frames. First run: 7.78 ms. |
| 2026-11-10 | ef2068b-dirty | 8k sprites, 35 px, blend off | ~19.5x | off | off | 6.96 | 9.80 | 143.7 | 1.374 | N/A | 16% | 73 C | 8,622 frames. First run: 7.33 ms (5% slower). |

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

Removed from the tables above. The frame times were real, but the rows were unusable or have been superseded.

| Date | PC | Commit | What | Why dropped |
|---|---|---|---|---|
| 2026-11-08 | B | n/a | 60k sprites, 0.1 NDC | GPU load misread; window size and commit not recorded. |
| 2026-11-08 | B | 4dbb38d | 2k, 4k, 8k, 10k sprites at 0.1 NDC, blend on and off (8 runs) | GPU load misread (fallback value). The 2k, 4k, 8k blend-on/off runs were redone on e615b36 and the 10k blend-on run too. **The 10k blend-off run has no replacement yet.** |
| 2026-11-10 | A | 9bb71d0-dirty | 10k sprites, 64 px, blend on, depth off | Superseded by the clean rerun on 898acb7. Kept for the run-to-run spread: 35.97 ms / 1.088 Gpix/s here against 34.37 ms / 1.138 Gpix/s (about 5%). |

## 5. Planned runs

- [x] Overdraw 5x, 10x, 20x, 25x with blending on, 35 px, depth off (PC-A done)
- [ ] Same on PC-B (the old PC-B rows are depth on and from older commits; rerun on the current commit)
- [x] Same overdraw levels with blending off, including 10k (PC-A done)
- [ ] Same on PC-B, including 10k with blend off (dropped run, no replacement)
- [x] Size sweep with depth off on PC-A (10k sprites at 35, 64, 96 px, blend on)
- [x] Blend-off repeat of the size sweep at 64 and 96 px on PC-A
- [ ] Same size sweep with depth off on PC-B
- [x] Rerun the PC-A 64 px run on a clean (non-dirty) commit (898acb7)
- [x] Rerun PC-A 8k and 10k blend-off once (10k 7.76 ms, 8k 6.96 ms; the flat step was mostly noise)
- [ ] Opaque sprites with depth testing, drawn front to back
- [ ] Re-run baseline after: camera, culling, atlas, render passes
- [ ] Run the same tests on PC-B
- [ ] CPU-side stage timings (update, buffer fill, swap)
- [ ] Optional: GPU timer queries
- [ ] Sweep 1 (fixed count, varying size) on PC-B, redo with depth off (PC-A done in 4.1 with 35/64/96 px, not the 0.05/0.1/0.2 NDC design)
- [ ] Sweep 2 (fixed overdraw, varying count) on PC-B (PC-A done: about 24.4x, 2,500 to 41,843 sprites, blend on and off)

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
- PC-A caution: the HD 5450 measured about 1.1 Gpix/s (64 px run), so a 100 ms frame allows
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
- PC-A (HD 5450), 10k sprites, 64 px, blend on: 34 ms frames, ~1.14 Gpix/s, CPU load 13%. This looks
  GPU-bound, but there is no GPU load counter on the radeon driver to confirm it. Confirm by halving
  sprite count or window area and checking that frame time roughly halves.
- PC-A blending cost: blend adds about 0.195 ms per 1x of overdraw at 700x700. At 2k sprites that is
  +0.96 ms (+31%); at 10k it is +4.77 ms (+61%). Frame time at 10k: 12.55 ms blend on, 7.78 ms blend off.
- PC-A fill rate (size sweep, 10k sprites, blend on, depth off): 35 / 64 / 96 px give 12.51 / 34.20 / 70.86 ms
  at 24.4x / 79.8x / 175.4x overdraw. The slope is a steady 0.39 ms per 1x (about 1.25 to 1.28 Gpix/s) and
  extrapolates to about 3 ms per frame of non-pixel cost. Run-average Gpix/s climbs toward that figure
  (0.955, 1.144, 1.213) as the fixed cost is amortized. CPU load falls (13%, 8%, 5%) as the pixel load grows,
  which is the evidence for GPU-bound without a GPU load counter. 128 px would be about 120 ms, over the cut-off.
- Same-overdraw sweep (S2, about 24.4x): going from 2,500 to 41,843 sprites (17x) raises frame time only
  31% with blend on (11.88 to 15.59 ms) and 21% with blend off (7.54 to 9.13 ms). Cost is mostly per pixel,
  with a small per-sprite part of roughly 0.09 us per sprite blend on and 0.04 us blend off (mixes CPU and GPU cost).
  CPU load rises with sprite count (11% to 18% blend on, 15% to 26% blend off).
- Blend penalty at equal overdraw grows as sprites get smaller: about 0.18 ms per 1x at 71 px, 0.195 at 50 and
  35 px, 0.22 at 25 px, 0.27 at 17 px. Possible cause: more partly covered pixel quads on small sprites
  (hypothesis, not tested).
- Blend-off size sweep (10k sprites, depth off): 35 / 64 / 96 px give 7.76 / 19.96 / 40.53 ms. The slope is
  0.217 ms per 1x (about 2.26 Gpix/s) against 0.386 ms per 1x (1.27 Gpix/s) with blend on, so blending
  costs about 1.8x per pixel on this card. The blend cost per 1x of overdraw is 0.195 ms at 35 px, 0.178 at
  64 px and 0.173 at 96 px, so it depends mostly on sprite size and not on how much overdraw there is.
  The 64 px result fell inside my predicted range and 96 px came out about 1% above it.
- Possible memory-bandwidth ceiling (hypothesis, not verified): at 4 bytes per pixel, blend off writes about
  9 GB/s and blend on (read plus write) moves about 10 GB/s, nearly the same. That fits a fill rate limited by
  memory bandwidth or ROPs. Confirming it needs this board's memory type, bus width and memory clock, which
  vary between HD 5450 models.
- Gpix/s rises with load in the 35 px runs (0.59 to 0.95 blend on); that is mostly the fixed per-frame cost,
  and part may be the GPU clock rising under load (hypothesis, not checked).
- Run-to-run spread on PC-A is usually under 1%: 10k, 35 px, blend off measured 7.78, 7.76 and 7.76 ms;
  blend on 12.55 and 12.51 ms; 64 px 34.37 and 34.20 ms. The outliers were the dirty 9bb71d0 64 px run
  (+5%) and the first 8k blend-off run (+5%).
- PC-B, depth on: frame time stays flat across a 25x to 160,000x nominal size sweep, so those runs do
  not measure fill rate (see the note under 4.2).
- Practical budget: aim for roughly 5x-10x average overdraw in real scenes; measure the real ceiling with the planned runs.
- PC-A temperature ranged 71 to 82 C across the 898acb7 and ef2068b runs (hottest: 10k sprites, blend off, 82 C after a series of back-to-back runs; 76 to 77 C after a cooldown). Frame time did not change between 76 and 82 C, so there is no sign of throttling in that range. A run left going for about 30 minutes reached about 85 C; 88 C was recorded in an earlier informal run. Keep stress runs short.

## 7. How to reproduce

- Build: `cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release` then `cmake --build build-release`
- Run with Mesa's overlay: `GALLIUM_HUD="fps,frametime" ./build-release/<executable>`
  (use `GALLIUM_HUD=help` to list the counters your driver exposes)
- Stress test controls: V toggles vsync, H toggles the HUD, D toggles depth test. The start state is set with command-line flags (see `--help`): `--count`, `--pixels`, `--blend`, `--depth`, `--vsync`, `--window`, `--duration`, `--warmup`, `--telemetry`, `--benchmark`. Since f2144dd `--blend` defaults to 1 (earlier commits defaulted to 0), so pass `--blend 0` for blend-off runs. Example: `./build-release/BasicRenderingUtilityHub --count 41843 --pixels 17 --blend 0`
- Note: `GALLIUM_HUD` only works with Mesa Gallium drivers. It also costs a little frame time, so compare with it off for final numbers.

## 8. Rules of thumb

- Append rows. Do not edit old results; move a bad run to section 4.3 and say why. Re-run the baseline after big engine changes to catch regressions.
- Change one variable at a time.
- If frame time grows with sprite count but not with size: CPU-bound. If it grows with size and overdraw: GPU fill-rate-bound.
- Stop a test as soon as frame time passes ~100 ms.
- Close the window once "collection complete" appears. The engine keeps rendering at full load after the 60 s run, and the report only prints on close.
