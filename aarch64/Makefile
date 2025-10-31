CPU = cortex-a57
CROSS_COMPILE ?= aarch64-none-elf-
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIP = $(CROSS_COMPILE)strip
SIZE = $(CROSS_COMPILE)size

TARGET = kernel
SRC_DIRS = . lib $(ARCH)
SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
ASMS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.S))
OBJS = $(SRCS:%.c=build/%.o) $(ASMS:%.S=build/%.o)

INC_DIRS = .
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

# 编译标志
CFLAGS = -mcpu=$(CPU) -O0 -g
CFLAGS += -Wall -Wextra -Wpedantic -Wno-int-conversion
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += $(INC_FLAGS)
CFLAGS += -std=c11
CFLAGS += -fno-builtin-printf
CFLAGS += -falign-functions=16 

LDFLAGS = -mcpu=$(CPU)
LDFLAGS += -Wl,--gc-sections -lgcc -lc -lm 
LDFLAGS += -nostdlib -nostartfiles -ffreestanding -specs=nosys.specs
LDFLAGS += -T $(ARCH)/link.ld

LIBS = -lc -lm -lnosys

all: build/$(TARGET).elf build/$(TARGET).bin  build/$(TARGET).asm size

build/$(TARGET).elf: $(OBJS) | build_dirs
	$(CC) -Wl,-Map=build/$(TARGET).map $(LDFLAGS) $(OBJS) $(LIBS)  -o $@

build/$(TARGET).bin: build/$(TARGET).elf
	@$(OBJCOPY) -O binary $< $@

build/$(TARGET).asm:  build/$(TARGET).elf
	@$(OBJDUMP) -d $< >$@

build/%.o: %.c | build_dirs
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) $< -o $@

build/%.o: %.S | build_dirs
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) $< -o $@

build_dirs:
	@mkdir -p $(addprefix build/,$(SRC_DIRS))

size: build/$(TARGET).elf
	@$(SIZE) $<

clean:
	@rm -rf build

run: build/$(TARGET).elf
	qemu-system-aarch64 -machine virt -m 128M -cpu $(CPU) -serial mon:stdio -nographic -kernel build/$(TARGET).bin

.PHONY: all clean size build_dirs
