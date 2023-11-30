    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 0
    mov dword ptr[rbp-0], eax
    mov rax, 0
    mov dword ptr[rbp-4], eax
_L_begin1:
    mov eax, [rbp-0]
    cmp rax, 10
    setl al
    movzx rax, al
    cmp rax, 0
    je _L_end1
    mov rax, rbp
    sub rax, 4
    push rax
    mov eax, [rbp-4]
    push rax
    mov eax, [rbp-0]
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
    mov rax, rbp
    push rax
    mov eax, [rbp-0]
    add rax, 1
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
    jmp _L_begin1
_L_end1:
    mov eax, [rbp-4]
    pop rbp
    add rsp, 8
    ret
