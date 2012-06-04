section .text               ;section declaration

            ;we must export the entry point to the ELF linker or
    global _start   ;loader. They conventionally recognize _start as their
            ;entry point. Use ld -e foo to override the default.

section .data               ;section declaration

    ; associates the value 0xa with newline "nl"
    nl equ 0xa

    ; associates a pointer to the byte sequence with the label msg
    ; db, b means bytes.  d might mean data?
    msg     db      "Hello, world",nl ; string, followed by newline

    ; associates the value with label len, not a pointer to value
    len     equ     $ - msg                 ;length of our dear string

    ; associates the number 1 with stdout, so it can be used for a file
    ; handle
    stdout  equ 1
    stderr  equ 2
    write   equ 4
    exit    equ 1
    kernel  equ 0x80

_start:

    ;write our string to stdout

    mov     rdx,len     ; third argument: message length
    mov     rcx,msg     ; second argument: pointer to message to write
    mov     rbx,stdout  ; first argument: file handle (stdout=1)
    mov     rax,write   ; system call number (sys_write = 4) : automatically 
                        ; use registers b,c,d in that order?
    int     kernel      ; software interrupt: call kernel, automatically 
                        ; uses value in eax

    ;and exit
    mov     rbx,0       ; first syscall argument: exit code
    mov     rax,exit    ; system call number (sys_exit)
    int     kernel      ; call kernel, automatically uses value in eax


