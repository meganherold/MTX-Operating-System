.globl _getcs
.globl _main,_prints,_syscall


        call _main

        mov   ax,#mes
        push  ax
        call  _prints
        pop   ax

dead:   jmp   dead

mes:    .asciz "BACK TO ASSEMBLY AGAIN\n\r"

_getcs:
        mov   ax, cs
        ret

_syscall:
        int    80
        ret
