#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>

#include "memkit.h"
#include "shadowhook.h"

#define LOG_TAG "Lucid"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static void* ssl_r_stub = NULL;
static void* ssl_w_stub = NULL;
static int (*orig_SSL_read)(void*, void*, int) = NULL;
static int (*orig_SSL_write)(void*, const void*, int) = NULL;

static __thread int g_re = 0;
static char g_path[512] = {0};
static bool g_path_ok = false;

extern void *__real_dlopen(const char*, int);
extern int __real_sh_linker_init(void);

void *__wrap_dlopen(const char* f, int fl) {
    if (f && g_path_ok) {
        const char* b = strrchr(f, '/'); if (!b) b = f; else b++;
        if (strcmp(b, "libshadowhook_nothing.so") == 0)
            return __real_dlopen(g_path, fl);
    }
    return __real_dlopen(f, fl);
}

int __wrap_sh_linker_init(void) {
    int r = __real_sh_linker_init();
    if (r) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "linker_init fail %d", r);
    return 0;
}

__attribute__((noinline)) static void save_path(void) {
    Dl_info i;
    if (dladdr((void*)save_path, &i) && i.dli_fname) {
        size_t l = strlen(i.dli_fname);
        if (l < sizeof(g_path) - 1) { memcpy(g_path, i.dli_fname, l+1); g_path_ok = true; }
    }
}

// ─── Output ───────────────────────────────────────────────────────────

static void hex(const char* tag, const uint8_t* d, int len) {
    char b[128];
    for (int i = 0; i < len && i < 100; i++) {
        if (i % 32 == 0) { if (i) { b[96] = 0; LOGI("%s%s", tag, b); } snprintf(b, 128, "  "); }
        snprintf(b + strlen(b), 128 - strlen(b), "%02x", d[i]);
    }
    if (len > 100) LOGI("%s... (%d bytes)", tag, len);
}

static void ascii(const uint8_t* d, int len) {
    int lim = len > 1024 ? 1024 : len;
    char b[1100]; int pos = 0;
    for (int i = 0; i < lim; i++) {
        if (d[i] >= 32 && d[i] < 127) b[pos++] = (char)d[i];
        else if (d[i] == '\n') { b[pos++] = '\\'; b[pos++] = 'n'; }
        else if (d[i] == '\r') { b[pos++] = '\\'; b[pos++] = 'r'; }
        else if (d[i] == '"' || d[i] == '\\') { b[pos++] = '\\'; b[pos++] = (char)d[i]; }
        else if (d[i] == '\t') { b[pos++] = '\\'; b[pos++] = 't'; }
        else b[pos++] = '.';
    }
    b[pos] = 0;
    LOGI("  body: %s", b);
    if (len > 1024) LOGI("  ... +%d bytes", len - 1024);
}

static void dump_body(const uint8_t* d, int len, const char* endpoint) {
    // Skip HTTP headers
    const uint8_t* body = d;
    int blen = len;
    uint8_t* h = (uint8_t*)memmem(d, (size_t)len, "\r\n\r\n", 4);
    if (h) { body = h + 4; blen = len - (int)(body - d); }

    // Skip protobuf prefix (FFFF + session)
    const uint8_t* json = body;
    int jlen = blen;
    if (blen > 4 && body[0] == 0xFF && body[1] == 0xFF) {
        // Skip FFFF header + session bytes
        ascii(body, blen > 200 ? 200 : blen);
        return;
    }

    LOGI("  body[%d]: %.*s", jlen, jlen > 500 ? 500 : jlen, json);
}

// ─── SSL Hooks ────────────────────────────────────────────────────────

static int my_SSL_read(void* s, void* b, int n) {
    if (g_re) return orig_SSL_read(s, b, n);
    g_re = 1;
    int r = orig_SSL_read(s, b, n);
    if (r > 0 && memcmp(b, "HTTP/", 5) == 0) {
        LOGI("<<< RESPONSE %d bytes ===", r);
        ascii(b, r > 300 ? 300 : r);
    }
    g_re = 0; return r;
}

static int my_SSL_write(void* s, const void* b, int n) {
    if (g_re) return orig_SSL_write(s, b, n);
    g_re = 1;

    if (n > 6 && memcmp(b, "POST /", 6) == 0) {
        // Extract path
        const char* start = (const char*)b + 5;
        const char* end = (const char*)memmem(start, (size_t)(n - 5), " ", 1);
        char path[128] = {0};
        int plen = end ? (int)(end - start) : 0;
        if (plen > 0 && plen < 127) { memcpy(path, start, (size_t)plen); path[plen] = 0; }

        LOGI(">>> %s ===", path);

        // Cari body
        uint8_t* h = (uint8_t*)memmem(b, (size_t)n, "\r\n\r\n", 4);
        if (h) {
            uint8_t* body = h + 4;
            int blen = n - (int)(body - (uint8_t*)b);
            if (blen > 0) {
                // Check if it's JSON or protobuf
                bool is_json = blen > 2 && body[0] == '{';
                bool is_protobuf = blen > 2 && body[0] == 0xFF && body[1] == 0xFF;

                if (is_json) {
                    dump_body(b, n, path);
                } else if (is_protobuf) {
                    LOGI("  protobuf[%d]:", blen);
                    // Show first bytes as hex
                    hex("  ", body, blen > 64 ? 64 : blen);
                    // Also show ASCII
                    ascii(body, blen > 300 ? 300 : blen);
                } else {
                    // Unknown format, show hex
                    LOGI("  raw[%d]:", blen);
                    hex("  ", body, blen > 64 ? 64 : blen);
                }
            }
        }
    }

    int r = orig_SSL_write(s, b, n);
    g_re = 0; return r;
}

// ─── Init ─────────────────────────────────────────────────────────────

static void* init(void* a) {
    (void)a;
    usleep(500000);
    save_path();

    int r = memkit_hook_init(SHADOWHOOK_MODE_UNIQUE, false);
    if (r) { LOGI("Hook init fail: %d", r); return NULL; }

    LOGI("=== Lucid Ready ===");

    ssl_r_stub = memkit_hook_by_symbol("libssl.so", "SSL_read", (void*)my_SSL_read, (void**)&orig_SSL_read);
    ssl_w_stub = memkit_hook_by_symbol("libssl.so", "SSL_write", (void*)my_SSL_write, (void**)&orig_SSL_write);
    LOGI("  SSL_R:%s  SSL_W:%s", ssl_r_stub?"OK":"FAIL", ssl_w_stub?"OK":"FAIL");

    return NULL;
}

__attribute__((constructor, visibility("default")))
static void on_load(void) {
    pthread_t t; pthread_attr_t a;
    pthread_attr_init(&a); pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &a, init, NULL); pthread_attr_destroy(&a);
}
