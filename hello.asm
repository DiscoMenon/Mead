EXTERN GetStdHandle:PROC
EXTERN WriteFile:PROC
EXTERN ExitProcess:PROC

.data
mead_written DWORD 0
mead_string_0 BYTE 61, 61, 61, 32, 86, 97, 114, 105, 97, 98, 108, 101, 115, 44, 32, 84, 121, 112, 101, 115, 32, 38, 32, 65, 114, 105, 116, 104, 109, 101, 116, 105, 99, 32, 61, 61, 61, 13, 10, 0
mead_string_1 BYTE 61, 61, 61, 32, 67, 111, 110, 116, 114, 111, 108, 32, 70, 108, 111, 119, 32, 61, 61, 61, 13, 10, 0
mead_string_2 BYTE 99, 111, 110, 100, 105, 116, 105, 111, 110, 58, 32, 116, 114, 117, 101, 13, 10, 0
mead_string_3 BYTE 99, 111, 110, 100, 105, 116, 105, 111, 110, 58, 32, 102, 97, 108, 115, 101, 13, 10, 0
mead_string_4 BYTE 61, 61, 61, 32, 70, 117, 110, 99, 116, 105, 111, 110, 115, 32, 61, 61, 61, 13, 10, 0
mead_string_5 BYTE 61, 61, 61, 32, 83, 116, 114, 105, 110, 103, 32, 73, 110, 116, 101, 114, 112, 111, 108, 97, 116, 105, 111, 110, 32, 61, 61, 61, 13, 10, 0
mead_string_6 BYTE 77, 101, 97, 100, 0
mead_string_7 BYTE 72, 101, 108, 108, 111, 44, 32, 0
mead_string_8 BYTE 33, 0
mead_string_9 BYTE 13, 10, 0
mead_string_10 BYTE 54, 32, 42, 32, 55, 32, 61, 32, 0
mead_string_11 BYTE 13, 10, 0
mead_string_12 BYTE 61, 61, 61, 32, 85, 110, 105, 116, 45, 65, 119, 97, 114, 101, 32, 84, 121, 112, 101, 115, 32, 61, 61, 61, 13, 10, 0
mead_string_13 BYTE 61, 61, 61, 32, 87, 97, 116, 99, 104, 32, 66, 108, 111, 99, 107, 115, 32, 61, 61, 61, 13, 10, 0
mead_string_14 BYTE 61, 61, 61, 32, 83, 101, 116, 116, 108, 101, 32, 76, 111, 111, 112, 115, 32, 61, 61, 61, 13, 10, 0
mead_string_15 BYTE 115, 101, 116, 116, 108, 101, 32, 100, 105, 100, 32, 110, 111, 116, 32, 102, 105, 110, 105, 115, 104, 13, 10, 0

.code
main PROC
    push rbp
    mov rbp, rsp
    sub rsp, 144
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_0
    mov r8d, 39
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, 10
    mov QWORD PTR [rbp-8], rax
    mov rax, 3
    mov QWORD PTR [rbp-16], rax
    mov rax, QWORD PTR [rbp-8]
    push rax
    mov rax, QWORD PTR [rbp-16]
    pop r10
    imul rax, r10
    push rax
    mov rax, 2
    pop r10
    add r10, rax
    mov rax, r10
    mov QWORD PTR [rbp-24], rax
    mov rax, QWORD PTR [rbp-24]
    mov ecx, eax
    call mead_print_int
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_1
    mov r8d, 22
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, QWORD PTR [rbp-24]
    push rax
    mov rax, 20
    pop r10
    cmp r10, rax
    setg al
    movzx rax, al
    test rax, rax
    jz mead_else_1
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_2
    mov r8d, 17
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    jmp mead_if_end_1
mead_else_1:
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_3
    mov r8d, 18
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
mead_if_end_1:
    mov rax, 0
    mov QWORD PTR [rbp-32], rax
mead_while_2:
    mov rax, QWORD PTR [rbp-32]
    push rax
    mov rax, 3
    pop r10
    cmp r10, rax
    setl al
    movzx rax, al
    test rax, rax
    jz mead_while_end_2
    mov rax, QWORD PTR [rbp-32]
    mov ecx, eax
    call mead_print_int
    mov rax, QWORD PTR [rbp-32]
    push rax
    mov rax, 1
    pop r10
    add r10, rax
    mov rax, r10
    mov QWORD PTR [rbp-32], rax
    jmp mead_while_2
mead_while_end_2:
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_4
    mov r8d, 19
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, 6
    push rax
    mov rax, 7
    push rax
    pop rdx
    pop rcx
    sub rsp, 32
    call mead_fn_multiply
    add rsp, 32
    mov QWORD PTR [rbp-40], rax
    mov rax, QWORD PTR [rbp-40]
    mov ecx, eax
    call mead_print_int
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_5
    mov r8d, 30
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    lea rax, mead_string_6
    mov QWORD PTR [rbp-48], rax
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_7
    mov r8d, 7
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, QWORD PTR [rbp-48]
    mov QWORD PTR [rbp-112], rax
    xor r8d, r8d
mead_string_len_loop_3:
    mov r10, QWORD PTR [rbp-112]
    cmp BYTE PTR [r10+r8], 0
    je mead_string_len_done_4
    inc r8d
    jmp mead_string_len_loop_3
mead_string_len_done_4:
    mov QWORD PTR [rbp-120], r8
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    mov rdx, QWORD PTR [rbp-112]
    mov r8d, DWORD PTR [rbp-120]
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_8
    mov r8d, 1
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_9
    mov r8d, 2
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_10
    mov r8d, 8
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, 6
    push rax
    mov rax, 7
    push rax
    pop rdx
    pop rcx
    sub rsp, 32
    call mead_fn_multiply
    add rsp, 32
    mov ecx, eax
    call mead_print_int_inline
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_11
    mov r8d, 2
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_12
    mov r8d, 26
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, 500
    push rax
    mov rax, 2
    imul rax, 1000
    pop r10
    add r10, rax
    mov rax, r10
    mov QWORD PTR [rbp-56], rax
    mov rax, 30
    push rax
    mov rax, 2
    imul rax, 60
    pop r10
    add r10, rax
    mov rax, r10
    mov QWORD PTR [rbp-64], rax
    mov rax, QWORD PTR [rbp-56]
    mov ecx, eax
    call mead_print_int
    mov rax, QWORD PTR [rbp-64]
    mov ecx, eax
    call mead_print_int
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_13
    mov r8d, 22
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, 0
    mov QWORD PTR [rbp-72], rax
    mov rax, QWORD PTR [rbp-72]
    mov QWORD PTR [rbp-80], rax
    mov rax, 1
    mov QWORD PTR [rbp-72], rax
    mov rax, QWORD PTR [rbp-72]
    cmp rax, QWORD PTR [rbp-80]
    je mead_watch_done_5
    mov QWORD PTR [rbp-80], rax
    mov rax, QWORD PTR [rbp-72]
    mov ecx, eax
    call mead_print_int
mead_watch_done_5:
    mov rax, 2
    mov QWORD PTR [rbp-72], rax
    mov rax, QWORD PTR [rbp-72]
    cmp rax, QWORD PTR [rbp-80]
    je mead_watch_done_6
    mov QWORD PTR [rbp-80], rax
    mov rax, QWORD PTR [rbp-72]
    mov ecx, eax
    call mead_print_int
mead_watch_done_6:
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_14
    mov r8d, 22
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov rax, 0
    mov QWORD PTR [rbp-88], rax
    mov QWORD PTR [rbp-104], 10
mead_settle_start_0:
    mov QWORD PTR [rbp-96], 0
    mov rax, QWORD PTR [rbp-88]
    push rax
    mov rax, 3
    pop r10
    cmp r10, rax
    setl al
    movzx rax, al
    test rax, rax
    jz mead_else_7
    mov rax, QWORD PTR [rbp-88]
    push rax
    mov rax, 1
    pop r10
    add r10, rax
    mov rax, r10
    cmp rax, QWORD PTR [rbp-88]
    je mead_settle_same_8
    mov QWORD PTR [rbp-88], rax
    mov QWORD PTR [rbp-96], 1
mead_settle_same_8:
mead_else_7:
    cmp QWORD PTR [rbp-96], 0
    je mead_settle_success_0
    sub QWORD PTR [rbp-104], 1
    cmp QWORD PTR [rbp-104], 0
    je mead_settle_failure_0
    jmp mead_settle_start_0
mead_settle_success_0:
    jmp mead_settle_done_0
mead_settle_failure_0:
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    lea rdx, mead_string_15
    mov r8d, 23
    lea r9, mead_written
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    mov ecx, 1
    call ExitProcess
mead_settle_done_0:
    mov rax, QWORD PTR [rbp-88]
    mov ecx, eax
    call mead_print_int
main_return:
    xor ecx, ecx
    call ExitProcess
    add rsp, 144
    pop rbp
    ret
main ENDP

mead_fn_multiply PROC
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov rax, QWORD PTR [rbp-8]
    push rax
    mov rax, QWORD PTR [rbp-16]
    pop r10
    imul rax, r10
    jmp multiply_return
multiply_return:
    add rsp, 64
    pop rbp
    ret
mead_fn_multiply ENDP


mead_print_int_inline PROC
    sub rsp, 58h
    movsxd rax, ecx
    xor r9d, r9d
    test rax, rax
    jns mead_pii_positive
    neg rax
    mov r9d, 1
mead_pii_positive:
    lea r10, [rsp+3Dh]
    mov r11, 10
    xor r8d, r8d
    test rax, rax
    jnz mead_pii_loop
    mov BYTE PTR [r10], 30h
    dec r10
    inc r8d
    jmp mead_pii_digits_done
mead_pii_loop:
    xor edx, edx
    div r11
    add dl, 30h
    mov BYTE PTR [r10], dl
    dec r10
    inc r8d
    test rax, rax
    jnz mead_pii_loop
mead_pii_digits_done:
    test r9d, r9d
    jz mead_pii_ready
    mov BYTE PTR [r10], 2Dh
    dec r10
    inc r8d
mead_pii_ready:
    lea rdx, [r10+1]
    mov QWORD PTR [rsp+40h], rdx
    mov DWORD PTR [rsp+48h], r8d
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    mov rdx, QWORD PTR [rsp+40h]
    mov r8d, DWORD PTR [rsp+48h]
    lea r9, [rsp+50h]
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    add rsp, 58h
    ret
mead_print_int_inline ENDP

mead_print_int PROC
    sub rsp, 58h
    movsxd rax, ecx
    xor r9d, r9d
    test rax, rax
    jns mead_pi_positive
    neg rax
    mov r9d, 1
mead_pi_positive:
    mov BYTE PTR [rsp+3Eh], 13
    mov BYTE PTR [rsp+3Fh], 10
    lea r10, [rsp+3Dh]
    mov r11, 10
    mov r8d, 2
    test rax, rax
    jnz mead_pi_loop
    mov BYTE PTR [r10], 30h
    dec r10
    inc r8d
    jmp mead_pi_digits_done
mead_pi_loop:
    xor edx, edx
    div r11
    add dl, 30h
    mov BYTE PTR [r10], dl
    dec r10
    inc r8d
    test rax, rax
    jnz mead_pi_loop
mead_pi_digits_done:
    test r9d, r9d
    jz mead_pi_ready
    mov BYTE PTR [r10], 2Dh
    dec r10
    inc r8d
mead_pi_ready:
    lea rdx, [r10+1]
    mov QWORD PTR [rsp+40h], rdx
    mov DWORD PTR [rsp+48h], r8d
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    mov rdx, QWORD PTR [rsp+40h]
    mov r8d, DWORD PTR [rsp+48h]
    lea r9, [rsp+50h]
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    add rsp, 58h
    ret
mead_print_int ENDP
END
