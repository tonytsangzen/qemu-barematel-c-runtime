#include <stdio.h>
#include <stdint.h>

extern int uart_putc(char ch);

int main(void) {
	printf("hello c-runtime\n");
	while(1);
	return 0;
}
