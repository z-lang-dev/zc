includelib msvcrt.lib
.code
a$ = -0
b$ = -4
c$ = -8
d$ = -12
e$ = -16
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov rax, 2
    mov a$[rbp], eax
    mov rax, 3
    mov b$[rbp], eax
    mov rax, 4
    mov c$[rbp], eax
    mov rax, 5
    mov d$[rbp], eax
    mov rax, 6
    mov e$[rbp], eax
    mov rax, a$[rbp]
    push rax
    mov rax, b$[rbp]
    push rax
    pop rdi
    pop rax
    imul rax, rdi
    push rax
    mov rax, c$[rbp]
    push rax
    mov rax, d$[rbp]
    push rax
    pop rdi
    pop rax
    imul rax, rdi
    push rax
    pop rdi
    pop rax
    add rax, rdi
    push rax
    mov rax, e$[rbp]
    push rax
    pop rdi
    pop rax
    add rax, rdi
    add rsp, 40
    pop rbp
    ret
main endp
end
