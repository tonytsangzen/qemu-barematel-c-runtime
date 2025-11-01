ENTRY(_entry)

MEMORY {
    RAM (rwx) : ORIGIN = 0x40080000, LENGTH = 1M
}

PHDRS {
    text PT_LOAD FLAGS(5); 
    data PT_LOAD FLAGS(6); 
    rodata PT_LOAD FLAGS(4);
}

SECTIONS {
    . = ALIGN(16);
    .text : { 
        KEEP(*(.init))
        *(.text*) 
    }  > RAM : text

    . = ALIGN(16);
    .rodata : {
        *(.rodata*)
    } > RAM :rodata 

    . = ALIGN(16);
    .data : { 
        *(.data*) 
    }  > RAM :data

    . = ALIGN(16);
    .bss : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > RAM :data
    
    . = ALIGN(4096);
    _stack_top = .;
    . = . + 0x10000; /*64KB*/
    _stack_end = .;
    
    . = ALIGN(4096);
    _heap_start = .;
    . = . + 0x10000; /*64KB*/
    _heap_end = .;

    . = ALIGN(4096);
    . = . + 0x100000; /*1MB*/
    
    . = ALIGN(4096);
    _page_tbl_start = .;
    . = . + 0x1000000; /*16MB*/
    _page_tbl_end = .;
}
