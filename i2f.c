#include <stdio.h>
#include <stdint.h>

static uint32_t downto(uint32_t a, int x, int y) {
  return x >= 31 ? (a >> y) : (a >> y) & ((1 << (x-y+1)) - 1);
}

typedef union {
  uint32_t u;
  int i;
  float f;
} UIF;

uint32_t i2f(uint32_t a) {
  uint32_t absl, shift = 32, carry;
  if (a == 0x00000000)
    return 0x00000000;
  else if (a == 0x80000000)
    return 0xcf000000;
  else {
    absl = a >> 31 ? downto(~a, 30, 0) + 1 : downto(a, 30, 0);
    while (!(absl >> 31)) {
      absl <<= 1;
      shift--;
    }
    carry = downto(absl, 7, 7) && (downto(absl, 8, 8) | downto(absl, 6, 1));
    return (a & 0x80000000) | ((0x7e + shift + ((downto(absl, 30, 8) == 0x7fffff) && carry)) << 23) | ((downto(absl, 30, 8) + carry) % 0x800000);
  }
}
