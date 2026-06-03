# Installation

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| Android NDK | r25b+ | Set `$ANDROID_NDK_HOME` or `$NDK_HOME` |
| CMake | 3.14+ | `apt install cmake` / `brew install cmake` |
| [Zygisk-Loader](https://github.com/HanSoBored/Zygisk-Loader) | Latest | Injection mechanism (or use LD_PRELOAD) |

## Build a Module

```bash
cd Reverie

cmake -B build/<module> \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-35 \
    -S modules/<ModuleName>

cmake --build build/<module> -j$(nproc)
```

Output is at `build/<module>/lib/lib<module>.so`.

## Custom ABI

```bash
cmake -B build/<module> \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=armeabi-v7a \        # instead of arm64-v8a
    -DANDROID_PLATFORM=android-35 \
    -S modules/<ModuleName>
```

## Clean Build

```bash
rm -rf build/<module> && cmake ... (same as above)
```
