includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
includelib std.lib
.data
    arg0 db 'hello.tmp', 0
    arg1 db 'HELLO\n', 0
.code
    externdef write_file:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    lea rcx, arg0
    lea rdx, arg1
    call write_file
    add rsp, 20h
    pop rbp
    xor eax, eax
    ret
main endp
end
