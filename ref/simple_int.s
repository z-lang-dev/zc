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
    # load address of msg into rdi
    lea rdi, [rip+fmt]
    # load 41 into rsi
    mov rsi, 41
    # call C function `puts`, rdi stores first argument
    call printf
    # return 0
    xor rax, rax
    # epilogue
    pop rbp
    ret

fmt:
    .asciz "%lld\n"