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
    ;fname:  times 255 db 0  ; reserve 255 bytes, initialzed, labeled by fname
    ;buffer: times 8192 db 0 ; reserve 8192 bytes, uninitialzed, labeled by buffer

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

    push rbp        ; save rbp
    mov rbp,rsp     ; now rbp holds the location of the stack pointer
    sub rsp,16      ; this repositions the stack pointer into the local
                    ; variables section, with two 8 byte spaces "above"
                    ; Since the stack grows to smaller memory addresses,
                    ; subtracting is moving "higher".
                    ;
                    ; now when we make a subroutine call, the return value will
                    ; get pushed into the correct place (the top of the 
                    ; stack) and not over our data! 

    ; since we're using the c lib, our entry is *very* different than stand
    ; alone.  main gets called like any other user land function.  
    ;   first  in rdi (int argc)
    ;   second in rsi (char** argv).  It *is* a pointer to pointers
    ;   third  in rdx if we have environment
    ; etc

    push rsi                ; first save rsi (char** argv) for later

    cmp rdi,2               ; compare argc to 2
    jne dousage             ; not equal?  Print usage and bail

    mov rsi, rdi            ; make argc the 2nd argument to printf
    mov rdi,argc_format     ; make a format string the first argument
    xor rax,rax             ; rax holds # of floating point arguments. set 0
    call printf             ; a call automatically pushes the return value
                            ; on the stack, and a "ret" call automatically
                            ; pops it and jumps to that location

    ; now let's print our first argument, the program name.
    ; we need to get back argv that we just pushed on the stack.
    ; I'l put it into this register, which is supposed to be owned
    ; by this routine. The right thing to do?
    pop r12

    mov rsi,[r12]           ; set 2nd argument to *argv[0]
    mov rdi,progname_format ; set 1st argument to the format string
    xor rax,rax             ; no floating point args
    call printf

    ; now the second argument, our filename
    mov rsi,[r12+8]         ; set 2nd argument to *argv[1]
    mov rdi,filename_format ; set 1st argument to format string
    xor rax,rax             ; no floating point args
    call printf

    ; read a word from stdin
    lea     rsi, [rbp-8]        ; use the space we "made" before
    mov     rdi,iword_format    ; read with format
    xor     rax,rax             ; no floating point args
    call    scanf

    ; make sure scanf returns nread=1
    cmp rax,1                   ; return value is in rax. 
                                ; make sure we read something
    jne dousage

    ; print the word we read
    mov rsi,[rbp-8]         ; set 2nd argument to [rbp-8], what we read
    mov rdi,iword_pformat   ; set 1st argument to format
    xor rax,rax             ; no floating point args
    call printf

    ;
    ; now read the rows of two doubles from stdin
    ;

    ; store expected read count in r12
    mov r12,[rbp-8]

    ;
    ; loop until we no longer read two doubles from stdin
    ;

    mov r13,0               ; keep track how many we read
rd2doubles:
    ; we need to reset registers each time, subroutines will use internally
    lea	rdx, [rbp-16]       ; 3rd argument address for a double
    lea	rsi, [rbp-8]        ; 2nd argument address for a double
    mov rdi, d2_fmt         ; 1st argument the format
    xor rax,rax             ; we don't need to say they are floats here
    call scanf

    cmp rax,2               ; if we didn't read 2, exit the loop
    jne finish

    inc r13

    ; for printing we need to put it into an xmm* register
    movsd xmm1, [rbp-16]    ; 3rd argument, a double, in special register
    movsd xmm0, [rbp-8]     ; 2rd argument, a double, in special register
    mov rdi, d2_pfmt;       ; 1st argument the format 
    mov	rax, 2              ; number of floating point vars
    call printf

    jmp rd2doubles          ; repeat

finish:
    ; make sure we read the proper amount
    cmp r13,r12
    jne wrongcount

    push 0
    jmp doexit


doexit:

    ; exit with code from stack
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
   
