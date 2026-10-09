
EXTERN ExitProcess:PROC

.code
main PROC
    sub rsp, 28h

    mov ecx, 42
    call ExitProcess

main ENDP
END
