includelib msvcrt.lib
includelib stdz.lib
.data
    ct0 db 'hello.txt', 0
.code
    externdef read_file:proc
main proc
    push rbp
    mov rbp, rsp
    lea rcx, ct0
    call read_file
    pop rbp
    ret
main endp
end
