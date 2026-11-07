# my-game-engine
my game engine

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
```
# Configure the project using Ninja as the generator
cmake -B build -G Ninja

# Build the project
cmake --build build
```
