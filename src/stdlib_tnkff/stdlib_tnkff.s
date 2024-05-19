BITS 64;
NUM_BUF_SIZE equ 20
SECTION .data

number_buf times NUM_BUF_SIZE db 0x00

SECTION .text
;===========================================================
; Scan int64_t value to specified address
; Arguments:
;       rdi - int64_t value storage address
; Return:
;       [rdi] - int64_t value
; Destr: -
;===========================================================
scan_int64_t:
        push    rbp
        mov     rbp, rsp

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
; Destr: rdi, rsi, rax, rbx, rcx, rdx,
;===========================================================
print_int64_t:
        push    rbp
        mov     rbp, rsp

        mov     rbx, number_buf

        mov     BYTE [rbx], 0xa ; "\n"
        inc     rbx

        mov     rcx, INT_PRECISION_POW
        cmp     rcx, 0
        je      integer_char    ; if number is totally integer
                                ; no need to print .0 after it

fractional_char:
        call    mod_10          ; rax = rdi % 10
        add     al, '0'
        mov     BYTE [rbx], al
        inc     rbx

        call    div_10          ; rdi = rdi / 10
        mov     rdi, rax

        sub     rcx, 1
        cmp     rcx, 1
        jg      fractional_char

        mov     al, '.'         ; floating point
        mov     BYTE [rbx], al
        inc     rbx

        ; assert rcx = 0

integer_char:
        call    mod_10          ; rax = rdi % 10
        add     al, '0'
        mov     BYTE [rbx], al
        inc     rbx

        call    div_10          ; rax = rdi / 10
        mov     rdi, rax

        cmp     rdi, 0
        jg      integer_char

        ; display buffer
        dec     rbx             ; = &last_char_added

put_char:
        mov     rsi, rbx        ; Load the address of the buffer into rsi
        mov     rdx, 1          ; Load the length  of the buffer into rdx
        mov     rdi, 1          ; File descriptor for standard output (STDOUT)

        mov     rax, 1          ; Syscall number for sys_write
        syscall                 ; Make the syscall to write to STDOUT

        dec     rbx

        cmp     rbx, number_buf
        jge     put_char

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