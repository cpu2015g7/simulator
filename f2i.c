#include <stdio.h>
#include <stdint.h>

static uint32_t downto(uint32_t a, int x, int y) {
  return x >= 31 ? (a >> y) : (a >> y) & ((1 << (x-y+1)) - 1);
}

uint32_t f2i(uint32_t a) {
  uint32_t tmp, shift, which;
  shift = (downto(a, 27, 24) + 1) % 0x10;
  tmp = (0x80000000 | (downto(a, 22, 0) << 8)) >> (31 - ((shift << 1) | downto(a, 23, 23)));
  which = (a >> 31) ? downto(~tmp, 31, 1) + !(tmp&1) : downto(tmp, 31, 1) + (tmp&1);
  if (downto(a, 30, 23) < 0x7e)
    return 0x00000000;
  else if (downto(a, 30, 23) >= 0x9e)
    return 0x80000000;
  else
    return (a & 0x80000000) | which;
}