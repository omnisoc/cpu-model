.section .text
.globl _start
_start:
    li x1, 100      # 加载立即数 100
    li x2, 50       # 加载立即数 50
    add x3, x1, x2  # x3 = x1 + x2 (期望结果 150)
    addi x3, x3, 5  # x3 = x3 + 5 (期望结果 155)
    ecall           # 系统调用，触发模拟器退出

