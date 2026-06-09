# Edge

Inline hooking template for Android native libraries.

Provides a structured pattern for hooking internal functions at known
offsets (RVAs) within a target shared library using ShadowHook inline
hooking via Android-Mem-Kit.

## How to Use

### 1. Copy & rename

```bash
cp -r modules/Edge custom/my-module
```

### 2. Edit `src/main.c`

a. Set `TARGET_LIB` to your target library name (e.g. `"libunity.so"`)
b. Set `LOG_TAG` to your module name
c. Add your original function pointers + proxy functions
d. Populate `g_hooks[]` with your target RVAs

### 3. Build

Build instructions in [docs/INSTALL.md](../../docs/INSTALL.md).

Inject the compiled shared library into target app using
[Zygisk-Loader](https://github.com/HanSoBored/Zygisk-Loader) or equivalent injection tool.

## Hook Patterns

| Pattern | Example Use | Implementation |
|---------|-------------|---------------|
| Void no-op | Recoil, spread application | Empty proxy body — original never called |
| Return override | Cooldown, speed, delay getters | Proxy returns constant value |
| Passthrough + inspect | Traffic interception | Call original, read/modify result |

See [docs/modules/Edge.md](../../docs/modules/Edge.md) for full documentation.
