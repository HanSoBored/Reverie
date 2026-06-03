# Lucid

Encrypted traffic, made clear.

Hooks `SSL_read` / `SSL_write` to intercept decrypted TLS traffic
from any Android process.

## Quick Start

Inject the compiled shared library into target app using
[Zygisk-Loader](https://github.com/HanSoBored/Zygisk-Loader) or equivalent injection tool.

View traffic via logcat:

```bash
adb logcat -s Lucid
```

Build instructions in [docs/INSTALL.md](../../docs/INSTALL.md).
Full documentation in [docs/modules/Lucid.md](../../docs/modules/Lucid.md).

## Output

```
>>> /api/StageEnd ===
  body: {"JT":"BattleInitParam","operateLog":[...]}

<<< RESPONSE 490 bytes ===
  HTTP/1.1 200 OK
  Content-Length: 43
```
