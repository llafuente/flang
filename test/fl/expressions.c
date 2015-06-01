#include <stdio.h>

// clang -S -emit-llvm -O0 test/fl/expressions.c -o test/fl/expressions.ll

int main() {
  double x;
  double y;

  x = 10;
  y = 5;

  printf("x + y = %f\n", x + y);
  printf("x - y = %f\n", x - y);
  printf("x * y = %f\n", x * y);
  printf("x / y = %f\n", x / y);

  return 0;
}
