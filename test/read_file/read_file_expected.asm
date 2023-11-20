includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
includelib std.lib
.data
    arg0 db 'hello.txt', 0
.code
    externdef read_file:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    lea rcx, arg0
    call read_file
    add rsp, 20h
    pop rbp
    xor eax, eax
    ret
main endp
end
