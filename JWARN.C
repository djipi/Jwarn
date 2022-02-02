/*
 *	The Jaguar Assembler Wait State Warning Generator
 *
 *	Copyright 1993 ATARI Corp.
 *
 *	Written in July 1993 by C. Gee and H.-M. Krober
 */

#include <portab.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "jwarn.h"
#include "table.h"

EXTERN OPCODE_TABLE *OpCodeTable;     /* from table.c		*/ 
EXTERN OPCODE_TABLE DSPOpCodeTable[]; /* from table.c		*/ 
EXTERN OPCODE_TABLE GPUOpCodeTable[]; /* from table.c		*/ 

EXTERN WORD errno;		/* for perror()			*/
GLOBAL BYTE *name  = "jwarn";	/* porgram name			*/
MLOCAL FILE *ifp;		/* input file ptr		*/
MLOCAL FILE *ofp;		/* output file ptr		*/
MLOCAL BYTE lbuf[LBUFSIZE];	/* line buffer			*/


/*
 *	an array to store the specified rules...
 */
MLOCAL WORD rules[MAXRULES];	/* use the following rules	*/
MLOCAL WORD rulesb[MAXRULES];	/* copy buffer for rules	*/

/*
 *	function declararions
 */

WORD	get_cpu();
WORD	get_input();
WORD	get_rules();
BYTE	*get_output();
VOID	get_lines();


/*	--------------------------------------------------------
 *	main(): C'mon to the Joyride....
 *
 *	Jwarn takes a listing file from gasm as its input and generates a file
 *	of warnings where wait states are going to occure.
 *
 * 	Usage: 
 * jwarn [-C{Gpu|Dsp}] [{+|-}r {1..10} {1..10} ...] [-ooutput] [input files]
 *
 *	If no input  and/or output file is specified stdin/stdout are used.
 *
 *	Examples:	jwarn tst.lst
 *			jwarn -otst.wrn tst.lst
 *			jwarn -CDsp tst.lst
 *			jwarn -r 1 4 6 10 tst.lst ; all warnings but 1,4,6,10
 *			jwarn +r 2 7 8 tst.lst ; show only type 2 7 8
 */

	VOID
main(argc, argv)
	WORD	argc;
	BYTE	*argv[];
{
	WORD	i;
	WORD	input = 0;
	WORD	cpu = 0;
	BYTE	*output = NULL;

	if ((argc > 1) && (strcmp(argv[1], "help") == 0)) {
		fprintf(stderr, "jwarn - Atari Jaguar Wait State Warning Generator\n");
 		fprintf(stderr, "usage: jwarn [-C{Gpu|Dsp}] [{+|-}r {1..10} {1..10}..] [-ooutput] [input files]\n");
		exit(0);
	} 

	/* 
	 *	Get the CPU type (Gpu or Dsp)
	 */
	if ((cpu = get_cpu(argc, argv)) == DSP_CODE)
		OpCodeTable = DSPOpCodeTable;
	else
		OpCodeTable = GPUOpCodeTable;
	
	/*
	 *	Get the output file name
	 */
	output = get_output(argc, argv);

	/*
	 *	Get the rules
	 */
	rules[0] = -1;
	get_rules(argc, argv);

	/*
	 *	Get input file names
	 */
	input = get_input(argc, argv);


	/*
	 *	If output not stdout, open file for writing
	 */
	if (output != NULL) {
		if ((ofp = fopen(output, "w")) == NULL) {
			perror(name);
			exit(errno);
		}
	} else 
		ofp = stdout;

	/*
	 *	If  input is stdin, process the lines...
	 */
	if (input == 0) {	/* stdin */
		ifp = stdin;
		get_lines(ifp, ofp);
	} 
	/*
	 *	...else, open the files first and print the file name
	 */
	else {
		for (i = input; i < argc; i++) {
			if ((ifp = fopen(argv[i], "r")) == NULL) {
				perror(name);
				exit(errno);
			}
			fprintf(ofp, "\nFILE: %s\n", argv[i]);
			get_lines(ifp, ofp);	/* process the lines */
		}
	}

	fclose(ifp);
	fclose(ofp);

	exit(0);
}

/*
 *	Scan the command line for the CPU type, (Gpu is default)
 */
	WORD
get_cpu(argc, argv)
	WORD	argc;
	BYTE	*argv[];
{
	REG WORD	i;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
		if ((argv[i][0] == '-') && (argv[i][1] == 'C')) {
			if ((strstr("Gpu", argv[i]) != NULL) &&
			    (strstr("Dsp", argv[i]) != NULL)) {
				fprintf(stderr, 
	"%s: wrong CPU specified. Has to be either Gpu or Dsp.\n", name);

			} else
			      return (argv[i][2] == 'D') ? DSP_CODE : GPU_CODE;
		}
	}
	return GPU_CODE;
}

/*
 *	Sacn the command line for an output file name
 */ 
	BYTE
*get_output(argc, argv)
	WORD	argc;
	BYTE	*argv[];
{
	REG WORD	i;

	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if ((argv[i][0] == '-') && (argv[i][1] == 'o')) {
				return &argv[i][2];
			}
		}
	}
	return NULL;
}

/*
 *	Get the input file names from the command line
 */
	WORD
get_input(argc, argv)
	WORD	argc;
	BYTE	*argv[];
{
	REG WORD	i;

	if (argc > 1) {
		for (i = 1; i < argc; i++)
		if ((argv[i][0] != '-') && (argv[i][0] != '+') && 
		    (!(isdigit(argv[i][0])))) {
			return i;
		}
	}
	return 0;
}

/*
 *	Get the rules to be checked
 */
	WORD
get_rules(argc, argv)
	WORD	argc;
	BYTE	*argv[];
{
	REG WORD	i, j, k;
	WORD		rule;
	WORD		*r, *rb;
	WORD		err = 0;
	WORD		mode = 0;
	WORD		flag;

	i = 1; j = 0; 
	rulesb[0] = -1;
	flag = FALSE;
	if (argc > 1) {
		for (k = 1; k < argc; k++) {
			if ((argv[k][1] == 'r') && ((argv[k][0] == '+') ||
			    (argv[k][0] == '-'))) {
				mode = argv[k][0];
				if ((mode == '+') || (mode == '-')) {
					if (flag) 
						err = 4;
					else {
					  for (i = k+1, j=0; i < argc; i++, j++) {
					  	if (!(isdigit(argv[i][0])))
							break;
						rule = atoi(&argv[i][0]);
						if ((rule < 1) || (rule > 10)) {
							err = 2;
							break;
						}
						if (j > MAXRULES) {
							err = 3;
							break;
						}
						rulesb[j] = rule;
						flag = TRUE;
					  }
					  rulesb[j] = -1;
					}
				} else
					err = 1;
			}
		}

	}
	switch (err) {
	case 1:
		fprintf(stderr, "%s: illigal option.\n", name);
		break;
	case 2:
		fprintf(stderr, "%s: illegal rule (rules: 1 to 10).\n", name);
		break;
	case 3:
		fprintf(stderr, "%s: too many rules.\n", name);
		break;
	case 4:
		fprintf(stderr, "%s: only one [-|+]r option allowed.\n", name);
		break;
	}
	if (err > 0)
		exit(3);

	if (mode == '-') {
		for (j = 1, r = rules; j <= NR_RULES; j++) {
			for (rb = rulesb; *rb != -1; rb++)
				if (j == *rb)
					break;
			if (*rb == -1)
				*r++ = (j - 1);
		}
		*r = -1;
	} else if (mode == '+') {
		for (rb = rulesb, r = rules; *rb != -1; r++, rb++ )
			*r = *rb - 1;
		*r = -1;
	}

	return (i - 1);
}

/*
 *	Main function tp go through the lines of the input files and
 *	check for the rules...
 */

	VOID
get_lines(ifp, ofp)
	FILE	*ifp;	/* input  file ptr			*/
	FILE	*ofp;	/* output file ptr			*/
{
	ULONG	addr;	/* address field in the listing file 	*/
	UWORD	code;	/* code field in the listing file    	*/
	WORD	n;	/* number of fileds in the current line	*/
	LONG	lnr;	/* line number 				*/

	InitCodeTable(); /* init the code table		 	*/
	lnr = 1L;	 /* 1st line				*/
	/*
	 *	step 1: read all lines into buffer
	 */
	while (fgets(lbuf, 1024, ifp) != NULL) { /* go through all lines */
			/* get address and code field from current line..*/
		n = sscanf(lbuf, "@'%lX %X\n", &addr, &code);
		
		if ((n >= 2) && (lbuf[11] != ' ')) {
			/* if addr & code present, push them onto table */
			if (PushData(lnr, code) == FALSE) {
				fprintf(stderr, "%s: Out of memory.\n", name);
				fclose(ifp);
				fclose(ofp);
				FreeCodeTable();
				exit(1);
			}
		}
		lnr++;
	}
	/*
	 *	step 2: check for rules
	 */
	if (rules[0] != -1) {
		if (WarnRules(ofp, rules) == FALSE) /* only specified rules */
	       		fprintf(ofp, "No wait states found.\n");
	} else if (WarnAll(ofp) == FALSE) {	   /* all rules		    */	
		fprintf(ofp, "No wait states found.\n");
	}
	FreeCodeTable();
}

