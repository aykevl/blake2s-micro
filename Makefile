
CFLAGS = -Wall -Werror -Os -flto -g -std=c99 -fno-unroll-loops
CFLAGS_ARM = $(CFLAGS) -mcpu=cortex-m0 -mthumb -Wl,--gc-sections -nostdlib -T test-mcu.ld -e blake2s_blocks

.PHONY: all run clean

all: build/test build/sizetest

run: build/test
	@$<

clean:
	rm -rf build

build/test: blake2s.c test.c
	@mkdir -p build
	$(CC) $(CFLAGS) -DDEBUG -o $@ $^

build/sizetest: blake2s.c test-stdlib.c
	@mkdir -p build
	@arm-none-eabi-gcc $(CFLAGS_ARM) -o $@ $^
	@arm-none-eabi-size $@

build/sizetest-noinline: blake2s.c test-stdlib.c
	@mkdir -p build
	@arm-none-eabi-gcc $(CFLAGS_ARM) -fno-inline -o $@ $^
	@arm-none-eabi-size $@
