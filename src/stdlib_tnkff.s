BITS 64

%define SYS_READ        0
%define SYS_WRITE       1
%define SYS_EXIT       60
%define STDIN           0
%define STDOUT          1
%define STDERR          2
%define NUM_BUF_SIZE   20

SECTION .data

number_buf      times NUM_BUF_SIZE db 0x00

newline         db 0x0a
newline_len     equ $ - newline

input_invite    db '>', '>', ' '
in_invite_len   equ $ - input_invite

input_err_msg   db "input error! wrong char!"
input_err_len   equ $ - input_err_msg

square_root     dq 0x00

SECTION .text
;===========================================================
; Scan int64_t value to specified address
; Arguments:
;       rdi - int64_t value storage address
; Return:
;       [rdi] - int64_t value
; Destr: rax, rbx, rcx, rdx, rsi, rdi, r8, r9, r11
;===========================================================
scan_int64_t:
        push    rbp
        mov     rbp, rsp

        push    rax
        push    rbx
        push    rcx
        push    rdx
        push    rsi
        push    rdi
        push    r8
        push    r9
        push    r11

        mov     rbx, rdi

        mov     rsi, input_invite       ; Load the address of the buffer into rsi
        mov     rdx, in_invite_len      ; Load the length  of the buffer into rdx
        mov     rdi, STDOUT             ; File descriptor for standard output (STDOUT)

        mov     rax, SYS_WRITE
        syscall

        xor     r9, r9                  ; r9 - result accumulator
        xor     rcx, rcx                ; rcx - stores current digit
        xor     r8, r8                  ; r8 - is num negative flag

        mov     rdi, STDIN
        mov     rsi, rbx                ; rsi = storage address
        mov     rdx, 1

        mov     BYTE [rsi], 0x00

.read_next:
        mov     rax, SYS_READ
        syscall

        xor     rcx, rcx
        mov     cl, BYTE [rsi]

        cmp     cl, 0x0a                ; "\n"
        je      .end_read

        cmp     cl, '-'
        je      .num_negative

        cmp     cl, '0'
        jb      .error_char
        cmp     cl, '9'
        ja      .error_char

        sub     cl, '0'

        imul    r9, 10
        add     r9, rcx
        jmp     .read_next

.error_char:
        mov     rsi, input_err_msg      ; Load the address of the buffer into rsi
        mov     rdx, input_err_len      ; Load the length  of the buffer into rdx
        mov     rdi, STDOUT             ; File descriptor for standard output (STDOUT)

        mov     rax, SYS_WRITE
        syscall
        jmp     .end_input

.num_negative:
        mov     r8, 1
        jmp     .read_next

.end_read:
        cmp     r8, 1
        jne     .store_num
        imul    r9, -1

.store_num:
        imul    r9, INT_PRECISION
        mov     QWORD [rsi], r9
        jmp     .end_input

.end_input:
        pop     r11
        pop     r9
        pop     r8
        pop     rdi
        pop     rsi
        pop     rdx
        pop     rcx
        pop     rbx
        pop     rax

        pop     rbp
        ret

;===========================================================
; Display int64_t value
; Arguments:
;       rdi - int64_t value
; Return:
;       -
; Note:
;       All user-defined values are stored in memory
;       multiplied by INT_PRECISION to simulate a floating-point number.
;       Therefore, when printing, we also print the decimal point
;
; Destr: rdi, rsi, rax, rbx, rcx, rdx, r11,
;===========================================================
print_int64_t:
        push    rbp
        mov     rbp, rsp

        push    rax
        push    rbx
        push    rcx
        push    rdx
        push    rdi
        push    rsi
        push    r8
        push    r11

        mov     rbx, number_buf

        xor     r8, r8

        cmp     rdi, 0
        jge      .not_negative
        neg     rdi
        mov     r8, 1           ; set is num negative flag

.not_negative:
        mov     BYTE [rbx], 0xa ; "\n"
        inc     rbx

        mov     rcx, INT_PRECISION_POW
        cmp     rcx, 0
        je      .integer_char   ; if number is totally integer
                                ; no need to print .0 after it

.fractional_char:
        call    mod_10          ; rax = rdi % 10
        add     al, '0'
        mov     BYTE [rbx], al
        inc     rbx

        call    div_10          ; rdi = rdi / 10
        mov     rdi, rax

        sub     rcx, 1
        cmp     rcx, 0
        jg      .fractional_char

        mov     al, '.'         ; floating point
        mov     BYTE [rbx], al
        inc     rbx

        ; assert rcx = 0

.integer_char:
        call    mod_10          ; rax = rdi % 10
        add     al, '0'
        mov     BYTE [rbx], al
        inc     rbx

        call    div_10          ; rax = rdi / 10
        mov     rdi, rax

        cmp     rdi, 0
        jg      .integer_char

        cmp     r8, 1
        jne     .after_add_minus

        mov     al, '-'
        mov     BYTE [rbx], al
        inc     rbx

.after_add_minus:
        ; display buffer
        dec     rbx             ; = &last_char_added

.put_char:
        mov     rsi, rbx        ; Load the address of the buffer into rsi
        mov     rdx, 1          ; Load the length  of the buffer into rdx
        mov     rdi, STDOUT     ; File descriptor for standard output (STDOUT)

        mov     rax, SYS_WRITE
        syscall

        dec     rbx

        cmp     rbx, number_buf
        jge     .put_char

        pop     r11
        pop     r8
        pop     rsi
        pop     rdi
        pop     rdx
        pop     rcx
        pop     rbx
        pop     rax

        pop     rbp
        ret

;===========================================================
; divide int_64_t value by 10
; Arguments:
;       rdi - int64_t value
; Return:
;       rax - division result
;
; Destr: rax, rcx, rdx,
;===========================================================
div_10:
        push    rbp
        mov     rbp, rsp

        sub     rsp, 8  ; space for locals
        push    rcx     ; rcx is used in "rep" in upper funcs

        mov     QWORD [rbp - 8], rdi
        mov     rcx, QWORD [rbp - 8]
        mov     rdx, 7378697629483820647
        mov     rax, rcx
        imul    rdx
        sar     rdx, 2
        mov     rax, rcx
        sar     rax, 63
        sub     rdx, rax
        mov     rax, rdx

        pop     rcx     ; rcx is used in "rep" in upper funcs
        add     rsp, 8  ; pop locals

        pop     rbp
        ret

;===========================================================
; res (int64_t) = val (int64_t) - val (int64_t) / 10 * 10
; val % 10
; Arguments:
;       rdi - int64_t val
; Return:
;       rax - result
;
; Destr: -
;===========================================================
mod_10:
        push    rbp
        mov     rbp, rsp

        push    rcx
        push    rdi

        call    div_10
        imul    rax, 10
        sub     rdi, rax
        mov     rax, rdi

        pop     rdi
        pop     rcx

        pop     rbp
        ret

;===========================================================
; res = sqrt (val)
; Arguments:
;       rdi - int64_t val
; Return:
;       rax - result
;
; Destr: -
;===========================================================
sqrt_int64_t:
        push    rbp
        mov     rbp, rsp

        cvtsi2sd xmm0, rdi
        mov     rdx, square_root
        movsd   QWORD [rdx], xmm0

        fld    QWORD [square_root]
        fsqrt
        fstp   QWORD [square_root]

        movsd  xmm0, QWORD [square_root]
        cvtsd2si rax, xmm0

        pop     rbp
        ret

;===========================================================
; res = a / b
; Arguments:
;       rdi - a
;       rsi - b
; Return:
;       rax - result
;
; Destr: rdi, rsi, rdx,
;===========================================================
divide:
        xor     rdx, rdx
        mov     rax, rdi
        cqo                    ; sign-extend the dividend into edx:eax

        idiv    rsi            ; no need to copy to ecx/rcx first
        ret