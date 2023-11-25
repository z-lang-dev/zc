includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
.data
    ct0 db 'Hello', 10, 0
.code
    externdef printf:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    lea rcx, ct0
    call printf
    mov rax, 2
    push rax
    mov rax, 5
    imul rax, 3
    push rax
    pop rdi
    pop rax
    add rax, rdi
    add rsp, 24
    pop rbp
    ret
main endp
end
