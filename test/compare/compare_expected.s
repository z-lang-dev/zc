    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 10
    mov dword ptr[rbp-0], eax
    mov rax, 20
    mov dword ptr[rbp-4], eax
    mov eax, [rbp-0]
    push rax
    mov eax, [rbp-4]
    push rax
    pop rdi
    pop rax
    cmp rax, rdi
    setl al
    movzx rax, al
    xor al, -1
    and al, 1
    movzx rax, al
    push rax
    mov rax, 1
    push rax
    pop rdi
    pop rax
    and rax, rdi
    pop rbp
    add rsp, 8
    ret
