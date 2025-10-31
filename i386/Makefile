CROSS_COMPILE ?= i686-elf-
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIP = $(CROSS_COMPILE)strip
SIZE = $(CROSS_COMPILE)size
ASM = $(CROSS_COMPILE)nasm
#CFLAGS = -march=x86-64

MACHINE = pc

CFLAGS = -m32 

TARGET = kernel
SRC_DIRS = . lib $(ARCH)
SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
ASMS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.S))
OBJS = $(SRCS:%.c=build/%.o) $(ASMS:%.S=build/%.o)

INC_DIRS = .
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

# 编译标志
CFLAGS += -O0 -g
CFLAGS += -Wall -Wextra -Wpedantic -Wno-int-conversion
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(INC_FLAGS)
CFLAGS += -std=c11
CFLAGS += -fno-builtin-printf

LDFLAGS += -Wl,--gc-sections
#LDFLAGS += -lgcc -lc -lm 
LDFLAGS += -nostdlib -nostartfiles -ffreestanding 
LDFLAGS += -T $(ARCH)/link.ld

#LIBS = -lc -lm -lnosys

all: build/$(TARGET).elf build/mbr build/$(TARGET).bin  build/$(TARGET).asm size

build/mbr:
	cp $(ARCH)/mbr build/mbr

build/$(TARGET).elf: $(OBJS) | build_dirs
	$(CC) -Wl,-Map=build/$(TARGET).map $(LDFLAGS) $(OBJS) $(LIBS)  -o $@

build/$(TARGET).bin: build/$(TARGET).elf
	@$(OBJCOPY) -O binary $< $@
	dd if=/dev/zero of=build/boot.img bs=1M count=1
	dd if=build/mbr of=build/boot.img bs=512  conv=notrunc
	dd if=build/kernel.bin of=build/boot.img  bs=512 seek=1 conv=notrunc

build/$(TARGET).asm:  build/$(TARGET).elf
	@$(OBJDUMP) -d $< >$@

build/%.o: %.c | build_dirs
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) $< -o $@

build/%.o: %.S | build_dirs
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

build_dirs:
	@mkdir -p $(addprefix build/,$(SRC_DIRS))

size: build/$(TARGET).elf
	@$(SIZE) $<

clean:
	@rm -rf build

run: build/$(TARGET).elf
	qemu-system-$(ARCH) -machine $(MACHINE) -m 128M -serial mon:stdio -drive format=raw,file=build/boot.img

.PHONY: all clean size build_dirs
