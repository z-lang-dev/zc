includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
.data
    ct0 db '%d', 10, 0
.code
    externdef printf:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    lea rcx, ct0
    mov rdx, 41
    call printf
    add rsp, 20h
    pop rbp
    xor eax, eax
    ret
main endp
end
