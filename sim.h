#ifndef _INCLUDE_SIM_
#define _INCLUDE_SIM_

#include <stdint.h>
#include <stdbool.h>

#define INST_ADDR (1024*1024*4)
#define DATA_ADDR (1024*1024*4)

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

//各命令実行回数のカウンタ
//nop
extern uint64_t nop_cnt;
//コア命令
extern uint64_t add_cnt;
extern uint64_t addi_cnt;
extern uint64_t sub_cnt;
extern uint64_t ori_cnt;
extern uint64_t sw_cnt;
extern uint64_t lw_cnt;
extern uint64_t slt_cnt;
extern uint64_t beq_cnt;
extern uint64_t bne_cnt;
extern uint64_t fslt_cnt;
extern uint64_t fneg_cnt;
//コア命令2
extern uint64_t sll_cnt;
extern uint64_t srl_cnt;
extern uint64_t j_cnt;
extern uint64_t jr_cnt;
extern uint64_t jal_cnt;
//特殊命令
extern uint64_t rsb_cnt;
extern uint64_t rrb_cnt;
extern uint64_t hlt_cnt;
//浮動小数点数命令
extern uint64_t fadd_cnt;
extern uint64_t fmul_cnt;
extern uint64_t finv_cnt;
extern uint64_t fsqrt_cnt;
extern uint64_t f2i_cnt;
extern uint64_t i2f_cnt;
extern uint64_t flr_cnt;
//全命令
extern uint64_t total_inst_cnt;

//pc毎の命令実行回数
extern uint64_t inst_cnt[INST_ADDR];

#endif
