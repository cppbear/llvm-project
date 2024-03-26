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

int foo(int x) {
  if ((x > 0 && x < 10) || x < 0) {
    return 1;
  }
  return 0;
}

int bar(int x) {
  if (x > 0 || (x > -10 && x < 0)) {
    return 2;
  }
  return 0;
}
