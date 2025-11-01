#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

extern int uart_putc(char ch);

void int_to_str(int num, char* buffer, int base) {
    char temp[32];
    int i = 0, j = 0;
    int is_negative = 0;
    
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
   
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }
    
    while (num != 0) {
        int remainder = num % base;
        temp[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
        num = num / base;
    }
    
    if (is_negative) {
        temp[i++] = '-';
    }
    
    // 反转字符串
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

int early_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int chars_printed = 0;
    const char* p = format;
    
    while (*p != '\0') {
        if (*p != '%') {
            uart_putc(*p);
            chars_printed++;
            p++;
            continue;
        }
        
        // 处理格式说明符
        p++; // 跳过 '%'
        
        switch (*p) {
            case 'd': // 十进制整数
                {
                    int num = va_arg(args, int);
                    char buffer[32];
                    int_to_str(num, buffer, 10);
                    for (int i = 0; buffer[i] != '\0'; i++) {
                        uart_putc(buffer[i]);
                        chars_printed++;
                    }
                }
                break;
                
            case 'c': // 字符
                {
                    char ch = (char)va_arg(args, int);
                    uart_putc(ch);
                    chars_printed++;
                }
                break;
                
            case 's': // 字符串
                {
                    char* str = va_arg(args, char*);
                    for (int i = 0; str[i] != '\0'; i++) {
                        uart_putc(str[i]);
                        chars_printed++;
                    }
                }
                break;
                
            case 'x': // 十六进制整数（小写）
                {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int_to_str(num, buffer, 16);
                    for (int i = 0; buffer[i] != '\0'; i++) {
                        uart_putc(buffer[i]);
                        chars_printed++;
                    }
                }
                break;
                
            case 'X': // 十六进制整数（大写）
                {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int_to_str(num, buffer, 16);
                    for (int i = 0; buffer[i] != '\0'; i++) {
                        uart_putc(toupper(buffer[i]));
                        chars_printed++;
                    }
                }
                break;
                
            case '%': // 输出%字符本身
                uart_putc('%');
                chars_printed++;
                break;
                
            default: // 未知格式符，原样输出
                uart_putc('%');
                uart_putc(*p);
                chars_printed += 2;
                break;
        }
        p++;
    }
    
    va_end(args);
    return chars_printed;
}

