includelib msvcrt.lib
includelib legacy_stdio_definitions.lib
; 添加std.lib
includelib std.lib

.data
    ; 要打印的消息，后面的`10`代表换行符
    msg db 'Hello, world!', 10, 0
    ; 文件名称；注意这里是文件名，所以字符串后面不要加'\n'
    fil db 'hello.tmp', 0

.code
    ; 声明外部函数
    externdef write_file:proc

main proc
    push rbp
    mov rbp, rsp
    sub rsp, 20h

    ; 填入第一个参数
    lea rcx, fil
    ; 填入第二个参数
    lea rdx, msg
    ; 调用函数
    call write_file
    
    add rsp, 20h
    pop rbp

    mov rcx, 0
    ret

main endp

end