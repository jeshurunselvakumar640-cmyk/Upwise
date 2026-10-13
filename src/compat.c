#include "compat.h"
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

#if defined(__MINGW32__) && !defined(__MINGW64__)

int __mingw_vsscanf(const char *s, const char *fmt, va_list ap) {
    return vsscanf(s, fmt, ap);
}

void sincosf(float x, float *s, float *c) {
    *s = sinf(x);
    *c = cosf(x);
}

void sincos(double x, double *s, double *c) {
    *s = sin(x);
    *c = cos(x);
}

static FILE *my_acrt_iob_func(unsigned i) {
    if (i == 0) return stdin;
    if (i == 1) return stdout;
    if (i == 2) return stderr;
    return NULL;
}

FILE *(* const _acrt_iob_func_ptr)(unsigned) __asm__("__imp____acrt_iob_func") = my_acrt_iob_func;

#endif

void init_compat_layer(void) {
    /* No-op, forces symbol inclusion */
}
