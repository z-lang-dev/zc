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
    mov eax, [rbp-0]
    add rax, 50
    mov eax, [rbp-0]
    add rax, 100
    jmp _L_end1
_L_else1:
    mov eax, [rbp-0]
    sub rax, 50
    mov eax, [rbp-0]
    sub rax, 100
_L_end1:
    pop rbp
    add rsp, 8
    ret
