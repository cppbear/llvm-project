#include <iostream>

// void loopStatementExample() {
//   for (int i = 0; i < 5; i++) {
//     std::cout << "Iteration " << i << std::endl;
//   }

//   int j = 0;
//   while (j < 5) {
//     std::cout << "While loop iteration " << j << std::endl;
//     j++;
//   }

//   int k = 0;
//   do {
//     std::cout << "Do-while loop iteration " << k << std::endl;
//     k++;
//   } while (k < 5);
// }

bool isEven(int num) { return num % 2 == 0; }

bool isOdd(int num) { return num % 2 != 0; }

void ifStatementExample(int x) {
  if (x > 0 && isEven(x)) {
    std::cout << "x is positive and even" << std::endl;
  } else if (x > 0 && isOdd(x)) {
    std::cout << "x is positive and odd" << std::endl;
  } else if (x < 0) {
    std::cout << "x is negative" << std::endl;
  } else {
    std::cout << "x is zero" << std::endl;
  }
}

// int foo(int x) {
//   if ((x > 0 && x < 10) || x < 0) {
//     return 1;
//   }
//   return 0;
// }

// int bar(int x) {
//   if (x > 0 || (x > -10 && x < 0)) {
//     return 2;
//   }
//   return 0;
// }

// void test(bool A, bool B, bool C, bool D) {
//   if (A && B) {
//     std::cout << "A and B\n";
//   }
//   if (C || D) {
//     std::cout << "C or D\n";
//   }
// }

// int testif(int a, int b, int c, int d, int e, int f, int g, int h, int i,
//             int j, int k) {
//   if (((((a > 10)) && b < 5) || (c == 20 && d != 30)) &&
//           ((e <= 15 || f > 25) && (g == 5 || h != 7)) ||
//       (i >= 100 && (j < 50 || k != 60))) {
//     return 3;
//   }
//   return 0;
// }

// void forStatementExample() {
//   int i = 0;
//   for (;;) {
//     std::cout << "Iteration " << i << std::endl;
//     ++i;
//     if (i == 5) {
//       break;
//     }
//   }
// }
