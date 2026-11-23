#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Helper function to run a command and check its status
void run_command(const char* command, const char* test_name) {
    printf("    Executing: %s\n", command);
    int status = system(command);
    if (status != 0) {
        fprintf(stderr, "\n🔴 Command failed for test '%s': %s\n", test_name, command);
        // We don't exit here to allow cleanup to run
        assert(status == 0);
    }
}

// Helper function to read a file into a string
char* read_output_file(const char* filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = malloc(len + 1);
    if (buffer) {
        fread(buffer, 1, len, f);
        buffer[len] = '\0';
    }
    fclose(f);
    return buffer;
}

void run_test(const char* test_name, const char* cless_code, const char* expected_output) {
    printf("Running test: %s...\n", test_name);

    char cless_filename[50];
    char output_filename[50];
    char compile_command[100];
    char run_command_str[100];

    // Setup filenames and commands
    sprintf(cless_filename, "%s.cless", test_name);
    sprintf(output_filename, "%s.txt", test_name);
    sprintf(compile_command, "./clessc %s", cless_filename);
    sprintf(run_command_str, "./cless.out > %s", output_filename);

    // 1. Create .cless file
    FILE* f = fopen(cless_filename, "w");
    assert(f != NULL);
    fprintf(f, "%s", cless_code);
    fclose(f);

    // 2. Compile
    run_command(compile_command, test_name);

    // 3. Run and capture output
    run_command(run_command_str, test_name);

    // 4. Check output
    char* actual_output = read_output_file(output_filename);
    printf("    Expected: '%s', Got: '%s'\n", expected_output, actual_output ? actual_output : "(null)");
    assert(actual_output != NULL);
    assert(strcmp(actual_output, expected_output) == 0);
    
    free(actual_output);

    // 5. Cleanup
    remove(cless_filename);
    remove("cless.out");
    remove(output_filename);

    printf("✅ Test %s PASSED\n\n", test_name);
}

int main() {
    printf("===== Running Codegen E2E Tests (v3) =====\n");

    // 10 != 20 -> EQN(801) が 1 -> JZ(701) はジャンプせず "A" を出力して HALT へジャンプ
    run_test(
        "if_not_equal",
        "50020099010200500200990202008012007012009907420001020026520070020099086200010200266200000200",
        "A"
    );

    // 10 == 10 -> EQN(801) が 0 -> JZ(701) で位置74へジャンプし "B" を出力
    run_test(
        "if_equal",
        "50020099010200500200990102008012007012009907420001020026520070020099086200010200266200000200",
        "B"
    );

    // PUSH 10, PUSH 99, POP(501), PUSH 32, ADD(600), PRINT(020), HALT(000) -> "42\n"
    run_test(
        "pop_and_add",
        "500200990102005002009909920050120050020099032200600200020200000200",
        "42\n"
    );

    // v3コメント (900200 / 901200) を含む文字列出力
    run_test(
        "v3_comments",
        "900200 single line comment\n901200 multi\nline comment 901200010200265200000200",
        "A"
    );

    printf("\nAll codegen tests passed!\n");
    return 0;
}
