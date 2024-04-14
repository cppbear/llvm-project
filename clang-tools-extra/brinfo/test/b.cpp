#include <iostream>

int foo() { return 2; }

int bar() {return 3;}

void loop(int x) {
  int t = foo() + bar();
  for (int i = 0; i < x && x > 10; i++) {
    if (i % 2 == 0)
      std::cout << "Even iteration " << i << std::endl;
    else
      std::cout << "Odd iteration " << i << std::endl;
  }
  int s = foo() - bar();
}
