#include <stdarg.h>
#include <errno.h>
#include "memkit.h"
#include "shadowhook.h"

// ============================================================================
// HOOKING V2: BY SYMBOL NAME WITH FLAGS
// Note: shadowhook_hook_sym_name_2 does NOT take variadic record params
// in the current ShadowHook API — record params are only for _2 variants
// of hook_func_addr, hook_sym_addr, and intercept functions.
// ============================================================================

void *memkit_hook_v2(const char *lib_name, const char *sym_name, void *new_addr, void **orig_addr, uint32_t flags) {
    if (!lib_name || !sym_name || !new_addr) {
        errno = EINVAL;
        return NULL;
    }

    void *stub = shadowhook_hook_sym_name_2(lib_name, sym_name, new_addr, orig_addr, flags);

    if (!stub) {
        errno = shadowhook_get_errno();
        if (orig_addr) *orig_addr = NULL;
        return NULL;
    }
    return stub;
}

void *memkit_hook_by_symbol_v2(const char *lib_name, const char *sym_name, void *new_addr, void **orig_addr, uint32_t flags) {
    /* Same implementation as memkit_hook_v2 — ShadowHook's _2 API has same signature for sym_name */
    return memkit_hook_v2(lib_name, sym_name, new_addr, orig_addr, flags);
}

// ============================================================================
// HOOKING V2: BY FUNCTION ADDRESS WITH FLAGS (variadic for RECORD)
// ============================================================================

void *memkit_hook_func_addr_2(void *func_addr, void *new_addr, void **orig_addr, uint32_t flags, ...) {
    if (!func_addr || !new_addr) {
        errno = EINVAL;
        return NULL;
    }

    void *stub;

    if (flags & MK_HOOK_RECORD) {
        va_list ap;
        va_start(ap, flags);
        const char *record_lib_name = va_arg(ap, const char *);
        const char *record_sym_name = va_arg(ap, const char *);
        va_end(ap);
        stub = shadowhook_hook_func_addr_2(func_addr, new_addr, orig_addr, flags, record_lib_name, record_sym_name);
    } else {
        stub = shadowhook_hook_func_addr_2(func_addr, new_addr, orig_addr, flags);
    }

    if (!stub) {
        errno = shadowhook_get_errno();
        if (orig_addr) *orig_addr = NULL;
        return NULL;
    }
    return stub;
}

// ============================================================================
// HOOKING V2: BY SYMBOL ADDRESS WITH FLAGS (variadic for RECORD)
// ============================================================================

void *memkit_hook_sym_addr_2(void *sym_addr, void *new_addr, void **orig_addr, uint32_t flags, ...) {
    if (!sym_addr || !new_addr) {
        errno = EINVAL;
        return NULL;
    }

    void *stub;

    if (flags & MK_HOOK_RECORD) {
        va_list ap;
        va_start(ap, flags);
        const char *record_lib_name = va_arg(ap, const char *);
        const char *record_sym_name = va_arg(ap, const char *);
        va_end(ap);
        stub = shadowhook_hook_sym_addr_2(sym_addr, new_addr, orig_addr, flags, record_lib_name, record_sym_name);
    } else {
        stub = shadowhook_hook_sym_addr_2(sym_addr, new_addr, orig_addr, flags);
    }

    if (!stub) {
        errno = shadowhook_get_errno();
        if (orig_addr) *orig_addr = NULL;
        return NULL;
    }
    return stub;
}

// ============================================================================
// HOOKING V2: BY SYMBOL NAME WITH CALLBACK AND FLAGS
// ============================================================================

void *memkit_hook_sym_name_callback_2(const char *lib_name, const char *sym_name, void *new_addr, void **orig_addr, uint32_t flags, MemKitHooked hooked, void *hooked_arg) {
    if (!lib_name || !sym_name || !new_addr) {
        errno = EINVAL;
        return NULL;
    }

    void *stub = shadowhook_hook_sym_name_callback_2(lib_name, sym_name, new_addr, orig_addr, flags, (shadowhook_hooked_t)hooked, hooked_arg);

    if (!stub) {
        errno = shadowhook_get_errno();
        if (orig_addr) *orig_addr = NULL;
        return NULL;
    }
    return stub;
}
