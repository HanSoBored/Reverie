# Edge

RVA-based native hooking template for Android shared libraries.
Provides a reusable pattern for hooking internal functions at known
offsets using ShadowHook inline hooking via Android-Mem-Kit.

```
[constructor] → spawn detached thread → init()
                                         │
                                         ├── memkit_hook_init()
                                         ├── wait for TARGET_LIB
                                         ├── walk HookEntry[] → memkit_hook()
                                         └── log "Edge Ready"
```

## Techniques

| Technique | Implementation |
|-----------|---------------|
| Inline hooking | ShadowHook (ByteDance) via Android-Mem-Kit |
| Function discovery | Hardcoded RVAs (relative virtual addresses) |
| Hook registration | Table-driven: `HookEntry` array + loop |
| Void no-op pattern | Empty proxy body — blocks original function |
| Return override pattern | Proxy returns constant value |
| Re-entrancy guard | `__thread` TLS flag prevents recursive hooks (add if proxying original) |
| Temp library redirect | `__wrap_dlopen` for Android 15+ linker compat |
| Init ordering | Constructor → detached thread → busy-wait for library |

## HookEntry Table

The core abstraction. Each entry describes one hook target:

```c
typedef struct {
    const char* name;     // logged on attach
    uintptr_t   rva;      // offset from library base
    void*       proxy;    // replacement function
    void**      orig;     // output: original function ptr
} HookEntry;
```

The init loop walks this array and calls `memkit_hook` for each entry.
Stubs are returned but not stored — add a `void** stub` field if you
need to unhook later.

## Patterns

### Void no-op (block behavior)

Use when you want to disable a function entirely:

```c
static void (*orig_apply)(void*) = NULL;
static void my_apply(void* _this) { }
```

### Return override (force value)

Use when a getter function should always return a specific value:

```c
static float (*orig_get_value)(const void*) = NULL;
static float my_get_value(const void* _this) {
    return 0.0f;
}
```

### Passthrough (inspect & forward)

Use when you need to read data flowing through a function:

```c
static int (*orig_recv)(void*, void*, int) = NULL;
static __thread int g_re = 0;

static int my_recv(void* s, void* b, int n) {
    if (g_re) return orig_recv(s, b, n);
    g_re = 1;
    int r = orig_recv(s, b, n);
    // inspect/modify b[0..r]
    g_re = 0;
    return r;
}
```

## Build

See [docs/INSTALL.md](../INSTALL.md).
