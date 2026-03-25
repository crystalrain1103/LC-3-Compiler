#include "lc3asm.h"

/**
 * append to a symbol table.
 * @param table pointer to symble table
 * @param label a string of label to append
 * @param addr the address of the label
 */
void append_symtable(symtable *table, char *label, int addr) {
    if (!(*table)) {
        *table = (symbol *) malloc(sizeof(symbol));
        if (!(*table)) {
            fprintf(stderr, "Created symbol table failed.");
            return;
        }
        strcpy((*table)->label, label);
        (*table)->addr = addr;
        (*table)->next = NULL;
    } else {
        append_symtable(&((*table)->next), label, addr);
    }
}

/**
 * search for given label.
 * @param table symbol table
 * @param label a string of label to search
 * @return address of given label, -1 if not found
 */
int search_symtable(symtable table, char *label) {
    if (!table)
        return -1;
    if (!strcmp(table->label, label))
        return table->addr;
    else
        return search_symtable(table->next, label); 
}

/**
 * free memory of symbol table.
 * @param table symbol table to destroy
 */
void destroy_symtable(symtable table) {
    if (!table) return;
    symbol *temp;
    while (table->next) {
        temp = table->next;
        table->next = temp->next;
        free(temp);
    }
    free(table);
}

/**
 * write symbol table into file.
 * @param table symbol table to write
 * @param f files (typed files_t) to write
 */
void write_symfile(symtable table, files *f) {
    fprintf(f->outsym, "--- Symbol Table ---\n");
    symbol *temp;
    temp = table;
    while (temp) {
        fprintf(f->outsym, "%s : x%x\n", temp->label, temp->addr);
        temp = temp->next;
    }
    fprintf(f->outsym, "--------------------\n");
}