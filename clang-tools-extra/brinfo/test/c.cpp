#include <iostream>

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
