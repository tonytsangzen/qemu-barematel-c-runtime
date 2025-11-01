#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char _heap_start;
extern char _heap_end;
static char *current_heap_end = &_heap_start;
extern void uart_putc(char c);

__attribute__((optimize("O0")))
void *_sbrk(int incr)
{
    uart_putc('s');
    char *prev_heap_end = current_heap_end;

    if (current_heap_end + incr > &_heap_end)
    {
        return (void *)-1; /* 堆溢出 */
    }

    current_heap_end += incr;
    return (void *)prev_heap_end;
}

__attribute__((optimize("O0")))
int _close(int file)
{
    uart_putc('c');
    (void)file;
    return -1;
}

__attribute__((optimize("O0")))
int _fstat(int file, struct stat *st)
{
    uart_putc('f');
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

__attribute__((optimize("O0")))
int _isatty(int file) { 
    uart_putc('t');
    (void)file;
    return 1; 
}

__attribute__((optimize("O0")))
int _lseek(int file, int ptr, int dir) { 
    (void)file;
    (void)ptr;
    (void)dir;
    return 0; 
}

__attribute__((optimize("O0")))
int _open(const char *name, int flags, int mode) {
    uart_putc('o');
    (void)name;
    (void)flags;
    (void)mode;
    return -1; 
}

__attribute__((optimize("O0")))
int _read(int file, char *ptr, int len)
{
    uart_putc('r');
    (void)file;
    (void)ptr;
    (void)len;
    return 0; 
}

__attribute__((optimize("O0")))
int _write(int file, char *ptr, int len)
{
    uart_putc('w');
    (void)file;
    for (int i = 0; i < len; i++)
    {
        uart_putc(ptr[i]);
    }
    return len;
}

void _exit(int status)
{
    (void)status;
    while (1);
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void) { 
    return 1; 
}

double __trunctfdf2(long double a) {
    return (double)a;
}
