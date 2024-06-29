#include "tiny_io.h"

#include "stdarg.h"
#include "usart.h"


int debug_putchar(int ch) {
    USART_ClearFlag(USART1,USART_FLAG_TC);
    USART_SendData(USART1, (uint8_t) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); // Block until the transfer is complete
    return ch;
}

void int_to_str(int value, char *buffer) {
    char temp[12];
    int pos = 0;
    int len = 0;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
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

void float_to_str(float value, char *buffer, int precision) {
    char temp[32];
    int pos = 0;
    int len = 0;
    int is_negative = 0;
    int int_part;
    float frac_part;

    if (value < 0) {
        is_negative = 1;
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
            } else if (*ptr == 's') {
                char *str = va_arg(args, char *);
                while (*str != '\0') {
                    debug_putchar(*str++);
                }
            } else if (*ptr == '.') {
                ptr++;
                int precision = *ptr - '0';
                ptr++;
                if (*ptr == 'f') {
                    float value = (float)va_arg(args, double);
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
