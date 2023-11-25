includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
.data
    ct0 db 'Hello, world!', 10, 0
.code
    externdef printf:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    lea rcx, ct0
    call printf
    add rsp, 24
    pop rbp
    ret
main endp
end
