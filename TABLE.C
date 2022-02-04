/* -----------------------------------------------
 * FILENAME: TABLE.C
 * 
 * Description: This file will contains routines
 *		that will take a 16bit word 
 *		GPU instruction and build a
 *		table containing:
 *		1) Line Numbers
 *		2) Instructions
 *		3) register 1
 *		4) register 2
 *  
 * Created				cjg	07/26/93
 * Store and Jump instructions have their
 * source and destination registers reversed.
 * Therefore, we switch it for them. Register 1
 * for our purposes, will always have source
 * and Register 2 will always have destination.
 * -----------------------------------------------
 */


/* INCLUDE FILES
 * ===============================================
 */
#include "portab.h"
#include <stdio.h>
#include <stdlib.h>
#include "table.h"



/* GLOBALS
 * ===============================================
 */
TABLE  *CodeTable;
TABLE  *CurPtr;
LONG   CodeCount;

OPCODE_TABLE  *OpCodeTable;

OPCODE_TABLE  GPUOpCodeTable[] = { 
/*
 *	O    I			R  R  R  S  A  U        W		S	S
 *	P    N			E  E  E  O  L  S        R 		E	T
 *	C    S			G  G  G  U  U  E        I		T	A
 *	O    T			1  2  3  R  F  S        T		S	T
 *	D    R			         C  L           E			E
 *	E    .		       (S)(D)    E  A           B   	F L A G S	S
 *				         S  G	   	K		   (TICKS)
 */
	0,  "ADD",    		1, 1, 0, 2, 1, 0,  	1,	CF|NF|ZF, 	4,	
	1,  "ADDC",   		1, 1, 0, 2, 1, CF, 	1,	CF|NF|ZF, 	4,
	2,  "ADDQ",   		0, 1, 0, 1, 1, 0,  	1,	CF|NF|ZF, 	4,
	3,  "ADDQT", 		0, 1, 0, 1, 1, 0,  	1,	0, 		4,
	4,  "SUB",    		1, 1, 0, 2, 1, 0,  	1,	CF|NF|ZF, 	4,
	5,  "SUBC",   		1, 1, 0, 2, 1, CF, 	1,	CF|NF|ZF, 	4,
	6,  "SUBQ",   		0, 1, 0, 1, 1, 0,  	1,	CF|NF|ZF, 	4,
	7,  "SUBQT",  		0, 1, 0, 1, 1, 0,  	1,	0, 		4,
	8,  "NEG",    		0, 1, 0, 1, 1, 0,  	1,	CF|NF|ZF, 	4,
	9,  "AND",    		1, 1, 0, 2, 1, 0,  	1,	NF|ZF, 		4,
	10, "OR",     		1, 1, 0, 2, 1, 0,  	1,	NF|ZF, 		4,
	11, "XOR",    		1, 1, 0, 2, 1, 0,  	1,	NF|ZF, 		4,
	12, "NOT",    		0, 1, 0, 1, 1, 0,  	1,	NF|ZF, 		4,
	13, "BTST",   		0, 1, 0, 1, 1, 0,  	0,	ZF, 		3,
	14, "BSET",   		0, 1, 0, 1, 1, 0,  	1,	NF|ZF, 		4,
	15, "BCLR",   		0, 1, 0, 1, 1, 0,  	1,	NF|ZF, 		4,
	16, "MULT",   		1, 1, 0, 2, 1, 0,  	1,	NF|ZF, 		4,
	17, "IMULT", 	 	1, 1, 0, 2, 1, 0,  	1,	NF|ZF, 		4,
	18, "IMULTN", 		1, 1, 0, 2, 1, 0,  	0,	NF|ZF, 		3, /* changed from 4 */
	19, "RESMAC", 		0, 1, 0, 1, 1, 0,  	1,	0, 		3, /* special case */ 
	20, "IMACN",  		1, 1, 0, 2, 1, 0,  	0,	0, 		3,
	21, "DIV",    		1, 1, 0, 2, 1, 0,  	1,	0, 		19, 
	22, "ABS",    		0, 1, 0, 1, 1, 0,  	1,	CF|NF|ZF, 	4,
	23, "SH",     		1, 1, 0, 2, 1, 0,  	1,	CF|NF|ZF, 	4,
	24, "SHLQ",   		0, 1, 0, 1, 1, 0,  	1,	CF|NF|ZF, 	4,
	25, "SHRQ",   		0, 1, 0, 1, 1, 0,  	1,	CF|NF|ZF, 	4,
	26, "SHA",    		1, 1, 0, 2, 1, 0, 	1,	CF|NF|ZF, 	4,
	27, "SHARQ",  		0, 1, 0, 1, 1, 0, 	1,	CF|NF|ZF, 	4,
	28, "ROR",    		1, 1, 0, 2, 1, 0, 	1,	CF|NF|ZF, 	4,
	29, "RORQ",  	 	0, 1, 0, 1, 1, 0, 	1,	CF|NF|ZF, 	4,
	30, "CMP",    		1, 1, 0, 2, 1, 0, 	0,	CF|NF|ZF, 	3,
	31, "CMPQ",   		0, 1, 0, 1, 1, 0, 	0,	CF|NF|ZF, 	3,
	32, "SAT8",   		0, 1, 0, 1, 1, 0, 	1,	NF|ZF, 		4,
	33, "SAT16",  		0, 1, 0, 1, 1, 0, 	1,	NF|ZF, 		4,
	34, "MOVE",   		1, 1, 0, 1, 0, 0, 	1,	0, 		3,
	35, "MOVEQ",  		0, 1, 0, 0, 0, 0, 	1,	0, 		3, /* !!! */
	36, "MOVETA", 		1, 1, 0, 1, 0, 0, 	1,	0, 		3,
	37, "MOVEFA", 		1, 1, 0, 1, 0, 0, 	1,	0, 		3,
	38, "MOVEI",  		0, 1, 0, 0, 0, 0, 	1,	0, 		3,/* changed from 4 */
	39, "LOADB",  		2, 1, 0, 1, 0, 0, 	1,	0, 		4,
	40, "LOADW",  		2, 1, 0, 1, 0, 0, 	1,	0, 		4,
	41, "LOAD",   		2, 1, 0, 1, 0, 0, 	1,	0, 		4, 
	42, "LOADP",  		2, 1, 0, 1, 0, 0, 	1,	0,	 	4,
	43, "LOAD(R14+n),rn", 	2, 1, 0, 1, 0, 0, 	1,	0, 		5,
	44, "LOAD(R15+n),rn", 	2, 1, 0, 1, 0, 0, 	1,	0, 		5,
	45, "STOREB", 		1, 2, 0, 1, 0, 0, 	0,	0, 		4,
	46, "STOREW", 		1, 2, 0, 1, 0, 0, 	0,	0, 		4,
	47, "STORE",  		1, 2, 0, 1, 0, 0, 	0,	0, 		4,
	48, "STOREP", 		1, 2, 0, 1, 0, 0, 	0,	0, 		4,
	49, "STORE Rn,(R14+n)",	1, 2, 0, 1, 0, 0, 	0,	0, 		5,
	50, "STORE Rn,(R15+n)",	1, 2, 0, 1, 0, 0, 	0,	0, 		5,
	51, "MOVE-PC", 		0, 1, 0, 1, 0, 0, 	1,	0, 		3,
	52, "JUMP", 		1, 0, 0, 1, 0, CF|NF|CF, 0,	0, 		3, 
	53, "JR", 		0, 0, 0, 0, 0, CF|NF|ZF, 0,	0,		3,   
	54, "MMULT", 		1, 1, 0, 1, 1, 0,	1,	CF|NF|ZF,	4, 
	55, "MTOI",  		1, 1, 0, 1, 1, 0,	1,	NF|ZF, 		4,
	56, "NORMI", 		1, 1, 0, 1, 1, 0,	1,	NF|ZF, 		4,
	57, "NOP",   		0, 0, 0, 0, 0, 0,	0,	0, 		1,
	58, "LOAD(R14+Rn),Rn", 	2, 1, 1, 2, 0, 0,	1,	0, 		5,
	59, "LOAD(R15+Rn),Rn", 	2, 1, 1, 2, 0, 0,	1,	0, 		5,
	60, "STORE Rn,(R14+Rn)",1, 2, 1, 2, 0, 0,	0,	0, 		5,
	61, "STORE Rn,(R15+Rn)",1, 2, 1, 2, 0, 0,	0,	0, 		5,
	62, "SAT24",  		0, 1, 0, 1, 1, 0,	1,	NF|ZF, 		4,
	63, "PACK/UNPACK",   	0, 1, 0, 1, 0, 0,	1,	0, 		4
};

OPCODE_TABLE  DSPOpCodeTable[] = { 
/*
 *	O    I			R  R  R  S  A  U	W		S	S
 *	P    N			E  E  E  O  L  S	R		E	T
 *	C    S			G  G  G  U  U  E	I		T	A
 *	O    T			1  2  3  R  F  S	T		S	T
 *	D    R			         C  L     	E			E
 *	E    .		       (S)(D)    E  A    	B	F L A G S	S
 *				         S  G		K	   	(TICKS)
 */
	0,  "ADD",    		1, 1, 0, 2, 1, 0,	1,	CF|NF|ZF, 	4,	
	1,  "ADDC",   		1, 1, 0, 2, 1, CF, 	1,	CF|NF|ZF, 	4,
	2,  "ADDQ",   		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF, 	4,
	3,  "ADDQT", 		0, 1, 0, 1, 1, 0,	1,	0, 		4,
	4,  "SUB",    		1, 1, 0, 2, 1, 0,	1,	CF|NF|ZF, 	4,
	5,  "SUBC",   		1, 1, 0, 2, 1, CF, 	1,	CF|NF|ZF, 	4,
	6,  "SUBQ",   		0, 1, 0, 1, 1, 0, 	1,	CF|NF|ZF, 	4,
	7,  "SUBQT",  		0, 1, 0, 1, 1, 0, 	1,	0, 		4,
	8,  "NEG",    		0, 1, 0, 1, 1, 0, 	1,	CF|NF|ZF, 	4,
	9,  "AND",    		1, 1, 0, 2, 1, 0, 	1,	NF|ZF, 		4,
	10, "OR",     		1, 1, 0, 2, 1, 0,	1,	NF|ZF, 		4,
	11, "XOR",    		1, 1, 0, 2, 1, 0,	1,	NF|ZF, 		4,
	12, "NOT",    		0, 1, 0, 1, 1, 0,	1,	NF|ZF, 		4,
	13, "BTST",   		0, 1, 0, 1, 1, 0,	0,	ZF, 		3,
	14, "BSET",   		0, 1, 0, 1, 1, 0,	1,	NF|ZF, 		4,
	15, "BCLR",   		0, 1, 0, 1, 1, 0,	1,	NF|ZF, 		4,
	16, "MULT",   		1, 1, 0, 2, 1, 0,	1,	NF|ZF, 		4,
	17, "IMULT", 	 	1, 1, 0, 2, 1, 0,	1,	NF|ZF, 		4,
	18, "IMULTN", 		1, 1, 0, 2, 1, 0,	0,	NF|ZF, 		3, /* changed to 3 */
	19, "RESMAC", 		0, 1, 0, 1, 1, 0,	1,	0, 		3, /* special case */ 
	20, "IMACN",  		1, 1, 0, 2, 1, 0,	0,	0, 		3,
	21, "DIV",    		1, 1, 0, 2, 1, 0,	1,	0, 		19, 
	22, "ABS",    		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF, 	4,
	23, "SH",     		1, 1, 0, 2, 1, 0,	1,	CF|NF|ZF, 	4,
	24, "SHLQ",   		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF, 	4,
	25, "SHRQ",   		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF, 	4,
	26, "SHA",    		1, 1, 0, 2, 1, 0,	1,	CF|NF|ZF, 	4,
	27, "SHARQ",  		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF, 	4,
	28, "ROR",    		1, 1, 0, 2, 1, 0,	1,	CF|NF|ZF, 	4,
	29, "RORQ",  	 	0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF, 	4,
	30, "CMP",    		1, 1, 0, 2, 1, 0,	0,	CF|NF|ZF, 	3,
	31, "CMPQ",   		0, 1, 0, 1, 1, 0,	0,	CF|NF|ZF, 	3,
	32, "SUBQMOD",		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF,	4,
	33, "SAT16S",		0, 1, 0, 1, 1, 0,	1,	NF|ZF,		4,
	34, "MOVE",   		1, 1, 0, 1, 0, 0, 	1,	0, 		3,
	35, "MOVEQ",  		0, 1, 0, 0, 0, 0, 	1,	0, 		3, /* !!! */
	36, "MOVETA", 		1, 1, 0, 1, 0, 0, 	1,	0, 		3,
	37, "MOVEFA", 		1, 1, 0, 1, 0, 0, 	1,	0, 		3,
	38, "MOVEI",  		0, 1, 0, 0, 0, 0, 	1,	0, 		3,/* changed from 4 */
	39, "LOADB",  		2, 1, 0, 1, 0, 0, 	1,	0, 		4,
	40, "LOADW",  		2, 1, 0, 1, 0, 0, 	1,	0, 		4,
	41, "LOAD",   		2, 1, 0, 1, 0, 0, 	1,	0, 		4, 
	42, "SAT32S",		0, 1, 0, 1, 1, 0,	1,	NF|CF,		4,
	43, "LOAD(R14+n),rn", 	2, 1, 0, 1, 0, 0,	1,	0, 		5,
	44, "LOAD(R15+n),rn", 	2, 1, 0, 1, 0, 0,	1,	0, 		5,
	45, "STOREB", 		1, 2, 0, 1, 0, 0,	0,	0, 		4,
	46, "STOREW", 		1, 2, 0, 1, 0, 0,	0,	0, 		4,
	47, "STORE",  		1, 2, 0, 1, 0, 0,	0,	0, 		4,
	48, "MIRROR",		0, 1, 0, 1, 1, 0,	1,	NF|ZF,		4,
	49, "STORE Rn,(R14+n)",	1, 2, 0, 1, 0, 0, 	0,	0, 		5,
	50, "STORE Rn,(R15+n)",	1, 2, 0, 1, 0, 0, 	0,	0, 		5,
	51, "MOVE-PC", 		0, 1, 0, 1, 0, 0, 	1,	0, 		3,
	52, "JUMP", 		1, 0, 0, 1, 0, CF|NF|CF, 0,	0, 		3, 
	53, "JR", 		0, 0, 0, 0, 0, CF|NF|ZF, 0,	0,		3,   
	54, "MMULT", 		1, 1, 0, 1, 1, 0,	1,	CF|NF|ZF,	4, 
	55, "MTOI",  		1, 1, 0, 1, 1, 0, 	1,	NF|ZF, 		4,
	56, "NORMI", 		1, 1, 0, 1, 1, 0, 	1,	NF|ZF, 		4,
	57, "NOP",   		0, 0, 0, 0, 0, 0, 	0,	0, 		1,
	58, "LOAD(R14+Rn),Rn", 	1, 1, 1, 2, 0, 0, 	1,	0, 		5,
	59, "LOAD(R15+Rn),Rn", 	1, 1, 1, 2, 0, 0, 	1,	0, 		5,
	60, "STORE Rn,(R14+Rn)",1, 1, 1, 2, 0, 0, 	0,	0, 		5,
	61, "STORE Rn,(R15+Rn)",1, 1, 1, 2, 0, 0, 	0,	0, 		5,
	63, "ADDQMOD",		0, 1, 0, 1, 1, 0,	1,	CF|NF|ZF,	4
};


#if 0
/* SAMPLE */
WORD   TestData[] = { 0x980B, 0x980A, 0x9809, 0x980C, 0x980D, 0x2d08,
		      0xA5A3, 0x088D, 0xA5A4, 0x088D, 0xA5A1, 0x088D,
		      0xA5A2, 0x088D, 0xE400, 0x8827, 0x4447, 0x4421,
		      0x4442, 0x6Da1, 0x6dA7, 0x6Da2, 0x00E7, 0xE400,
		      0x8846, 0x0087, 0xe400, 0x8825, 0x10c1, 0x0061,
		      0x0828, 0x7909, 0xd4f8, 0xe400, 0x00a6, 0x78ca,
		      0xd462, 0xe400, 0xd164, 0xe400, 0xbd88, 0x981e,
		      0x981f, 0xbffe, 0xe400
		    };
#endif

/* FUNCTIONS
 * ===============================================
 */


VOID
InitCodeTable( VOID )
{
     CurPtr = ( TABLE *)NULLPTR;
     CodeTable = ( TABLE *)NULLPTR;
     CodeCount = 0L;
}



TABLE
*GetNewItem( VOID )
{
      TABLE *newptr;

      newptr = ( TABLE *)malloc( ( size_t )sizeof( TABLE ) );

      if( !newptr )
         return( ( TABLE *)NULLPTR );

      newptr->LineNumber = 0L;      
      newptr->opcode     = 0;      
      newptr->reg1       = 0;      
      newptr->reg2       = 0;      
      newptr->next       = ( TABLE *)NULLPTR;
      newptr->prev       = ( TABLE *)NULLPTR;

      return( newptr );
}




VOID
FreeCodeTable( VOID )
{
    TABLE *curptr;
    TABLE *nextptr;

    curptr = CodeTable;
    
    while( curptr ) {
	nextptr = curptr->next;
	free( curptr );
	curptr = nextptr;
    }
}




WORD
PushData( LONG LineNumber, WORD GPU_Code )
{
     TABLE *newptr;

     newptr = GetNewItem();
 
     if( !newptr )
	return( FALSE );

     if( !CodeTable ) {	/* We're the first! */
	CodeTable = CurPtr = newptr;
     }
     else {		/* We're the n + 1 and more! */
	CurPtr->next = newptr;
	newptr->prev = CurPtr;	
	CurPtr = newptr;
     }

     CurPtr->LineNumber = LineNumber;

     CurPtr->opcode = ( ( GPU_Code & 0xFC00 ) >> 10 );


     switch( CurPtr->opcode ) {
        
	/* These special case opcodes have their
	 * source and destination registers reversed
	 */
        case 47:	/* STORE Rn,(Rn)    */
	case 49:	/* STORE Rn,(R14+n) */
	case 50:        /* STORE Rn,(R15+n) */
	case 60:	/* STORE Rn,(R14+Rn)*/
	case 61:        /* STORE Rn,(R15+Rn)*/
	case 45:	/* STOREB Rn,(Rn)   */
	case 48:	/* STOREP Rn,(Rn)   */
	case 46:	/* STOREW Rn,(Rn)   */
#if 0
	case 52:	/* JUMP cc,(Rn)	    */
#endif
     		if( OpCodeTable[ CurPtr->opcode ].reg1 )
		    CurPtr->reg1   = ( GPU_Code & 0x001F );
	        else
	            CurPtr->reg1 = (WORD)-1;

	        if( OpCodeTable[ CurPtr->opcode ].reg2 )
	            CurPtr->reg2   = ( ( GPU_Code & 0x03E0 ) >> 5 );
	        else
	            CurPtr->reg2 = (WORD)-1;

		if (CurPtr->opcode == 49)	/* STORE Rn, (R14+n) */
			CurPtr->reg1 = 14;
		else if (CurPtr->opcode == 50)	/* STORE Rn, (R15+n) */
			CurPtr->reg1 = 15;

		if (CurPtr->opcode == 60)	/* STORE Rn, (R14+Rn) */
			CurPtr->reg3 = 14;
		else if (CurPtr->opcode == 61)	/* STORE Rn, (R15+Rn) */
			CurPtr->reg3 = 15;
		else
			CurPtr->reg3 = (WORD)-1;
	        break;

	/* These opcodes have their registers
	 * as source,destination
	 */
	default:	
     		if( OpCodeTable[ CurPtr->opcode ].reg1 )
		    CurPtr->reg1   = ( ( GPU_Code & 0x03E0 ) >> 5 );
	        else
	            CurPtr->reg1 = (WORD)-1;

	        if( OpCodeTable[ CurPtr->opcode ].reg2 )
	            CurPtr->reg2   = ( GPU_Code & 0x001F );
	        else
	            CurPtr->reg2 = (WORD)-1;

		if (CurPtr->opcode == 43)	/* LOAD (R14+n), Rn */
			CurPtr->reg1 = 14;
		if (CurPtr->opcode == 44)	/* LOAD (R15+n), Rn */
			CurPtr->reg1 = 15;

		if (CurPtr->opcode == 58)	/* LOAD (R14+Rn), Rn */
			CurPtr->reg3 = 14;
		else if (CurPtr->opcode == 59)	/* LOAD (R15+Rn), Rn */
			CurPtr->reg3 = 15;
		else
			CurPtr->reg3 = (WORD)-1;

	        break;
     }

     CodeCount++;

     return( TRUE );
}



#if 1

VOID
debugTable( VOID )
{

    TABLE *curptr;
#if 0
    InitCodeTable();

    for( LONG i = 0L; i < 45L; i++ )
	PushData( i, TestData[i] );
#endif

    curptr = CodeTable;

    while( curptr ) {


      printf( "LINE#: %ld, Opcode: %s, REG1: %d, REG2: %d\r\n", curptr->LineNumber, 
	       OpCodeTable[ curptr->opcode ].optext, curptr->reg1, curptr->reg2 );

      curptr = curptr->next;
    }
#if 0
    FreeCodeTable();

    exit(0);
#endif
}
#endif
