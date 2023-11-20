    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+arg0]
    call read_file
    pop rbp
    xor rax, rax
    ret
arg0:
    .asciz "hello.txt"
