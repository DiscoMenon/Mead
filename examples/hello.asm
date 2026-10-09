EXTERN GetStdHandle:PROC
EXTERN WriteFile:PROC
EXTERN ExitProcess:PROC

.data
mead_written DWORD 0
mead_string_0 BYTE 65, 114, 105, 116, 104, 109, 101, 116, 105, 99, 58, 13, 10
mead_string_1 BYTE 120, 32, 105, 115, 32, 115, 109, 97, 108, 108, 101, 114, 13, 10
mead_string_2 BYTE 117, 110, 101, 120, 112, 101, 99, 116, 101, 100, 13, 10

.code
main PROC
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov rax, 10
    mov QWORD PTR [rbp-8], rax
    mov rax, 20
    mov QWORD PTR [rbp-16], rax
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_0
    mov r8d, 13
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, QWORD PTR [rbp-8]
    push rax
    mov rax, QWORD PTR [rbp-16]
    pop r10
    add rax, r10
    mov ecx, eax
    call mead_print_int
    mov rax, QWORD PTR [rbp-8]
    push rax
    mov rax, QWORD PTR [rbp-16]
    pop r10
    imul rax, r10
    mov ecx, eax
    call mead_print_int
    mov rax, QWORD PTR [rbp-8]
    push rax
    mov rax, QWORD PTR [rbp-16]
    pop r10
    sub r10, rax
    mov rax, r10
    mov ecx, eax
    call mead_print_int
    mov rax, QWORD PTR [rbp-8]
    push rax
    mov rax, QWORD PTR [rbp-16]
    pop r10
    cmp r10, rax
    setl al
    movzx rax, al
    test rax, rax
    jz mead_else_0
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_1
    mov r8d, 14
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    jmp mead_if_end_0
mead_else_0:
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_2
    mov r8d, 12
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
mead_if_end_0:
    mov rax, 0
    mov QWORD PTR [rbp-24], rax
mead_while_1:
    mov rax, QWORD PTR [rbp-24]
    push rax
    mov rax, 3
    pop r10
    cmp r10, rax
    setl al
    movzx rax, al
    test rax, rax
    jz mead_while_end_1
    mov rax, QWORD PTR [rbp-24]
    mov ecx, eax
    call mead_print_int
    mov rax, QWORD PTR [rbp-24]
    push rax
    mov rax, 1
    pop r10
    add rax, r10
    mov QWORD PTR [rbp-24], rax
    jmp mead_while_1
mead_while_end_1:
    xor ecx, ecx
    call ExitProcess
main ENDP

mead_print_int PROC
    sub rsp, 58h
    movsxd rax, ecx
    xor r9d, r9d
    test rax, rax
    jns mead_pi_positive
    neg rax
    mov r9d, 1
mead_pi_positive:
    lea r11, [rsp+43h]
    mov r10, 10
    xor r8d, r8d
    test rax, rax
    jnz mead_pi_loop
    mov BYTE PTR [r11], 30h
    dec r11
    inc r8d
    jmp mead_pi_digits_done
mead_pi_loop:
    xor edx, edx
    div r10
    add dl, 30h
    mov BYTE PTR [r11], dl
    dec r11
    inc r8d
    test rax, rax
    jnz mead_pi_loop
mead_pi_digits_done:
    test r9d, r9d
    jz mead_pi_no_sign
    dec r11
    mov BYTE PTR [r11], 2Dh
    inc r8d
mead_pi_no_sign:
    lea rdx, [r11+1]
    mov BYTE PTR [rsp+44h], 0Dh
    mov BYTE PTR [rsp+45h], 0Ah
    add r8d, 2
    mov QWORD PTR [rsp+48h], rdx
    mov DWORD PTR [rsp+50h], r8d
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    mov rdx, QWORD PTR [rsp+48h]
    mov r8d, DWORD PTR [rsp+50h]
    lea r9, [rsp+54h]
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    add rsp, 58h
    ret
mead_print_int ENDP
END
