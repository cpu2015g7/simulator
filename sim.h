#ifndef _INCLUDE_SIM_
#define _INCLUDE_SIM_

#include <stdint.h>
#include <stdbool.h>

#define TYPE 27

#define INST_ADDR (1024*1024*4)
#define DATA_ADDR (1024*1024*4)

//rsbの出力をcoreと異なる表示(16進数表示)にするかどうか
extern bool NON_CORE;
//浮動小数点数命令でFPUのC実装を使うかどうか
extern bool USE_FPU;
//実行命令回数表示時にソートするかどうか
extern bool SORT;

//命令用メモリ
extern uint32_t INST_MEM[INST_ADDR];
//データ用メモリ
extern uint32_t DATA_MEM[DATA_ADDR];

//レジスタ
//$zero		reg[0]			定数の0
//$at		reg[1]
//$v0,v1	reg[2],reg[3]
//$a0-$a3	reg[4]-reg[7]	関数の引数
//$t0-$t7	reg[8]-reg[15]
//$s0-$s7	reg[16]-reg[23]
//$t8,$t9	reg[24],reg[25]
//$k0,$k1	reg[26],reg[27]	使わない
//$gp		reg[28]
//$sp		reg[29]			スタックポインタ
//$fp		reg[30]			使わない
//$ra		reg[31]			戻りアドレス
extern uint32_t reg[32];

enum Instruction {
	//nop
	nop_cnt,
	//コア命令
	add_cnt, addi_cnt, sub_cnt, ori_cnt, sw_cnt, lw_cnt, slt_cnt, beq_cnt, bne_cnt, fslt_cnt, fneg_cnt,
	//コア命令2
	sll_cnt, srl_cnt, j_cnt, jr_cnt, jal_cnt,
	//特殊命令
	rsb_cnt, rrb_cnt, hlt_cnt,
	//浮動小数点数命令
	fadd_cnt, fmul_cnt, finv_cnt, fsqrt_cnt, f2i_cnt, i2f_cnt, flr_cnt
};
//各命令実行回数
extern uint64_t each_inst_cnt[TYPE];

//全命令実行回数
extern uint64_t total_inst_cnt;

//pc毎の命令実行回数
extern uint64_t pc_inst_cnt[INST_ADDR];

#endif
