includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
; 添加std.lib
includelib std.lib

.data
    ; 文件名称；注意这里是文件名，所以字符串后面不要加'\n'，即`,10`。
    fil db 'hello.z', 0

.code
    ; 声明外部函数
    externdef read_file:proc

main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h

    ; 填入第一个参数
    lea rcx, fil
    ; 调用函数
    call read_file
    
    add rsp, 20h
    pop rbp

    mov rcx, 0
    ret

main endp

end