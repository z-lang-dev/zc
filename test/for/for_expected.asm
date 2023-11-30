includelib msvcrt.lib
.code
i$ = -0
sum$ = -4
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rax, 0
    mov i$[rbp], eax
    mov rax, 0
    mov sum$[rbp], eax
_L_begin1:
    mov rax, i$[rbp]
    cmp rax, 10
    setl al
    movzx rax, al
    cmp rax, 0
    je _L_end1
    mov rax, rbp
    sub rax, 4
    push rax
    mov rax, sum$[rbp]
    push rax
    mov rax, i$[rbp]
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
    mov rax, i$[rbp]
    add rax, 1
    push rax
    pop rdi
    pop rax
    mov dword ptr[rax], edi
    jmp _L_begin1
_L_end1:
    mov rax, sum$[rbp]
    add rsp, 24
    pop rbp
    ret
main endp
end
