#include "tiny_io.h"

#include <stdbool.h>
#include "stdarg.h"

#include "usart.h"


int debug_putchar(int ch) {
    USART_ClearFlag(USART1, USART_FLAG_TC);
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // Block until the transfer is complete
    return ch;
}

void int_to_str(int64_t value, char *buffer) {
    char temp[21];  // Can hold up to 20 digits for a 64-bit integer
    int pos = 0;
    int len = 0;
    bool is_negative = false;

    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    do {
        temp[pos++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        temp[pos++] = '-';
    }

    while (pos > 0) {
        buffer[len++] = temp[--pos];
    }

    buffer[len] = '\0';
}

void uint_to_str(uint64_t value, char *buffer) {
    char temp[21];  // Can hold up to 20 digits for a 64-bit integer
    int pos = 0;
    int len = 0;

    do {
        temp[pos++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    while (pos > 0) {
        buffer[len++] = temp[--pos];
    }

    buffer[len] = '\0';
}

void float_to_str(double value, char *buffer, int precision) {
    char temp[32];
    int pos = 0;
    int len = 0;
    bool is_negative = false;
    int int_part;
    double frac_part;

    if (value < 0) {
        is_negative = true;
        value = -value;
    }

    int_part = (int)value;
    frac_part = value - int_part;

    int_to_str(int_part, temp);

    while (temp[pos] != '\0') {
        buffer[len++] = temp[pos++];
    }

    buffer[len++] = '.';

    for (int i = 0; i < precision; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        buffer[len++] = digit + '0';
        frac_part -= digit;
    }

    buffer[len] = '\0';
}

void debug_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    const char *ptr = format;
    char buffer[32];

    while (*ptr != '\0') {
        if (*ptr == '%') {
            ptr++;
            if (*ptr == 'd') {
                int value = va_arg(args, int);
                int_to_str(value, buffer);
                for (char *buf_ptr = buffer; *buf_ptr != '\0'; buf_ptr++) {
                    debug_putchar(*buf_ptr);
                }
            } else if (*ptr == 'l') {
                ptr++;
                if (*ptr == 'd') {
                    long value = va_arg(args, long);
                    int_to_str(value, buffer);
                } else if (*ptr == 'u') {
                    unsigned long value = va_arg(args, unsigned long);
                    uint_to_str(value, buffer);
                }
                for (char *buf_ptr = buffer; *buf_ptr != '\0'; buf_ptr++) {
                    debug_putchar(*buf_ptr);
                }
            } else if (*ptr == 'L') {
                ptr++;
                if (*ptr == 'd') {
                    long long value = va_arg(args, long long);
                    int_to_str(value, buffer);
                } else if (*ptr == 'u') {
                    unsigned long long value = va_arg(args, unsigned long long);
                    uint_to_str(value, buffer);
                }
                for (char *buf_ptr = buffer; *buf_ptr != '\0'; buf_ptr++) {
                    debug_putchar(*buf_ptr);
                }
            } else if (*ptr == 'u') {
                unsigned int value = va_arg(args, unsigned int);
                uint_to_str(value, buffer);
                for (char *buf_ptr = buffer; *buf_ptr != '\0'; buf_ptr++) {
                    debug_putchar(*buf_ptr);
                }
            } else if (*ptr == '.') {
                ptr++;
                int precision = *ptr - '0';
                ptr++;
                if (*ptr == 'f' || *ptr == 'l') {
                    double value = va_arg(args, double);
                    float_to_str(value, buffer, precision);
                    for (char *buf_ptr = buffer; *buf_ptr != '\0'; buf_ptr++) {
                        debug_putchar(*buf_ptr);
                    }
                }
            } else {
                debug_putchar('%');
                debug_putchar(*ptr);
            }
        } else {
            debug_putchar(*ptr);
        }
        ptr++;
    }

    va_end(args);
}
