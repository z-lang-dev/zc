includelib msvcrt.lib
.code
a$ = -0
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov rax, 10
    mov a$[rbp], eax
    mov rax, rbp
    mov dword ptr [rax], 5
    mov rax, a$[rbp]
    add rsp, 24
    pop rbp
    ret
main endp
end
