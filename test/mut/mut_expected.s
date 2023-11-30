    .intel_syntax noprefix
    .text
    .global main
main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, 10
    mov dword ptr[rbp-0], eax
    mov rax, rbp
    mov dword ptr [rax], 5
    mov eax, [rbp-0]
    pop rbp
    add rsp, 8
    ret
