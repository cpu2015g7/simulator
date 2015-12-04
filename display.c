#include <stdio.h>
#include <inttypes.h>
#include "sim.h"

typedef union {
	uint32_t u;
	float f;
} UF;

//レジスタ状況を表示
void display_register(void)
{
	float freg[32];
	int i;
	UF uf;
	for (i = 0; i < 32; i++) {
		uf.u = reg[i];
		freg[i] = uf.f;
	}
	fprintf(stderr, "decimal hex float\n");
	fprintf(stderr, "$zero(reg[0]): %"PRIu32" %"PRIx32" %.07f\n", reg[0], reg[0], freg[0]);
	fprintf(stderr, "$at(reg[1]):   %"PRIu32" %"PRIx32" %.07f\n", reg[1], reg[1], freg[1]);
	fprintf(stderr, "$v0(reg[2]):   %"PRIu32" %"PRIx32" %.07f\n", reg[2], reg[2], freg[2]);
	fprintf(stderr, "$v1(reg[3]):   %"PRIu32" %"PRIx32" %.07f\n", reg[3], reg[3], freg[3]);
	fprintf(stderr, "$a0(reg[4]):   %"PRIu32" %"PRIx32" %.07f\n", reg[4], reg[4], freg[4]);
	fprintf(stderr, "$a1(reg[5]):   %"PRIu32" %"PRIx32" %.07f\n", reg[5], reg[5], freg[5]);
	fprintf(stderr, "$a2(reg[6]):   %"PRIu32" %"PRIx32" %.07f\n", reg[6], reg[6], freg[6]);
	fprintf(stderr, "$a3(reg[7]):   %"PRIu32" %"PRIx32" %.07f\n", reg[7], reg[7], freg[7]);
	fprintf(stderr, "$t0(reg[8]):   %"PRIu32" %"PRIx32" %.07f\n", reg[8], reg[8], freg[8]);
	fprintf(stderr, "$t1(reg[9]):   %"PRIu32" %"PRIx32" %.07f\n", reg[9], reg[9], freg[9]);
	fprintf(stderr, "$t2(reg[10]):  %"PRIu32" %"PRIx32" %.07f\n", reg[10], reg[10], freg[10]);
	fprintf(stderr, "$t3(reg[11]):  %"PRIu32" %"PRIx32" %.07f\n", reg[11], reg[11], freg[11]);
	fprintf(stderr, "$t4(reg[12]):  %"PRIu32" %"PRIx32" %.07f\n", reg[12], reg[12], freg[12]);
	fprintf(stderr, "$t5(reg[13]):  %"PRIu32" %"PRIx32" %.07f\n", reg[13], reg[13], freg[13]);
	fprintf(stderr, "$t6(reg[14]):  %"PRIu32" %"PRIx32" %.07f\n", reg[14], reg[14], freg[14]);
	fprintf(stderr, "$t7(reg[15]):  %"PRIu32" %"PRIx32" %.07f\n", reg[15], reg[15], freg[15]);
	fprintf(stderr, "$s0(reg[16]):  %"PRIu32" %"PRIx32" %.07f\n", reg[16], reg[16], freg[16]);
	fprintf(stderr, "$s1(reg[17]):  %"PRIu32" %"PRIx32" %.07f\n", reg[17], reg[17], freg[17]);
	fprintf(stderr, "$s2(reg[18]):  %"PRIu32" %"PRIx32" %.07f\n", reg[18], reg[18], freg[18]);
	fprintf(stderr, "$s3(reg[19]):  %"PRIu32" %"PRIx32" %.07f\n", reg[19], reg[19], freg[19]);
	fprintf(stderr, "$s4(reg[20]):  %"PRIu32" %"PRIx32" %.07f\n", reg[20], reg[20], freg[20]);
	fprintf(stderr, "$s5(reg[21]):  %"PRIu32" %"PRIx32" %.07f\n", reg[21], reg[21], freg[21]);
	fprintf(stderr, "$s6(reg[22]):  %"PRIu32" %"PRIx32" %.07f\n", reg[22], reg[22], freg[22]);
	fprintf(stderr, "$s7(reg[23]):  %"PRIu32" %"PRIx32" %.07f\n", reg[23], reg[23], freg[23]);
	fprintf(stderr, "$t8(reg[24]):  %"PRIu32" %"PRIx32" %.07f\n", reg[24], reg[24], freg[24]);
	fprintf(stderr, "$t9(reg[25]):  %"PRIu32" %"PRIx32" %.07f\n", reg[25], reg[25], freg[25]);
	fprintf(stderr, "$k0(reg[26]):  %"PRIu32" %"PRIx32" %.07f\n", reg[26], reg[26], freg[26]);
	fprintf(stderr, "$k1(reg[27]):  %"PRIu32" %"PRIx32" %.07f\n", reg[27], reg[27], freg[27]);
	fprintf(stderr, "$gp(reg[28]):  %"PRIu32" %"PRIx32" %.07f\n", reg[28], reg[28], freg[28]);
	fprintf(stderr, "$sp(reg[29]):  %"PRIu32" %"PRIx32" %.07f\n", reg[29], reg[29], freg[29]);
	fprintf(stderr, "$fp(reg[30]):  %"PRIu32" %"PRIx32" %.07f\n", reg[30], reg[30], freg[30]);
	fprintf(stderr, "$ra(reg[31]):  %"PRIu32" %"PRIx32" %.07f\n", reg[31], reg[31], freg[31]);
}

//命令実行回数を表示
void display_instruction(void)
{
	//各命令実行回数を表示
	fprintf(stderr, "nop:   %"PRIu64"\n", nop_cnt);
	fprintf(stderr, "add:   %"PRIu64"\n", add_cnt);
	fprintf(stderr, "addi:  %"PRIu64"\n", addi_cnt);
	fprintf(stderr, "sub:   %"PRIu64"\n", sub_cnt);
	fprintf(stderr, "ori:   %"PRIu64"\n", ori_cnt);
	fprintf(stderr, "sw:    %"PRIu64"\n", sw_cnt);
	fprintf(stderr, "lw:    %"PRIu64"\n", lw_cnt);
	fprintf(stderr, "slt:   %"PRIu64"\n", slt_cnt);
	fprintf(stderr, "beq:   %"PRIu64"\n", beq_cnt);
	fprintf(stderr, "bne:   %"PRIu64"\n", bne_cnt);
	fprintf(stderr, "fslt:  %"PRIu64"\n", fslt_cnt);
	fprintf(stderr, "fneg:  %"PRIu64"\n", fneg_cnt);
	fprintf(stderr, "f2i:   %"PRIu64"\n", f2i_cnt);
	fprintf(stderr, "i2f:   %"PRIu64"\n", i2f_cnt);
	fprintf(stderr, "flr:   %"PRIu64"\n", flr_cnt);
	fprintf(stderr, "sll:   %"PRIu64"\n", sll_cnt);
	fprintf(stderr, "srl:   %"PRIu64"\n", srl_cnt);
	fprintf(stderr, "j:     %"PRIu64"\n", j_cnt);
	fprintf(stderr, "jr:    %"PRIu64"\n", jr_cnt);
	fprintf(stderr, "jal:   %"PRIu64"\n", jal_cnt);
	fprintf(stderr, "rsb:   %"PRIu64"\n", rsb_cnt);
	fprintf(stderr, "rrb:   %"PRIu64"\n", rrb_cnt);
	fprintf(stderr, "hlt:   %"PRIu64"\n", hlt_cnt);
	fprintf(stderr, "fadd:  %"PRIu64"\n", fadd_cnt);
	fprintf(stderr, "fmul:  %"PRIu64"\n", fmul_cnt);
	fprintf(stderr, "finv:  %"PRIu64"\n", finv_cnt);
	fprintf(stderr, "f2i:   %"PRIu64"\n", f2i_cnt);
	fprintf(stderr, "fsqrt: %"PRIu64"\n", fsqrt_cnt);
	fprintf(stderr, "# of total instructions: %"PRIu64"\n", total_inst_cnt);
}

//スタックの状況を表示
void display_stack(void)
{
	uint32_t i;
	UF uf;
	fprintf(stderr, "decimal hex float\n");
	for (i = reg[29]; i < DATA_ADDR; i++) {
		uf.u = DATA_MEM[i];
		fprintf(stderr, "%"PRIu32" %"PRIx32" %f\n", DATA_MEM[i], DATA_MEM[i], uf.f);
	}

}

//ヒストグラム
void display_inst_address_histgram()
{
	int32_t i, j;
	int32_t width = 256;
	int32_t max_tile = 140;
	char tile_str[500] = {};
	for(i = 0; i <= INST_ADDR - width; i += width) {
		uint64_t sum = 0;
		for(j = 0; j < width; j++) {
			sum += inst_cnt[i + j];
		}
		if(sum > 0) {
			int32_t tile = (sum * max_tile + total_inst_cnt / 2) / total_inst_cnt;
			for(j = 0; j < tile - 1; j++) {
				tile_str[j] = '=';
			}
			if(tile > 0) {
			   	tile_str[tile - 1] = '*';
			}
			tile_str[tile] = '\0';
			fprintf(stderr, "0x%05x-0x%05x: %11"PRIu64":%s\n", i, i + width - 1, sum,  tile_str);
		}
	}
}
