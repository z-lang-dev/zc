includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
.data
    msg db 'Hello, world!', 10, 0
.code
    externdef printf:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    lea rcx, msg
    call printf
    add rsp, 20h
    pop rbp
    mov rcx, 0
    ret
main endp
end
