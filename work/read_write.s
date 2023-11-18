    # config for intel syntax and no prefix
    .intel_syntax noprefix
    # text section stores code
    .text
    # export main function to global space
    .global main

# definition of main function
main:
    # prologue
    push rbp
    mov rbp, rsp
    # load address of fil into rdi
    lea rdi, [rip+fil]
    # load address of msg into rsi
    lea rsi, [rip+msg]
    # call C function `write_file`
    call write_file
    # load address of fil into rdi
    lea rdi, [rip+fil]
    # call C function `read_file`
    call read_file
    # epilogue
    pop rbp
    # return 0
    xor eax, eax
    ret


fil:
    .asciz "hello.tmp"
msg:
    .asciz "Hello, world!\n"
