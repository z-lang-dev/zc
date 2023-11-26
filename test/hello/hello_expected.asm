includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
.data
    ct0 db 'Hello, world!', 10, 0
.code
    externdef printf:proc
main proc
    push rbp
    mov rbp, rsp
    lea rcx, ct0
    call printf
    pop rbp
    ret
main endp
end
