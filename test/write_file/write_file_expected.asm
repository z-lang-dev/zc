includelib msvcrt.lib
includelib stdz.lib
.data
    ct0 db 'hello.tmp', 0
    ct1 db 'HELLO\n', 0
.code
    externdef write_file:proc
main proc
    push rbp
    mov rbp, rsp
    lea rcx, ct0
    lea rdx, ct1
    call write_file
    pop rbp
    ret
main endp
end
