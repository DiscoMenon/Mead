
EXTERN GetStdHandle:PROC
EXTERN WriteFile:PROC
EXTERN ExitProcess:PROC

.data
msg BYTE "Hello from Mead!", 13, 10
written DWORD 0

.code
main PROC
    sub rsp, 28h

    ; Get standard output handle
    mov ecx, -11
    call GetStdHandle

    ; Write the message to the console
    mov rcx, rax
    lea rdx, msg
    mov r8d, LENGTHOF msg
    lea r9, written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile

    xor ecx, ecx
    call ExitProcess
main ENDP
END
