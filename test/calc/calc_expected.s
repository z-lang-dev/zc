    .intel_syntax noprefix
    .text
    .global main
main:
    mov rax, 2
    imul rax, 3
    push rax
    mov rax, 4
    imul rax, 5
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    mov rax, 1
    imul rax, 7
    push rax
    pop rdi
    pop rax
    sub rax, rdi
    ret
