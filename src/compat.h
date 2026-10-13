#ifndef COMPAT_H
#define COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Compatibility layer for linking raylib on MinGW with MSVCRT */
void init_compat_layer(void);

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_H */
