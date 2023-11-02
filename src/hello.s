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
  lea rdi, [rip+msg]
  # call C function `puts`, rdi stores first argument
  call puts
  # epilogue
  pop rbp
  # return 0
  xor eax, eax
  ret

msg:
  .asciz "Hello, world!"