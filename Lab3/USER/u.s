        .globl _main,_syscall,_exit,_getcs
        .globl _getc, _putc, _color

	
        call _main
	
! if main() ever return, exit(0)
	push  #0
        call  _exit

_syscall:
        int    80
        ret


	
_getcs:
        mov   ax, cs
        ret


	
	!---------------------------------------------
        !  char getc( ): call BIOS to get a char
        !---------------------------------------------
_getc:
        xorb   ah,ah           ! clear ah
        int    0x16            ! call BIOS to get a char in AX
        andb   al,#0x7F        ! 7-bit ascii  
        ret 
	
        !----------------------------------------------
        ! int putc(char c): call BIOS to print a char
        !----------------------------------------------
_putc:           
        push   bp
	mov    bp,sp

        movb   al,4[bp]        ! get the char into aL
        movb   ah,#14          ! aH = 14
        mov    bx,_color       ! cyan
        int    0x10            ! call BIOS to display the char

        pop    bp
	ret
