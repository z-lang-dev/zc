    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+ct0]
    call read_file
    pop rbp
    xor rax, rax
    ret
ct0:
    .asciz "hello.txt"
