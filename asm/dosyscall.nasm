; this program is stand alone, no c library.  So the argc and each argv are
; represented on the stack to compile
;
;   yasm -f elf64 dosyscall.nasm && ld -o dosyscall dosyscall.o

section .text               ;section declaration

            ;we must export the entry point to the ELF linker or
    global _start ;loader. They conventionally recognize _start as their

section .data               ;section declaration

    ; associates the value 0xa with newline "nl"
    nl equ 0xa

    ; null terminated string "\n"
    newlinestr: db nl,0

    ; associates the number 1 with stdout, so it can be used for a file
    ; handle
    stdout  equ 1
    stderr  equ 2
    write   equ 1
    exit    equ 60

_start:

    ; this is standalone, not clib, so our incoming stack is full of the int
    ; argc and char* arguments.  Not int argc and char** argv
    ; so we just pop them

    ; first pop un-needed argc
    pop rsi

    ; to use syscall on 64 bit linux, we 
    ;  - put a syscall number in rax
    ;  - put first argument in rdi
    ;  - put second argument in rsi
    ;  - put third argument in rdx
    ; up to six and put sent.  The rest go in
    ;   r10, r8 and r9

    ; print program name
    mov rdx,11         ; set size to 7 for now
    pop rsi            ; pop first arg (prog name) into rsi
    mov rdi,stdout
    mov rax,write
    syscall

    ; print newline
    mov rdx,1
    mov rsi,newlinestr
    mov rdi,stdout
    mov rax,write
    syscall


    ; exit with code 0
    mov rdi,0
    mov rax,exit
    syscall


