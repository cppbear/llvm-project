#include <iostream>

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

void switchStatementExample(char grade) {
  switch (grade) {
  case 'A':
    std::cout << "Excellent!" << std::endl;
    break;
  case 'B':
    std::cout << "Good job!" << std::endl;
    break;
  case 'C':
    std::cout << "Passing grade" << std::endl;
    break;
  default:
    std::cout << "Invalid grade" << std::endl;
  }
}

double divide(double dividend, double divisor) {
  if (divisor == 0) {
    throw std::runtime_error("Divisor cannot be zero");
  }
  return dividend / divisor;
}

void tryCatchStatementExample(double x, double y) {
  try {
    double result = divide(x, y);
    if (result > 0) {
      std::cout << "Result is positive" << std::endl;
    } else if (result < 0) {
      std::cout << "Result is negative" << std::endl;
    } else {
      std::cout << "Result is zero" << std::endl;
    }
    std::cout << "Result: " << result << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
  }
}

void loopStatementExample() {
  for (int i = 0; i < 5; i++) {
    std::cout << "Iteration " << i << std::endl;
  }

  int j = 0;
  while (j < 5) {
    std::cout << "While loop iteration " << j << std::endl;
    j++;
  }

  int k = 0;
  do {
    std::cout << "Do-while loop iteration " << k << std::endl;
    k++;
  } while (k < 5);
}

int main() {
  int x = 10;
  ifStatementExample(x);

  char grade = 'B';
  switchStatementExample(grade);

  tryCatchStatementExample(10, 3);

  loopStatementExample();

  return 0;
}
