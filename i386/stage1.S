# boot.S - x86实模式启动代码，加载内核并进入保护模式

.code16                         # 16位实模式代码
.section .text
.globl _start
_start:
    jmp real_start

# BIOS参数块（BPB），确保引导扇区被识别
.skip 62, 0                     # 填充BPB区域（实际引导扇区需要更完整的BPB）

real_start:
    # 设置段寄存器
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw $0x7C00, %sp           # 设置栈指针

    # 打印消息 "Load Kernel..."
    movw $msg, %si
    call print_string

    # 从硬盘读取128KB（256个扇区）
    movw $0x8000, %bx           # 临时缓冲区地址 ES:BX = 0:0x8000
    movw $256, %cx              # 要读取的扇区数 (128KB / 512B = 256)
    movw $1, %ax                # 起始扇区号（LBA，从0开始计数）
    call read_sectors

    # 准备进入保护模式
    cli                         # 禁用中断
    lgdt gdt_desc               # 加载GDT
    movl %cr0, %eax
    orl $0x1, %eax              # 设置PE位
    movl %eax, %cr0             # 进入保护模式

    # 远跳转以刷新流水线
    ljmp $0x08, $pm_start       # 使用代码段选择子

.code32
pm_start:
    # 设置保护模式段寄存器
    movw $0x10, %ax             # 数据段选择子
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $0x9FFFFF, %esp        # 设置栈指针（根据需要调整）

    # 将内核从临时缓冲区复制到1MB处
    movl $0x8000, %esi          # 源地址
    movl $0x100000, %edi        # 目标地址 (1MB)
    movl $0x20000, %ecx         # 128KB = 0x20000字节
    rep movsb                   # 复制内存

    # 跳转到内核
    ljmp $0x08, $0x100000       # 跳转到1MB处执行

# 16位实模式函数
.code16
print_string:
    lodsb                       # 加载SI指向的字节到AL
    orb %al, %al                # 检查是否为NULL字符
    jz print_done
    movb $0x0E, %ah             # BIOS打印字符功能
    int $0x10                   # 调用BIOS中断
    jmp print_string
print_done:
    ret

read_sectors:
    # 参数: AX=起始LBA, CX=扇区数, ES:BX=缓冲区地址
    pushal
    movw %cx, %bp               # 保存扇区数

    # 转换为CHS地址（简化版，假设LBA < 1024）
    xorw %dx, %dx
    movw $18, %di               # 每磁道扇区数
    divw %di                    # AX = 磁道, DX = 扇区-1
    movb %dl, %cl               # CL[0-5] = 扇区
    incb %cl
    xorw %dx, %dx
    movw $2, %di                # 磁头数
    divw %di                    # AX = 柱面, DX = 磁头
    shlb $6, %ah                # 柱面高2位
    orb %ah, %cl                # CL[6-7] = 柱面高2位
    movb %dl, %dh               # DH = 磁头号
    movb %al, %ch               # CH = 柱面低8位

    movw $0x0201, %ax           # AH=02h(读磁盘), AL=1(每次读1个扇区)
    movb $0x80, %dl             # DL=80h(第一硬盘)

read_loop:
    int $0x13                   # 调用BIOS中断
    jc disk_error               # 如果出错
    addw $512, %bx              # 移动缓冲区指针
    jnc no_inc_es
    movw %es, %ax
    addw $0x1000, %ax
    movw %ax, %es
no_inc_es:
    incw -2(%bp)                # 递增LBA
    loop read_loop              # 循环直到读完所有扇区
    popal
    ret

disk_error:
    movw $error_msg, %si
    call print_string
    jmp .

# 数据段
msg: .asciz "Load Kernel..."
error_msg: .asciz "Disk Error!"

# GDT定义
.p2align 2
gdt:
    .quad 0x0000000000000000     # 空描述符
    .quad 0x00CF9A000000FFFF     # 代码段 (0-4GB, 可读/执行)
    .quad 0x00CF92000000FFFF     # 数据段 (0-4GB, 可读/写)
gdt_desc:
    .word gdt_desc - gdt - 1     # GDT界限
    .long gdt                    # GDT基址

# 引导扇区签名
.org 510
.word 0xAA55
