/*
 *	rules.c 
 *
 *	Part of jwarn - Atari Jaguar Wait State Warning Generator
 *	This part checks all rules to see if waite stats are incurred.
 *
 *	See Jaguar Software Reference Manual - Writing Fast GPU Programs
 *
 *	Copyright 1993 ATARI Corp.
 */
#include "portab.h"
#include <stdio.h>
#include "table.h"
#include "message.h"

#ifndef _WFCTP
typedef WORD (*WFCTP)(FILE* ofp, OPCODE_TABLE* op, TABLE* ct);	/* pointer to function which returns a word */
#endif


/* 
 *	from table.c
 */
EXTERN TABLE		*CodeTable;	
EXTERN LONG		CodeCount;
EXTERN OPCODE_TABLE 	*OpCodeTable;

/*
 *	All rules fct. def.
 */

WORD rule_01(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_02(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_03(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_04(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_05_06(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
//WORD	rule_06();
WORD rule_07(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_08(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_09(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_10(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD warn_01(FILE *ofp, OPCODE_TABLE *op, TABLE *ct);
WORD rule_11(FILE* ofp, OPCODE_TABLE* op, TABLE* ct);
WORD rule_12(FILE* ofp, OPCODE_TABLE* op, TABLE* ct);


/*
 *	Array of rules so we can jump there using an index
 */

WFCTP rules_fct[] = {
	rule_01,
	rule_02,
	rule_03,
	rule_04,
	rule_05_06,
	rule_05_06,
	rule_07,
	rule_08,
	rule_09,
	rule_10,
	rule_11,
	rule_12,
	NULL
};
			
WORD	JUST_TEST;	/* Global Flag, just check for a rule, don't print */
			/* warning!					   */

/*
 *	Check all rules...
 */

WORD WarnAll(FILE* ofp)
{
	REG TABLE	*ct;
	WORD		state = FALSE;	
	WORD		r4; 

	for (ct = CodeTable; ct != NULL; ct = ct->next, r4 = 0) {
 		state += rule_01(ofp, OpCodeTable, ct);
 		state += rule_02(ofp, OpCodeTable, ct);
	 	state += rule_03(ofp, OpCodeTable, ct);
  		state += rule_04(ofp, OpCodeTable, ct);
 		state += rule_05_06(ofp, OpCodeTable, ct);
 		state += rule_07(ofp, OpCodeTable, ct);
 		state += rule_08(ofp, OpCodeTable, ct);
 		state += rule_09(ofp, OpCodeTable, ct);
 		state += rule_10(ofp, OpCodeTable, ct);
 		state += warn_01(ofp, OpCodeTable, ct);
	}
	return state;
}

/*
 *	Just go through the specified rules
 */

WORD WarnRules(FILE* ofp, WORD rules[])
{
	REG TABLE	*ct;
	WORD		state = FALSE;	
	WORD		*r;

	for (ct = CodeTable; ct != NULL; ct = ct->next) {
		for (r = rules; *r != (WORD)-1; r++) {
	 		state += rules_fct[*r](ofp, OpCodeTable, ct);
		}
	 	state += warn_01(ofp, OpCodeTable, ct);
	}

	return state;
}

/*
 *	Rule_01
 *
 *	An instruction reads a register containing the result o the previous
 *	instruction, one tick of wait is incurred until the previous 
 *	operation completes.
 *
 *	Returns TRUE, if rule 1 applies and FALSE otherwise.
 */

WORD rule_01(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	TABLE		*cz;
	OPCODE_TABLE	*x, *y, *z;

	x = &op[ct->opcode];	/* opcode information of current instr. */
	if (ct->next != NULL)
		y = &op[(ct->next)->opcode]; /* opcode info of next instr. */
	else
		y = NULL;
	if (ct->next && ct->next->next) {
		cz = ct->next->next;
		z = &op[cz->opcode];	/* about to be executed instr. */
	} else
		z = NULL;


	/*
	 *	rule 1
	 */			/* the 4 was a 2 */
	if ((y != NULL) && (x->states >= 4) && (((x->reg2 > 0) && (y->reg1 > 0)
	 && (ct->reg2 == ct->next->reg1)) || ((ct->reg2 == ct->next->reg3) && 
	 (x->reg2 > 0) && (y->reg3 > 0) ))) {
		fprintf(ofp, WARN_01, ct->next->LineNumber);
/* NEW: */
#if 0
		if ((z != NULL)	&& (z->sources > 0))
			fprintf(ofp, WARN_01A, cz->LineNumber);
#endif
		return TRUE;
	}

	return FALSE;
}

/*
 *	Rule_02
 *
 *	An instruction uses the flags from the prvious instruction, one tick
 *	of wait is incurred until the previous operation completes.
 *
 *	Returns TRUE, if rule 2 applies and FALSE otherwise.
 */

WORD rule_02(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	OPCODE_TABLE	*x, *y;

	x = &op[ct->opcode];			/* current instr. */
	if (ct->next != NULL)
		y = &op[(ct->next)->opcode];	/* next instr.	  */
	else
		y = NULL;

	/*
 	 * rule 2
	 */
	if ((y != NULL) && y->UseFlags && x->SetFlags) {
		WORD useflags = ct->next->opcode & 0x1f;

		if ( ( (useflags & 1) && (x->SetFlags & ZF) )||
		     ( (useflags & 2) && (x->SetFlags & ZF) )||
		     ( (useflags & 4) && (useflags & 4) && (x->SetFlags & NF))||
		     ( (useflags & 4) && (useflags & 8) && (x->SetFlags & NF))||
		     (!(useflags & 4) && (useflags & 4) && (x->SetFlags & NF))||
		     (!(useflags & 4) && (useflags & 8) && (x->SetFlags & NF) )
		   ) 
		{
			fprintf(ofp, WARN_02, ct->next->LineNumber);
			return TRUE;
		}
	}
	return FALSE;
}

/*
 *	:Rule_03
 *
 *	An ALU result, memory load value or divide result has to be written 
 *	back and neither instruction about to be executed matches, one tick
 *	of wait is incurred ti let the data be written
 *
 *	Returns TRUE, if rule 3 applies and FALSE otherwise.
 */

WORD rule_03(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	OPCODE_TABLE	*x, *y, *z;
	TABLE		*cx, *cy, *cz;

	cx = ct;
	x = &op[cx->opcode];		/* current instr. */
	if (ct->next != NULL) {
		cy = ct->next;
		y = &op[cy->opcode];	/* next instr.	  */
	} else
		y = NULL;

	if (ct->next && ct->next->next) {
		cz = ct->next->next;
		z = &op[cz->opcode];	/* about to be executed instr. */
	} else
		z = NULL;

	if (y != NULL) {
		JUST_TEST = TRUE;
		if (rule_04(ofp, op, cy) == TRUE) {
			JUST_TEST = FALSE;
			return FALSE;
		}
		JUST_TEST = FALSE;
	}

	/*
	 *	rule 3:
	 *		add	r1, r2
	 *		move	r3, r4
	 *		move	r5, r6 ; @ src & dst != r2 & r4 
	 */
	if (
	    (y != NULL) && (z != NULL) &&
	    (x->states == 4) && (x->reg2 == 1) &&
	    /*(x->ALU_Flag == 1) &&*/			/* NEW */
	    (y->states >= 2) && (y->states <= 3) && (y->reg2 > 0) &&
	    (
	     ((z->reg2 > 0) && (z->WriteFlag) && (z->reg1 == 0) && 
	      (cx->reg2 != cz->reg2) && (cy->reg2 != cz->reg2))
	    ||
	     ((z->reg2 > 0) && (z->reg1 > 0) && /* V WRITE BACKS!!! DON'T LOOK AT SRC REG!!! */
	      (z->WriteFlag) &&
	      (cx->reg2 != cz->reg1) && (cy->reg2 != cz->reg1) && 
	      (cx->reg2 != cz->reg2) && (cy->reg2 != cz->reg2))
	    )
 	   ) {
		fprintf(ofp, WARN_03, ct->next->next->LineNumber, ct->LineNumber);

 		return TRUE;
	} 
	/*
	 *	still rule 3:
	 *		move 	r3, r4
	 *		add	r5, r6	; @ src & dst != r3
	 *	BUT
	 *		move	r3, r4
	 *		addq	#1, r5	; ok because only one source reg !!!
	 */

	else if (
	    (y != NULL) &&
	    (x->states >= 2) && (x->states <= 3) && (x->reg2 == 1) &&
	    (y->states == 4) && (y->sources > 1) && (y->reg2 > 0) &&
	    (
	     ((y->reg2 > 0) && (y->reg1 == 0) && (y->WriteFlag) && 
	      (cx->reg2 != cy->reg2))
	    ||
	     ((y->reg2 > 0) && (y->reg1 > 0) && (y->WriteFlag) &&  
	      (cx->reg2 != cy->reg1) &&  /* WRITE BACKS!!! DON'T LOOK AT SRC REG!!! */
	      (cx->reg2 != cy->reg2))
	    )
 	   ) {
		fprintf(ofp, WARN_03, ct->next->LineNumber, ct->LineNumber);

		return TRUE;
	}

	return FALSE;
}

/*
 *	Rule_04
 *
 *	Two values are to be written back at once, one tick of wait is
 *	incurred.
 *
 *	Returns TRUE, if rule 4 applies and FALSE otherwise.
 */

WORD rule_04(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	OPCODE_TABLE	*ot, *opcode[19];
	TABLE		*instr[19];
	REG TABLE	*cp;
	REG WORD	i, j;
	WORD		ticks;
	WORD		flag;

	flag = FALSE;
	ot = &op[ct->opcode];

	if ((ot->reg2 == 1)) { 
		/*
		 *	Count the states (ticks) of the next instructions.
		 *	Break out of loop when the number of ticks is greater
		 *	than the ticks of the current instr. (that's when the
		 *	write back will occur).
		 */
		for (i = 0, ticks = 0, cp = ct; i < ot->states; i++) {
			if ((cp = cp->next) == NULL)
				break;
			instr[i] = cp;
			opcode[i] = &op[cp->opcode]; /* store instr. */
			ticks += opcode[i]->states;
			if (ticks >= ot->states) /* over the magic number? */	
				break;
		}
		/*
		 *	Now check the stored instructions for rule 4
		 */
		for (j = 0; j < i; j++) {
			/*
			 *	rule 4
			 */
			if ((opcode[j]->reg2 > 0) && (opcode[j]->WriteFlag) &&
 			    ((opcode[j]->states + j + 1) == ot->states)) {
				if (!JUST_TEST) 
#if 0
					if (ot->reg1 == 2) {
					    	fprintf(ofp, WARN_04A, 
						    instr[j]->LineNumber + 1,
						    ct->LineNumber);
					} else {
#endif 
					    	fprintf(ofp, WARN_04, 
						    instr[j]->LineNumber + 1,
						    ct->LineNumber);
#if 0
					}
#endif
				flag = TRUE;
			}
		}
		if (flag)
			return TRUE;
	}
	return FALSE;
}

/*
 *	Rule_05_06
 *
 *	An instr. attempts to use the result of a divide inst. before it's
 *	ready. Wait states are inserted until the divide unit completes the
 *	divide, between one and sixteen wait states can be incurred.
 *
 *	A dividie instr. is about to be executed and the previous one has not
 *	completed, between one and sixteen wait states can be incurred.
 *
 *	Returns TRUE, if rule 5 or 6 applies and FALSE otherwise.
 */

WORD rule_05_06(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	OPCODE_TABLE	*opcode[19];
	TABLE		*instr[19];
	REG TABLE	*cp;
	REG WORD	i, j;
	WORD		ticks;
	WORD		flag;

	flag = FALSE;
	if (ct->opcode == 21 /* DIV */) {
		/*
		 *	Count the states (ticks) of the next 19 instructions.
		 *	Break out of loop when the number of ticks is greater
		 *	than 19.
		 */
		for (i = 0, ticks = 0, cp = ct; i < 19; i++) {
			if ((cp = cp->next) == NULL)
				break;
			instr[i] = cp;
			opcode[i] = &op[cp->opcode]; /* store instr. */
			ticks += opcode[i]->states;
			if (ticks >= 19)	  /* over the magix number? */	
				break;
		}
		/*
		 *	Now check the stored instructions for rule 5 and 6
		 */
		for (j = 0; j < i; j++) {
			/*
			 *	rule 5
			 */
			if ((opcode[j]->reg1 > 0) && 
			    (instr[j]->reg1 == ct->reg2)) {
			    	fprintf(ofp, WARN_05, instr[j]->LineNumber,
					ct->LineNumber);
				flag = TRUE;
			}
			/*
			 *	rule 6
			 */
			if (instr[j]->opcode == 21 /* DIV */) {
			    	fprintf(ofp, WARN_06, instr[j]->LineNumber,
					ct->LineNumber);
				flag = TRUE;

			}
		}
		if (flag)
			return TRUE;
	}
	return FALSE;
}

/*
 *	Rule_07
 *
 *	An instr. reads a register which is awaiting data from an incomplete
 *	memory read, this will be no more than one tick from internal memory, 
 *	but several ticks from external memory.
 *	
 *	Returns TRUE, if rule 7 applies and FALSE otherwise.
 */

WORD rule_07(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	OPCODE_TABLE	*opcode[19];
	TABLE		*instr[19];
	REG TABLE	*cp;
	REG WORD	i, j;
	WORD		ticks;
	WORD		flag;

	flag = FALSE;

	/* 
	 *	check for all loads 
	 */
	if (((ct->opcode >= 39) && (ct->opcode <= 44)) ||
	    ((ct->opcode >= 58) && (ct->opcode <= 59))) {
		/*
		 *	get instrs. for the next 4 ticks
		 */
		for (i = 0, ticks = 0, cp = ct; i < 4; i++) {
			if ((cp = cp->next) == NULL)
				break;
			instr[i] = cp;
			opcode[i] = &op[cp->opcode];
			ticks += opcode[i]->states;
			if (ticks >= 4)
				break;
		}
		/*
		 *	check them for rule 7
		 */
		for (j = 0; j < i; j++) {
			if ((opcode[j]->reg1 > 0) && 
	        	    (instr[j]->reg1 == ct->reg2)) {
				    fprintf(ofp, WARN_07, instr[j]->LineNumber,
					ct->LineNumber);
				flag = TRUE;
			}
		}
	}
	if (flag)
		return TRUE;
	else
		return FALSE;
}

/*
 *	Rule_08
 *
 *	A load or store instr. is about tob be executed and the memory 
 *	interface has not completed the transfer for the previous ones (one
 *	internal load/store or two external loads/stores can be pending 
 *	without holding up instruction flow).
 *	
 *	Returns TRUE, if rule 8 applies and FALSE otherwise.
 */

WORD rule_08(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	TABLE		*instr[16];
	REG TABLE	*cp;
	REG WORD	i, j;
	WORD		ticks;
	WORD		flag;
	WORD		opcode;

	flag = FALSE;

	/* 
	 *	check for all loads & stores 
	 */
	if (((ct->opcode >= 39) && (ct->opcode <= 44)) ||
	    ((ct->opcode >= 58) && (ct->opcode <= 59)) ||
	    ((ct->opcode >= 45) && (ct->opcode <= 50)) ||
	    ((ct->opcode >= 60) && (ct->opcode <= 61))) {
		/*
		 *	get instrs. for the next 4 ticks
		 */
		for (i = 0, ticks = 0, cp = ct; i < 4; i++) {
			if ((cp = cp->next) == NULL)
				break;
			instr[i] = cp;
			ticks += op[cp->opcode].states;
			if (ticks >= 4)
				break;
		}
		/*
		 *	check for rule 8
		 */

		for (j = 0; j < i; j++) {
			opcode = instr[j]->opcode;
			if (((opcode >= 39) && (opcode <= 44)) ||
			    ((opcode >= 58) && (opcode <= 59)) ||
			    ((opcode >= 45) && (opcode <= 50)) ||
			    ((opcode >= 60) && (opcode <= 61))) {
				    fprintf(ofp, WARN_08, instr[j]->LineNumber,
					ct->LineNumber);
				flag = TRUE;
			}
		}
	}
	if (flag)
		return TRUE;
	else
		return FALSE;
}

/*
 *	Rule_09
 *
 *	After a store instr. with indexed addressing mode
 *	
 *	Returns TRUE, if rule 9 applies and FALSE otherwise.
 */

WORD rule_09(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{

	/*
	 *	check for store with index address mode
	 */
	if ((ct->opcode == 49) || (ct->opcode == 50) ||
	    (ct->opcode == 60) || (ct->opcode == 61)) {	  
		fprintf(ofp, WARN_09, ct->LineNumber);
		return TRUE;
	}
	return FALSE;
}

/*
 *	Rule_10
 *
 *	After a jump or jr (three ticks if executing out of internal memory).
 *	
 *	Returns TRUE, if rule 10 applies and FALSE otherwise.
 */

WORD rule_10(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{
	/*
	 *	check for jr or jump
	 */
	if ((ct->opcode == 52) || (ct->opcode == 53)) {	  
		fprintf(ofp, WARN_10, ct->LineNumber);
		return TRUE;
	}
	return FALSE;
}

/*
 *	Rule_11
 *
 *	Detects when two adjacent instructions use the same destination register.
 *	This can lead to scoreboard stalls if the second depends on the result
 *	of the first before it’s written back.
 *
 *	Returns TRUE if hazard found.
 */
WORD rule_11(FILE *ofp, OPCODE_TABLE *op, TABLE *ct) {
	if (ct->next == NULL) return FALSE;

	OPCODE_TABLE *x = &op[ct->opcode];
	OPCODE_TABLE *y = &op[ct->next->opcode];

	if ((x->reg2 > 0) && (y->reg2 > 0) &&
	    (ct->reg2 != (WORD)-1) && (ct->reg2 == ct->next->reg2)) {

		fprintf(ofp,
			"[11] line %ld: interleaving hazard – reg r%d reused by next instruction (line %ld).\n",
			ct->next->LineNumber, ct->reg2, ct->LineNumber);
		return TRUE;
	}

	return FALSE;
}
/*
 *	Rule_12
 *
 *	Detects hazards where a long-latency instruction (e.g., DIV) is followed
 *	too closely by a read from its destination register.
 *
 *	Returns TRUE if hazard found.
 */
WORD rule_12(FILE *ofp, OPCODE_TABLE *op, TABLE *ct) {
	if (ct->next == NULL) return FALSE;

	OPCODE_TABLE *x = &op[ct->opcode];
	OPCODE_TABLE *y = &op[ct->next->opcode];

	if ((x->states >= 6) && (x->reg2 > 0) && (ct->reg2 != (WORD)-1) &&
	    ((ct->reg2 == ct->next->reg1) || (ct->reg2 == ct->next->reg2))) {

		fprintf(ofp,
			"[12] line %ld: latency hazard – r%d used too soon after high-latency op (line %ld).\n",
			ct->next->LineNumber, ct->reg2, ct->LineNumber);
		return TRUE;
	}

	return FALSE;
}


/*	
 *	Warning 1: Problem in the score board after a jump/jr instr.
 *
 *	Loop: 	...
 *		...
 *		jr 	cs, Loop
 *		addq	#1, R0
 *		moveq	#0, R0	; <- This will not happen!!!
 */

WORD warn_01(FILE* ofp, OPCODE_TABLE* op, TABLE* ct)
{

	OPCODE_TABLE	*x, *y, *z;
	TABLE		*cx, *cy, *cz;

	cx = ct;
	x = &op[cx->opcode];		/* current instr. */
	if (ct->next != NULL) {
		cy = ct->next;
		y = &op[cy->opcode];	/* next instr.	  */
	} else
		y = NULL;

	if (ct->next && ct->next->next) {
		cz = ct->next->next;
		z = &op[cz->opcode];	/* about to be executed instr. */
	} else
		z = NULL;

	/*
	 *	warn 01:
	 */
	if (
	    (y != NULL) && (z != NULL) &&
	    ((cx->opcode == 52) || (cx->opcode == 53)) && /* jump or jr */
	    (y->ALU_Flag == 1) && (y->reg2 > 0) && (z->reg2 >0) &&
	    (cy->reg2 == cz->reg2)
	   ) {
		fprintf(ofp, CWARN_01, cz->LineNumber, cy->LineNumber, 
			cx->LineNumber);
		return TRUE;
	} 
	return FALSE;
}
