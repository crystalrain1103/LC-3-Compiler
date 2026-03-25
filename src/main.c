#include "lc3asm.h"

int main(int argc, char *argv[]) {
    char *p;
    files f;
    
    if (argc == 2)
    {
        strcpy(f.in_fname, argv[1]);
        p = strstr(f.in_fname, ".asm");
        if (!p) {
            printf("Please input a file in assembly format\n");
            return 1;
        }
        else {
            strcpy(f.out_bin_fname, f.in_fname);
            p = strstr(f.out_bin_fname, ".asm");
            strcpy(p, ".bin");
            
            strcpy(f.out_sym_fname, f.in_fname);
            p = f.out_sym_fname + strlen(f.out_sym_fname);
            strcpy(p, "_symtable.txt");
            
            f.in = fopen(f.in_fname, "r");
            if (!f.in) {
                printf("Error while opening input file \"%s\"\n", f.in_fname);
                return 1;
            }
            
            f.outbin = fopen(f.out_bin_fname, "w");
            f.outsym = fopen(f.out_sym_fname, "w");
            
            if (!(f.outbin && f.outsym)) {
                printf("Error while opening output files\n");
                return 1;
            }
        }
    }
    else {
        printf("Usage: %s {filename}\n", argv[0]);
        return 1;
    }
    
    assemble(&f);
    
    fclose(f.in);
    fclose(f.outbin);
    fclose(f.outsym);
    return 0;
}