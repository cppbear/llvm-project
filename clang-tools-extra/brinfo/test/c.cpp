#include <iostream>

class Reader {
public:
  using Char = char;
  using Location = const Char *;

  Reader();

  bool good() const;

private:

  bool readComment();
  bool readCStyleComment();
  bool readCppStyleComment();

  Char getNextChar();

  void addComment(Location begin, Location end, int placement);

  bool containsNewLine(Location begin, Location end);


  Location begin_{};
  Location end_{};
  Location current_{};
  Location lastValueEnd_{};
  bool collectComments_{};
}; // Reader

char Reader::getNextChar() {
  return 'a';
}

bool Reader::readComment() {
  Location commentBegin = current_ - 1;
  Char c = getNextChar();
  bool successful = false;
  if (c == '*')
    successful = readCStyleComment();
  else if (c == '/')
    successful = readCppStyleComment();
  if (!successful)
    return false;

  if (collectComments_) {
    int placement = 0;
    if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
      if (c != '*' || !containsNewLine(commentBegin, current_))
        placement = 1;
    }

    addComment(commentBegin, current_, placement);
  }
  return true;
}

// bool isEven(int num) { return num % 2 == 0; }

// bool isOdd(int num) { return num % 2 != 0; }

// void ifExample(int x) {
//   if (x > 0 && isEven(x)) {
//     std::cout << "x is positive and even" << std::endl;
//   } else if (x > 0 && isOdd(x)) {
//     std::cout << "x is positive and odd" << std::endl;
//   } else if (x < 0) {
//     std::cout << "x is negative" << std::endl;
//   } else {
//     std::cout << "x is zero" << std::endl;
//   }
// }
