includelib msvcrt.lib
.code
a$ = -0
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rax, 10
    mov a$[rbp], eax
    mov rax, a$[rbp]
    cmp rax, 10
    setg al
    movzx rax, al
    cmp rax, 0
    je _L_else1
    mov rax, rbp
    push rax
    mov rax, a$[rbp]
    add rax, 100
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
    jmp _L_end1
_L_else1:
    mov rax, rbp
    push rax
    mov rax, a$[rbp]
    sub rax, 100
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
_L_end1:
    mov rax, a$[rbp]
    add rsp, 24
    pop rbp
    ret
main endp
end
