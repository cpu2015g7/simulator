//注意点
//一段遅延分岐->beq,bne命令の直後の命令は如何なる場合も実行される
//尚、jump系の命令(j, jr, jal)はすぐさま飛ぶ
//breakpointの止める命令は0xf4000000とした

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <signal.h>
#include "sim.h"
#include "fpu.h"
#include "display.h"

//rsbの出力をcoreと同じ出力にするかどうか
bool NON_CORE = false;
//浮動小数点数命令でFPUのC実装を使うかどうか
bool USE_FPU = false;
//実行命令回数表示時にソートするかどうか
bool SORT = false;

//命令用メモリ
uint32_t INST_MEM[INST_ADDR];
//データ用メモリ
uint32_t DATA_MEM[DATA_ADDR];

//プログラムカウンタ
uint32_t pc;
//pcに加算する値
//pcは基本的に+1で良いが、一段遅延分岐が関わる命令(beq,bne)では要注意
uint32_t dpc = 1;

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

//各命令実行回数
uint64_t each_inst_cnt[TYPE];

//全命令
uint64_t total_inst_cnt;

//pc毎の命令実行回数
uint64_t pc_inst_cnt[INST_ADDR];

//breakpointのstep実行
bool step;

// halt when receiving SIGINT
void handler(int signum){
	if(signum == SIGINT){
		each_inst_cnt[hlt_cnt]++;
	}
}

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

//breakpoint
void breakpoint(void)
{
	char cmd[15];

	while (1) {
		fscanf(stdin, "%14s", cmd);
		if (cmd[strlen(cmd)-1] != '\n') {
			while(getchar() != '\n');
		}
		if (!strcmp(cmd, "continue"))
			break;
		else if (!strcmp(cmd, "reg"))
			display_register();
		else if (!strcmp(cmd, "instruction"))
			display_instruction();
		else if (!strcmp(cmd, "stack"))
			display_stack();
		else if (!strcmp(cmd, "step")) {
			step = true;
			break;
		}
	}
}

//R形式の命令を実行
void execute_R(uint32_t instruction)
{
	uint32_t rs = bits(instruction, 6, 10);
	uint32_t rt = bits(instruction, 11, 15);
	uint32_t rd = bits(instruction, 16, 20);
	uint32_t shamt = bits(instruction, 21, 25);
	uint32_t funct = bits(instruction, 26, 31);

	union {
		uint32_t reg;
		float freg;
	} u;
	float frs, frt;

	switch (funct) {
		//add
		case 0x20:
			reg[rd] = reg[rs] + reg[rt];
			each_inst_cnt[add_cnt]++;
			break;
		//sub
		case 0x22:
			reg[rd] = reg[rs] - reg[rt];
			each_inst_cnt[sub_cnt]++;
			break;
		//slt
		case 0x2a:
			reg[rd] = ((int)reg[rs] < (int)reg[rt]) ? 1 : 0;
			each_inst_cnt[slt_cnt]++;
			break;
		//fslt
		case 0x2b:
			u.reg = reg[rs];
			frs = u.freg;
			u.reg = reg[rt];
			frt = u.freg;
			reg[rd] = (frs < frt) ? 1 : 0;
			each_inst_cnt[fslt_cnt]++;
			break;
		//fneg
		case 0x2c:
			reg[rd] = reg[rs] ^ 0x80000000;
			each_inst_cnt[fneg_cnt]++;
			break;
		//f2i
		case 0x2d:
			if (USE_FPU)
				reg[rd] = f2i(reg[rs]);
			else
				reg[rd] = f2i_soft(reg[rs]);
			each_inst_cnt[f2i_cnt]++;
			break;
		//i2f
		case 0x2e:
			if (USE_FPU)
				reg[rd] = i2f(reg[rs]);
			else
				reg[rd] = i2f_soft(reg[rs]);
			each_inst_cnt[i2f_cnt]++;
			break;
		//flr
		case 0x2f:
			u.reg = reg[rs];
			frs = u.freg;
			u.freg = floorf(frs);
			reg[rd] = u.reg;
			each_inst_cnt[flr_cnt]++;
			break;
		//sll
		case 0x00:
			reg[rd] = reg[rt] << shamt;
			each_inst_cnt[sll_cnt]++;
			break;
		//srl
		case 0x02:
			reg[rd] = reg[rt] >> shamt;
			each_inst_cnt[srl_cnt]++;
			break;
		//jr
		case 0x08:
			pc = reg[rs];
			each_inst_cnt[jr_cnt]++;
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
			each_inst_cnt[fadd_cnt]++;
			break;
		//fmul
		case 0x01:
			if (USE_FPU)
				reg[rd] = fmul(reg[rs], reg[rt]);
			else {
				u.freg = frs * frt;
				reg[rd] = u.reg;
			}
			each_inst_cnt[fmul_cnt]++;
			break;
		//finv
		case 0x03:
			if (USE_FPU)
				reg[rd] = finv(reg[rs]);
			else {
				u.freg = 1.0 / frs;
				reg[rd] = u.reg;
			}
			each_inst_cnt[finv_cnt]++;
			break;
		//fsqrt
		case 0x18:
			u.freg = sqrtf(frs);
			reg[rd] = u.reg;
			each_inst_cnt[fsqrt_cnt]++;
			break;
		//f2i
		case 0x2d:
			if (USE_FPU)
				reg[rd] = f2i(reg[rs]);
			else
				reg[rd] = f2i_soft(reg[rs]);
			each_inst_cnt[f2i_cnt]++;
			break;
		//i2f
		case 0x2e:
			if (USE_FPU)
				reg[rd] = i2f(reg[rs]);
			else
				reg[rd] = i2f_soft(reg[rs]);
			each_inst_cnt[i2f_cnt]++;
			break;
		//flr
		case 0x2f:
			u.reg = reg[rs];
			frs = u.freg;
			u.freg = floorf(frs);
			reg[rd] = u.reg;
			each_inst_cnt[flr_cnt]++;
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
	char rrb_buf;


	pc += dpc;
	dpc = 1;

	//nop
	if (instruction == 0) {
		each_inst_cnt[nop_cnt]++;
		return;
	}
	//hlt
	if (instruction == 0xf0000000) {
		each_inst_cnt[hlt_cnt]++;
		return;
	}
	//breakpoint
	if (instruction == 0xf4000000) {
		breakpoint();
		return;
	}
	//nop, hlt, breakpoint以外
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
			each_inst_cnt[addi_cnt]++;
			break;
		//ori
		case 0x0d:
			reg[rt] = reg[rs] | addr_imm;
			each_inst_cnt[ori_cnt]++;
			break;
		//sw
		case 0x2b:
			//符号拡張
			if ((addr_imm & 0x8000) != 0)
				addr_imm |= 0xffff0000;
			DATA_MEM[reg[rs] + addr_imm] = reg[rt];
			each_inst_cnt[sw_cnt]++;
			break;
		//lw
		case 0x23:
			//符号拡張
			if ((addr_imm & 0x8000) != 0)
				addr_imm |= 0xffff0000;
			reg[rt] = DATA_MEM[reg[rs] + addr_imm];
			each_inst_cnt[lw_cnt]++;
			break;
		//beq
		case 0x04:
			if (reg[rt] == reg[rs]) {
				//符号拡張
				if ((addr_imm & 0x8000) != 0)
					addr_imm |= 0xffff0000;
				dpc = addr_imm;
			}
			each_inst_cnt[beq_cnt]++;
			break;
		//bne
		case 0x05:
			if (reg[rt] != reg[rs]) {
				//符号拡張
				if ((addr_imm & 0x8000) != 0)
					addr_imm |= 0xffff0000;
				dpc = addr_imm;
			}
			each_inst_cnt[bne_cnt]++;
			break;
		//rsb
		case 0x3f:
			if (!NON_CORE)
				fprintf(stdout, "%c", (char)(reg[rs] & 0xff));
			else
				fprintf(stdout, "%02x\n", reg[rs] & 0xff);
			each_inst_cnt[rsb_cnt]++;
			break;
		//rrb
		case 0x3e:
			scanf("%c", &rrb_buf);
			reg[rt] >>= 8; reg[rt] <<= 8; reg[rt] |= rrb_buf;
			each_inst_cnt[rrb_cnt]++;
			break;

		//J形式
		//j
		case 0x02:
			pc = addr;
			each_inst_cnt[j_cnt]++;
			break;
		//jal
		case 0x03:
			reg[31] = pc;
			pc = addr;
			each_inst_cnt[jal_cnt]++;
			break;
		default:
			assert(false);
	}

	if (step) {
		step = false;
		breakpoint();
	}

}



int main(int argc, char *argv[])
{
	FILE *inst_file;
	uint64_t instruction_line;
	unsigned long max_inst; //最大命令実行数
	unsigned long i;

	struct sigaction act = {
		.sa_handler = handler,
		.sa_flags = 0,
	};
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);

	//初期設定(大域変数は自動で0に初期化されるのでpc(プログラムカウンタ)、reg[0]($zero)、DATA_MEMの初期化はしない)
	//命令を格納
	inst_file = fopen(argv[1], "r");
	instruction_line = store_instruction(inst_file);
	fclose(inst_file);

	//最大命令実行数
	max_inst = atol(argv[2]);

	//オプションの設定
	for (int i = 3; i < argc; i++) {
		if (!strcmp(argv[i], "-ncore"))
			NON_CORE = true;
		else if (!strcmp(argv[i], "-fpu"))
			USE_FPU = true;
		else if (!strcmp(argv[i], "-sort"))
			SORT = true;
	}

	//命令実行
	for (i = 0; i < max_inst; i++) {
		pc_inst_cnt[pc]++;
		total_inst_cnt++;
		execute(INST_MEM[pc]);
		if (each_inst_cnt[hlt_cnt]) break;
	}

	//レジスタ状況の表示
	fprintf(stderr, "REGISTER\n");
	display_register();
	//命令実行回数の表示
	fprintf(stderr, "INSTRUCTION\n");
	display_instruction();
	//ヒストグラム
	display_inst_address_histgram();

	return 0;
}


