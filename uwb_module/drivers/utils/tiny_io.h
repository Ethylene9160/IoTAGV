#ifndef UWB_MODULE_TINY_IO_H
#define UWB_MODULE_TINY_IO_H

#ifdef __cplusplus
extern "C" {
#endif

int debug_putchar(int ch);

void int_to_str(int value, char *buffer);
void float_to_str(float value, char *buffer, int precision);

void debug_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif