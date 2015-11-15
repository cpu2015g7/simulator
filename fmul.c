#include <stdio.h>
#include <stdint.h>
#include "fmul.h"

static uint32_t sgn(uint32_t x)
{
  return x >> 31;
}

static uint32_t ex(uint32_t x)
{
  return (x >> 23) & 0xff;
}

static uint32_t man(uint32_t x)
{
  return x & 0x7fffff;
}

static uint32_t synth(uint32_t sgn, uint32_t ex, uint32_t man)
{
  return (sgn << 31) | (ex << 23) | man;
}

static uint32_t bigger(uint32_t x, uint32_t y)
{
  return (x & 0x7fffffff) > (y & 0x7fffffff) ? x : y;
}

static uint32_t smaller(uint32_t x, uint32_t y)
{
  return y == bigger(x, y) ? x : y;
}

uint32_t fmul(uint32_t a, uint32_t b)
{
  uint32_t big, small;
  uint32_t sgnans, manans;
  uint32_t ex0, ex1, ex2, shift;
  uint64_t manprd0, manprd1;
  uint32_t mansub0, mansub1, mancut;
  int carrysign = 0;

  big = bigger(a, b);
  small = smaller(a, b);

  sgnans = (sgn(a) + sgn(b)) & 1; // xor

  if (ex(big) == 0xff) { // NaN, inf
    if (man(big)) { // NaN
      if (ex(small) == 0xff && man(small)) // NaN * NaN
        return b | 0x400000;
      else // NaN * x
        return big | 0x400000;
    } else { // inf
      if (small & 0x7fffffff) // inf * x(not 0)
        return synth(sgnans, 0xff, 0);
      else // inf * 0
        return 0xffc00000;
    }
  }

  if ((small & 0x7fffffff) == 0)
    return synth(sgnans, 0, 0);

  if (ex(small)) {
    ex0 = ex(a) + ex(b);
    manprd0 = (0x800000 | (uint64_t)man(a)) * (0x800000 | (uint64_t)man(b));
  } else { // x * Sub
    ex0 = ex(big)+1;
    manprd0 = (0x800000 | (uint64_t)man(big)) * (uint64_t)man(small);
    while (ex0 > 0 && (manprd0 >> 46) == 0) {
      manprd0 <<= 1;
      ex0 -= 1;
    } //if (ex0 == 0)
      //return synth(sgnans, 0, 0);
  }

  if ((manprd0 >> 47) & 1) {
    carrysign = manprd0 & 1;
    ex1 = ex0+1;
    manprd1 = manprd0 >> 1;
  } else {
    ex1 = ex0;
    manprd1 = manprd0;
  }

  if (ex1 < 104)
    return synth(sgnans, 0, 0); // 0
  else if (ex1 < 128) { // underflow?
    shift = 55 - (ex1 & 0x1f);
    mansub0 = manprd1 >> (shift-1);
    mansub1 = mansub0 >> 1;
    mancut = manprd1 - (mansub1 << shift);
    if ((mansub0 & 1) && (mancut || carrysign || (mansub1 & 1))) { // carry or rounding
      if (mansub1 == 0x7fffff) // actually, this is not needed in this C code
        return synth(sgnans, 1, 0);
      else
        manans = mansub1+1;
    } else
      manans = mansub1;
    return synth(sgnans, 0, manans);
  } else if (ex1 < 382) { // Norm
    ex2 = ex1-127;
    if (((manprd1 >> 22) & 1) && ((manprd1 & 0x3fffff) || carrysign || ((manprd1 >> 23) & 1))) { // carry or rounding (round even)
      if ((manprd1 >> 23) == 0xffffff)
        return synth(sgnans, ex2+1, 0);
      else
        manans = ((manprd1 >> 23)+1) & 0x7fffff;
    } else
      manans = (manprd1 >> 23) & 0x7fffff;
    return synth(sgnans, ex2, manans);
  } else // overflow
    return synth(sgnans, 0xff, 0);
}