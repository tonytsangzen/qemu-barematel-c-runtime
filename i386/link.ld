ENTRY(_entry)

MEMORY {
    RAM (rwx) : ORIGIN = 0x100000, LENGTH = 1M
}

PHDRS {
    text PT_LOAD FLAGS(5); 
    data PT_LOAD FLAGS(6); 
    rodata PT_LOAD FLAGS(4);
}

SECTIONS
{
    .text : {
		KEEP(*(.init))
        *(.text)
        *(.text.*)
    } > RAM : text

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }  > RAM :rodata

    .data : {
        *(.data)
        *(.data.*)
    } > RAM :data

    .bss : { 
        _sbss = .;
        *(.bss*)
        _ebss = .;
    } > RAM
    
    . = ALIGN(4096);
    _stack_top = .;
    . = . + 0x10000; /*64KB*/
    _stack_end = .;
    
    . = ALIGN(4096);
    _heap_start = .;
    . = . + 0x10000; /*64KB*/
    _heap_end = .;
}
