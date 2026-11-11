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
if [[ -z "$PROG_PATH" && -x "$SCRIPT_DIR/$EXE_NAME" ]]; then
    PROG_PATH="$SCRIPT_DIR/$PROG_NAME"
fi
# Safety Check
if [[ -z "$PROG_PATH" ]]; then
    echo "❌ Error: Could not find '$PROG_NAME' in any standard build directory." >&2
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
COMMON_FLAGS="--window 700 700 --vsync 0 --telemetry 0 --autoexit 1 --duration 2 --warmup 2 --benchmark 1"
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
)

# --- Cooldown Per Run in second ---
cooldown=5 # 120 seconds = 2 minutes

RUN_PATH="$SCRIPT_DIR/runs"
mkdir -p "$RUN_PATH"
# --- Run Benchmarks ---
total=${#benchmarks[@]}
count=0
for entry in "${benchmarks[@]}"; do
    # Count
    count=$((count + 1))

    # Split entry at '|' into 'name' and 'raw_flags'
    IFS='|' read -r name raw_flags <<< "$entry"

    # Timestamp
    timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
    # Log path per run
    log_path="$RUN_PATH/run_${count}_${timestamp}.log"

    # Convert flags into an array
    read -ra flag_array <<< "$raw_flags"

    # Append the run note first
    printf "Run: %s\n\n" "$name" > "$log_path"

    # Run the benchmark in background and catch its PID
    "$PROG_PATH" "${flag_array[@]}" >> "$log_path" 2>&1 &
    PROG_ID=$!

    echo "=========================================="
    echo "Run Count: $count/$total"
    echo "Running:   $name"
    echo "=========================================="

    wait "$PROG_ID"
    PROG_ID="" # Reset PID

    echo "Run '$name' completed."
    echo "Benchmark log saved @ '$log_path'."

    # Sleep if not the last run
    if [[ "$count" -lt "$total" ]]; then
        echo "Cooldown in $cooldown seconds."
        sleep "$cooldown"
    fi
done

echo "Benchmarks completed! All logs saved in '$RUN_PATH'."
