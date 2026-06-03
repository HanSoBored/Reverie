# Lucid

Decrypted TLS traffic interception & logging.

Hooks `SSL_read` / `SSL_write` via ShadowHook inline hooking to
capture decrypted application-layer data before it leaves the
process. Detects and pretty-prints HTTP headers, JSON payloads,
and protobuf binary streams to logcat.

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

## Build

See [docs/INSTALL.md](../INSTALL.md).
