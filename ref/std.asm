includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
includelib std.lib

.data
    msg db 'Hello, world!!', 10, 0
    fil db 'std.txt', 0

.code
    ; externdef printf:proc
    externdef write_file:proc

main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h

    ;lea rcx, msg ; lea: load the address of a variable into a register
    ;call print_str
    lea rcx, fil
    lea rdx, msg
    call write_file
    
    add rsp, 20h
    pop rbp

    mov rcx, 0
    ret

main endp

end