#include <cstring>
#include <iostream>
#include <limits>

int main() {
  std::string s1, s2;
  std::string input;
  std::cin >> s1 >> s2;
  getline(std::cin, input);
  std::cout << input << "\n";
  return 0;
}