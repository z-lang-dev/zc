includelib msvcrt.lib
includelib stdz.lib
.data
    ct0 db 'hello.txt', 0
.code
    externdef read_file:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h
    lea rcx, ct0
    call read_file
    add rsp, 20h
    pop rbp
    xor eax, eax
    ret
main endp
end
