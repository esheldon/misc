; this program uses the c standard library and main gets
; called like any userland function
; to compile and link
;
;   yasm -f elf64 readf.nasm && gcc -o readf readf.o -lc

section .text               ;section declaration

    global main
    extern  printf, scanf, fflush

section .data               ;section declaration

    ; associates the value 0xa with newline "nl"
    nl equ 0xa ; 10

    ;fname:  255  ; reserve 255 bytes, uninitialzed, labeled by fname
    ;buffer: resb 8192 ; reserve 8192 bytes, uninitialzed, labeled by buffer
    fname:  times 255 db 0  ; reserve 255 bytes, initialzed, labeled by fname
    buffer: times 8192 db 0 ; reserve 8192 bytes, uninitialzed, labeled by buffer

    usage_mes: db "usage: int1 int2 dbl1 | readf filename",nl,0

    ; associates the number 1 with stdout, so it can be used for a file
    ; handle
    stdout  equ 1
    stderr  equ 2
    write   equ 1
    exit    equ 60

    progname_format: db "program: '%s'",nl,0
    filename_format: db "filename: '%s'",nl,0

    double_format: db "%lf",0
    double_pformat: db "%.16g",nl,0

    d2_fmt: db "%lf %lf",0
    d2_pfmt: db "%.16g %.16g",nl,0

    int_format: db "%d",0
    iword_format: db "%ld",nl,0
    iword_pformat: db "%ld",nl,0
    argc_format: db "argc: %d",nl,0

    iword2_format: db "%ld %ld",0
    iword2_pformat: db "%ld %ld",nl,0

    wrongcount_fmt: db "expected to read %ld but read %ld",nl,0
main:

    push rbp
    mov rbp,rsp
    sub rsp,16   ; this makes space for 2 8 byte objects 
                 ; since printf will inherit rsp?

    ; since we're using the c lib, our entry is *very*
    ; different than stand alone.  main gets called
    ; like any other user land function.  
    ;   first  in rdi (int argc)
    ;   second in rsi (char** argv).  It *is* a pointer to pointers
    ;   third  in rdx if we have environment
    ; etc

    ; make our first argument (rdi) the second argument of printf (rsi)
    ; first save rsi (char** argv) for later
    push rsi

    ; if we don't have two arguments then print usage and bail out
    cmp rdi,2
    jne dousage 

    mov rsi, rdi
    ; now the first argument to printf is our format string
    mov rdi,argc_format
    xor rax,rax
    call printf

    ; now let's print our first argument, the program name.
    ; we need to get back argv that we just pushed on the stack.
    ; we put it into a register. The right thing to do?
    pop r12

    mov rsi,[r12]
    mov rdi,progname_format
    xor rax,rax
    call printf

    ; now the second argument, our filename
    mov rsi,[r12+8]
    mov rdi,filename_format
    xor rax,rax
    call printf

    ; read a word from stdin
    lea     rsi, [rbp-8]
    mov     rdi,iword_format
    xor     rax,rax
    call    scanf

    ; make sure scanf returns nread=1
    cmp rax,1
    jne dousage

    ; print the word we read
    mov rsi,[rbp-8]
    mov rdi,iword_pformat
    xor rax,rax
    call printf

    ; keep track of expected read count
    mov r12,[rbp-8]

    ; loop until we no longer read two doubles from stdin
    mov r13,0
rd2doubles:
	lea	rdx, [rbp-16]
	lea	rsi, [rbp-8]
    mov rdi, d2_fmt
    xor rax,rax
    call scanf

    cmp rax,2
    jne finish

    inc r13

    ; for printing we need to put it into an xmm* register
	movsd	xmm1, [rbp-16]
	movsd	xmm0, [rbp-8]
    mov rdi, d2_pfmt
	mov	rax, 2  ; number of floating point vars
    call printf

    jmp rd2doubles

finish:
    ; make sure we read the proper amount
    cmp r13,r12
    jne wrongcount

    push 0
    jmp doexit


doexit:

    ; fflush(null) will flush stdout
    ;mov rsi,0
    ;xor rax,rax
    ;call fflush

    ; exit with code code from stack
    pop rdi
    mov rax,exit
    syscall


dousage:
    mov rdi,usage_mes
    xor rax,rax
    call printf
    push 1
    jmp doexit

wrongcount:
    ; print the word we read
    mov rdx,r13
    mov rsi,r12
    mov rdi,wrongcount_fmt
    xor rax,rax
    call printf
    push 1
    jmp doexit
   
