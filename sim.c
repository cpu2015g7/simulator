//注意点
//一段遅延分岐->beq,bne命令の直後の命令は如何なる場合も実行される
//尚、jump系の命令(j, jr, jal)はすぐさま飛ぶ

//#include <assert.h>より前にNDEBUGマクロがあるとassert()は何もしない
//#define NDEBUG

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "finv.h"
#include "fmul.h"

//rsbの出力をcoreと同じ出力にするかどうか
#define CORRESPOND_CORE true

//浮動小数点数命令でFPUのC実装を使うかどうか
#define USE_FPU false

#define INST_ADDR (1024*1024*4)
#define DATA_ADDR (1024*1024*4)

//命令用メモリ
uint32_t INST_MEM[INST_ADDR];
//データ用メモリ
uint32_t DATA_MEM[DATA_ADDR];

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
uint32_t reg[32];

//プログラムカウンタ
uint32_t pc;
//pcに加算する値
//pcは基本的に+1で良いが、一段遅延分岐が関わる命令(beq,bne)では要注意
uint32_t dpc = 1;

//各命令実行回数のカウンタ
//nop
uint32_t nop_cnt;
//コア命令
uint32_t add_cnt;
uint32_t addi_cnt;
uint32_t sub_cnt;
uint32_t ori_cnt;
uint32_t sw_cnt;
uint32_t lw_cnt;
uint32_t slt_cnt;
uint32_t beq_cnt;
uint32_t bne_cnt;
uint32_t fslt_cnt;
uint32_t fneg_cnt;
//コア命令2
uint32_t sll_cnt;
uint32_t srl_cnt;
uint32_t j_cnt;
uint32_t jr_cnt;
uint32_t jal_cnt;
//特殊命令
uint32_t rsb_cnt;
uint32_t rrb_cnt;
//浮動小数点数命令
uint32_t fadd_cnt;
uint32_t fmul_cnt;
uint32_t finv_cnt;
uint32_t f2i_cnt;
uint32_t fsqrt_cnt;

//2進数文字列を10進数符号無し整数(32bits)に変換
uint32_t bin2uint32(const char *ptr)
{
    uint32_t val = 0;
    while( *ptr != '\0' ) {
        switch( *ptr++ ) {
            case '0':
                val *= 2;
                break;
            case '1':
                val = val * 2 + 1;
                break;
        }
    }
    return val;
}

//nbit目からmbit目を取り出す
uint32_t bits(uint32_t buf, int n, int m)
{
	return ((buf << n) >> n) >> (31 - m);
}

//INST_MEMへの命令読み込みを行う
uint32_t store_instruction(FILE *inst_file)
{
	uint32_t pos = 0;
	//文字列状態の命令を格納(32bits+'\n'+'\0')
	char buf[34];
	while(fgets(buf, sizeof(buf), inst_file)) {
		strtok(buf, "\n"); //'\n'を'\0'にする
		//2進数(文字列)を10進数(符号無し32bits整数)に変換して命令用メモリに格納
		INST_MEM[pos] = bin2uint32(buf);
		pos++;
	}

	return pos;
}

//R形式の命令を実行
void execute_R(uint32_t instruction)
{
	uint32_t rs = bits(instruction, 6, 10);
	uint32_t rt = bits(instruction, 11, 15);
	uint32_t rd = bits(instruction, 16, 20);
	uint32_t shamt = bits(instruction, 21, 25);
	uint32_t funct = bits(instruction, 26, 31);

	//fslt用
	union {
		uint32_t reg;
		float freg;
	} u;
	float frs, frt;

	switch (funct) {
		//add
		case 0x20:
			reg[rd] = reg[rs] + reg[rt];
			add_cnt++;
			break;
		//sub
		case 0x22:
			reg[rd] = reg[rs] - reg[rt];
			sub_cnt++;
			break;
		//slt
		case 0x2a:
			reg[rd] = ((int)reg[rs] < (int)reg[rt]) ? 1 : 0;
			slt_cnt++;
			break;
		//fslt
		case 0x2b:
			u.reg = reg[rs];
			frs = u.freg;
			u.reg = reg[rt];
			frt = u.freg;
			reg[rd] = (frs < frt) ? 1 : 0;
			fslt_cnt++;
			break;
		//fneg
		case 0x2c:
			reg[rd] = reg[rs] ^ 0x80000000;
			fneg_cnt++;
			break;
		//sll
		case 0x00:
			reg[rd] = reg[rt] << shamt;
			sll_cnt++;
			break;
		//srl
		case 0x02:
			reg[rd] = reg[rt] >> shamt;
			srl_cnt++;
			break;
		//jr
		case 0x08:
			pc = reg[rs];
			jr_cnt++;
			break;
		default:
			assert(false);
	}
}

//R形式(浮動小数点数)の命令を実行
void execute_R_f(uint32_t instruction)
{
	uint32_t rs = bits(instruction, 6, 10);
	uint32_t rt = bits(instruction, 11, 15);
	uint32_t rd = bits(instruction, 16, 20);
	uint32_t funct = bits(instruction, 26, 31);

	union {
		uint32_t reg;
		float freg;
	} u;
	float frs, frt;
	u.reg = reg[rs];
	frs = u.freg;
	u.reg = reg[rt];
	frt = u.freg;

	switch (funct) {
		//fadd
		case 0x20:
			u.freg = frs + frt;
			reg[rd] = u.reg;
			fadd_cnt++;
			break;
		//fmul
		case 0x01:
			if (USE_FPU)
				reg[rd] = fmul(reg[rs], reg[rt]);
			else {
				u.freg = frs * frt;
				reg[rd] = u.reg;
			}
			fmul_cnt++;
			break;
		//finv
		case 0x03:
			if (USE_FPU)
				reg[rd] = finv(reg[rs]);
			else {
				u.freg = 1.0 / frs;
				reg[rd] = u.reg;
			}
			finv_cnt++;
			break;
		//f2i
		case 0x08:
			reg[rd] = (uint32_t)roundf(frs);
			f2i_cnt++;
			break;
		//fsqrt
		case 0x18:
			u.freg = sqrtf(frs);
			reg[rd] = u.reg;
			fsqrt_cnt++;
			break;
		default:
			assert(false);
	}
}

//命令実行
//R形式ならばexecute_R(),execute_R_f()を呼び出し、
//I,J形式ならば実行する
//R,I,J,どの形式でもpc(プログラムカウンタ)の制御はここで行う
void execute(uint32_t instruction)
{
	uint32_t op = bits(instruction, 0, 5);
	 //I形式用
	uint32_t rs = bits(instruction, 6, 10);
	uint32_t rt = bits(instruction, 11, 15);
	uint32_t addr_imm = bits(instruction, 16, 31);
	//J形式用
	uint32_t addr = bits(instruction, 6, 31);
	//rrb用
	uint8_t rrb_buf;


	pc += dpc;
	dpc = 1;

	//nop
	if (instruction == 0) {
		nop_cnt++;
		return;
	}
	//nop以外
	switch (op) {
		//R形式
		case 0x00:
			execute_R(instruction);
			break;

		//R形式(浮動小数点数命令)
		case 0x0b:
			execute_R_f(instruction);
			break;

		//I形式
		//addi
		case 0x0c:
			//符号拡張
			if ((addr_imm & 0x8000) != 0)
				addr_imm |= 0xffff0000;
			reg[rt] = reg[rs] + addr_imm;
			addi_cnt++;
			break;
		//ori
		case 0x0d:
			reg[rt] = reg[rs] | addr_imm;
			ori_cnt++;
			break;
		//sw
		case 0x2b:
			//符号拡張
			if ((addr_imm & 0x8000) != 0)
				addr_imm |= 0xffff0000;
			DATA_MEM[reg[rs] + addr_imm] = reg[rt];
			sw_cnt++;
			break;
		//lw
		case 0x23:
			//符号拡張
			if ((addr_imm & 0x8000) != 0)
				addr_imm |= 0xffff0000;
			reg[rt] = DATA_MEM[reg[rs] + addr_imm];
			lw_cnt++;
			break;
		//beq
		case 0x04:
			if (reg[rt] == reg[rs]) {
				//符号拡張
				if ((addr_imm & 0x8000) != 0)
					addr_imm |= 0xffff0000;
				dpc = addr_imm;
			}
			beq_cnt++;
			break;
		//bne
		case 0x05:
			if (reg[rt] != reg[rs]) {
				//符号拡張
				if ((addr_imm & 0x8000) != 0)
					addr_imm |= 0xffff0000;
				dpc = addr_imm;
			}
			bne_cnt++;
			break;
		//rsb
		case 0x3f:
			if (CORRESPOND_CORE)
				fprintf(stdout, "%c", (char)(reg[rs] & 0xff));
			else //実装し始めに、自分で見やすい為に作った。
				 //もう使うことはないでしょう。。。
				fprintf(stdout, "%02x\n", reg[rs] & 0xff);
			rsb_cnt++;
			break;
		//rrb
		case 0x3e:
			scanf("%"SCNu8"", &rrb_buf);
			reg[rt] >>= 8; reg[rt] <<= 8; reg[rt] |= rrb_buf;
			rrb_cnt++;
			break;

		//J形式
		//j
		case 0x02:
			pc = addr;
			j_cnt++;
			break;
		//jal
		case 0x03:
			reg[31] = pc;
			pc = addr;
			jal_cnt++;
			break;
		default:
			assert(false);
	}
}

//レジスタ状況を表示
void display_register(void)
{
	fprintf(stderr, "\nREGISTER\n");
	fprintf(stderr, "$zero(reg[0]): %"PRIu32"\n", reg[0]);
	fprintf(stderr, "$at(reg[1]):   %"PRIu32"\n", reg[1]);
	fprintf(stderr, "$v0(reg[2]):   %"PRIu32"\n", reg[2]);
	fprintf(stderr, "$v1(reg[3]):   %"PRIu32"\n", reg[3]);
	fprintf(stderr, "$a0(reg[4]):   %"PRIu32"\n", reg[4]);
	fprintf(stderr, "$a1(reg[5]):   %"PRIu32"\n", reg[5]);
	fprintf(stderr, "$a2(reg[6]):   %"PRIu32"\n", reg[6]);
	fprintf(stderr, "$a3(reg[7]):   %"PRIu32"\n", reg[7]);
	fprintf(stderr, "$t0(reg[8]):   %"PRIu32"\n", reg[8]);
	fprintf(stderr, "$t1(reg[9]):   %"PRIu32"\n", reg[9]);
	fprintf(stderr, "$t2(reg[10]):  %"PRIu32"\n", reg[10]);
	fprintf(stderr, "$t3(reg[11]):  %"PRIu32"\n", reg[11]);
	fprintf(stderr, "$t4(reg[12]):  %"PRIu32"\n", reg[12]);
	fprintf(stderr, "$t5(reg[13]):  %"PRIu32"\n", reg[13]);
	fprintf(stderr, "$t6(reg[14]):  %"PRIu32"\n", reg[14]);
	fprintf(stderr, "$t7(reg[15]):  %"PRIu32"\n", reg[15]);
	fprintf(stderr, "$s0(reg[16]):  %"PRIu32"\n", reg[16]);
	fprintf(stderr, "$s1(reg[17]):  %"PRIu32"\n", reg[17]);
	fprintf(stderr, "$s2(reg[18]):  %"PRIu32"\n", reg[18]);
	fprintf(stderr, "$s3(reg[19]):  %"PRIu32"\n", reg[19]);
	fprintf(stderr, "$s4(reg[20]):  %"PRIu32"\n", reg[20]);
	fprintf(stderr, "$s5(reg[21]):  %"PRIu32"\n", reg[21]);
	fprintf(stderr, "$s6(reg[22]):  %"PRIu32"\n", reg[22]);
	fprintf(stderr, "$s7(reg[23]):  %"PRIu32"\n", reg[23]);
	fprintf(stderr, "$t8(reg[24]):  %"PRIu32"\n", reg[24]);
	fprintf(stderr, "$t9(reg[25]):  %"PRIu32"\n", reg[25]);
	fprintf(stderr, "$k0(reg[26]):  %"PRIu32"\n", reg[26]);
	fprintf(stderr, "$k1(reg[27]):  %"PRIu32"\n", reg[27]);
	fprintf(stderr, "$gp(reg[28]):  %"PRIu32"\n", reg[28]);
	fprintf(stderr, "$sp(reg[29]):  %"PRIu32"\n", reg[29]);
	fprintf(stderr, "$fp(reg[30]):  %"PRIu32"\n", reg[30]);
	fprintf(stderr, "$ra(reg[31]):  %"PRIu32"\n", reg[31]);
}

//統計情報を表示
void display_statistics(void)
{
	//各命令実行回数を表示
	fprintf(stderr, "\nEACH INSTRUCTION EXECUTION TIMES\n");
	fprintf(stderr, "nop:   %"PRIu32"\n", nop_cnt);
	fprintf(stderr, "add:   %"PRIu32"\n", add_cnt);
	fprintf(stderr, "addi:  %"PRIu32"\n", addi_cnt);
	fprintf(stderr, "sub:   %"PRIu32"\n", sub_cnt);
	fprintf(stderr, "ori:   %"PRIu32"\n", ori_cnt);
	fprintf(stderr, "sw:    %"PRIu32"\n", sw_cnt);
	fprintf(stderr, "lw:    %"PRIu32"\n", lw_cnt);
	fprintf(stderr, "slt:   %"PRIu32"\n", slt_cnt);
	fprintf(stderr, "beq:   %"PRIu32"\n", beq_cnt);
	fprintf(stderr, "bne:   %"PRIu32"\n", bne_cnt);
	fprintf(stderr, "fslt:  %"PRIu32"\n", fslt_cnt);
	fprintf(stderr, "fneg:  %"PRIu32"\n", fneg_cnt);
	fprintf(stderr, "sll:   %"PRIu32"\n", sll_cnt);
	fprintf(stderr, "srl:   %"PRIu32"\n", srl_cnt);
	fprintf(stderr, "j:     %"PRIu32"\n", j_cnt);
	fprintf(stderr, "jr:    %"PRIu32"\n", jr_cnt);
	fprintf(stderr, "jal:   %"PRIu32"\n", jal_cnt);
	fprintf(stderr, "rsb:   %"PRIu32"\n", rsb_cnt);
	fprintf(stderr, "rrb:   %"PRIu32"\n", rrb_cnt);
	fprintf(stderr, "fadd:  %"PRIu32"\n", fadd_cnt);
	fprintf(stderr, "fmul:  %"PRIu32"\n", fmul_cnt);
	fprintf(stderr, "finv:  %"PRIu32"\n", finv_cnt);
	fprintf(stderr, "f2i:   %"PRIu32"\n", f2i_cnt);
	fprintf(stderr, "fsqrt: %"PRIu32"\n", fsqrt_cnt);
}

int main(int argc, char *argv[])
{
	FILE *inst_file;
	uint32_t instruction_line; //アセンブリ命令行数
	unsigned long instruction_size; //実行する命令数
	unsigned long i;

	//初期設定(大域変数は自動で0に初期化されるのでpc(プログラムカウンタ)、reg[0]($zero)、DATA_MEMの初期化はしない)
	//命令を格納
	inst_file = fopen(argv[1], "r");
	instruction_line = store_instruction(inst_file);
	fclose(inst_file);
	//実行する命令数の設定
	instruction_size = (unsigned long)(atol(argv[2]));

	//命令実行
	for (i = 0; i < instruction_size; i++) {
		execute(INST_MEM[pc]);
	}

	//レジスタ状況の表示
	display_register();

	//統計情報の表示
	display_statistics();

	return 0;
}


