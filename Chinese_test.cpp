#include <cstring>
#include <iostream>

int main() {
  char a[100] = "hello";
  char b[100] = "b";
  strcpy(a, b);
  std::cout << strcmp(a, b);
  return 0;
}