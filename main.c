#include <stdio.h>
#include <stdint.h>

int main(void) {
	extern int uart_putc(char ch);
	uart_putc('m');
	printf("hello c-runtime\n");
	uart_putc('h');
	while(1);
	return 0;
}
