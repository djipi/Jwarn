/*
 *	message.h
 */

#define WARN_01 "[01] line %ld: src reg must wait for the result of the previous instruction.\n"
#define WARN_01A "(1A) line %ld: reg. read delayed because of wait from previous instr.\n"
#define WARN_02 "[02] line %ld: instr. uses flags of previous instr.\n"
#define WARN_03 "[03] line %ld: a result is written back; src or dst not dst of line %ld.\n"
#define WARN_04 "[04] line %ld: two values are written back at once (1st value: line %ld).\n"
#define WARN_04A "(4A) line %ld: delay might happen when write back occurs from line %ld.\n"
#define WARN_05 "[05] line %ld: instr. uses result of a divide (line %ld).\n"
#define WARN_06 "[06] line %ld: divide might have to wait for divide from line %ld.\n"
#define WARN_07 "[07] line %ld: instr. might have to wait for memory load (line %ld).\n"
#define WARN_08 "[08] line %ld: instr. might have to wait for previous memory transfer. (line %ld).\n"
#define WARN_09 "[09] line %ld: store instr. with index cause one wait state.\n"
#define WARN_10 "[10] line %ld: jump or jr (three ticks if internal memory).\n"

#define CWARN_01 "WARNING: line %ld: dest reg will have result of line %ld because of jump @%ld.\n"
