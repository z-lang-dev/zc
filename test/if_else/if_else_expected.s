    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 10
    mov dword ptr[rbp-0], eax
    mov eax, [rbp-0]
    cmp rax, 10
    setg al
    movzx rax, al
    cmp rax, 0
    je _L_else1
    mov rax, rbp
    push rax
    mov eax, [rbp-0]
    add rax, 100
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
    jmp _L_end1
_L_else1:
    mov rax, rbp
    push rax
    mov eax, [rbp-0]
    sub rax, 100
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
_L_end1:
    mov eax, [rbp-0]
    pop rbp
    add rsp, 8
    ret
