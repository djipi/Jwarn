/*
 *	table.h
 */

/* DEFINES
 * ===============================================
 */


/* EXTERNS
 * ===============================================
 */




/* STRUCTURES
 * ===============================================
 */
typedef struct table {

    LONG     LineNumber;
    WORD     opcode;
    WORD     reg1;
    WORD     reg2;
    WORD     reg3;
    struct table *next;
    struct table *prev;
} TABLE;

#define ZF 1	/* Zero Flag 		*/
#define NF 2	/* Negative Flag 	*/
#define CF 4	/* Carry Flag		*/

typedef struct opcode_table {
    WORD     opcode;		
    BYTE     optext[20];	/* 12 character text string for opcode 	   */
    WORD     reg1;		/* If non-zero, its in use for this opcode */
    WORD     reg2;		/* If non-zero, its in use for this opcode */
    WORD     reg3;		/* If non-zero, its in use for this opcode */
    WORD     sources;		/* Number of source registers		   */
    WORD     ALU_Flag;		/* if non-zero, its an ALU instruction     */
    WORD     UseFlags;		/* If non-zero, it depends on ZNC flags    */
    WORD     WriteFlag;		/* If zero, it doesn't do a writeback      */

    WORD     SetFlags;		/* If non-zero, it sets ZNC flags          */
    WORD     states;
}OPCODE_TABLE;


/* PROTOTYPES
 * ===============================================
 */
VOID	InitCodeTable( VOID );
TABLE	*GetNewItem( VOID );
WORD	PushData( LONG LineNumber, WORD GPU_Code );
VOID	FreeCodeTable( VOID );

