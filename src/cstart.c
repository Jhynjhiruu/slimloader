// this file has been quite heavily modified to remove unnecessary stuff

void			_start	( void );
extern int		code	( void );

void _start_cpt( void )
{
	#pragma asm

	GLOBAL	__START
	__START:

    ; // don't really need to do system initialisation,
    ; // since we're just using BIOS context

    LD SC, #0h

	LD	[BR:27h],#0FFh
	LD	[BR:28h],#0FFh
	LD	[BR:29h],#0FFh
	LD	[BR:2Ah],#0FFh

	#pragma endasm
    
    // return after code() instead of exiting
    
    code();
}

#pragma asm
	DEFSECT	".tbss", DATA, TINY, CLEAR
	SECT	".tbss"
#pragma endasm
