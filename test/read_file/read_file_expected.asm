includelib msvcrt.lib
includelib stdz.lib
.data
    ct0 db 'hello.txt', 0
.code
    externdef read_file:proc
main proc
    push rbp
    mov rbp, rsp
    sub rsp, 24
    lea rcx, ct0
    call read_file
    add rsp, 24
    pop rbp
    ret
main endp
end
