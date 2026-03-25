#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// CONSTANTS
#define LINE_LEN 100
#define TOKEN_LEN 100
#define TOKEN_NUM 7
#define FILENAME_LEN 100

// EXCEPTIONS
#define printLine(addr) printf("Exception at instruction [%s] in address x%x\n", l.raw_line, addr);
#define checkLackArgumentsError(opname, opoffset, addr) if (!l.tokens[oploc+opoffset][0]) {printLine(addr) fprintf(stderr, "Invaild format for "opname" : lack arguments\n"); return 0;}
#define checkTooManyArgumentsError(opname, opoffset, addr) if (l.tokens[oploc+opoffset][0]) {printLine(addr) fprintf(stderr, "Invaild format for "opname" : too many arguments\n"); return 0;}
#define raiseInvalidArgumentError(opname, argument, opoffset, addr) {printLine(addr) fprintf(stderr, "Invalid operands for "opname" : expected "argument", but got [%s]\n", l.tokens[oploc+opoffset]); return 0;}
#define raiseArgumentOutOfRangeError(opname, argument, opoffset, addr) {printLine(addr) fprintf(stderr, "Invalid operands for "opname" : "argument" [%s] out of range\n", l.tokens[oploc+opoffset]); return 0;}
#define raiseLabelNotFoundError(opname, opoffset, addr) {printLine(addr) fprintf(stderr, "Label not found for "opname" : [%s]\n", l.tokens[oploc+opoffset]); return 0;}

// DATA STRUCTURES

// inout files structure
typedef struct files_t {
    char in_fname[FILENAME_LEN], out_bin_fname[FILENAME_LEN], out_sym_fname[FILENAME_LEN];
    // input assembly code
    FILE *in;
    // output binary code
    FILE *outbin;
    // output symbol table
    FILE *outsym;
} files;

// line structure
typedef struct line_t {
    char raw_line[LINE_LEN];
    char tokens[TOKEN_NUM][TOKEN_LEN];
} line;

// symbol table implemented by linked-list
typedef struct symbol_t {
    char label[TOKEN_LEN];
    unsigned int addr;
    struct symbol_t *next;
} symbol, *symtable;

// PROTOTYPES

// parse.c
void readline(line *l, files *f);
void reescape(char *raw);
int getnumber(char *token);
int isoperation(char *token);

// symbol.c
void append_symtable(symtable *table, char *label, int addr);
int search_symtable(symtable table, char *label);
void destroy_symtable(symtable table);
void write_symfile(symtable table, files *f);

// instruction.c
char *parsereg(char *token);
char *int2bin(int n, char *bin, int size);
int process_inst(files *f, line l, int oploc, symtable table, int *LC);

// assemble.c
void assemble(files *f);
int firstpass(files *f, symtable *table);
int secondpass(files *f, symtable table);