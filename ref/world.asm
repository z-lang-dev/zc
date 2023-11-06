;---ASM Hello World Win64 MessageBox
; build: ml64.exe world.asm /link /subsystem:windows /entry:main
includelib kernel32.lib
includelib user32.lib

externdef MessageBoxA: proc 
externdef ExitProcess: proc

.data
    ttl db 'Win64 Hello', 0
    msg db 'Hello World!', 0

.code
main proc
    sub rsp, 28h    
    mov rcx, 0         ; hWnd = HWND_DESKTOP
    lea rdx, msg       ; LPCSTR lpText
    lea r8,    ttl     ; LPCSTR lpCaption
    mov r9d, 0         ; uType = MB_OK
    call MessageBoxA
    add rsp, 28h    
    mov ecx, eax       ; uExitCode = MessageBox(...)
    call ExitProcess
main endp

End