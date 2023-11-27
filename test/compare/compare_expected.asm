includelib msvcrt.lib
.code
a$ = -0
b$ = -4
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rax, 10
    mov a$[rbp], eax
    mov rax, 20
    mov b$[rbp], eax
    mov rax, a$[rbp]
    push rax
    mov rax, b$[rbp]
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
    add rsp, 24
    pop rbp
    ret
main endp
end
