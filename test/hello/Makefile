# Test: Hello World Test
ASM = riscv64-unknown-elf-as
LD = riscv64-unknown-elf-ld

OBJ = $(BUILD_DIR)/hello_hello.o
ELF = $(BUILD_DIR)/hello_hello.elf

all: $(ELF)

$(OBJ): hello.s
	$(ASM) -o $@ $<

$(ELF): $(OBJ)
	$(LD) -Ttext=0x10000 -o $@ $^

clean:
	rm -f $(OBJ) $(ELF)
