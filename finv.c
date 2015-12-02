#include <stdio.h>
#include <stdint.h>
#include "fpu.h"

static uint32_t synth(uint32_t sgn, uint32_t ex, uint32_t man) {
  return (sgn << 31) | (ex << 23) | man;
}

static uint64_t maninv_22downto2(uint32_t man_geta) {
  uint32_t manans = 0;
  uint32_t man[22];

  man[0] = (0x1000000 - man_geta) << 1;

  if (man[0] >= man_geta) {
    manans = manans | 0x400000;
    man[1] = (man[0] - man_geta) << 1;
  } else
    man[1] = man[0] << 1;
  if (man[1] >= man_geta) {
    manans = manans | 0x200000;
    man[2] = (man[1] - man_geta) << 1;
  } else
    man[2] = man[1] << 1;
  if (man[2] >= man_geta) {
    manans = manans | 0x100000;
    man[3] = (man[2] - man_geta) << 1;
  } else
    man[3] = man[2] << 1;
  if (man[3] >= man_geta) {
    manans = manans | 0x80000;
    man[4] = (man[3] - man_geta) << 1;
  } else
    man[4] = man[3] << 1;
  if (man[4] >= man_geta) {
    manans = manans | 0x40000;
    man[5] = (man[4] - man_geta) << 1;
  } else
    man[5] = man[4] << 1;
  if (man[5] >= man_geta) {
    manans = manans | 0x20000;
    man[6] = (man[5] - man_geta) << 1;
  } else
    man[6] = man[5] << 1;
  if (man[6] >= man_geta) {
    manans = manans | 0x10000;
    man[7] = (man[6] - man_geta) << 1;
  } else
    man[7] = man[6] << 1;
  if (man[7] >= man_geta) {
    manans = manans | 0x8000;
    man[8] = (man[7] - man_geta) << 1;
  } else
    man[8] = man[7] << 1;
  if (man[8] >= man_geta) {
    manans = manans | 0x4000;
    man[9] = (man[8] - man_geta) << 1;
  } else
    man[9] = man[8] << 1;
  if (man[9] >= man_geta) {
    manans = manans | 0x2000;
    man[10] = (man[9] - man_geta) << 1;
  } else
    man[10] = man[9] << 1;
  if (man[10] >= man_geta) {
    manans = manans | 0x1000;
    man[11] = (man[10] - man_geta) << 1;
  } else
    man[11] = man[10] << 1;
  if (man[11] >= man_geta) {
    manans = manans | 0x800;
    man[12] = (man[11] - man_geta) << 1;
  } else
    man[12] = man[11] << 1;
  if (man[12] >= man_geta) {
    manans = manans | 0x400;
    man[13] = (man[12] - man_geta) << 1;
  } else
    man[13] = man[12] << 1;
  if (man[13] >= man_geta) {
    manans = manans | 0x200;
    man[14] = (man[13] - man_geta) << 1;
  } else
    man[14] = man[13] << 1;
  if (man[14] >= man_geta) {
    manans = manans | 0x100;
    man[15] = (man[14] - man_geta) << 1;
  } else
    man[15] = man[14] << 1;
  if (man[15] >= man_geta) {
    manans = manans | 0x80;
    man[16] = (man[15] - man_geta) << 1;
  } else
    man[16] = man[15] << 1;
  if (man[16] >= man_geta) {
    manans = manans | 0x40;
    man[17] = (man[16] - man_geta) << 1;
  } else
    man[17] = man[16] << 1;
  if (man[17] >= man_geta) {
    manans = manans | 0x20;
    man[18] = (man[17] - man_geta) << 1;
  } else
    man[18] = man[17] << 1;
  if (man[18] >= man_geta) {
    manans = manans | 0x10;
    man[19] = (man[18] - man_geta) << 1;
  } else
    man[19] = man[18] << 1;
  if (man[19] >= man_geta) {
    manans = manans | 0x8;
    man[20] = (man[19] - man_geta) << 1;
  } else
    man[20] = man[19] << 1;
  if (man[20] >= man_geta) {
    manans = manans | 0x4;
    man[21] = (man[20] - man_geta) << 1;
  } else
    man[21] = man[20] << 1;

  return ((uint64_t)man[21] << 32) | manans;
}

static uint64_t maninv_22downto1(uint32_t man_geta) {
  uint32_t manans, man21, man22;
  manans = maninv_22downto2(man_geta) & 0xffffffff;
  man21 = maninv_22downto2(man_geta) >> 32;

  if (man21 >= man_geta) {
    manans = manans | 0x2;
    man22 = (man21 - man_geta) << 1;
  } else
    man22 = man21 << 1;

  return ((uint64_t)man22 << 32) | manans;
}

static uint64_t maninv_22downto0(uint32_t man_geta) {
  uint32_t manans, man22, man23;
  manans = maninv_22downto1(man_geta) & 0xffffffff;
  man22 = maninv_22downto1(man_geta) >> 32;

  if (man22 >= man_geta) {
    manans = manans | 0x1;
    man23 = (man22 - man_geta) << 1;
  } else
    man23 = man22 << 1;

  return ((uint64_t)man23 << 32) | manans;
}

static uint32_t manulp(uint64_t cat, uint32_t man_geta, int i) { // rounding (round even) or carry
  uint32_t man, manans;
  manans = cat & 0xffffff;
  man = cat >> 32;
  return ((man == man_geta && (manans & 1)) || man > man_geta) ? manans+(1<<i) : manans;
}

static uint32_t maninv_sub0(uint32_t man_geta) {
  return manulp(maninv_22downto2(man_geta), man_geta, 2) >> 2;
}

static uint32_t maninv_sub1(uint32_t man_geta) {
  return manulp(maninv_22downto1(man_geta), man_geta, 1) >> 1;
}

static uint32_t maninv_norm(uint32_t man_geta) {
  return manulp(maninv_22downto0(man_geta), man_geta, 0);
}

uint32_t finv(uint32_t a) {

  uint32_t exman_a, ex_a, man_a;
  uint32_t sgnans;

  exman_a = a & 0x7fffffff;
  ex_a = exman_a >> 23;
  man_a = a & 0x7fffff;
  sgnans = a >> 31;

  if (exman_a <= 0x200000) // Sub -> inf
    return synth(sgnans, 0xff, 0);
  else if (exman_a < 0x800000) { // Sub -> Norm
    if (man_a < 0x400000)
      return synth(sgnans, 0xfe, maninv_norm(man_a << 2));
  else if (man_a == 0x400000) // to restrict the size of man_geta to (23 downto 0)
      return synth(sgnans, 0xfe, 0);
    else
      return synth(sgnans, 0xfd, maninv_norm(man_a << 1));
  } else if (exman_a <= 0x7e800000) { // Norm -> Norm
    if (man_a == 0)
      return synth(sgnans, 254-ex_a, 0);
    else
      return synth(sgnans, 253-ex_a, maninv_norm(0x800000 | man_a));
  } else if (exman_a < 0x7f800000) { // Norm -> Sub
    if (exman_a < 0x7f000000)
      return synth(sgnans, 0, 0x400000 | (maninv_sub1(0x800000 | man_a)));
    else if (exman_a == 0x7f000000)
      return synth(sgnans, 0, 0x400000);
    else
      return synth(sgnans, 0, 0x200000 | (maninv_sub0(0x800000 | man_a)));
  } else if (exman_a == 0x7f800000) // inf -> 0
    return synth(sgnans, 0, 0);
  else // NaN
    return synth(sgnans, 0xff, man_a | 0x400000);
}