; this program was written in tinkov language, mne poxyi ya v americu

BITS 64
INT_PRECISION_POW equ 2
INT_PRECISION equ 100
SECTION .data

calc_stack times 2048 db 0x00		; calc stack (r15)

%include "/home/netort/language/src/stdlib_tnkff/stdlib_tnkff.s"

SECTION .text

; PROLOGUE
global main

main:

; ===== mapping of calc stack (CPUSH, CPOP) =====
mov r15, calc_stack
add r15, 2048

; ===============================================
call ЦАРЬ		; calling program entry point func

; EPILOGUE
mov rax, SYS_EXIT	; syscall exit
xor rdi, rdi
syscall

ret


ЦАРЬ:			; function (id = 3, n_params = 0)
			; var int64_t      a @ rbp-0x8
			; var int64_t      b @ rbp-0x10

	push rbp
	mov rbp, rsp
	sub rsp, 16			; reserve space for locals
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 8], rax	; a = rax

	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 16], rax	; b = rax

	push rdi			; SCAN begin
	lea rdi, QWORD [rbp - 8]		; >> a
	call scan_int64_t
	pop rdi				; SCAN end

	push rdi			; SCAN begin
	lea rdi, QWORD [rbp - 16]		; >> b
	call scan_int64_t
	pop rdi				; SCAN end


					; PRINT compound statement begin
	mov rax, QWORD [rbp - 8]	; rax = a
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	mov rax, QWORD [rbp - 16]	; rax = b
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax
	mov rax, QWORD [r15+8]
	add rax, QWORD [r15]
	add r15, 8			; push add-instr result instead of 2 operands

	mov [r15], rax
	pop rax

	mov rdi, QWORD [r15]		; cpop
	add r15, 8
	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 200700
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_1			; rax = ret_val
func_end_1:
	add rsp, 16			; pop locals
	pop rbp
	ret


