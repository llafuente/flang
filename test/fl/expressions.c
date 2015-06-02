#include <stdio.h>

// clang -S -emit-llvm -O0 test/fl/expressions.c -o test/fl/expressions.ll

int main() {
  // sign maddness
/*
  unsigned int uy = 0;
  unsigned char uz = 0;

  double x = 0.5;
  int y = 0;
  char z = 0;
  y = (int) x; //  %3 = fptosi double %2 to i32
  x = (double) y; // %5 = sitofp i32 %4 to double

  z = (char) y; // trunc i32 %6 to i8
  y = (int) z; // sext i8 %8 to i32
*/

/*
  uz = (unsigned char) y; //  %3 = trunc i32 %2 to i8
  uy = (unsigned int) z; // sext i8 %4 to i32
  z = (char) uz; // direct
  y = (int) uy; // direct

  char a; int b;
  unsigned char ua; unsigned  int ub;

  b = a; // sext i8 %2 to i32
  ub = ua; // zext i8 %4 to i32
  // grow and chane sign
  ub = a; // sext i8 %6 to i32
  b = ua; // zext i8 %8 to i32

  // truncate
  a = b; // trunc i32 %10 to i8
  ua = ub; // trunc i32 %12 to i8
  a = ub; // trunc i32 %14 to i8
  ua = b; // trunc i32 %16 to i8
*/
  char x = 100;
  float y = 50;

  double z = x + y;


  /*
  double x;
  double y;

  x = 10;
  y = 5;

  printf("x + y = %f\n", x + y);
  printf("x - y = %f\n", x - y);
  printf("x * y = %f\n", x * y);
  printf("x / y = %f\n", x / y);

  int l = 65;
  int r = 129;

  printf("x / y = %d\n", l & r);
  printf("x / y = %d\n", l | r);
  printf("x / y = %d\n", l ^ r);
  //printf("x / y = %d\n", l ~ r);

  printf("x / y = %d\n", x || y);
  printf("x / y = %d\n", x && y);
  */


  return 0;
}
