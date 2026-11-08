# Basic Rendering Utility Hub
Basic Rendering Utility Hub Engine, or BRUH Engine for short.

## Development Tools
### Ubuntu 24.04
```bash
sudo apt install build-essential cmake ninja-build ccache libsdl2-dev
```

### CachyOS
```fish
sudo pacman -S cmake ninja ccache
```

## Build
### Configure the project using Ninja as the generator
```
# Debug
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Release
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Build the project
```
# Debug
cmake --build build-debug

# Release
cmake --build build-release
```

### Run It
```
# With HUD
GALLIUM_HUD="fps,frametime" ./build-debug/BasicRenderingUtilityHub
GALLIUM_HUD="fps,frametime" ./build-release/BasicRenderingUtilityHub

GALLIUM_HUD=".w1.h1fps+cpu+GPU-load+temperature,.w200.h200frametime;.w1.h1requested-VRAM+VRAM-usage"
```
