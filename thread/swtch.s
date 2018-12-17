#if alpht		! initialize an ALPHA stack
	! ALPHA swtch
	! ALAHA startup
#elif mips		! initialize an MIPS stack
	! MIPS swtch
	! MIPS startup
#elif sparc		! initialize an SPARC stack
	! SPARC swtch
.global __swtch
.align 4
.proc  4
__swtch:save	%sp, -(8+64),%sp
		st		%fp,[%sp+64+0]		! save from's frame pointer
		st		%i7,[%sp+64+4]		! save from;s return address
		ta		3					! flush from's registers
		st		%sp,[%10]			! save from's stack pointer
		ld		[%i1],%sp			! load to's stack pointer
		ld		[%sp+64+0],%fp		! restore to's frame pointer
		ld		[%sp+64+4],%i7		! restore to's return address
		ret							! continue execution of to
		restore
	! SPARC startup
.global __start
.align	4
.proc	4
__start:ld		[%sp+64+4],%o0
		ld		[%sp+64],%o1
		call	%o1;nop
		call	_Thread_exit; nop
		unimp 	0
.global	__ENDMONITOR
__ENDMONITOR:
#elif arm		! initialize an ARM stack
	! ARM swtch
	! ARM startup
#else
	Unsupported platform
#endif
