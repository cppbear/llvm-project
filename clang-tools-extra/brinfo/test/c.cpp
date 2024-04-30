#include <iostream>

// class Reader {
// public:
//   using Char = char;
//   using Location = const Char *;

//   Reader();

//   bool good() const;

// private:

//   bool readComment();
//   bool readCStyleComment();
//   bool readCppStyleComment();

//   Char getNextChar();

//   void addComment(Location begin, Location end, int placement);

//   bool containsNewLine(Location begin, Location end);

//   Location begin_{};
//   Location end_{};
//   Location current_{};
//   Location lastValueEnd_{};
//   bool collectComments_{};
// }; // Reader

// char Reader::getNextChar() {
//   return 'a';
// }

// bool Reader::readComment() {
//   Location commentBegin = current_ - 1;
//   Char c = getNextChar();
//   bool successful = false;
//   if (c == '*')
//     successful = readCStyleComment();
//   else if (c == '/')
//     successful = readCppStyleComment();
//   if (!successful)
//     return false;

//   if (collectComments_) {
//     int placement = 0;
//     if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
//       if (c != '*' || !containsNewLine(commentBegin, current_))
//         placement = 1;
//     }

//     addComment(commentBegin, current_, placement);
//   }
//   return true;
// }

void readComment() {
  for (int i = 0; i < 10; i++) {
    if (i % 2 == 0) {
      std::cout << "Even\n";
    }
  }
}
