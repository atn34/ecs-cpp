#ifndef CIRCULAR_INTERVAL
#define CIRCULAR_INTERVAL

#include <cstddef>

struct CircularInterval {
  std::size_t modulus;
  std::size_t start;   // 0 <= length < modulus
  std::size_t length;  // 0 <= length <= modulus

  // Precondition: 0 <= index < modulus
  bool contains(std::size_t index) {
    return (start <= index && index < start + length) ||
           (start <= index + modulus && index + modulus < start + length);
  }

  // Precondition: 0 <= index < modulus
  // Postcondition: contains(index)
  //
  // Always increases the size of the interval (up to modulus)
  void expand(std::size_t index) {
    if (start == index) {
      if (length == 0) {
        length = 1;
      } else {
        length = modulus;
      }
    } else if (start <= index) {
      length = index - start + 1;
    } else {
      length = index - start + modulus + 1;
    }
  }
};

#endif /* ifndef CIRCULAR_INTERVAL */
