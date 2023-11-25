includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
.data
    ct0 db '%d', 10, 0
.code
    externdef printf:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    lea rcx, ct0
    mov rdx, 41
    call printf
    add rsp, 24
    pop rbp
    ret
main endp
end
