# Basic Arithmetic Test
.section .text
.globl _start
_start:
    li x1, 100
    li x2, 50
    add x3, x1, x2
    addi x3, x3, 5
    sub x4, x3, x1
    ecall
