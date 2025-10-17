#include <stdint.h>

/*for qemu virt*/
#define UART0 ((0x09000000))

/* serial port register offsets */
#define UART_DATA 0x00
#define UART_FLAGS 0x18
#define UART_INT_ENABLE 0x0e
#define UART_INT_TARGET 0x0f
#define UART_INT_CLEAR 0x11

/* serial port bitmasks */
#define UART_RECEIVE 0x10
#define UART_TRANSMIT 0x20

#define get32(addr) (*((volatile uint32_t *)(addr)))
#define put32(addr, val) (*((volatile uint32_t *)(addr)) = (uint32_t)(val))

int uart_putc(char ch)
{
    while (get32(UART0 + UART_FLAGS) & UART_TRANSMIT)
        ;
    /* write the character */
    put32(UART0 + UART_DATA, ch);
    return ch;
}
