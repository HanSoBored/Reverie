#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "memkit.h"

// ─── CONFIG ──────────────────────────────────────────────────────────
// Edit these for your target application/library.
#define TARGET_LIB  "libtarget.so"
#define LOG_TAG     "Edge"

// ─── LOGGING ─────────────────────────────────────────────────────────
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ─── TARGET HOOKS ────────────────────────────────────────────────────
// Define your original function pointer + proxy for each target function.
// Two common patterns are shown below.

// Pattern A: void function — block/no-op the original behavior
static void (*orig_func_void)(void*) = NULL;
static void my_func_void(void* _this) {
    (void)_this;
}

// Pattern B: float getter — override the return value
static float (*orig_func_float)(const void*) = NULL;
static float my_func_float(const void* _this) {
    (void)_this;
    return 0.0f;
}

// ─── HOOK ENTRY TABLE ────────────────────────────────────────────────
// Add your target RVAs here. The init loop will walk this array and hook
// every entry automatically. The stub is stored for unhooking later if
// needed (currently unused but kept for reference).
static uintptr_t g_lib_base = 0;

typedef struct {
    const char* name;     // human-readable label for logging
    uintptr_t   rva;      // offset from library base
    void*       proxy;    // replacement function
    void**      orig;     // output: original function pointer
} HookEntry;

static HookEntry g_hooks[] = {
    { "func_void_01",  0x000000UL, (void*)my_func_void,   (void**)&orig_func_void   },
    { "func_float_01", 0x000000UL, (void*)my_func_float,  (void**)&orig_func_float  },
};

static const int g_hook_count = sizeof(g_hooks) / sizeof(g_hooks[0]);

// ─── HOOK HELPER ─────────────────────────────────────────────────────
static void hook_at(const HookEntry* e) {
    uintptr_t addr = g_lib_base + e->rva;
    void* stub = memkit_hook((void*)addr, e->proxy, e->orig);
    LOGI("  %s @ 0x%06lX → %s", e->name, e->rva, stub ? "OK" : "FAIL");
}

// ─── INIT ────────────────────────────────────────────────────────────
static void* init(void* arg) {
    (void)arg;

    int r = memkit_hook_init(SHADOWHOOK_MODE_UNIQUE, false);
    if (r) {
        LOGE("memkit_hook_init fail: %d", r);
        return NULL;
    }

    LOGI("Waiting for %s...", TARGET_LIB);
    while (!(g_lib_base = memkit_get_lib_base_v2(TARGET_LIB)))
        usleep(500000);

    LOGI("%s loaded at 0x%" PRIxPTR, TARGET_LIB, g_lib_base);

    usleep(2000000);

    for (int i = 0; i < g_hook_count; i++)
        hook_at(&g_hooks[i]);

    LOGI("=== Edge Ready ===");
    return NULL;
}

__attribute__((constructor, visibility("default")))
static void on_load(void) {
    pthread_t t;
    pthread_attr_t a;
    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &a, init, NULL);
    pthread_attr_destroy(&a);
}
