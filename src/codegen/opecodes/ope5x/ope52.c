#include <stdio.h>
#include <stdlib.h>

#include "../../../tokenizer.h"
#include "ope52.h"

// POP命令 (501): スタックのトップから値を1つ破棄する
void opecode_52(FILE *out_file) {
    if (!consume_separator()) {
        fprintf(stderr, "Error: Expected '200' after 501\n");
        exit(1);
    }
    fprintf(out_file, "    # POP\n");
    fprintf(out_file, "    pop rax\n");
}
