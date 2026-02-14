# Register Magic Number Test
.section .text
.globl _start
_start:
    li x28, 0xBEEF
    ecall
