#include "lc3asm.h"

/**
 * transform reg(R*) to binary.
 * @param token reg name
 * @return string of its binary form
 */
char *parsereg(char *token) {
    if (token[0] == 'R') {
        switch (token[1]) {
            case '0': return "000";
            case '1': return "001";
            case '2': return "010";
            case '3': return "011";
            case '4': return "100";
            case '5': return "101";
            case '6': return "110";
            case '7': return "111";
            default: return NULL;
        }
    }
    return NULL;
}

/**
 * transform decimal integer to its binary string form.
 * @param n integer to transform
 * @param bin buffers to hold return string, length must at least be size+1
 * @param size binary string length
 * @return first location of the string
 */
char *int2bin(int n, char *bin, int size) {
    char *p = bin;
    p += size;
    *p = '\0';
    p --;
    for (int i = 0; i < size; i ++) {
        *p = (n & 1) ? '1' : '0';
        n >>= 1;
        p --;
    }
    return p+1;
}

/**
 * process and generate all 33 instructions that LC-3 supports.
 * @param f files (typed file_t) containing output file
 * @param l whole line (typed line_t)
 * @param oploc operation location of this line
 * @param table symbol tabel
 * @param LC location counter
 * @return 1 if succeeded, 0 if failed
 */
int process_inst(files *f, line l, int oploc, symtable table, int *LC) {
    if (!strcmp(l.tokens[oploc], ".BLKW")) {
        // specially process .BLKW (because .BLKW will fill several locations)
        int block;
        sscanf(l.tokens[oploc+1], "%d", &block);
        for (int i = 0; i < block; i ++) {
            fprintf(f->outbin, "0000000000000000\n");
            (*LC) ++;
        }
        return 1;
    } else if (!strcmp(l.tokens[oploc], ".STRINGZ")) {
        // specially process .STRINGZ (because .STRINGZ will fill several locations)
        int char_ascii;
        char char_bin[17];
        for (int i = 0; i < strlen(l.tokens[oploc+1]); i ++) {
            char_ascii = (int)(l.tokens[oploc+1][i]);
            int2bin(char_ascii, char_bin, 16);
            fprintf(f->outbin, "%s\n", char_bin);
            (*LC) ++;
        }

        // write in '\0'
        strcpy(char_bin, "0000000000000000");
        fprintf(f->outbin, "%s\n", char_bin);
        (*LC) ++;
        return 1;
    } else {
        // case for other instructions
        char inst[17];

        if (!strcmp(l.tokens[oploc], "ADD")) {

            checkLackArgumentsError("ADD", 1, (*LC));
            checkTooManyArgumentsError("ADD", 4, (*LC));

            strcpy(inst, "0001");
            char *DR, *SR1, *SR2, imm5[6];
            int imm;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("ADD", "DR", 1, (*LC));
            }
            strcat(inst, DR);
            SR1 = parsereg(l.tokens[oploc+2]);
            if (!SR1) {
                raiseInvalidArgumentError("ADD", "SR1", 2, (*LC));
            }
            strcat(inst, SR1);
            SR2 = parsereg(l.tokens[oploc+3]);
            if (!SR2) {
                imm = getnumber(l.tokens[oploc+3]);
                if (imm == INT_MAX) {
                    raiseInvalidArgumentError("ADD", "imm5", 3, (*LC));
                } else if (imm < -16 || imm > 15) {
                    raiseArgumentOutOfRangeError("ADD", "imm5", 3, (*LC));
                } else {
                    strcat(inst, "1");
                    int2bin(imm, imm5, 5);
                    strcat(inst, imm5);
                }
            } else {
                strcat(inst, "000");
                strcat(inst, SR2);
            }

        } else if (!strcmp(l.tokens[oploc], "AND")) {

            checkLackArgumentsError("AND", 1, (*LC));
            checkTooManyArgumentsError("AND", 4, (*LC));

            strcpy(inst, "0101");
            char *DR, *SR1, *SR2, imm5[6];
            int imm;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("AND", "DR", 1, (*LC));
            }
            strcat(inst, DR);
            SR1 = parsereg(l.tokens[oploc+2]);
            if (!SR1) {
                raiseInvalidArgumentError("AND", "SR1", 2, (*LC));
            }
            strcat(inst, SR1);
            SR2 = parsereg(l.tokens[oploc+3]);
            if (!SR2) {
                imm = getnumber(l.tokens[oploc+3]);
                if (imm == INT_MAX) {
                    raiseInvalidArgumentError("AND", "imm5", 3, (*LC));
                } else if (imm < -16 || imm > 15) {
                    raiseArgumentOutOfRangeError("AND", "imm5", 3, (*LC));
                } else {
                    strcat(inst, "1");
                    int2bin(imm, imm5, 5);
                    strcat(inst, imm5);
                }
            } else {
                strcat(inst, "000");
                strcat(inst, SR2);
            }

        } else if (!strcmp(l.tokens[oploc], "NOT")) {

            checkLackArgumentsError("NOT", 1, (*LC));
            checkTooManyArgumentsError("NOT", 3, (*LC));

            strcpy(inst, "1001");
            char *DR, *SR;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("NOT", "DR", 1, (*LC));
            }
            strcat(inst, DR);
            SR = parsereg(l.tokens[oploc+2]);
            if (!SR) {
                raiseInvalidArgumentError("NOT", "SR", 2, (*LC));
            }
            strcat(inst, SR);
            strcat(inst, "111111");

        } else if (!strcmp(l.tokens[oploc], "LD")) {
            
            checkLackArgumentsError("LD", 1, (*LC));
            checkTooManyArgumentsError("LD", 3, (*LC));

            strcpy(inst, "0010");
            char *DR;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("LD", "DR", 1, (*LC));
            }
            strcat(inst, DR);
            
            char PCoffset9[10];
            int offset = getnumber(l.tokens[oploc+2]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+2]);
                if (addr == -1) {
                    raiseLabelNotFoundError("LD", 2, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -256 || offset > 255) {
                raiseArgumentOutOfRangeError("LD", "source", 2, (*LC));
            }
            int2bin(offset, PCoffset9, 9);
            strcat(inst, PCoffset9);

        } else if (!strcmp(l.tokens[oploc], "LDI")) {

            checkLackArgumentsError("LDI", 1, (*LC));
            checkTooManyArgumentsError("LDI", 3, (*LC));

            strcpy(inst, "1010");
            char *DR;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("LDI", "DR", 1, (*LC));
            }
            strcat(inst, DR);
            
            char PCoffset9[10];
            int offset = getnumber(l.tokens[oploc+2]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+2]);
                if (addr == -1) {
                    raiseLabelNotFoundError("LDI", 2, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -256 || offset > 255) {
                raiseArgumentOutOfRangeError("LDI", "source", 2, (*LC));
            }
            int2bin(offset, PCoffset9, 9);
            strcat(inst, PCoffset9);

        } else if (!strcmp(l.tokens[oploc], "LDR")) {

            checkLackArgumentsError("LDR", 1, (*LC));
            checkTooManyArgumentsError("LDR", 4, (*LC));

            strcpy(inst, "0110");
            char *DR, *BaseR;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("LDR", "DR", 1, (*LC));
            }
            strcat(inst, DR);

            BaseR = parsereg(l.tokens[oploc+2]);
            if (!BaseR) {
                raiseInvalidArgumentError("LDR", "BaseR", 2, (*LC));
            }
            strcat(inst, BaseR);
            
            char offset6[7];
            int offset = getnumber(l.tokens[oploc+3]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+3]);
                if (addr == -1) {
                    raiseLabelNotFoundError("LDR", 3, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -32 || offset > 31) {
                raiseArgumentOutOfRangeError("LDR", "source", 3, (*LC));
            }
            int2bin(offset, offset6, 6);
            strcat(inst, offset6);

        } else if (!strcmp(l.tokens[oploc], "LEA")) {

            checkLackArgumentsError("LEA", 1, (*LC));
            checkTooManyArgumentsError("LEA", 3, (*LC));

            strcpy(inst, "1110");
            char *DR;
            DR = parsereg(l.tokens[oploc+1]);
            if (!DR) {
                raiseInvalidArgumentError("LEA", "DR", 1, (*LC));
            }
            strcat(inst, DR);
            
            char PCoffset9[10];
            int offset = getnumber(l.tokens[oploc+2]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+2]);
                if (addr == -1) {
                    raiseLabelNotFoundError("LEA", 2, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -256 || offset > 255) {
                raiseArgumentOutOfRangeError("LEA", "source", 2, (*LC));
            }
            int2bin(offset, PCoffset9, 9);
            strcat(inst, PCoffset9);

        } else if (!strcmp(l.tokens[oploc], "ST")) {

            checkLackArgumentsError("ST", 1, (*LC));
            checkTooManyArgumentsError("ST", 3, (*LC));

            strcpy(inst, "0011");
            char *SR;
            SR = parsereg(l.tokens[oploc+1]);
            if (!SR) {
                raiseInvalidArgumentError("ST", "SR", 1, (*LC));
            }
            strcat(inst, SR);
            
            char PCoffset9[10];
            int offset = getnumber(l.tokens[oploc+2]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+2]);
                if (addr == -1) {
                    raiseLabelNotFoundError("ST", 2, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -256 || offset > 255) {
                raiseArgumentOutOfRangeError("ST", "destination", 2, (*LC));
            }
            int2bin(offset, PCoffset9, 9);
            strcat(inst, PCoffset9);

        } else if (!strcmp(l.tokens[oploc], "STI")) {

            checkLackArgumentsError("STI", 1, (*LC));
            checkTooManyArgumentsError("STI", 3, (*LC));

            strcpy(inst, "1011");
            char *SR;
            SR = parsereg(l.tokens[oploc+1]);
            if (!SR) {
                raiseInvalidArgumentError("STI", "SR", 1, (*LC));
            }
            strcat(inst, SR);
            
            char PCoffset9[10];
            int offset = getnumber(l.tokens[oploc+2]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+2]);
                if (addr == -1) {
                    raiseLabelNotFoundError("STI", 2, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -256 || offset > 255) {
                raiseArgumentOutOfRangeError("STI", "destination", 2, (*LC));
            }
            int2bin(offset, PCoffset9, 9);
            strcat(inst, PCoffset9);

        } else if (!strcmp(l.tokens[oploc], "STR")) {

            checkLackArgumentsError("STR", 1, (*LC));
            checkTooManyArgumentsError("STR", 4, (*LC));

            strcpy(inst, "0111");
            char *SR, *BaseR;
            SR = parsereg(l.tokens[oploc+1]);
            if (!SR) {
                raiseInvalidArgumentError("STR", "DR", 1, (*LC));
            }
            strcat(inst, SR);

            BaseR = parsereg(l.tokens[oploc+2]);
            if (!BaseR) {
                raiseInvalidArgumentError("STR", "BaseR", 2, (*LC));
            }
            strcat(inst, BaseR);
            
            char offset6[7];
            int offset = getnumber(l.tokens[oploc+3]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+3]);
                if (addr == -1) {
                    raiseLabelNotFoundError("STR", 3, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -32 || offset > 31) {
                raiseArgumentOutOfRangeError("STR", "destination", 3, (*LC));
            }
            int2bin(offset, offset6, 6);
            strcat(inst, offset6);

        } else if (strstr(l.tokens[oploc], "BR")) {

            strcpy(inst, "0000");
            if (!(strcmp(l.tokens[oploc], "BR") && strcmp(l.tokens[oploc], "BRnzp"))) {
                strcat(inst, "111");
            } else if (!strcmp(l.tokens[oploc], "BRn")) {
                strcat(inst, "100");
            } else if (!strcmp(l.tokens[oploc], "BRz")) {
                strcat(inst, "010");
            } else if (!strcmp(l.tokens[oploc], "BRp")) {
                strcat(inst, "001");
            } else if (!strcmp(l.tokens[oploc], "BRnz")) {
                strcat(inst, "110");
            } else if (!strcmp(l.tokens[oploc], "BRnp")) {
                strcat(inst, "101");
            } else if (!strcmp(l.tokens[oploc], "BRzp")) {
                strcat(inst, "011");
            } else {
                printLine((*LC));
                fprintf(stderr, "Invalid condition for BR : [%s]\n", l.tokens[oploc]);
                return 0;
            }
            checkLackArgumentsError("BR", 1, (*LC));
            checkTooManyArgumentsError("BR", 2, (*LC));

            char PCoffset9[10];
            int offset = getnumber(l.tokens[oploc+1]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+1]);
                if (addr == -1) {
                    raiseLabelNotFoundError("BR", 1, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -256 || offset > 255) {
                raiseArgumentOutOfRangeError("BR", "destination", 1, (*LC));
            }
            int2bin(offset, PCoffset9, 9);
            strcat(inst, PCoffset9);

        } else if (!strcmp(l.tokens[oploc], "JMP")) {

            checkLackArgumentsError("JMP", 1, (*LC));
            checkTooManyArgumentsError("JMP", 2, (*LC));

            strcpy(inst, "1100000");
            char *BaseR;
            BaseR = parsereg(l.tokens[oploc+1]);
            if (!BaseR) {
                raiseInvalidArgumentError("JMP", "BaseR", 1, (*LC));
            }
            strcat(inst, BaseR);
            strcat(inst, "000000");

        } else if (!strcmp(l.tokens[oploc], "JSR")) {

            checkLackArgumentsError("JSR", 1, (*LC));
            checkTooManyArgumentsError("JSRR", 2, (*LC));

            strcpy(inst, "01001");
            char PCoffset11[12];
            int offset = getnumber(l.tokens[oploc+1]);
            if (offset == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+1]);
                if (addr == -1) {
                    raiseLabelNotFoundError("JSR", 1, (*LC));
                }
                offset = addr - ((*LC) + 1);
            }
            if (offset < -1024 || offset > 1023) {
                raiseArgumentOutOfRangeError("JSR", "destination", 1, (*LC));
            }
            int2bin(offset, PCoffset11, 11);
            strcat(inst, PCoffset11);

        } else if (!strcmp(l.tokens[oploc], "JSRR")) {

            checkLackArgumentsError("JSRR", 1, (*LC));
            checkTooManyArgumentsError("JSRR", 2, (*LC));

            strcpy(inst, "0100000");
            char *BaseR;
            BaseR = parsereg(l.tokens[oploc+1]);
            if (!BaseR) {
                raiseInvalidArgumentError("JSRR", "BaseR", 1, (*LC));
            }
            strcat(inst, BaseR);
            strcat(inst, "000000");

        } else if (!strcmp(l.tokens[oploc], "RET")) {

            checkTooManyArgumentsError("RET", 1, (*LC));
            strcpy(inst, "1100000111000000");

        } else if (!strcmp(l.tokens[oploc], "RTI")) {

            checkTooManyArgumentsError("RTI", 1, (*LC));
            strcpy(inst, "1000000000000000");

        } else if (!strcmp(l.tokens[oploc], ".FILL")) {

            checkLackArgumentsError(".FILL", 1, (*LC));
            checkTooManyArgumentsError(".FILL", 2, (*LC));

            char fill[17];
            int value = getnumber(l.tokens[oploc+1]);
            if (value == INT_MAX) {
                int addr = search_symtable(table, l.tokens[oploc+1]);
                if (addr == -1) {
                    raiseLabelNotFoundError(".FILL", 1, (*LC));
                }
                value = addr;
            } else if (value < -32768 || value > 65535) {
                raiseArgumentOutOfRangeError(".FILL", "Fill value", 1, (*LC));
            }
            int2bin(value, fill, 16);
            strcat(inst, fill);

        } else if (!strcmp(l.tokens[oploc], "TRAP")) {

            checkLackArgumentsError("TRAP", 1, (*LC));
            checkTooManyArgumentsError("TRAP", 2, (*LC));

            strcpy(inst, "11110000");
            char trapvect8[9];
            int trapvector = getnumber(l.tokens[oploc+1]);
            if (trapvector == INT_MAX) {
                raiseInvalidArgumentError("TRAP", "trapvect8", 1, (*LC));
            } else if (trapvector < 0 || trapvector > 0xFF) {
                raiseArgumentOutOfRangeError("TRAP", "trapvect8", 1, (*LC));
            }
            int2bin(trapvector, trapvect8, 8);
            strcat(inst, trapvect8);

        } else if (!strcmp(l.tokens[oploc], "GETC")) {

            checkTooManyArgumentsError("GETC", 1, (*LC));
            strcpy(inst, "1111000000100000");

        } else if (!strcmp(l.tokens[oploc], "OUT")) {

            checkTooManyArgumentsError("OUT", 1, (*LC));
            strcpy(inst, "1111000000100001");

        } else if (!strcmp(l.tokens[oploc], "PUTS")) {

            checkTooManyArgumentsError("PUTS", 1, (*LC));
            strcpy(inst, "1111000000100010");

        } else if (!strcmp(l.tokens[oploc], "IN")) {

            checkTooManyArgumentsError("IN", 1, (*LC));
            strcpy(inst, "1111000000100011");

        } else if (!strcmp(l.tokens[oploc], "PUTSP")) {

            checkTooManyArgumentsError("PUTSP", 1, (*LC));
            strcpy(inst, "1111000000100100");

        } else if (!strcmp(l.tokens[oploc], "HALT")) {

            checkTooManyArgumentsError("HALT", 1, (*LC));
            strcpy(inst, "1111000000100101");

        } else {
            printLine((*LC));
            fprintf(stderr, "Unsupported instruction : [%s]\n", l.tokens[oploc]);
            return 0;
        }

        fprintf(f->outbin, "%s\n", inst);
        (*LC) ++;
        return 1;
    }
}