#include <iostream>

void processNumber(int number) {
  if (number > 0) {
    std::cout << "Number is positive." << std::endl;
  } else if (number < 0) {
    std::cout << "Number is negative." << std::endl;
  } else {
    std::cout << "Number is zero." << std::endl;
  }
}

int main() {
  int num = 10;
  processNumber(num);

  return 0;
}
