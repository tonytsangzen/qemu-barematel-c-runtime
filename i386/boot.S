.set KERNEL_VIRTUAL_BASE, 0xC0000000
.set KERNEL_PAGE_TABLE,  0x100000

.code32
.section .text

.global _entry
_entry:
    # Clear direction flag
    cld

    # Print booting message
    mov $msg, %esi
    call early_print

    # Set up stack pointer
    mov $stack_top, %esp

    call mmu_init
	jmp main

# Simple early print function (assumes VGA text buffer at 0xB8000)
early_print:
    mov $0xB8000, %edi
    mov $0x0F, %ah       # White on black

.print_char:
    lodsb
    test %al, %al
    jz .print_done
    stosw
    jmp .print_char

.print_done:
    ret

.section .data
msg: .asciz "booting..."

.section .bss
.align 4096
pagedir: .space 4096     # Page directory (1024 entries)
stack_bottom: .space 4096 * 4
stack_top:
