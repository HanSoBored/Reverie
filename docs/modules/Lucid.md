# Lucid

Decrypted TLS traffic interception & logging.

Hooks `SSL_read` / `SSL_write` via ShadowHook inline hooking to
capture decrypted application-layer data before it leaves the
process. Detects and pretty-prints HTTP headers, JSON payloads,
and protobuf binary streams to logcat.

```
[constructor] → spawn detached thread → init()
                                         │
                                         ├── memkit_hook_init()
                                         ├── memkit_hook_by_symbol("libssl.so", "SSL_read", ...)
                                         ├── memkit_hook_by_symbol("libssl.so", "SSL_write", ...)
                                         └── log "Lucid Ready"
```

```
>>> POST /api/StageEnd ===
  body: {"JT":"BattleInitParam","operateLog":[{"ty":1,"d":[...]}]}

<<< RESPONSE 490 bytes ===
  HTTP/1.1 200 OK
  Content-Length: 43
  FFFF...
```

## Techniques

| Technique | Implementation |
|-----------|---------------|
| Inline hooking | ShadowHook (ByteDance) via Android-Mem-Kit |
| TLS interception | `SSL_read` / `SSL_write` in `libssl.so` |
| Data parsing | HTTP header extraction, JSON/protobuf detection |
| Re-entrancy guard | `__thread` TLS flag prevents recursive hooks |
| Temp library redirect | `__wrap_dlopen` for Android 15+ linker compat |

## Hook Pattern

Passthrough with inspection — calls the original then reads/modifies
the data:

```c
static int (*orig_SSL_read)(void*, void*, int) = NULL;
static __thread int g_re = 0;

static int my_SSL_read(void* s, void* b, int n) {
    if (g_re) return orig_SSL_read(s, b, n);
    g_re = 1;
    int r = orig_SSL_read(s, b, n);
    if (r > 0 && memcmp(b, "HTTP/", 5) == 0) {
        LOGI("<<< RESPONSE %d bytes ===", r);
        ascii(b, r > 300 ? 300 : r);
    }
    g_re = 0;
    return r;
}
```

Symbol-based hooking via `memkit_hook_by_symbol` — no RVAs needed,
works on any library that exports the target symbol.

## Build

See [docs/INSTALL.md](../INSTALL.md).