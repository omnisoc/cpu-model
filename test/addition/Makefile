# Test: Addition Test
ASM = riscv64-unknown-elf-as
LD = riscv64-unknown-elf-ld

OBJ = $(BUILD_DIR)/addition_add.o
ELF = $(BUILD_DIR)/addition_add.elf

all: $(ELF)

$(OBJ): add.s
	$(ASM) -o $@ $<

$(ELF): $(OBJ)
	$(LD) -Ttext=0x10000 -o $@ $^

clean:
	rm -f $(OBJ) $(ELF)
