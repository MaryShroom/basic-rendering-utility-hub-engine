#!/usr/bin/bash
set -euo pipefail

# --- Benchmark Program Path ---
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROG_NAME="BasicRenderingUtilityHub"
# Possible build directories name
POSSIBLE_DIRS=("build" "Build" "build-release" "out" "bin")
# Search for the executable
echo "Searching for $PROG_NAME..."
PROG_PATH=""
for dir in "${POSSIBLE_DIRS[@]}"; do
    if [[ -x "$SCRIPT_DIR/$dir/$PROG_NAME" ]]; then
        PROG_PATH="$SCRIPT_DIR/$dir/$PROG_NAME"
        break # Found it! Stop searching.
    fi
done
# If not found in build directories, check in script's root directory
if [[ -z "$PROG_PATH" && -x "$SCRIPT_DIR/$PROG_NAME" ]]; then
    PROG_PATH="$SCRIPT_DIR/$PROG_NAME"
fi
# Safety Check
if [[ -z "$PROG_PATH" ]]; then
    echo "Error: Could not find '$PROG_NAME' in any standard build directory." >&2
    echo "Please ensure the program is compiled and named correctly." >&2
    exit 1
fi

echo "Found it! $PROG_PATH"

PROG_ID=""
CLEANED_UP=false
cleanup() {
    local exit_code=$?
    set +e

    if [[ "$CLEANED_UP" == true ]]; then
        return
    fi
    CLEANED_UP=true

    # Check if PROG_ID is set and the process is still alive
    if [[ -n "$PROG_ID" ]] && kill -0 "$PROG_ID" 2>/dev/null; then
        echo -e "\nStopping process $PROG_ID..."
        kill "$PROG_ID" 2>/dev/null

        echo "Waiting for process $PROG_ID to close..."
        wait "$PROG_ID" 2>/dev/null
    fi

    exit "$exit_code"
}

# Trap normal exit, Ctrl+C (SIGINT), kill (SIGTERM), and terminal close (SIGHUP)
trap cleanup EXIT SIGINT SIGTERM SIGHUP

# --- Benchmark Flags ---
# Default Flags across all runs, no changes.
COMMON_FLAGS="--window 700 700 --vsync 0 --telemetry 0 --autoexit 1 --duration 60 --warmup 2 --benchmark 1"
# List of benchmarks setup.
benchmarks=(
  "Overdraw 5x, Blend On, Depth Off|$COMMON_FLAGS --count 2000 --pixels 35 --blend 1 --depth 0"
  "Overdraw 10x, Blend On, Depth Off|$COMMON_FLAGS --count 4000 --pixels 35 --blend 1 --depth 0"
  "Overdraw 20x, Blend On, Depth Off|$COMMON_FLAGS --count 8000 --pixels 35 --blend 1 --depth 0"
  "Overdraw 25x, Blend On, Depth Off|$COMMON_FLAGS --count 10000 --pixels 35 --blend 1 --depth 0"

  "Overdraw 5x, Blend Off, Depth Off|$COMMON_FLAGS --count 2000 --pixels 35 --blend 0 --depth 0"
  "Overdraw 10x, Blend Off, Depth Off|$COMMON_FLAGS --count 4000 --pixels 35 --blend 0 --depth 0"
  "Overdraw 20x, Blend Off, Depth Off|$COMMON_FLAGS --count 8000 --pixels 35 --blend 0 --depth 0"
  "Overdraw 25x, Blend Off, Depth Off|$COMMON_FLAGS --count 10000 --pixels 35 --blend 0 --depth 0"

  "Fixed count, varying size, Depth off, Blend On, 64px|$COMMON_FLAGS --count 10000 --pixels 64 --blend 1 --depth 0"
  "Fixed count, varying size, Depth off, Blend On, 96px|$COMMON_FLAGS --count 10000 --pixels 96 --blend 1 --depth 0"
  "Fixed count, varying size, Depth off, Blend On, 128px|$COMMON_FLAGS --count 10000 --pixels 128 --blend 1 --depth 0"

  "Fixed overdraw, varying count, Depth off, Blend On, 71px, 2500 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 2500 --pixels 71 --blend 1 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend On, 50px, 5000 sprites, ~24.6x Overdraw|$COMMON_FLAGS --count 5000 --pixels 50 --blend 1 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend On, 35px, 10000 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 10000 --pixels 35 --blend 1 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend On, 25px, 19460 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 19460 --pixels 25 --blend 1 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend On, 17px, 41843 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 41843 --pixels 17 --blend 1 --depth 0"

  "Fixed overdraw, varying count, Depth off, Blend Off, 71px, 2500 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 2500 --pixels 71 --blend 0 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend Off, 50px, 5000 sprites, ~24.6x Overdraw|$COMMON_FLAGS --count 5000 --pixels 50 --blend 0 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend Off, 35px, 10000 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 10000 --pixels 35 --blend 0 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend Off, 25px, 19460 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 19460 --pixels 25 --blend 0 --depth 0"
  "Fixed overdraw, varying count, Depth off, Blend Off, 17px, 41843 sprites, ~24.4x Overdraw|$COMMON_FLAGS --count 41843 --pixels 17 --blend 0 --depth 0"
)

# --- Cooldown Per Run in second ---
cooldown=120 # 120 seconds = 2 minutes

RUN_PATH="$SCRIPT_DIR/runs"
mkdir -p "$RUN_PATH"

# Timestamp
timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
log_path="$RUN_PATH/run_${timestamp}.log"

# --- Run Benchmarks ---
total=${#benchmarks[@]}
count=0
for entry in "${benchmarks[@]}"; do
    # Count
    count=$((count + 1))

    # Split entry at '|' into 'name' and 'raw_flags'
    IFS='|' read -r name raw_flags <<< "$entry"

    # Convert flags into an array
    read -ra flag_array <<< "$raw_flags"

    # --- Extract duration and warmup to dynamically compute a safe timeout ---
    run_duration=60
    run_warmup=2
    prev_arg=""
    for arg in $raw_flags; do
        if [[ "${prev_arg:-}" == "--duration" ]]; then run_duration="$arg"; fi
        if [[ "${prev_arg:-}" == "--warmup" ]]; then run_warmup="$arg"; fi
        prev_arg="$arg"
    done
    # Timeout = Duration + Warmup + 30s buffer grace period
    timeout_sec=$(awk "BEGIN {print int($run_duration + $run_warmup + 30)}")

    # Append the run note first
    printf "Run: %s\n\n" "$name" >> "$log_path"

    echo "==============================================="
    echo "Run Count: $count/$total"
    echo "Running:   $name"
    echo "Timeout:   ${timeout_sec}s"
    echo "==============================================="

    # Capture exit code safely without triggering set -e early
    set +e

    # Run the benchmark in background under timeout and catch its PID
    timeout -k 10s "${timeout_sec}s" "$PROG_PATH" "${flag_array[@]}" >> "$log_path" 2>&1 &
    PROG_ID=$!

    wait "$PROG_ID"
    exit_code=$?
    set -e
    PROG_ID="" # Reset PID

    # Handle exit codes
    if [[ "$exit_code" -eq 255 ]]; then
        echo "Error: Argument error detected (exit 255) for '$name'. Aborting batch..." >&2
        printf "RUN FAILED (exit 255)\n\n" >> "$log_path"
        printf "\n%s\n\n" "======================END======================" >> "$log_path"
        exit 255
    elif [[ "$exit_code" -eq 124 ]]; then
        echo "Error: Run '$name' timed out after ${timeout_sec}s (hang / frozen window detected). Skipping..."
        printf "RUN FAILED (exit 124 - TIMEOUT / HANG)\n\n" >> "$log_path"
    elif [[ "$exit_code" -eq 2 ]]; then
        echo "Warning: Run '$name' was INCOMPLETE (hitch cut-off / exit code 2). Skipping and continuing..."
        printf "RUN FAILED (exit 2 - INCOMPLETE)\n\n" >> "$log_path"
    elif [[ "$exit_code" -ne 0 ]]; then
        echo "Warning: Run '$name' failed with exit code $exit_code (e.g. hitch cut-off or crash). Logging and continuing..."
        printf "RUN FAILED (exit %d)\n\n" "$exit_code" >> "$log_path"
    else
        echo "Run '$name' completed successfully."
    fi

    echo "Benchmark log saved @ '$log_path'."

    # Append the border
    printf "\n%s\n\n" "======================END======================" >> "$log_path"

    # Sleep if not the last run
    if [[ "$count" -lt "$total" ]]; then
        echo "Cooldown in $cooldown seconds."
        sleep "$cooldown"
    fi
done

echo "Benchmarks completed! All logs saved in '$RUN_PATH'."
