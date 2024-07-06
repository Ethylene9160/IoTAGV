#ifndef UWB_MODULE_TINY_IO_H
#define UWB_MODULE_TINY_IO_H

#ifdef __cplusplus
extern "C" {
#endif

int debug_putchar(int ch);
void debug_printf(const char *format, ...); /* %s, %d, %ld, %Ld, %u, %lu, %Lu, %.xf, %.xlf (%.xl ?) */

#ifdef __cplusplus
}
#endif

#endif