#include "lc3asm.h"

/**
 * separate lines to tokens from source code.
 * @param l pointer to line_t, @param f source code file
 */
void readline(line *l, files *f) {
    int token_cnt = 0;
    char temp_line[LINE_LEN];

    fgets(l->raw_line, LINE_LEN, f->in);
    int end_of_line = strcspn(l->raw_line, ";\n");
    l->raw_line[end_of_line] = '\0';
    
    strcpy(temp_line, l->raw_line);
    reescape(temp_line);
    if (strstr(temp_line, ".STRINGZ")) {
        // case .STRINGZ
        char msg[LINE_LEN];
        char *first = strchr(temp_line, '\"') + 1;
        char *last = strrchr(temp_line, '\"') - 1;
        int msglen = (last - first + 1) * sizeof(char);
        strncpy(msg, first, msglen);
        msg[msglen] = '\0';

        char *p = strtok(temp_line, ", ");
        while (p) {
            strcpy(l->tokens[token_cnt], p);
            if (!strcmp(l->tokens[token_cnt], ".STRINGZ")) break;
            token_cnt ++;
            p = strtok(NULL, ", ");
        }
        strcpy(l->tokens[token_cnt+1], msg);
        l->tokens[token_cnt+2][0] = '\0';
    } else {
        char *p = strtok(temp_line, ", ");
        while (p) {
            strcpy(l->tokens[token_cnt], p);
            token_cnt ++;
            p = strtok(NULL, ", ");
        }
        l->tokens[token_cnt][0] = '\0';
    }


}

/**
 * transform the '\\''n' to '\n'.
 * @param raw string of line without comments
 */
void reescape(char *raw) {
    int slashloc;
    char *s = strchr(raw, '\\');
    while (s) {
        slashloc = strcspn(raw, "\\");
        switch (*(s + 1)) {
            case 'a': *s = '\a'; break;
            case 'b': *s = '\b'; break;
            case 'f': *s = '\f'; break;
            case 'n': *s = '\n'; break;
            case 'r': *s = '\r'; break;
            case 't': *s = '\t'; break;
            case 'v': *s = '\v'; break;
            case '\\': *s = '\\'; break;
            case '?': *s = '\?'; break;
            case '\'': *s = '\''; break;
            case '\"': *s = '\"'; break;
            default: break;
        }
        memmove(s+1, s+2, strlen(raw)-slashloc);
        s = strchr(raw, '\\');
    }
}

/**
 * transform string number (#* or x*) to decimal interger.
 * @param token string of number
 * @return integer number in decimal
 */
int getnumber(char *token) {
    int n;
    switch (*token) {
        case '#': sscanf(token, "%*c%d", &n); break;
        case 'x': sscanf(token, "%*c%x", &n); break;
        default: n = INT_MAX; break;
    }
    return n;
}

/**
 * judge if a token is an operation in LC-3.
 * @param token string of token
 * @return 1 if token is an operation, 0 if not.
 */
int isoperation(char *token) {
    const char OPs[][TOKEN_LEN] = { "ADD", "AND", "NOT",
                                    "LD", "LDI", "LDR", "LEA", "ST", "STI", "STR", 
                                    "BR", "BRn", "BRz", "BRp", "BRnz", "BRzp", "BRnp", "BRnzp",
                                    "JMP", "JSR", "JSRR", "RET", "RTI",
                                    ".FILL", ".BLKW", ".STRINGZ",
                                    "TRAP", "GETC", "OUT", "PUTS", "IN", "PUTSP", "HALT"};
    for (int i = 0; i < 33; i ++) {
        if (!strcmp(token, OPs[i]))
            return 1;
    }
    return 0;
}