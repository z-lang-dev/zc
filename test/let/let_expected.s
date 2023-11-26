    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rax, 2
    mov dword ptr[rbp-0], eax
    mov rax, 3
    mov dword ptr[rbp-4], eax
    mov rax, 4
    mov dword ptr[rbp-8], eax
    mov rax, 5
    mov dword ptr[rbp-12], eax
    mov rax, 6
    mov dword ptr[rbp-16], eax
    mov eax, [rbp-0]
    push rax
    mov eax, [rbp-4]
    push rax
    pop rdi
    pop rax
    imul rax, rdi
    push rax
    mov eax, [rbp-8]
    push rax
    mov eax, [rbp-12]
    push rax
    pop rdi
    pop rax
    imul rax, rdi
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    mov eax, [rbp-16]
    push rax
    pop rdi
    pop rax
    add rax, rdi
    pop rbp
    add rsp, 24
    ret
