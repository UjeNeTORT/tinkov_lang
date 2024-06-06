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


ДИСКРИМИНАНТ:			; function (id = 4, n_params = 3)
			; par int64_t      a @ rbp+0x20
			; par int64_t      b @ rbp+0x18
			; par int64_t      c @ rbp+0x10

	push rbp
	mov rbp, rsp
	sub rsp, 0			; reserve space for locals
	mov rax, QWORD [rbp - -24]	; rax = b
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	mov rax, QWORD [rbp - -24]	; rax = b
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	sub r15, 8			; cpush
	mov QWORD [r15], 400
	mov rax, QWORD [rbp - -32]	; rax = a
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	mov rax, QWORD [rbp - -16]	; rax = c
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	push rax
	mov rax, QWORD [r15+8]
	sub rax, QWORD [r15]
	add r15, 8			; push sub-instr result instead of 2 operands

	mov [r15], rax
	pop rax

			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_1			; rax = ret_val
func_end_1:
	add rsp, 0			; pop locals
	pop rbp
	ret

ЦАРЬ:			; function (id = 13, n_params = 0)
			; var int64_t кси @ rbp-0x8
			; var int64_t джо @ rbp-0x10
			; var int64_t олег @ rbp-0x18
			; var int64_t обама @ rbp-0x20
			; var int64_t    a_0 @ rbp-0x28
			; var int64_t ярик @ rbp-0x30
			; var int64_t песков @ rbp-0x38
			; var int64_t галицкий @ rbp-0x40

	push rbp
	mov rbp, rsp
	sub rsp, 64			; reserve space for locals
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 8], rax	; кси = rax

	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 16], rax	; джо = rax

	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 24], rax	; олег = rax

	push rdi			; SCAN begin
	lea rdi, QWORD [rbp - 8]	; >> кси
	call scan_int64_t
	pop rdi				; SCAN end

	push rdi			; SCAN begin
	lea rdi, QWORD [rbp - 16]	; >> джо
	call scan_int64_t
	pop rdi				; SCAN end

	push rdi			; SCAN begin
	lea rdi, QWORD [rbp - 24]	; >> олег
	call scan_int64_t
	pop rdi				; SCAN end

	push rax
				; begin transfer param олег
	mov rax, QWORD [rbp - 24]	; rax = олег
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; repush begin
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	xchg rax, QWORD [rsp]		; repush end
				; end transfer param олег
				; begin transfer param джо
	mov rax, QWORD [rbp - 16]	; rax = джо
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; repush begin
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	xchg rax, QWORD [rsp]		; repush end
				; end transfer param джо
				; begin transfer param кси
	mov rax, QWORD [rbp - 8]	; rax = кси
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; repush begin
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	xchg rax, QWORD [rsp]		; repush end
				; end transfer param кси
	call ДИСКРИМИНАНТ
	add rsp, 24			; pop ДИСКРИМИНАНТ function params
	sub r15, 8			; cpush
	mov QWORD [r15], rax		; cpush ret val
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 32], rax	; обама = rax

			; cond_0
	push rax
	mov rax, QWORD [rbp - 8]	; rax = кси
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15 + 8]
	cmp rax, QWORD [r15]
	je cpush_true_0
cpush_false_0:
	mov rax, 0
	jmp end_cond_0
cpush_true_0:
	mov rax, 1
end_cond_0:
	add r15, 16			; cpop compared vals
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	cmp rax, 0
	je else_0
			; cond_1
	push rax
	mov rax, QWORD [rbp - 16]	; rax = джо
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15 + 8]
	cmp rax, QWORD [r15]
	je cpush_true_1
cpush_false_1:
	mov rax, 0
	jmp end_cond_1
cpush_true_1:
	mov rax, 1
end_cond_1:
	add r15, 16			; cpop compared vals
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	cmp rax, 0
	je else_1
			; cond_2
	push rax
	mov rax, QWORD [rbp - 24]	; rax = олег
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15 + 8]
	cmp rax, QWORD [r15]
	je cpush_true_2
cpush_false_2:
	mov rax, 0
	jmp end_cond_2
cpush_true_2:
	mov rax, 1
end_cond_2:
	add r15, 16			; cpop compared vals
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	cmp rax, 0
	je else_2

	mov rdi, 3 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 0
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_2			; rax = ret_val
	jmp end_if_2
else_2:

	mov rdi, 0 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end



	mov rdi, -1 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 200700
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_2			; rax = ret_val
end_if_2:


	jmp end_if_1
else_1:
end_if_1:


	jmp end_if_0
else_0:
end_if_0:


			; cond_3
	push rax
	mov rax, QWORD [rbp - 8]	; rax = кси
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15 + 8]
	cmp rax, QWORD [r15]
	je cpush_true_3
cpush_false_3:
	mov rax, 0
	jmp end_cond_3
cpush_true_3:
	mov rax, 1
end_cond_3:
	add r15, 16			; cpop compared vals
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	cmp rax, 0
	je else_3

	mov rdi, 1 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], -100
	mov rax, QWORD [rbp - 24]	; rax = олег
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	mov rax, QWORD [rbp - 16]	; rax = джо
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; div begin
	push rdi
	push rsi
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	push rdi			; right operand precision correction begin
	push rsi
	mov rsi, 100
	mov rdi, rax
	call divide
	pop rsi
	pop rdi				; right operand precision correction end
	mov rsi, rax
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov rdi, rax
	call divide
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rsi
	pop rdi
	pop rax				; div end

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 40], rax	; a_0 = rax


	mov rdi, [rbp - 40]		; PRINT "a_0" begin
	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 201000
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_2			; rax = ret_val
	jmp end_if_3
else_3:
end_if_3:


			; cond_4
	push rax
	mov rax, QWORD [rbp - 32]	; rax = обама
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15 + 8]
	cmp rax, QWORD [r15]
	jl cpush_true_4
cpush_false_4:
	mov rax, 0
	jmp end_cond_4
cpush_true_4:
	mov rax, 1
end_cond_4:
	add r15, 16			; cpop compared vals
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	cmp rax, 0
	je else_4

	mov rdi, 0 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 0
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_2			; rax = ret_val
	jmp end_if_4
else_4:
end_if_4:


			; cond_5
	push rax
	mov rax, QWORD [rbp - 32]	; rax = обама
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [r15 + 8]
	cmp rax, QWORD [r15]
	je cpush_true_5
cpush_false_5:
	mov rax, 0
	jmp end_cond_5
cpush_true_5:
	mov rax, 1
end_cond_5:
	add r15, 16			; cpop compared vals
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	cmp rax, 0
	je else_5

	mov rdi, 1 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], -100
	mov rax, QWORD [rbp - 16]	; rax = джо
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	sub r15, 8			; cpush
	mov QWORD [r15], 200
	mov rax, QWORD [rbp - 8]	; rax = кси
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	push rax			; div begin
	push rdi
	push rsi
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	push rdi			; right operand precision correction begin
	push rsi
	mov rsi, 100
	mov rdi, rax
	call divide
	pop rsi
	pop rdi				; right operand precision correction end
	mov rsi, rax
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov rdi, rax
	call divide
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rsi
	pop rdi
	pop rax				; div end

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 48], rax	; ярик = rax


	mov rdi, [rbp - 48]		; PRINT "ярик" begin
	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 0
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_2			; rax = ret_val
	jmp end_if_5
else_5:
end_if_5:



	mov rdi, 2 * 100		; PRINT immediate val (int64_t) begin

	call print_int64_t		; PRINT end


	mov rax, QWORD [rbp - 32]	; rax = обама
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rdi
	push rax
	mov rdi, QWORD [r15]		; cpop
	add r15, 8
	imul rdi, 100
	call sqrt_int64_t
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rax
	pop rdi

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 32], rax	; обама = rax

	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [rbp - 16]	; rax = джо
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax
	mov rax, QWORD [r15+8]
	sub rax, QWORD [r15]
	add r15, 8			; push sub-instr result instead of 2 operands

	mov [r15], rax
	pop rax

	mov rax, QWORD [rbp - 32]	; rax = обама
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax
	mov rax, QWORD [r15+8]
	sub rax, QWORD [r15]
	add r15, 8			; push sub-instr result instead of 2 operands

	mov [r15], rax
	pop rax

	sub r15, 8			; cpush
	mov QWORD [r15], 200
	mov rax, QWORD [rbp - 8]	; rax = кси
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	push rax			; div begin
	push rdi
	push rsi
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	push rdi			; right operand precision correction begin
	push rsi
	mov rsi, 100
	mov rdi, rax
	call divide
	pop rsi
	pop rdi				; right operand precision correction end
	mov rsi, rax
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov rdi, rax
	call divide
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rsi
	pop rdi
	pop rax				; div end

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 56], rax	; песков = rax

	sub r15, 8			; cpush
	mov QWORD [r15], 0
	mov rax, QWORD [rbp - 16]	; rax = джо
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax
	mov rax, QWORD [r15+8]
	sub rax, QWORD [r15]
	add r15, 8			; push sub-instr result instead of 2 operands

	mov [r15], rax
	pop rax

	mov rax, QWORD [rbp - 32]	; rax = обама
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax
	mov rax, QWORD [r15+8]
	add rax, QWORD [r15]
	add r15, 8			; push add-instr result instead of 2 operands

	mov [r15], rax
	pop rax

	sub r15, 8			; cpush
	mov QWORD [r15], 200
	mov rax, QWORD [rbp - 8]	; rax = кси
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	push rax			; mul begin
	mov rax, QWORD [r15+8]
	imul rax, QWORD [r15]
	push rdi			; precision correction begin
	push rsi
	mov rdi, rax
	mov rsi, 100
	call divide
	pop rsi
	pop rdi				; precision correction end
	add r15, 8			; push imul-instr result instead of 2 operands

	mov [r15], rax
	pop rax				; mul end

	push rax			; div begin
	push rdi
	push rsi
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	push rdi			; right operand precision correction begin
	push rsi
	mov rsi, 100
	mov rdi, rax
	call divide
	pop rsi
	pop rdi				; right operand precision correction end
	mov rsi, rax
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov rdi, rax
	call divide
	sub r15, 8			; cpush
	mov QWORD [r15], rax
	pop rsi
	pop rdi
	pop rax				; div end

	mov rax, QWORD [r15]		; cpop
	add r15, 8
	mov QWORD [rbp - 64], rax	; галицкий = rax


	mov rdi, [rbp - 56]		; PRINT "песков" begin
	call print_int64_t		; PRINT end



	mov rdi, [rbp - 64]		; PRINT "галицкий" begin
	call print_int64_t		; PRINT end


	sub r15, 8			; cpush
	mov QWORD [r15], 0
			; rax = ret_val & return
	mov rax, QWORD [r15]		; cpop
	add r15, 8
	jmp func_end_2			; rax = ret_val
func_end_2:
	add rsp, 64			; pop locals
	pop rbp
	ret


