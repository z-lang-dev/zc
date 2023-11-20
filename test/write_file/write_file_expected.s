    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    lea rdi, [rip+arg0]
    lea rsi, [rip+arg1]
    call write_file
    pop rbp
    xor rax, rax
    ret
arg0:
    .asciz "hello.tmp"
arg1:
    .asciz "HELLO\n"
