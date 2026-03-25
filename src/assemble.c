#include "lc3asm.h"

/**
 * assemble source code.
 * @param f files (typed file_t) containing source code
 */
void assemble(files *f) {
    int success;
    printf("----------\n");
    printf("Assembling \"%s\"\n", f->in_fname);
    printf("First Pass\n");
    symtable table = NULL;
    success = firstpass(f, &table);
    if (!success) {
        printf("First Pass failed\n");
        return;
    }
    printf("First Pass succeeded\n");
    write_symfile(table, f);
    printf("----------\n");
    printf("Second Pass\n");
    success = secondpass(f, table);
    if (!success) {
        printf("Second Pass failed\n");
        return;
    }
    printf("Second Pass succeeded\n");
    printf("Assemble \"%s\" Done\n", f->in_fname);
    printf("----------\n");
    destroy_symtable(table);
}

/**
 * first pass of assembling, deal with labels.
 * @param f files (typed file_t) containing source code
 * @param table symbol table to write in
 * @return 1 if succeed, 0 if failed
 */
int firstpass(files *f, symtable *table) {
    int LC, oploc = 0;
    int foundORIG = 0, foundEND = 0;
    line l;
    
    // find .ORIG
    while (!feof(f->in)) {
        readline(&l, f);
        if (!strcmp(l.tokens[0], ".ORIG")) {
            
            checkLackArgumentsError(".ORIG", 1, 0);
            checkTooManyArgumentsError(".ORIG", 2, 0);

            foundORIG = 1;
            LC = getnumber(l.tokens[1]);
            if (LC == INT_MAX) {
                raiseInvalidArgumentError(".ORIG", "origin address", 1, LC);
            } else if (LC < 0 || LC > 65536) {
                raiseArgumentOutOfRangeError(".ORIG", "origin address", 1, LC);
            }
            break;
        }
    }
    if (!foundORIG) {
        fprintf(stderr, ".ORIG not found.\n");
        return 0;
    }

    // continue parse and find .END
    while(!feof(f->in)) {
        readline(&l, f);

        if (!strcmp(l.tokens[0], ".END")) {
            foundEND = 1;
            break;
        }

        if (!strcmp(l.tokens[0], ".ORIG")) {
            fprintf(stderr, "Extra .ORIG appeared without matched amount of .END\n");
            return 0;
        }

        if (l.tokens[0][0]) {
            if (isoperation(l.tokens[0])) {
                // this line has no label
                oploc = 0;
            } else {
                if (search_symtable(*table, l.tokens[0]) != -1) {
                    printLine(LC);
                    fprintf(stderr, "Label [%s] is repeated\n", l.tokens[0]);
                    return 0;
                }
                append_symtable(table, l.tokens[0], LC);
                if (l.tokens[1][0]) {
                    // this line has both label and operation
                    oploc = 1;
                } else {
                    // this line has only label
                    continue;
                }
            }
            
            if (!strcmp(l.tokens[oploc], ".BLKW")) {
                // case ".BLKW"
                checkLackArgumentsError(".BLKW", 1, LC);
                checkTooManyArgumentsError(".BLKW", 2, LC);

                int blank;
                sscanf(l.tokens[oploc+1], "%d", &blank);
                LC += blank;
            } else if (!strcmp(l.tokens[oploc], ".STRINGZ")) {
                // case ".STRINGZ"
                checkLackArgumentsError(".STRINGZ", 1, LC);
                checkTooManyArgumentsError(".STRINGZ", 2, LC);

                LC += strlen(l.tokens[oploc+1]) + 1;
            } else {
                // case other operation
                LC ++;
            }
        }
    }

    if (!foundEND) {
        fprintf(stderr, ".END not found\n");
        return 0;
    }

    return 1;
}

/**
 * second pass of assembling, process and generate instructions.
 * @param f files (typed file_t) containing source code
 * @param table symbol table to look up
 * @return 1 if succeed, 0 if failed
 */
int secondpass(files *f, symtable table) {
    int LC, oploc;
    line l;
    
    rewind(f->in);
    // deal with .ORIG, get start LC
    while(!feof(f->in)) {
        readline(&l, f);
        if (!strcmp(l.tokens[0], ".ORIG")) {
            if (l.tokens[1][0]) {
                LC = getnumber(l.tokens[1]);
                char ORIGaddr[17];
                int2bin(LC, ORIGaddr, 16);
                fprintf(f->outbin, "%s\n", ORIGaddr);
                break;
            }
        }
    }

    // deal with the rest
    while(!feof(f->in)) {
        readline(&l, f);

        if (!strcmp(l.tokens[0], ".END")) break;

        if (l.tokens[0][0]) {
            if (isoperation(l.tokens[0])) {
                // this line has no label
                oploc = 0;
            } else {
                if (l.tokens[1][0]) {
                    // this line has both label and operation
                    oploc = 1;
                } else {
                    // this line has only label
                    continue;
                }
            }

            if (!process_inst(f, l, oploc, table, &LC)) {
                printf("Generation failed\n");
                return 0;
            }
        }
    }
    
    return 1;
}