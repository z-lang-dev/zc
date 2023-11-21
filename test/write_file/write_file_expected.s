    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+ct0]
    lea rsi, [rip+ct1]
    call write_file
    pop rbp
    xor rax, rax
    ret
ct0:
    .asciz "hello.tmp"
ct1:
    .asciz "HELLO\n"
