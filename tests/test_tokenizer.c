#include "../src/tokenizer.h"
#include "../src/vm.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Global variables are defined in vm.c, we just use them via extern
// declarations in vm.h

// --- Test Utilities ---
void setup_ip(const char *source) { ip = (char *)source; }

// --- Helper for testing crashes ---
void expect_crash(void (*test_function)(), const char *test_name) {
  printf("Running test: %s... ", test_name);
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) { // Child process
    // Suppress stderr from the child to keep test output clean
    freopen("/dev/null", "w", stderr);
    test_function();
    exit(0); // Should not be reached if crash occurs
  } else {   // Parent process
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
      printf("OK (crashed as expected)\n");
    } else {
      printf("FAIL (did not crash or exited with wrong status)\n");
      if (WIFEXITED(status)) {
        printf("  Exited with status: %d\n", WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("  Terminated by signal: %d\n", WTERMSIG(status));
      }
      exit(1); // Make the whole test suite fail
    }
  }
}

// --- Test Cases ---

// --- Test Cases ---

void test_get_opcode_simple() {
  printf("Running test: test_get_opcode_simple... ");
  setup_ip("010200");
  assert(get_opcode() == 10);
  printf("OK\n");
}

void test_get_opcode_with_whitespace() {
  printf("Running test: test_get_opcode_with_whitespace... ");
  setup_ip("  015  200");
  assert(get_opcode() == 15);
  printf("OK\n");
}

void test_get_opcode_with_single_line_comment() {
  printf("Running test: test_get_opcode_with_single_line_comment... ");
  setup_ip("900200 this is a comment\n025200");
  assert(get_opcode() == 25);
  printf("OK\n");
}

void test_get_opcode_with_multi_line_comment() {
  printf("Running test: test_get_opcode_with_multi_line_comment... ");
  setup_ip("901200 multi\nline\ncomment 901200 030 200");
  assert(get_opcode() == 30);
  printf("OK\n");
}

void test_get_number_literal() {
  printf("Running test: test_get_number_literal... ");
  setup_ip("12345200");
  assert(get_number_literal() == 12345);
  printf("OK\n");
}

void test_consume_separator() {
  printf("Running test: test_consume_separator... ");
  setup_ip("200010");
  assert(consume_separator() == 1);
  assert(*ip == '0');
  setup_ip("010200");
  assert(consume_separator() == 0);
  assert(*ip == '0');
  printf("OK\n");
}

// --- クラッシュを期待するテストケース ---

void crash_test_invalid_comment_in_opcode() {
  setup_ip("01900200comment200"); // 01 は未完成 (2桁) の状態でコメント開始 -> エラー
  get_opcode();                   // exit(1) されることを期待
}

void crash_test_invalid_multiline_comment_in_opcode() {
  setup_ip("01901200comment901200200"); // 01 は未完成 (2桁) の状態でコメント開始 -> エラー
  get_opcode();                         // exit(1) されることを期待
}

void crash_test_legacy_2digit_opcode() {
  setup_ip("01200"); // 旧2桁オペコード -> v3ではエラー
  get_opcode();      // exit(1) されることを期待
}

void crash_test_number_literal_too_long() {
  // 64桁の数値リテラル（バッファ上限63桁超過）
  setup_ip(
      "9901111111111111111111111111111111111111111111111111111111111111111200");
  get_number_literal(); // exit(1) されることを期待
}

// --- テストランナー ---

int main() {
  // 正常系
  test_get_opcode_simple();
  test_get_opcode_with_whitespace();
  test_get_opcode_with_single_line_comment();
  test_get_opcode_with_multi_line_comment();
  test_get_number_literal();
  test_consume_separator();

  // 異常系（クラッシュ確認）
  expect_crash(crash_test_invalid_comment_in_opcode,
               "test_get_opcode_with_invalid_comment");
  expect_crash(crash_test_invalid_multiline_comment_in_opcode,
               "test_get_opcode_with_invalid_multiline_comment");
  expect_crash(crash_test_legacy_2digit_opcode,
               "test_legacy_2digit_opcode");
  expect_crash(crash_test_number_literal_too_long,
               "test_number_literal_too_long");

  printf("\nAll tokenizer tests passed!\n");
  return 0;
}