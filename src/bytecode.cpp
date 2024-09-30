#include "bytecode.h"
#include "node.h"

void AddImm(long long val, char sz, machine_code& ret);
void EmplaceLeaInst(char reg_dst, char reg_base, int offset, char reg_sz, descend_func_ret *out);
void EmplaceComment(descend_func_ret* ret, char* comment);

char *BufferToHeapCStr(char *start, int sz)
{
	auto data = (char *)malloc(sz + 1);
	memcpy(data, start, sz);
	data[sz] = 0;
	return data;
}
void EmplaceValueToCharVector(own_std::vector<unsigned char> &code, long long val, char size)
{
	switch(size)
	{
	case 1:
	{
		code.emplace_back((char)val & 0xff);
	}break;
	case 2:
	{
		unsigned short u16 = (unsigned short)val;
		code.emplace_back(u16 & 0xff);
		code.emplace_back((val >> 8) & 0xff);
	}break;
	case 8:
	case 4:
	{
		unsigned int u32 = val;
		code.emplace_back(u32 & 0xff);\
		code.emplace_back((u32 >> 8) & 0xff);\
		code.emplace_back((u32 >> 16) & 0xff);\
		code.emplace_back((u32 >> 24) & 0xff);\
	}break;
	default:
	ASSERT(false)
	}
}
#define CREATE_IMM_2_R0(op)\
			switch(reg_sz)\
			{\
			case 2:\
			{\
				unsigned short u16 = bc->bin.rhs.u16; \
				ret.code.emplace_back(0x66);\
				ret.code.emplace_back(op);\
				ret.code.emplace_back(u16 & 0xff);\
				ret.code.emplace_back((u16 >> 8) & 0xff);\
			}break;\
			case 4:\
			{\
				unsigned int u32 = bc->bin.rhs.u32; \
				ret.code.emplace_back(op);\
				ret.code.emplace_back(u32 & 0xff);\
				ret.code.emplace_back((u32 >> 8) & 0xff);\
				ret.code.emplace_back((u32 >> 16) & 0xff);\
				ret.code.emplace_back((u32 >> 24) & 0xff);\
			}break;\
			case 8:\
			{\
				unsigned short u32 = bc->bin.rhs.u32; \
				ret.code.emplace_back(0x48);\
				ret.code.emplace_back(op);\
				ret.code.emplace_back(u32 & 0xff);\
				ret.code.emplace_back((u32 >> 8) & 0xff);\
				ret.code.emplace_back((u32 >> 16) & 0xff);\
				ret.code.emplace_back((u32 >> 24) & 0xff);\
			}break;\
			default:\
				ASSERT(false)\
			break;\
			}

#define CREATE_IMM_BYTE_2_REG(op_b, op)
			

#define CREATE_IMM_2_REG(op_b, op)\
			switch(reg_sz)\
			{\
			case 1:\
			{\
				unsigned short u8 = bc->bin.rhs.u8; \
				ret.code.emplace_back(op_b);\
				ret.code.emplace_back(0xc0 + (char) bc->bin.lhs.reg);\
				ret.code.emplace_back(u8);\
			}break;\
			case 2:\
			{\
				unsigned short u16 = bc->bin.rhs.u16; \
				ret.code.emplace_back(0x66);\
				ret.code.emplace_back(op);\
				ret.code.emplace_back(0xc0 + (char) bc->bin.lhs.reg);\
				ret.code.emplace_back(u16 & 0xff);\
				ret.code.emplace_back((u16 >> 8) & 0xff);\
			}break;\
			case 4:\
			{\
				unsigned short u32 = bc->bin.rhs.u32; \
				ret.code.emplace_back(op);\
				ret.code.emplace_back(0xc0 + (char) bc->bin.lhs.reg);\
				ret.code.emplace_back(u32 & 0xff);\
				ret.code.emplace_back((u32 >> 8) & 0xff);\
				ret.code.emplace_back((u32 >> 16) & 0xff);\
				ret.code.emplace_back((u32 >> 24) & 0xff);\
			}break;\
			case 8:\
			{\
				unsigned short u32 = bc->bin.rhs.u32; \
				ret.code.emplace_back(0x48);\
				ret.code.emplace_back(op);\
				ret.code.emplace_back(0xc0 + (char) bc->bin.lhs.reg);\
				ret.code.emplace_back(u32 & 0xff);\
				ret.code.emplace_back((u32 >> 8) & 0xff);\
				ret.code.emplace_back((u32 >> 16) & 0xff);\
				ret.code.emplace_back((u32 >> 24) & 0xff);\
			}break;\
			default:\
				ASSERT(false)\
			break;\
			}
char FromBCRegToAsmReg(char bc_reg);
void MaybePoppingPramRegsInDivAndMulInsts(lang_state *, func_byte_code* final_func, tkn_type2 tp, descend_func_ret* ret);
void MaybePushingPramRegsInDivAndMulInsts(lang_state *, func_byte_code* final_func, tkn_type2 tp, descend_func_ret* ret);

void CreateRegToReg(byte_code *bc, char byte_op, char word_op, machine_code *ret)
{
	char reg_dst = FromBCRegToAsmReg(bc->bin.lhs.reg);
	char reg_src = FromBCRegToAsmReg(bc->bin.rhs.reg);

	bc->bin.lhs.reg = reg_dst;
	bc->bin.rhs.reg = reg_src;

	bool dst_rex = IS_FLAG_ON(reg_dst, 0x80);
	bool src_rex = IS_FLAG_ON(reg_src, 0x80);
	AddPreMemInsts(bc->bin.lhs.reg_sz, byte_op, word_op, ((char)src_rex) | ((char)dst_rex <<1), ret->code, true);

	reg_dst &= 0xf;
	reg_src &= 0xf;
	char modrm = (3<<6) | (reg_src << 3) | (reg_dst);
	ret->code.emplace_back(modrm);


}
void CreateImmToReg(char r0_byte_imm_byte, char r0_imm, char inst_reg, byte_code *bc, machine_code &ret)
{
	bc->bin.lhs.reg = FromBCRegToAsmReg(bc->bin.lhs.reg);
	int reg_sz = bc->bin.lhs.reg_sz;
	if(bc->bin.lhs.reg == 0 && reg_sz == 1)
	{
		ret.code.emplace_back(r0_byte_imm_byte);
		ret.code.emplace_back(bc->bin.rhs.u8);
	}
	else
	{
		char inst = (3 << 6) | (inst_reg << 3) | bc->bin.lhs.reg;

		if(bc->bin.rhs.u64 >= 0x80)
		{
			// mov (rax eax ..), imm16/32
			if(bc->bin.lhs.reg == 0)
			{
				AddPreMemInsts(bc->bin.lhs.reg_sz, r0_byte_imm_byte, r0_imm, false, ret.code, bc->bin.lhs.reg == 4);
				AddImm(bc->bin.rhs.u64, bc->bin.lhs.reg_sz, ret);

			}
			else
			{
				AddPreMemInsts(bc->bin.lhs.reg_sz, 0x80, 0x81, false, ret.code, bc->bin.lhs.reg == 4);
				ret.code.emplace_back(inst);
				AddImm(bc->bin.rhs.u64, bc->bin.lhs.reg_sz, ret);
			}
		}
		else
		{
			AddPreMemInsts(bc->bin.lhs.reg_sz, 0x80, 0x83, false, ret.code, bc->bin.lhs.reg == 4);
			ret.code.emplace_back(inst);
			ret.code.emplace_back(bc->bin.rhs.u8);
		}

	}
}
char FromAsmRegToWasmReg(unsigned char bc_reg)
{
	switch(bc_reg)
	{
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
		return 3;
	case 4:
		return 4;
	case 5:
		return 5;
	case 6:
		return 6;
	case 7:
		return 7;
	case 0x80:
		return 8;
	case 0x81:
			return 9;
	case 10:
		return 10;
	case 11:
		return 11;
	case 12:
		return 12;
	case 13:
		return 13;
	default:

		ASSERT(false)
	}
	return -1;
}
char FromBCRegToAsmReg(char bc_reg)
{
	switch(bc_reg)
	{
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
		return 3;
	case 4:
		return 6;
	case 5:
		return 4;
	case 6:
		return 1;
	case 7:
		return 2;
	case 8:
		return 0 | (1 << 7);
	case 9:
		return 1 | (1 << 7);
	case 32:
		return 6;
	case 33:
		return 7;
	case 34:
		return 5;
	default:

		ASSERT(false)
	}
	return -1;
}
char MakeModRM(bool has_disp, int disp, char rm, char reg)
{
	char ret;
	if(has_disp)
	{
		if(disp == 0)
		{
			ret = ((reg)  << 3) | (rm);
		}
		else if((unsigned int)disp < DISP_BYTE_MAX)
		{
			ret = (1 << 6) | ((reg)  << 3) | (rm );
		}
		else
		{
			ret = (2 << 6) | ((reg )  << 3) | (rm);
		}
	}
	else
	{
		ret = (3 << 6) | ((reg & 0xf)  << 3) | (rm & 0xf);
	}
	return ret;
}
void AddPreMemInsts(char size, char byte, char greater_byte, char is_rex, own_std::vector<unsigned char> &code, bool add_rsp_op)
{
#define REX_BYTE \
		if(add_rsp_op && is_rex == 0)\
			code.emplace_back(0x40);\
		else if(is_rex == 1)\
			code.emplace_back(0x44);\
		else if(is_rex == 2)\
			code.emplace_back(0x41);\
		else if(is_rex == 3)\
			code.emplace_back(0x45);

	// is_rex contains the regs that are rex
	// 1 means the rhs is rex
	// 2 means the lhs is rex
	// 3 means the both are
	switch(size)
	{
	case 1:
		REX_BYTE

		code.emplace_back(byte);
	break;
	case 2:
		code.emplace_back(0x66);
		REX_BYTE
		code.emplace_back(greater_byte);
	break;
	case 4:
		REX_BYTE
		code.emplace_back(greater_byte);
	break;
	case 8:
		if(is_rex == 0)
			code.emplace_back(0x48);
		else if(is_rex == 1)
			code.emplace_back(0x4c);
		else if(is_rex == 2)
			code.emplace_back(0x49);
		else if(is_rex == 3)
			code.emplace_back(0x4d);
		
		code.emplace_back(greater_byte);
	break;
	default:
		ASSERT(false)
	break;
	}
}
void AddModRM(bool has_disp, int disp, char rm, char reg, machine_code& ret, short sib = 0)
{
	char mod_rm = MakeModRM(has_disp, disp, rm, reg);

	ret.code.emplace_back(mod_rm);

	if (rm == 4)
	{
		if(sib != 0)
		{
			unsigned char mul_idx = (sib >> 4) & 0xf;
			unsigned char reg = sib & 0xf;
			unsigned char sib = (mul_idx << 6) | (4 << 3) | reg;
			ret.code.emplace_back(sib);
		}
		else
			ret.code.emplace_back(0x24);
	}
}
void AddImm(long long val, char sz, machine_code& ret)
{
	// max imm to add is four bytes
	sz = sz <= 4 ? sz : 4;
	EmplaceValueToCharVector(ret.code, val, sz);
}
void CreateMemToReg(byte_code *bc, char byte, char greater_byte, bool is_rex_param, machine_code &ret, char base_reg = 5)
{
	base_reg = FromBCRegToAsmReg(bc->bin.rhs.reg);
	char reg = FromBCRegToAsmReg(bc->bin.lhs.reg);
	bool is_rex = IS_FLAG_ON(reg, 0x80);

	bc->bin.rhs.reg = base_reg;
	bc->bin.lhs.reg = reg;

	// even though if it's rex on the lhs, when we're dealing with rm, the rex will
	// as if it we at the rhs, that's why we're not shifting it one bit to the right (is_rex << 1)
	AddPreMemInsts(bc->bin.lhs.reg_sz, byte, greater_byte, ((char)is_rex), ret.code);
	//ret.code.emplace_back(byte);

	AddModRM(true, bc->bin.rhs.voffset, base_reg, reg, ret);
	if(bc->bin.rhs.voffset > 0)
		AddImm(bc->bin.rhs.voffset, bc->bin.rhs.voffset < 0x80 ? 1 : 4, ret);
}
void CreateRegToMem(byte_code* bc, char byte, char greater_byte, machine_code& ret, char base_reg = 5)
{
	base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);


	char reg = FromBCRegToAsmReg(bc->bin.rhs.reg);
	
	bc->bin.lhs.reg = base_reg;
	bc->bin.rhs.reg = reg;

	bool is_rex = IS_FLAG_ON(reg, 0x80);
	AddPreMemInsts(bc->bin.lhs.reg_sz, byte, greater_byte, is_rex, ret.code);
	//ret.code.emplace_back(byte);
	AddModRM(true, bc->bin.lhs.voffset, base_reg, reg & 0xf, ret, bc->bin.lhs.sib);


	if(bc->bin.lhs.voffset > 0)
		AddImm(bc->bin.lhs.voffset, bc->bin.lhs.voffset < DISP_BYTE_MAX ? 1 : 4, ret);
}
void CreateStoreImmToMem(byte_code *bc, machine_code &ret)
{
	char base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);

	AddPreMemInsts(bc->bin.lhs.reg_sz, 0xc6, 0xc7, false, ret.code);
	AddModRM(true, bc->bin.lhs.voffset, base_reg, 0, ret);
	// mem offset first
	if(bc->bin.lhs.voffset != 0)
		AddImm(bc->bin.lhs.voffset, (unsigned int)(bc->bin.lhs.voffset) < DISP_BYTE_MAX ? 1 : 4, ret);
	AddImm(bc->bin.rhs.s32, bc->bin.lhs.reg_sz, ret);
}
void CreateImmToMem(byte_code *bc, char byte, machine_code &ret, char base_reg = 5)
{
	base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);

	if (bc->bin.rhs.u64 <= 0xff)
		AddPreMemInsts(bc->bin.lhs.reg_sz, 0x80, 0x83, false, ret.code);
	else
		AddPreMemInsts(bc->bin.lhs.reg_sz, 0x80, 0x81, false, ret.code);
	
	//ret.code.emplace_back(byte);

	AddModRM(true, bc->bin.lhs.voffset, base_reg, byte, ret);
	//char mod_rm = MakeModRM(true, bc->bin.lhs.voffset, base_reg, byte);
	//ret.code.emplace_back(mod_rm);
	
	if(bc->bin.lhs.voffset != 0)
		// adding displacement
		AddImm(bc->bin.lhs.voffset, ((unsigned int)bc->bin.lhs.voffset) < DISP_BYTE_MAX ? 1 : 4, ret);

	// adding imm
	AddImm(bc->bin.rhs.u64, bc->bin.rhs.u64 <= 0xff ? 1 : 4, ret);

}
void AddJump(byte_code *bc, char jmp, machine_code& ret)
{
	unsigned int offset = bc->val * 8;
	if (offset <= 0x80)
	{
		ret.jmp_rels.emplace_back(jmp_rel(1, ret.code.size() + 1, bc));

		ret.code.emplace_back(jmp);
		ret.code.emplace_back((char)offset);
	}
	else
	{
		ret.jmp_rels.emplace_back(jmp_rel(4, ret.code.size() + 2, bc));

		ret.code.emplace_back(0x0f);
		ret.code.emplace_back(jmp + 0x10);
		AddImm(offset, 4, ret);
	}
}
void StoreMemToMem(byte_code *bc, int dst_offset, machine_code &ret)
{
	auto prev_bin = bc->bin;
	bc->bin.lhs.reg = 0;
	bc->bin.lhs.reg_sz = 8;
	CreateMemToReg(&*bc, 0x8a, 0x8b, false, ret);

	bc->bin.lhs.reg = 5;
	bc->bin.lhs.voffset = dst_offset;
	//bc->bin.lhs.var_size = 4;
	bc->bin.rhs.reg = 0;
	bc->bin.rhs.reg_sz = 8;
	CreateRegToMem(&*bc, 0x88, 0x89, ret);

	bc->bin = prev_bin;
}
void ResolveJmpInsts(machine_code *ret)
{
	FOR_VEC(j, ret->jmp_rels)
	{
		unsigned int dst_offset = j->dst_bc->machine_code_idx;
		if(j->jmp_inst_size == 1)
		{
			short diff = dst_offset - (j->src_inst + 1);
			char *to_fill = (char *)&ret->code[j->src_inst];
			*to_fill = diff;
		}
		else if(j->jmp_inst_size == 4)
		{
			int diff = dst_offset - (j->src_inst + 4);
			int *to_fill = (int *)&ret->code[j->src_inst];
			*to_fill = diff;
		}
		else
		{
			ASSERT(false)
		}
	}
}
void CreateSSERegToSSEReg(byte_code *bc, char op, machine_code *ret)
{
	unsigned char src = bc->bin.lhs.reg;
	unsigned char dst = bc->bin.rhs.reg;
	ret->code.emplace_back(0xf3);
	ret->code.emplace_back(0x0f);
	ret->code.emplace_back(op);
	char modrm = (3 << 6) | (dst & 0xf) | (src << 3);
	ret->code.emplace_back(modrm);
}
void CreateMemToSSE(byte_code* bc, char op, machine_code* ret, char reg_base = 5)
{
	char dst = bc->bin.lhs.reg;
	reg_base = FromBCRegToAsmReg(bc->bin.rhs.reg);

	ret->code.emplace_back(0xf3);
	ret->code.emplace_back(0x0f);
	ret->code.emplace_back(op);
	AddModRM(true, bc->bin.rhs.voffset, reg_base, dst & 0xf, *ret);
	if(bc->bin.rhs.voffset != 0)
		AddImm(bc->bin.rhs.voffset, (unsigned int) bc->bin.rhs.voffset < DISP_BYTE_MAX ? 1 : 4, *ret);
}
void CreateSSERegToMem(byte_code *bc, char op, machine_code *ret, char reg_base = 4, bool mov_mem_to_reg = true)
{
	// if the instruction isn't a mov to mem, we need to first transfer the mem to a reg, perform the op and then mov it to mem
	if(op != 0x11)
	{
		// mem
		auto last_lhs = bc->bin.lhs;
		// src_reg
		auto last_rhs = bc->bin.rhs;

		// movss xmm5, mem
		bc->bin.rhs = last_lhs;
		bc->bin.lhs.reg = last_lhs.reg;

		// if the lhs is a rm, we dont need to mov into a register 
		if (mov_mem_to_reg)
			CreateMemToSSE(bc, 0x10, ret);
		else
			CreateMemToSSE(bc, 0x10, ret, bc->bin.rhs.reg);
		

		// sseop xmm5, src_reg
		bc->bin.lhs.reg = last_lhs.reg;
		bc->bin.rhs = last_rhs;
		CreateSSERegToSSEReg(bc, op, ret);

		
		// finally we can move the src_reg to mem
		// movss mem, src_reg
		bc->bin.lhs = last_lhs;
		bc->bin.rhs.reg = last_lhs.reg;
		CreateSSERegToMem(bc, 0x11, ret, last_lhs.reg);
	}
	else
	{
		char src =  bc->bin.rhs.reg;
		char dst = FromBCRegToAsmReg(bc->bin.lhs.reg);
		ret->code.emplace_back(0xf3);
		ret->code.emplace_back(0x0f);
		ret->code.emplace_back(op);

		AddModRM(true, bc->bin.lhs.voffset, dst, src & 0xf, *ret);
		if(bc->bin.lhs.voffset != 0)
			AddImm(bc->bin.lhs.voffset, ((unsigned int)bc->bin.lhs.voffset) < DISP_BYTE_MAX ? 1 : 4, *ret);
	}
}

void Create0FMemToReg(byte_code *bc, char op, machine_code *ret)
{
	char dst = bc->bin.lhs.reg;
	auto reg_base = FromBCRegToAsmReg(bc->bin.rhs.reg);

	AddPreMemInsts(8, 0x0f, 0x0f, false, ret->code);

	if (bc->bin.rhs.reg_sz == 2)
		ret->code.emplace_back(op + 1);

	else if(bc->bin.rhs.reg_sz == 1)
		ret->code.emplace_back(op);

	else
		ASSERT(false)

	AddModRM(true, bc->bin.rhs.voffset, reg_base, dst & 0xf, *ret);
	if(bc->bin.rhs.voffset != 0)
		AddImm(bc->bin.rhs.voffset, (unsigned int) bc->bin.rhs.voffset < DISP_BYTE_MAX ? 1 : 4, *ret);

}
/*
own_std::vector<char> AsmDebugStr(unsigned char* code, own_std::vector<byte_code>* bcs)
{
	ud_t ud_obj;

	ud_init(&ud_obj);

	ud_set_mode(&ud_obj, 64);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	//
	//printf("\t%s\n", );

	own_std::vector<char> ret;
	int idx = 0;
	FOR_VEC(bc, *bcs)
	{
		char* inst;
		switch (bc->type)
		{
		case COMMENT:
		{
			ret.emplace_back('\n');
			InsertIntoCharVector(&ret, bc->name, strlen(bc->name));

		}break;
		default:
			char addr[16];
			snprintf(addr, 12, "%x", (unsigned int)bc->machine_code_idx);
			InsertIntoCharVector(&ret, (void*)addr, strlen(addr));

			ret.emplace_back('\t');
			ud_set_input_buffer(&ud_obj, (unsigned char*)&code[bc->machine_code_idx], 16);
			ud_disassemble(&ud_obj);
			inst = (char*)ud_insn_asm(&ud_obj);
			InsertIntoCharVector(&ret, (void*)inst, strlen(inst));
			
		}
		idx++;
		ret.emplace_back('\n');
	}
	return ret;
}
*/

void AddLeaInst(machine_code &ret, char reg_dst)
{
	ret.code.emplace_back(0x48);
	ret.code.emplace_back(0x8d);
	ret.code.emplace_back(0x5 | (reg_dst << 3));
	AddImm(0, 4, ret);
}
void FromByteCodeToX64(lang_state *lang_stat, own_std::vector<func_byte_code *> *funcs, machine_code &ret)
{
// we're adding one because we're getting the difference for the jmp, with the offset of the next inst
#define CHANGE_JMP_DST_BC ret.jmp_rels.back().dst_bc = &f->bcodes[i + bc->val + 1];
	ret.code.reserve(128);

	FOR_VEC(func, *funcs)
	{
		
		auto f = *func;

		// we don't have to generate code for outsider functions
		if (f->is_outsider)
		{
			machine_sym msym = {};
			if(f->is_link_name)
				msym.type = machine_sym_type::SYM_DLL_FUNC_NO_IMP_ADDING;
			else
				msym.type = machine_sym_type::SYM_DLL_FUNC;

			//if (f->is_link_name == "__glewGetShaderiv")
			//	auto a = 0;

			msym.idx = ret.code.size();
			msym.name = (char*)f->name.c_str();
			ret.symbols.emplace_back(msym);
			continue;
		}
		// unamed functions begin with a number, so we're checking because
		// we only want add name functions
		if(!(f->name[0] >= '0' && f->name[0] <= '9'))
		{
			// skipping internal functions
			if (IS_FLAG_ON(f->fdecl->flags, FUNC_DECL_INTERNAL))
				continue;

			machine_sym msym;
			msym.name = (char *)f->name.c_str();
			if (IS_FLAG_ON(f->fdecl->flags, FUNC_DECL_EXTERN))
			{
				msym.type = SYM_LIB_FUNC;
				ret.symbols.emplace_back(msym);
				continue;
			}
			else
			{
				msym.type = SYM_FUNC;
				msym.idx  = ret.code.size();
				ret.symbols.emplace_back(msym);
			}
			
		}
		int i = 0;

		f->start_idx = ret.code.size();
		int sum_of_onter_insts = lang_stat->code_sect.size() - lang_stat->already_inserted_of_type_sect_in_code_sect;;
		f->fdecl->code_start_idx = ret.code.size() + sum_of_onter_insts;
		
		FOR_VEC(bc, f->bcodes)
		{

			char* data = (char*)ret.code.data();
			bc->machine_code_idx = ret.code.size();
			switch(bc->type)
			{
			case ASSIGN_FUNC_SIZE:
			{
				bc->fdecl->code_size = (ret.code.size() + sum_of_onter_insts) - bc->fdecl->code_start_idx;
			}break;
			case CVTSD_REG_2_SS:
			{
				char mod = MakeModRM(false, 0, bc->bin.lhs.reg, bc->bin.rhs.reg);
				ret.code.emplace_back(0xf2);
				ret.code.emplace_back(0x0f);
				ret.code.emplace_back(0x5a);
				ret.code.emplace_back(mod);
			}break;
			case INT3:
				ret.code.emplace_back(0xcc);
			break;
			case MOVSX_R:
			{
				char mod = MakeModRM(false, 0, bc->bin.lhs.reg, bc->bin.rhs.reg);
				if(bc->bin.rhs.reg_sz == 4)
				{
					ret.code.emplace_back(0x48);
					ret.code.emplace_back(0x63);
					ret.code.emplace_back(mod);
				}
				else
				{
					char i_sz = bc->bin.rhs.reg_sz == 1 ? 0xbe : 0xbf;

					ret.code.emplace_back(0x48);
					ret.code.emplace_back(0x0f);
					ret.code.emplace_back(i_sz);
					ret.code.emplace_back(mod);

				}
			}break;
			case MOVZX_R:
			{
				char mod = MakeModRM(false, 0, bc->bin.lhs.reg, bc->bin.rhs.reg);
				if(bc->bin.rhs.reg_sz == 4)
				{
					char lhs_r = bc->bin.lhs.reg;
					// mov edi, rhs
					bc->bin.lhs.reg = 7;
					bc->bin.lhs.reg_sz = 4;
					bc->bin.rhs.reg_sz = 4;
					bc->bin.rhs.reg_sz = 4;
					CreateRegToReg(&*bc, 0x88, 0x89, &ret);

					// mov lhs, edi
					bc->bin.lhs.reg = lhs_r;
					bc->bin.lhs.reg_sz = 4;
					bc->bin.rhs.reg_sz = 7;
					bc->bin.rhs.reg_sz = 4;
					CreateRegToReg(&*bc, 0x88, 0x89, &ret);

				}
				else
				{
					char i_sz = bc->bin.rhs.reg_sz == 1 ? 0xb6 : 0xb7;

					ret.code.emplace_back(0x48);
					ret.code.emplace_back(0x0f);
					ret.code.emplace_back(i_sz);
					ret.code.emplace_back(mod);

				}
			}break;
			case MOVSX_M:
			{
				char mod = MakeModRM(false, 0, 4, bc->bin.lhs.reg);
				if (bc->bin.rhs.reg_sz == 4)
				{
					CreateMemToReg(&*bc, 0x63, 0x63, false, ret);
					//ret.code.emplace_back(0x48);
					//ret.code.emplace_back(0x63);
					//ret.code.emplace_back(mod);
				}
				else
					Create0FMemToReg(&*bc, 0xbe, &ret);
				
			}break;
			case MOVZX_M:
			{
				Create0FMemToReg(&*bc, 0xb6, &ret);
			}break;
			case RELOC:
			{
				bool is_rel_func = bc->rel.type == rel_type::REL_FUNC;
				bool is_get_func_addr = bc->rel.type == rel_type::REL_GET_FUNC_ADDR;

				if (is_rel_func || is_get_func_addr)
				{
                    auto fdecl = bc->rel.call_func;
                    
                    if(IS_FLAG_ON(fdecl->flags, FUNC_DECL_INTERNAL))
                    {

                        ASSERT(lang_stat->internal_funcs_addr.find(fdecl->name.c_str()) != lang_stat->internal_funcs_addr.end())
                    }
                    else
                    {
                        auto found_ptr_func = IsThereAFunction(lang_stat, (char *)bc->rel.call_func->name.c_str());

                        // asserting that the function was already compiled
                        ASSERT(found_ptr_func && IS_FLAG_ON(fdecl->flags, FUNC_DECL_CODE_WAS_GENERATED))

                    }

					int offset_addr = is_rel_func ? 1 : 3;
					auto fdecl_flags = bc->rel.call_func->flags;

					// check if function is defined by the compiler, by means of testing if the fdecl has a body
					if(!IS_FLAG_ON(fdecl_flags, FUNC_DECL_EXTERN) 
						&& !IS_FLAG_ON(fdecl_flags, FUNC_DECL_IS_OUTSIDER)
						&& !IS_FLAG_ON(fdecl_flags, FUNC_DECL_LINK_NAME))
					{
						ret.call_rels.emplace_back(call_rel(ret.code.size() + offset_addr, bc->rel.call_func));
					}
					else
						ret.rels.emplace_back(machine_reloc(INSIDE_FUNC, ret.code.size() + offset_addr, bc->rel.name));

					if(is_rel_func)
					{
						ret.code.emplace_back(0xe8);
						AddImm(0, 4, ret);
					}
					else
						AddLeaInst(ret, bc->rel.reg_dst);

				}
				else if (bc->rel.type == rel_type::REL_TYPE)
				{
					own_std::string name = own_std::string("$$") + bc->rel.name;

					// puttinf the name into the heap
					char *new_str = std_str_to_heap(lang_stat, &name);

					ret.rels.emplace_back(machine_reloc(machine_rel_type::TYPE_DATA, ret.code.size() + 3, new_str));

					ASSERT((unsigned char)bc->rel.reg_dst < 7)

					// emplacing a lea instruction here
					AddLeaInst(ret, bc->rel.reg_dst);
					/*
					ret.code.emplace_back(0x48);
					ret.code.emplace_back(0x8d);
					ret.code.emplace_back(0x5 | (bc->rel.reg_dst << 3));
					AddImm(0, 4, ret);
					*/

				}
				else if (bc->rel.type == rel_type::REL_DATA)
				{
					char* data_sym_name = (char*)AllocMiscData(lang_stat, 16);
					snprintf(data_sym_name, 16, "$%d", ret.generated_data_symbols++);
					
					ret.symbols.emplace_back(machine_sym(lang_stat, machine_sym_type::SYM_DATA, bc->rel.offset, data_sym_name));
					// float 
					if(IS_FLAG_ON(bc->rel.reg_dst, 0x40))
					{
						ret.rels.emplace_back(machine_reloc(machine_rel_type::DATA, ret.code.size() + 4, data_sym_name));

						// movssinstruction here
						ret.code.emplace_back(0xf3);
						ret.code.emplace_back(0x0f);
						ret.code.emplace_back(0x10);
						ret.code.emplace_back(0x5 | ((bc->rel.reg_dst &0xf) << 3));
						AddImm(0, 4, ret);
					}
					else
					{
						ret.rels.emplace_back(machine_reloc(machine_rel_type::DATA, ret.code.size() + 3, data_sym_name));

						// emplacing a lea instruction here
						AddLeaInst(ret, bc->rel.reg_dst);
						/*
						ret.code.emplace_back(0x48);
						ret.code.emplace_back(0x8d);
						ret.code.emplace_back(0x5 | (bc->rel.reg_dst << 3));
						AddImm(0, 4, ret);
						*/
					}
				}
			}break;
			case INST_CALL_OUTSIDER:
			{
				
				machine_rel_type r_type = bc->out_func.is_link_name ? DLL_FUNC :DLL_FUNC_NO_IMP_ADDING; 

				ret.rels.emplace_back(machine_reloc(DLL_FUNC, ret.code.size() + 2, bc->name));

				ret.code.emplace_back(0xff);
				ret.code.emplace_back(0x15);
				AddImm(0, 4, ret);
			}break;
			case JMP_E:
				//int jmp_sz = (bc->val * 4) > 0xff ? 4 : 1;
				AddJump(&*bc, 0x74, ret);
				CHANGE_JMP_DST_BC
				//ret.jmp_rels.emplace_back(jmp_sz
				break;
			case JMP_NE:
				AddJump(&*bc, 0x75, ret);
				CHANGE_JMP_DST_BC
				break;
			//unsigned
			case JMP_B:
				AddJump(&*bc, 0x72, ret);
				CHANGE_JMP_DST_BC
				break;
			case JMP_BE:
				AddJump(&*bc, 0x76, ret);
				CHANGE_JMP_DST_BC
				break;
			case JMP_A:
				AddJump(&*bc, 0x77, ret);
				CHANGE_JMP_DST_BC
				break;
			case JMP_AE:
				AddJump(&*bc, 0x73, ret);
				CHANGE_JMP_DST_BC
			break;
			//signed
			case JMP_L:
				AddJump(&*bc, 0x7c, ret);
				CHANGE_JMP_DST_BC
				break;
			case JMP_LE:
				AddJump(&*bc, 0x7e, ret);
				CHANGE_JMP_DST_BC
				break;
			case JMP_G:
				AddJump(&*bc, 0x7d, ret);
				CHANGE_JMP_DST_BC
				break;
			case JMP_GE:
				AddJump(&*bc, 0x7d, ret);
				CHANGE_JMP_DST_BC
			break;
			case MOV_F_2_REG_PARAM:
			{
				if (bc->bin.lhs.reg <= 9)
					CreateSSERegToSSEReg(&*bc, 0x10, &ret);
				else
					CreateSSERegToMem(&*bc, 0x11, &ret);
			}break;
			case MOV_R_2_SSE:
			case MOV_SSE_2_R:
			{
				CreateSSERegToSSEReg(&*bc, 0x2c, &ret);
			}break;
			case MOV_SSE_2_SSE:
			{
				CreateSSERegToSSEReg(&*bc, 0x10, &ret);
				//CreateMemToSSE(&*bc, 0x10, &ret);
			}break;
			case MOV_M_2_SSE:
			{
				CreateMemToSSE(&*bc, 0x10, &ret);
			}break;
			
			case MOV_SSE_2_RM:
				CreateSSERegToMem(&*bc, 0x11, &ret, bc->bin.lhs.reg, false);
			break;
			case MOV_SSE_2_MEM:
			{
				CreateSSERegToMem(&*bc, 0x11, &ret);
			}break;
			case MOV_SSE_2_REG_PARAM:
			{
				if (bc->bin.lhs.reg <= 9)
				{
					bc->bin.lhs.reg = bc->bin.lhs.reg - 6;
					CreateSSERegToSSEReg(&*bc, 0x10, &ret);
				}
				else
				{
					bc->bin.lhs.reg = 5;
					CreateSSERegToMem(&*bc, 0x11, &ret);
				}
			}break;
			case POP_REG_PARAM:
			{
				if (bc->bin.lhs.reg <= 9)
				{
					CreateMemToReg(&*bc, 0x8a, 0x8b, false, ret);
				}
				else
				{
					StoreMemToMem(&*bc, bc->bin.rhs.voffset, ret);
				}
			}break;
			case PUSH_REG_PARAM:
			{
				if (bc->bin.rhs.reg <= 9)
				{
					CreateRegToMem(&*bc, 0x88, 0x89, ret);
				}
				else
				{
					StoreMemToMem(&*bc, bc->bin.lhs.voffset, ret);
				}
			}break;
			case MOV_R_2_REG_PARAM:
			{
				if (bc->bin.lhs.reg <= 9)
				{
					char base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);
					bool base_reg_is_rex = IS_FLAG_ON(base_reg, 0x800);

					char src_reg = FromBCRegToAsmReg(bc->bin.rhs.reg);
					bool src_reg_is_rex = IS_FLAG_ON(src_reg, 0x800);

					char rex = ((char)base_reg_is_rex << 1) | ((char)src_reg_is_rex);
					AddPreMemInsts(bc->bin.lhs.reg_sz, 0x88, 0x89, rex, ret.code);

					base_reg &= 0xf;
					src_reg &= 0xf;
					char inst = (3 << 6) | (base_reg) | (src_reg << 3);
					ret.code.emplace_back(inst);
				}
				if (bc->bin.lhs.reg > 9)
				{
					char stack_reg = ((short)bc->bin.lhs.reg - 10) * 8 + 32;
					auto bin = bc->bin;
					bc->bin.lhs.reg = 5;
					bc->bin.lhs.voffset  = stack_reg;
					bc->bin.lhs.var_size = 8;
					CreateRegToMem(&*bc, 0x88, 0x89, ret);
					bc->bin = bin;
				}
			}break;
			case MOV_M_2_REG_PARAM:
			{
				if (bc->bin.lhs.reg <= 9)
				{
					CreateMemToReg(&*bc, 0x8a, 0x8b, false, ret);
				}
				if (bc->bin.lhs.reg > 9)
				{
					char stack_reg = (short)bc->bin.lhs.reg - 10;
					StoreMemToMem(&*bc, stack_reg * 8 + 32, ret);
				}
			}break;
			case MOV_I_2_REG_PARAM:
			{
				if (bc->bin.lhs.reg <= 9)
				{
					char dst_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);
					bool reg_is_rex = IS_FLAG_ON(dst_reg, 0x800);
					AddPreMemInsts(bc->bin.lhs.reg_sz, 0xc6, 0xc7, ((char)reg_is_rex) << 1, ret.code);
					ret.code.emplace_back(0xc0 + (char)(dst_reg & 0xf));
					AddImm(bc->bin.rhs.u64, bc->bin.lhs.reg_sz, ret);
				}
				if (bc->bin.lhs.reg > 9)
				{
					char stack_reg = (short)bc->bin.lhs.reg - 10;
					bc->bin.lhs.voffset = stack_reg * 8 + 32;
					bc->bin.lhs.var_size = bc->bin.lhs.reg_sz;
					bc->bin.lhs.reg = 5;
					CreateStoreImmToMem(&*bc, ret);
				}
			}break;
			case STORE_REG_PARAM:
			{
				if ((bc->bin.rhs.reg & 0xf)<= 9)
				{
					bool is_sse = IS_FLAG_ON(bc->bin.rhs.reg, 0x10);
					if (is_sse)
					{
						bc->bin.rhs.reg &= 0xf;
						CreateSSERegToMem(&*bc, 0x11, &ret);
					}
					else
						CreateRegToMem(&*bc, 0x88, 0x89, ret);
				}
			}break;
			case JMP:
			{
				int jmp = (int)bc->val * 4;
				if ((unsigned int)jmp <= 0x80)
				{
					ret.jmp_rels.emplace_back(jmp_rel(1, ret.code.size() + 1, &*bc));
					CHANGE_JMP_DST_BC

					ret.code.emplace_back(0xeb);
					ret.code.emplace_back(0x00);
				}
				else
				{
					ret.jmp_rels.emplace_back(jmp_rel(4, ret.code.size() + 1, &*bc));
					CHANGE_JMP_DST_BC

					ret.code.emplace_back(0xe9);

					ret.code.emplace_back(0x00);
					ret.code.emplace_back(0x00);
					ret.code.emplace_back(0x00);
					ret.code.emplace_back(0x00);
				}
			}break;


			case CMP_R_2_M:
			{
				CreateRegToMem(&*bc, 0x38, 0x39, ret);
			}break;
			case CMP_R_2_R:
			{
				CreateRegToReg(&*bc, 0x38, 0x39, &ret);
			}break;
			case CMP_I_2_RM:
			{
				CreateImmToMem(&*bc, 7, ret);
			}break;
			//
			case CMP_MEM_2_SSE:
			{
				// we moving the mem to reg 4, so the lhs sse must be a different reg
				ASSERT(bc->bin.rhs.reg != 4)

				auto last_lhs = bc->bin.lhs;
				
				// first we're moving the mem to xmm4
				bc->bin.lhs.reg = 4;

				// movss xmm4, mem
				CreateMemToSSE(&*bc, 0x10, &ret);

				bc->bin.lhs = last_lhs;
				bc->bin.rhs.reg = 4;

				// comiss xmm?, xmm4
				Create0FMemToReg(&*bc, 0x2f, &ret);
			}break;
			case CMP_SSE_2_MEM:
				// not handled
				ASSERT(false)
				CreateSSERegToMem(&*bc, 0x58, &ret);
			break;
			case CMP_SSE_2_RMEM:
				// not handled
				ASSERT(false)
				break;
			case CMP_SSE_2_SSE:
			{
				char mod = MakeModRM(false, 0, bc->bin.rhs.reg, bc->bin.lhs.reg);

				ret.code.emplace_back(0x0f);
				ret.code.emplace_back(0x2f);
				ret.code.emplace_back(mod);
			}break;
			//
			case CMP_I_2_M:
			{
				CreateImmToMem(&*bc, 7, ret);
			}break;
			case CMP_M_2_R:
				CreateMemToReg(&*bc, 0x3a, 0x3b, false, ret);
				break;
			case CMP_I_2_R:
				CreateImmToReg(0x3c, 0x3d, 7, &*bc, ret);
				break;

			case STORE_RM_2_R:
			case INST_LEA:
			{
				char base_reg = FromBCRegToAsmReg(bc->bin.rhs.lea.reg_base);
				char dst_reg = FromBCRegToAsmReg(bc->bin.rhs.lea.reg_dst);

				AddPreMemInsts(bc->bin.rhs.lea.size, 0x8d, 0x8d, false, ret.code);
				//ret.code.emplace_back(byte);

				AddModRM(true, bc->bin.rhs.lea.offset, base_reg, dst_reg, ret);
				if(bc->bin.rhs.lea.offset != 0)
					AddImm(bc->bin.rhs.lea.offset, ((unsigned int)bc->bin.rhs.lea.offset) < DISP_BYTE_MAX ? 1 : 4, ret);
			}break;
			case COMMENT:
			{
				if(f->name == "it_init{entity}")
					int a = 0;
			}
				break;
			case MOV_R:
			{
				CreateRegToReg(&*bc, 0x88, 0x89, &ret);
			}break;
			case MOV_RM:
			{
				CreateMemToReg(&*bc, 0x8a, 0x8b, false, ret, bc->bin.rhs.reg);
			}break;
			case MOV_M:
			{
				CreateMemToReg(&*bc, 0x8a, 0x8b, false, ret);
			}break;
			case STORE_R_2_RM:
			{
				CreateRegToMem(&*bc, 0x88, 0x89, ret, bc->bin.lhs.reg);
			}break;
			case STORE_RM_2_M:
			case STORE_R_2_M:
			{

				CreateRegToMem(&*bc, 0x88, 0x89, ret);

			}break;
			case STORE_I_2_M:
			{
				CreateStoreImmToMem(&*bc, ret);
			}break;
			case REP_B:
			{
				ret.code.emplace_back(0xf3);
				ret.code.emplace_back(0xa4);
			}break;
			case NOT_M:
			{
				ASSERT(false)
			}break;
			case NOT_R:
			{
				char dst = FromBCRegToAsmReg(bc->bin.lhs.reg);
				AddPreMemInsts(bc->bin.lhs.reg_sz, 0xf6, 0xf7, false, ret.code);
				char mod = MakeModRM(false,0, dst, 2);
				ret.code.emplace_back(mod);
			}break;
			case NEG_M:
			{
				ASSERT(false)
			}break;
			case NEG_R:
			{
				char dst = FromBCRegToAsmReg(bc->bin.lhs.reg);
				AddPreMemInsts(bc->bin.lhs.reg_sz, 0xf6, 0xf7, false, ret.code);
				char mod = MakeModRM(false,0, dst, 3);
				ret.code.emplace_back(mod);
			}break;

			case DIV_R:
			{
				// multiplying rax by src
				AddPreMemInsts(bc->bin.lhs.reg_sz, 0xf6, 0xf7, false, ret.code);
				char src = FromBCRegToAsmReg(bc->bin.lhs.reg);

				char mod = MakeModRM(false,0, src, 7);
				ret.code.emplace_back(mod);
			}break;
			case DIV_MEM_2_SSE:
				CreateMemToSSE(&*bc, 0x5e, &ret);
				break;
			case DIV_SSE_2_MEM:
				CreateSSERegToMem(&*bc, 0x5e, &ret);
				break;
			case DIV_SSE_2_RMEM:
				CreateSSERegToMem(&*bc, 0x5e, &ret, bc->bin.lhs.reg, false);
				break;
			case DIV_SSE_2_SSE:
				CreateSSERegToSSEReg(&*bc, 0x5e, &ret);
				break;

			case XOR_MEM_2_SSE:
				CreateMemToSSE(&*bc, 0x57, &ret);
				break;
			case XOR_SSE_2_MEM:
				CreateSSERegToMem(&*bc, 0x57, &ret);
				break;
			case XOR_SSE_2_RMEM:
				CreateSSERegToMem(&*bc, 0x57, &ret, bc->bin.lhs.reg, false);
				break;
			case XOR_SSE_2_SSE:
			{
				char mod = MakeModRM(false, 0, bc->bin.lhs.reg, bc->bin.rhs.reg);
				ret.code.emplace_back(0x0f);
				ret.code.emplace_back(0x57);
				ret.code.emplace_back(mod);

			}break;

			case MUL_R_2_R:
			{
				char dst = FromBCRegToAsmReg(bc->bin.lhs.reg);
				dst &= 0xf;
				char src = FromBCRegToAsmReg(bc->bin.rhs.reg);
				src &= 0xf;

				
				// moving dst to rax
				bc->bin.lhs.reg = 0;
				bc->bin.rhs.reg = dst;

				if (bc->bin.lhs.reg != bc->bin.rhs.reg)
					CreateRegToReg(&*bc, 0x88, 0x89, &ret);
				

				// multiplying rax by src
				AddPreMemInsts(bc->bin.lhs.reg_sz, 0xf6, 0xf7, false, ret.code);

				//  I'm not sure why the third byte reg should be 4
				char mod = MakeModRM(false,0, src, 4);
				ret.code.emplace_back(mod);
			
				// moving rax to dst
				bc->bin.lhs.reg = dst;
				bc->bin.rhs.reg = 0;
				if (bc->bin.lhs.reg != bc->bin.rhs.reg)
					CreateRegToReg(&*bc, 0x88, 0x89, &ret);

			}break;
			case MUL_I_2_R:
			{
				char base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);

				base_reg &= 0xf;
				bool less_than_byte = bc->bin.rhs.u32 < 0x80;

				if(less_than_byte)
					AddPreMemInsts(bc->bin.lhs.reg_sz, 0x6b, 0x6b, false, ret.code);
				else
					AddPreMemInsts(bc->bin.lhs.reg_sz, 0x69, 0x69, false, ret.code);

				char r = (3 << 6) | (base_reg << 3) | base_reg;

				ret.code.emplace_back(r);

				AddImm(bc->bin.rhs.u32, less_than_byte ? 1 : 4, ret);
			}break;
			case MUL_MEM_2_SSE:
				CreateMemToSSE(&*bc, 0x59, &ret);
				break;
			case MUL_SSE_2_MEM:
				CreateSSERegToMem(&*bc, 0x59, &ret);
				break;
			case MUL_SSE_2_RMEM:
				CreateSSERegToMem(&*bc, 0x59, &ret, bc->bin.lhs.reg, false);
				break;
			case MUL_SSE_2_SSE:
				CreateSSERegToSSEReg(&*bc, 0x59, &ret);
				break;


			case MOV_I:
			{
					char base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);
					bool reg_is_rex = IS_FLAG_ON(base_reg, 0x800);
					AddPreMemInsts(bc->bin.lhs.reg_sz, 0xc6, 0xc7, reg_is_rex, ret.code);
					ret.code.emplace_back(0xc0 + (char)(base_reg & 0xf));
					AddImm(bc->bin.rhs.u64, bc->bin.lhs.reg_sz, ret);
			}break;

			
			case MOV_ABS:
			{
					char base_reg = FromBCRegToAsmReg(bc->bin.lhs.reg);
					bool reg_is_rex = IS_FLAG_ON(base_reg, 0x800);
					AddPreMemInsts(8, 0xb8, 0xb8, reg_is_rex, ret.code);
					AddImm(bc->bin.rhs.u64, 4, ret);
					AddImm(bc->bin.rhs.u64 >> 32, 4, ret);
			}break;

			case AND_I_2_R:
				CreateImmToReg(0x24, 0x25, 4, &*bc, ret);
				break;
			case AND_R_2_R:
				CreateRegToReg(&*bc, 0x20, 0x21, &ret);
			break;


			case SUB_I_2_M:
				CreateImmToMem(&*bc, 0x5, ret);
				break;
			case SUB_R_2_M:
				CreateRegToMem(&*bc, 0x28, 0x29, ret);
				break;
			case SUB_MEM_2_SSE:
				CreateMemToSSE(&*bc, 0x5c, &ret);
				break;
			case SUB_SSE_2_MEM:
				CreateSSERegToMem(&*bc, 0x5c, &ret);
				break;
			case SUB_SSE_2_RMEM:
				CreateSSERegToMem(&*bc, 0x5c, &ret, bc->bin.lhs.reg, false);
				break;
			case SUB_SSE_2_SSE:
				CreateSSERegToSSEReg(&*bc, 0x5c, &ret);
				break;
			case SUB_M_2_R:
				CreateMemToReg(&*bc, 0x2a, 0x2b, false, ret);
				break;
			case SUB_R_2_R:
				CreateRegToReg(&*bc, 0x28, 0x29, &ret);
				break;
			case SUB_I_2_R:
				CreateImmToReg(0x2c, 0x2d, 5, &*bc, ret);
				break;



			case ADD_R_2_RM:
				CreateRegToMem(&*bc, 0x0, 0x1, ret, bc->bin.lhs.reg);
			break;
			case ADD_I_2_M:
				CreateImmToMem(&*bc, 0x0, ret);
			break;
			case ADD_R_2_M:
				CreateRegToMem(&*bc, 0x0, 0x1, ret);
			break;
			case ADD_MEM_2_SSE:
				CreateMemToSSE(&*bc, 0x58, &ret);
			break;
			case ADD_SSE_2_MEM:
				CreateSSERegToMem(&*bc, 0x58, &ret);
			break;
			case ADD_SSE_2_RMEM:
				CreateSSERegToMem(&*bc, 0x58, &ret, bc->bin.lhs.reg, false);
				break;
			case ADD_SSE_2_SSE:
				CreateSSERegToSSEReg(&*bc, 0x58, &ret);
			break;
			case ADD_M_2_R:
				CreateMemToReg(&*bc, 0x2, 0x3, false, ret);
				break;
			case ADD_R_2_R:
				CreateRegToReg(&*bc, 0x0, 0x1, &ret);
				break;
			case ADD_I_2_RM:
				CreateImmToMem(&*bc, 0x0, ret, bc->bin.lhs.reg);
				break;
			case ADD_I_2_R:
				CreateImmToReg(0x4, 0x5, 0, &*bc, ret);
				break;

			case OR_R_2_RM:
				CreateRegToMem(&*bc, 0x08, 0x9, ret, bc->bin.lhs.reg);
			break;
			case OR_I_2_M:
				CreateImmToMem(&*bc, 0x1, ret);
			break;
			case OR_R_2_M:
				CreateRegToMem(&*bc, 0x8, 0x9, ret);
			break;
			case OR_M_2_R:
				CreateMemToReg(&*bc, 0xa, 0xb, false, ret);
				break;
			case OR_R_2_R:
				CreateRegToReg(&*bc, 0x8, 0x9, &ret);
				break;
			case OR_I_2_RM:
				CreateImmToMem(&*bc, 0x1, ret, bc->bin.lhs.reg);
				break;
			case OR_I_2_R:
				CreateImmToReg(0xc, 0xd, 1, &*bc, ret);
				break;
			case INST_CALL_REG:
			{
				ret.code.emplace_back(0xff);
				ret.code.emplace_back(0xd0 + bc->val);
			}break;

			case POP_STACK_SIZE:
			case X64_WASM_BEGIN_BLOCK:
			case X64_WASM_END_BLOCK:
			case PUSH_STACK_SIZE:
			{
			}break;
			case PUSH_R:
			{
				char reg = FromBCRegToAsmReg(bc->val);
				ret.code.emplace_back(0x50 + (reg & 0xf));
			}break;
			case RET:
			{
				ret.code.emplace_back(0xc3);
			}break;
			case POP_R:
			{
				char reg = FromBCRegToAsmReg(bc->val);
				ret.code.emplace_back(0x58 + (reg & 0xf));
			}break;
			default:
			{
				ASSERT(false)
			}break;
			}
			i++;
		}
		
	}
}

func_byte_code *interpreter::SearchFinalFunc(own_std::string name)
{
	FOR_VEC(f, (*ar))
	{
		if((*f)->name == name)
			return (*f);
	}
	return nullptr;

}

/*
void interpreter::SetBCode(lang_state *lang_stat, own_std::vector<func_byte_code *> *ar)
{
	memset(this, 0, sizeof(*this));
	this->ar = ar;

	//setting bcode
	FOR_VEC(f, (*ar))
	{
		(*f)->start_idx = bcode.size();
		INSERT_VEC(bcode, (*f)->bcodes);
		for (int i = 0; i < 2; i++)
		{
			bcode.emplace_back(byte_code_enum::NOP);
		}
	}
	FOR_VEC(bc, bcode)
	{
		if(bc->type == byte_code_enum::RELOC)
		{
			if (bc->rel.type == rel_type::REL_FUNC)
			{
				auto found_func = SearchFinalFunc(bc->rel.name);
				ASSERT(found_func != nullptr)
					bc->type = byte_code_enum::INST_CALL;
				bc->val = found_func->start_idx;
			}
			if (bc->rel.type == rel_type::REL_DATA)
			{
				long long data_addr = (long long)&lang_stat->data_sect.data()[bc->rel.offset];
				bc->type = byte_code_enum::MOV_I;
				bc->bin.lhs.reg = 0;
				bc->bin.rhs.s64 = data_addr;
			}
		}

	}
	
}
*/
enum byte_code_instr
{
	BC_EQUAL,
	BC_CMP,
	BC_PLUS,
	BC_PLUS_EQUAL,
	BC_MINUS_EQUAL,
	BC_MUL,
	BC_DIV,
	BC_MODULO,
	BC_SUB,
	BC_POP,
	BC_PUSH,
	BC_RET,
	BC_OR,
	BC_NEG,
	BC_NOT,
	BC_AND,
	BC_CALL,
	BC_XOR,
};

// check if the descend func is the enum
#define DESCEND_FTP_EQ(t, enm) (t->type == fret_type::enm)
// DRF is descend ret func
#define FROM_DFR_TO_BIN_BC_VAR(dfr, bin) \
						bin.reg = dfr->reg;\
						bin.var = dfr->var;\
						bin.voffset = dfr->voffset;\
						bin.reg_sz = dfr->reg_sz;\
						bin.var_size = dfr->var_size;


#define NEW_INST(NAME)\
		if(DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_INT))\
		{\
			ret.type = byte_code_enum::NAME##_I_2_RM;	\
			ret.bin.lhs.lea = lhs->lea;\
			ret.bin.rhs.var = rhs->var;\
		}\
		if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_MEM))\
		{\
			ret.type = byte_code_enum::NAME##_M_2_M;	\
			ret.bin.lhs.var = lhs->var;\
			ret.bin.rhs.var = rhs->var;\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_INT))\
		{\
			ret.type = byte_code_enum::NAME##_I_2_RM;	\
			ret.bin.rhs.i = rhs->i;\
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_REG))\
		{\
			ret.type = byte_code_enum::NAME##_R_2_RM;	\
			ret.bin.rhs.r = rhs->r;\
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_SSE_REG))\
		{\
			ret.type = byte_code_enum::NAME##_SSE_2_RMEM;	\
			ret.bin.rhs.r = rhs->r;\
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_SSE_REG))\
		{\
			ret.type = byte_code_enum::NAME##_SSE_2_MEM;	\
			ret.bin.rhs.r = rhs->r;\
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_SSE_REG) && DESCEND_FTP_EQ(rhs, R_MEM))\
		{\
			ret.type = byte_code_enum::NAME##_MEM_2_SSE;	\
			ret.bin.lhs.r = lhs->r;\
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_SSE_REG) && DESCEND_FTP_EQ(rhs, R_SSE_REG))\
		{\
			ret.type = byte_code_enum::NAME##_SSE_2_SSE;	\
			ret.bin.lhs.r = lhs->r;\
			ret.bin.rhs.r = rhs->r;\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_MEM))\
		{\
			ret.type = byte_code_enum::NAME##_M_2_R;	\
			ret.bin.lhs.r = lhs->r;\
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_INT))\
		{\
			ret.type = byte_code_enum::NAME##_I_2_R;	\
			ret.bin.lhs.r = lhs->r;\
			ret.bin.rhs.s32 = rhs->i;\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_REG))\
		{\
			ret.type = byte_code_enum::NAME##_R_2_R;	\
			ret.bin.lhs.r = lhs->r;\
			ret.bin.rhs.r = rhs->r;\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_REG))\
		{\
			ret.type = byte_code_enum::NAME##_R_2_M;	\
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);\
			ret.bin.rhs.r = rhs->r;\
		}\
		else if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_INT))\
		{\
			ret.type = byte_code_enum::NAME##_I_2_M;	\
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);\
			ret.bin.rhs.s32 = rhs->i;\
		}\
		else\
		{\
			ASSERT(false)\
		}\



byte_code ReturnBCode(byte_code_instr op, descend_func_ret *lhs, descend_func_ret *rhs)
{
	byte_code ret;
	switch(op)
	{
	case byte_code_instr::BC_RET:
	{
		ret.type = byte_code_enum::INST_RET;
	}break;
	case byte_code_instr::BC_CALL:
	{
		ret.type = byte_code_enum::INST_CALL;	
	}break;
	case byte_code_instr::BC_PUSH:
	{
		if(DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::PUSH_R;
			ret.un.val.reg = rhs->reg;
		}
	}break;
	case byte_code_instr::BC_POP:
	{
		if(DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::POP_R;
			ret.val = rhs->reg;
		}
	}break;
	case byte_code_instr::BC_XOR:
	{
		NEW_INST(XOR)
	}break;
	case byte_code_instr::BC_OR:
	{
		NEW_INST(OR)
	}break;
	case byte_code_instr::BC_AND:
	{
		NEW_INST(AND)
	}break;
	case byte_code_instr::BC_DIV:
	{
		NEW_INST(DIV)
	}break;
	case byte_code_instr::BC_MUL:
	{
		NEW_INST(MUL)
	}break;
	case byte_code_instr::BC_CMP:
	{
		NEW_INST(CMP)
	}break;
	case byte_code_instr::BC_MINUS_EQUAL:
	case byte_code_instr::BC_SUB:
	{
		NEW_INST(SUB)
		
	}break;
	case byte_code_instr::BC_PLUS_EQUAL:
	case byte_code_instr::BC_PLUS:
	{
		NEW_INST(ADD)
		
	}break;
	case byte_code_instr::BC_NOT:
	{

		if(DESCEND_FTP_EQ(lhs, R_MEM))
		{
			ret.type = byte_code_enum::NOT_M;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
		}
		else if(DESCEND_FTP_EQ(lhs, R_REG))
		{
			ret.type = byte_code_enum::NOT_R;
			ret.bin.lhs.r = lhs->r;
		}
		else
		{
			ASSERT(false)
		}

	}break;
	case byte_code_instr::BC_NEG:
	{

		if(DESCEND_FTP_EQ(lhs, R_MEM))
		{
			ret.type = byte_code_enum::NEG_M;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
		}
		else if(DESCEND_FTP_EQ(lhs, R_REG))
		{
			ret.type = byte_code_enum::NEG_R;
			ret.bin.lhs.r = lhs->r;
		}
		else
		{
			ASSERT(false)
		}

	}break;
	case byte_code_instr::BC_EQUAL:
	{
		if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_RMEM))
		{
			ret.type = byte_code_enum::STORE_RM_2_M;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
		}
		if (DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_RMEM))
		{
			ret.type = byte_code_enum::STORE_RM_2_M;
			
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
			ret.bin.lhs.reg = lhs->reg;
		}
		else if (DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_RMEM))
		{
			ret.type = byte_code_enum::MOV_RM;
			ret.bin.lhs.r = lhs->r;
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);
		}
		else if (DESCEND_FTP_EQ(lhs, R_PARAM_REG) && DESCEND_FTP_EQ(rhs, R_SSE_REG))
		{
			ret.type = byte_code_enum::MOV_SSE_2_REG_PARAM;
			ret.bin.lhs.r = lhs->r;
			//ret.bin.lhs = lhs->voffset;
			ret.bin.rhs.r = rhs->r;
		}
		else if (DESCEND_FTP_EQ(lhs, R_PARAM_REG) && DESCEND_FTP_EQ(rhs, R_FLOAT))
		{
			ret.type = byte_code_enum::MOV_F_2_REG_PARAM;
			ret.bin.lhs.r = lhs->r;
			//ret.bin.lhs = lhs->voffset;
			ret.bin.rhs.f32 = rhs->f32;
		}
		else if (DESCEND_FTP_EQ(lhs, R_PARAM_REG) && DESCEND_FTP_EQ(rhs, R_RMEM))
		{
			ret.type = byte_code_enum::MOV_R_2_REG_PARAM;
			ret.bin.lhs.r = lhs->r;
			//ret.bin.lhs = lhs->voffset;
			ret.bin.rhs.lea = rhs->lea;
		}
		else if (DESCEND_FTP_EQ(lhs, R_PARAM_REG) && DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::MOV_R_2_REG_PARAM;
			ret.bin.lhs.r = lhs->r;
			//ret.bin.lhs = lhs->voffset;
			ret.bin.rhs.r = rhs->r;
		}
		else if (DESCEND_FTP_EQ(lhs, R_PARAM_REG) && DESCEND_FTP_EQ(rhs, R_INT))
		{
			ret.type = byte_code_enum::MOV_I_2_REG_PARAM;
			ret.bin.lhs.r = lhs->r;
			//ret.bin.lhs = lhs->voffset;
			ret.bin.rhs.s32 = rhs->i;
		}
		else if (DESCEND_FTP_EQ(lhs, R_PARAM_REG) && DESCEND_FTP_EQ(rhs, R_MEM))
		{
			ret.type = byte_code_enum::MOV_M_2_REG_PARAM;
			ret.bin.lhs.reg = lhs->reg;
			ret.bin.lhs.r = lhs->r;
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);
		}
		else if (DESCEND_FTP_EQ(lhs, R_SSE_REG) && DESCEND_FTP_EQ(rhs, R_MEM))
		{
			ret.type = byte_code_enum::MOV_M_2_SSE;
			ret.bin.lhs.r = lhs->r;
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);
		}
		else if (DESCEND_FTP_EQ(lhs, R_SSE_REG) && DESCEND_FTP_EQ(rhs, R_SSE_REG))
		{
			ret.type = byte_code_enum::MOV_SSE_2_SSE;
			ret.bin.lhs.r = lhs->r;
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);
		}
		else if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_MEM))
		{
			ret.type = byte_code_enum::STORE_M_2_M;	
			ret.bin.lhs.var = lhs->var;
			ret.bin.rhs.var = rhs->var;
		}
		else if (DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_RMEM))
		{
			ret.type = byte_code_enum::STORE_R_2_M;
			ret.bin.lhs.var = lhs->var;
			ret.bin.rhs.r   = rhs->r;
		}
		else if(DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_MEM))
		{
			ret.type = byte_code_enum::MOV_M;	
			ret.bin.lhs.reg = lhs->reg;
			ret.bin.lhs.r = lhs->r;
			FROM_DFR_TO_BIN_BC_VAR(rhs, ret.bin.rhs);

		}
		else if(DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_INT))
		{
			ret.type = byte_code_enum::MOV_I;	
			ret.bin.lhs.r = lhs->r;
			ret.bin.rhs.s32 = rhs->i;

		}
		else if (DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_INT))
		{
			ret.type = byte_code_enum::STORE_I_2_M;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
			ret.bin.rhs.u64 = rhs->i;

		}
		else if (DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_SSE_REG))
		{
			ret.type = byte_code_enum::MOV_SSE_2_RM;
			ret.bin.rhs.lea = rhs->lea;
			ret.bin.lhs.r = lhs->r;

		}
		else if (DESCEND_FTP_EQ(lhs, R_RMEM) && DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::STORE_R_2_RM;
			ret.bin.lhs.lea = lhs->lea;
			ret.bin.rhs.reg = rhs->reg;
			ret.bin.rhs.r = rhs->r;

		}
		else if(DESCEND_FTP_EQ(lhs, R_REG) && DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::MOV_R;	
			ret.bin.lhs.r = lhs->r;
			ret.bin.rhs.r = rhs->r;

		}
		else if (DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_SSE_REG))
		{
			ret.type = byte_code_enum::MOV_SSE_2_MEM;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);

			ret.bin.rhs.r = rhs->r;

		}
		else if (DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::STORE_R_2_M;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);

			ret.bin.rhs.r = rhs->r;

		}
		else if(DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_INT))
		{
			ret.type = byte_code_enum::STORE_I_2_M;	
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
			ret.bin.rhs.s32 = rhs->i;

		}
		else if (DESCEND_FTP_EQ(lhs, R_MEM) && DESCEND_FTP_EQ(rhs, R_REG))
		{
			ret.type = byte_code_enum::STORE_R_2_M;
			FROM_DFR_TO_BIN_BC_VAR(lhs, ret.bin.lhs);
			ret.bin.rhs.s32 = rhs->i;

		}
		else
		{
			ASSERT(false)
		}
	}break;
	}
	return ret;
}
byte_code_instr TokenToInst(tkn_type2 tp)
{
	switch(tp)
	{
		case tkn_type2::T_COND_NE:
		case tkn_type2::T_COND_EQ:
		case tkn_type2::T_LESSER_EQ:
		case tkn_type2::T_GREATER_EQ:
		case tkn_type2::T_LESSER_THAN:
		case tkn_type2::T_GREATER_THAN: return BC_CMP;
		case tkn_type2::T_OPEN_BRACKETS:
		case tkn_type2::T_EQUAL: return BC_EQUAL;
		case tkn_type2::T_MINUS_EQUAL: return BC_MINUS_EQUAL;
		case tkn_type2::T_MINUS: return BC_SUB;
		case tkn_type2::T_PLUS: return BC_PLUS;
		case tkn_type2::T_MUL: return BC_MUL;
		case tkn_type2::T_DIV: return BC_DIV;
		case tkn_type2::T_PERCENT: return BC_MODULO;
		case tkn_type2::T_AMPERSAND: return BC_AND;
		case tkn_type2::T_PIPE: return BC_OR;
		case tkn_type2::T_PLUS_EQUAL: return BC_PLUS_EQUAL;
		default: ASSERT(false)
	}
	return byte_code_instr::BC_EQUAL;
}
void EmplacePopStackSize(func_byte_code *func, descend_func_ret *out)
{
	out->bcodes.emplace_back(byte_code(byte_code_enum::POP_STACK_SIZE));
}
void EmplacePushStackSize(func_byte_code *func, int size, descend_func_ret *out)
{
	out->bcodes.emplace_back(byte_code(byte_code_enum::PUSH_STACK_SIZE, size));
}
#define IREG_TO_REG(reg_src, reg_dst, reg_sz, ret, inst)\
								EmplaceInstRegToReg(final_func,  reg_dst, reg_src, reg_sz, byte_code_instr::inst, &ret);

void EmplaceInstRegToReg(func_byte_code *func, char reg_dst, char reg_src, char reg_sz, byte_code_instr inst, descend_func_ret *out, bool is_sse = false)
{
	if (reg_dst == reg_src && inst == byte_code_instr::BC_EQUAL)
		return;

	descend_func_ret r_src;
	descend_func_ret r_dst;
	if (is_sse)
	{
		r_src.type = fret_type::R_SSE_REG;
		r_dst.type = fret_type::R_SSE_REG;
	}
	else
	{
		r_src.type = fret_type::R_REG;
		r_dst.type = fret_type::R_REG;
	}

	r_src.reg = reg_src;
	r_src.reg_sz  = reg_sz;
	r_dst.reg = reg_dst;
	r_dst.reg_sz  = reg_sz;

	out->bcodes.emplace_back(ReturnBCode(inst, &r_dst , &r_src));

}
void EmplaceRetInst(func_byte_code *func)
{
	func->aux_buffer.emplace_back(ReturnBCode(byte_code_instr::BC_RET, nullptr, nullptr));
}
void EmplacePopReg(func_byte_code *final_func, char reg, descend_func_ret *out)
{
	descend_func_ret r;
	CREATE_REG(r, reg);
	out->bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_POP, nullptr, &r));

}
void EmplaceInstRMToReg(func_byte_code *func, char reg_dst, char reg_base, int offset, byte_code_instr inst, descend_func_ret *out)
{
	
	descend_func_ret r;
	descend_func_ret rm;
	rm.type = fret_type::R_RMEM;
	rm.reg	   = reg_base;
	rm.voffset = offset;
	rm.reg_sz = 8;
	CREATE_REG(r, reg_dst);
	out->bcodes.emplace_back(ReturnBCode(inst, &r, &rm));
	
}
void EmplaceInstImmToMem(func_byte_code *func, int val, decl2 *decl, byte_code_instr inst, descend_func_ret *out)
{
	descend_func_ret imm;
	descend_func_ret var;
	CREATE_VAR(var, decl);
	CREATE_IMM(imm, val);
	out->bcodes.emplace_back(ReturnBCode(inst, &var, &imm));
}
void EmplaceInstImmToMem(func_byte_code* func, int val, decl2* decl, int offset, byte_code_instr inst, descend_func_ret* out, char imm_sz = 4)
{
	descend_func_ret imm;
	descend_func_ret var;
	CREATE_VAR(var, decl);
	CREATE_IMM(imm, val);
	var.voffset = offset;
	var.reg_sz = imm_sz;
	out->bcodes.emplace_back(ReturnBCode(inst, &var, &imm));
}
#define IREG_TO_MEM(reg, ret, off, var, reg_sz, mem_reg, inst)\
EmplaceInstRegToMemOffset(final_func, reg, reg_sz, off, 8, byte_code_instr ::inst, &ret); 

void EmplaceInstRegToMemOffset(func_byte_code* func, char reg, char reg_sz, int offset, char mem_sz, byte_code_instr inst, descend_func_ret* out, bool is_float = false)
{
	descend_func_ret r;
	descend_func_ret var;

	var.type = fret_type::R_MEM;
	var.voffset = offset;
	var.var_size = mem_sz;
	var.reg = 5;
	var.reg_sz = reg_sz;
	r.reg_sz = reg_sz;


	CREATE_REG(r, reg);

	if (is_float)
		r.type = fret_type::R_SSE_REG;
	out->bcodes.emplace_back(ReturnBCode(inst, &var, &r));
}
void EmplaceInstRegToMemOffset(func_byte_code *func, char reg, int offset, byte_code_instr inst, descend_func_ret *out)
{
	descend_func_ret r;
	descend_func_ret var;

	var.type = fret_type::R_MEM;
	var.voffset = offset;
	var.var_size = 8;
	var.reg = 5;

	CREATE_REG(r, reg);
	out->bcodes.emplace_back(ReturnBCode(inst, &var, &r));
}
void EmplaceInstRegToMemSib(func_byte_code *func, char reg, int offset, char var_size, char sib, byte_code_instr inst, descend_func_ret *out)
{
	descend_func_ret r;
	descend_func_ret var;
	var.type = fret_type::R_MEM;
	var.voffset = offset;
	var.var_size = var_size;
	var.sib = 0;
	var.reg = 5;
	CREATE_REG(r, reg);
	out->bcodes.emplace_back(ReturnBCode(inst, &var, &r));
}
void EmplaceInstRegToMem(func_byte_code *func, char reg, decl2 *decl, byte_code_instr inst, descend_func_ret *out, int reg_sz = 8)
{
	descend_func_ret r;
	descend_func_ret var;
	CREATE_VAR(var, decl)
	CREATE_REG(r, reg);
	var.reg_sz = reg_sz;
	r.reg_sz = reg_sz;
	out->bcodes.emplace_back(ReturnBCode(inst, &var, &r));
}
#define IMEM_TO_REG(reg, ret, off, var, reg_sz, mem_reg)\
					EmplaceInstMemOffsetToReg2(final_func, reg, off, mem_reg, byte_code_instr::BC_EQUAL, &ret, var, reg_sz);

void EmplaceInstMemOffsetToReg2(func_byte_code* func, char reg, long long offset, char mem_reg, byte_code_instr inst, descend_func_ret* out, decl2 *decl, char reg_sz = 8, char ptr = 0)
{
	descend_func_ret r;
	descend_func_ret var;
	if(decl->type.type == enum_type2::TYPE_F32 && ptr == 0)
	{
		CREATE_VAR(var, decl)
		r.type = fret_type::R_SSE_REG;
	}
	else
	{
		if (inst == byte_code_instr::BC_EQUAL && ptr == 0 
			&& decl->type.type != TYPE_VOID
			&& decl->type.type != TYPE_STRUCT
			&& decl->type.type != TYPE_STRUCT_TYPE)
		{
			bool is_unsigned = is_type_unsigned(decl->type.type);
			byte_code bc;
			char tp_size = GetTypeSize(&decl->type);
			// if the mem is 4 bytes and unsigned, we'll just do an mov, instead of a movzx
			if (tp_size < 8 && (tp_size != 4 && is_unsigned) || 
				(tp_size <= 4 && !is_unsigned))
			{
				if (is_unsigned)
					bc.type = byte_code_enum::MOVZX_M;

				else
					bc.type = byte_code_enum::MOVSX_M;

				bc.bin.rhs.reg_sz = tp_size;
				bc.bin.lhs.reg = reg;
				bc.bin.lhs.reg_sz = 8;
				bc.bin.rhs.reg = mem_reg;
				bc.bin.rhs.voffset = offset;

				out->bcodes.emplace_back(bc);
				return;
			}
		}
		CREATE_VAR(var, decl)
		CREATE_REG(r, reg);
	}
	r.reg = reg;
	r.reg_sz = reg_sz;
	var.voffset = offset;
	var.reg = mem_reg;
	

	out->bcodes.emplace_back(ReturnBCode(inst, &r, &var));
}

void EmplaceInstMemOffsetToReg(func_byte_code *func, char reg, int offset, byte_code_instr inst, descend_func_ret *out, bool is_sse = false, char mem_reg = 5)
{
	descend_func_ret r;
	descend_func_ret var;

	var.type = fret_type::R_MEM;
	var.voffset = offset;
	var.var_size = 8;
	var.reg = mem_reg;

	CREATE_REG(r, reg);
	if (is_sse)
		r.type = fret_type::R_SSE_REG;

	out->bcodes.emplace_back(ReturnBCode(inst, &r, &var));
}

void EmplaceInstMemToReg(func_byte_code *func, char reg, decl2 *decl, int offset, byte_code_instr inst, descend_func_ret *out)
{
	EmplaceInstMemOffsetToReg2(func, reg, offset, 5, inst, out, decl);
}
void EmplaceInstMemToReg(func_byte_code *func, char reg, decl2 *decl, byte_code_instr inst, descend_func_ret *out)
{
	EmplaceInstMemOffsetToReg2(func, reg, decl->offset, 5, inst, out, decl);

	/*
	descend_func_ret r;
	descend_func_ret var;
	if(decl->type.type == enum_type2::TYPE_F32)
	{
		CREATE_VAR(var, decl)
		r.type = fret_type::R_SSE_REG;
		r.reg  = reg;
		CREATE_VAR(var, decl)
	}
	else
	{
		CREATE_VAR(var, decl)
		CREATE_REG(r, reg);
	}
	out->bcodes.emplace_back(ReturnBCode(inst, &r, &var));
	*/
}
#define IIMM_TO_REG(imm, reg_dst, reg_sz, ret, inst)\
EmplaceInstImmToReg(final_func, reg_dst, imm, byte_code_instr::inst, &ret, reg_sz)

void EmplaceInstImmToReg(func_byte_code *func, char reg, int imm, byte_code_instr inst, descend_func_ret *out, char reg_sz = 8)
{
	descend_func_ret sp;
	descend_func_ret imm_strct;
	memset(&imm_strct, 0, sizeof(descend_func_ret));
	memset(&sp, 0, sizeof(descend_func_ret));
	CREATE_IMM(imm_strct, imm)
	CREATE_REG(sp, reg);
	sp.reg_sz = reg_sz;
	out->bcodes.emplace_back(ReturnBCode(inst, &sp, &imm_strct));
}
void EmplacePushReg(func_byte_code *final_func, char reg, descend_func_ret *out)
{
	descend_func_ret r;
	CREATE_REG(r, reg);
	out->bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_PUSH, nullptr, &r));

}
void EmplaceInstRepStos(func_byte_code* func, char dst_mem_base, int offset_dst, char src_mem_base, int offset_src, char stos_sz, int bytes, descend_func_ret* out)
{
	EmplaceLeaInst(33, dst_mem_base, offset_dst, 8, out);
	EmplaceLeaInst(32, src_mem_base, offset_src, 8, out);
	EmplaceInstImmToReg(func, 1, bytes, byte_code_instr::BC_EQUAL, out, 4);


	byte_code bc;
	bc.bin.lhs.reg = dst_mem_base;
	bc.bin.lhs.voffset = offset_dst;
	bc.bin.rhs.reg = src_mem_base;
	bc.bin.rhs.voffset = offset_src;
	switch (stos_sz)
	{
	case 1:
	{
		bc.type = REP_B;
	}break;
	default:
		ASSERT(false)
	}
	out->bcodes.emplace_back(bc);

}

void interpreter::PushVal(long long  val)
{
	long long *ptr = (long long *)(regs.sp.i64);
	*ptr = val;
	regs.sp.i64 += 8;
}
void EmplaceRetGroup(func_byte_code *final_func, func_decl *fdecl, descend_func_ret *rhs_ret, descend_func_ret * out)
{
	descend_func_ret r0;

	EmplaceComment(out, "// return");
	CREATE_REG(r0, 0);

	if ((rhs_ret && rhs_ret->type == R_SSE_REG) || fdecl->ret_type.IsFloat())
		r0.type = R_SSE_REG;

	if(rhs_ret != nullptr && IS_FLAG_OFF(rhs_ret->flags, DFR_STRCT_VAL_ADDRESS_ALREADY_IN_RAX))
		out->bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_EQUAL, &r0, rhs_ret));


	// moving rsi to rax
	IREG_TO_REG(34, 32, 8, *out, BC_EQUAL);

	// poping last rbx
	int offset = final_func->fdecl->saved_lhs_offset ;
	EmplaceInstMemOffsetToReg(final_func, 3, offset, byte_code_instr::BC_EQUAL, out);
	// poping last rbp
	EmplaceInstMemOffsetToReg(final_func, 34, offset + 8, byte_code_instr::BC_EQUAL, out);

	// moving rax to rsp
	IREG_TO_REG(32, 5, 8, *out, BC_EQUAL);

	// adding the sp reg to the last value
	//EmplaceInstImmToReg(final_func, 5, fdecl->stack_size, byte_code_instr::BC_PLUS, out);


	

	out->bcodes.emplace_back(byte_code(byte_code_enum::RET, nullptr));

}
descend_func_ret DescendFunc(lang_state *, func_byte_code *final_func, node *n, scope *scp, int = 0);

#define INC_PRS_CALL_REGS_USED 1

own_std::vector<descend_func_ret> DescendCommaFunc(lang_state *lang_stat, func_byte_code *final_func, node *n, scope *scp, int flags = INC_PRS_CALL_REGS_USED)
{
	own_std::vector<descend_func_ret> ret;
	if(IS_COMMA(n))
	{
		auto lvalue = DescendCommaFunc(lang_stat, final_func, n->l, scp, flags);
		if (!lvalue.empty())
			ret.insert(ret.end(), lvalue.begin(), lvalue.end());

		auto rvalue = DescendCommaFunc(lang_stat, final_func, n->r, scp, flags);
		if (!rvalue.empty())
			ret.insert(ret.end(), rvalue.begin(), rvalue.end());
	}
	else
	{
		int start_idx = final_func->aux_buffer.size();
		auto lvalue = DescendFunc(lang_stat, final_func, n, scp);

		if(IS_FLAG_ON(flags, INC_PRS_CALL_REGS_USED))
			lang_stat->call_regs_used++;

		int end_idx = final_func->aux_buffer.size();

		lvalue.aux_buffer_start = start_idx;
		lvalue.aux_buffer_end   = end_idx;

		ret.emplace_back(lvalue);
		
	}
	return ret;
}
void EmplaceCondJmpInst(tkn_type2 t, int offset, descend_func_ret *out, bool is_unsigned)
{
	byte_code bc;

	if(is_unsigned)
	{
		switch(t)
		{
		case tkn_type2::T_EXCLAMATION:
		case tkn_type2::T_COND_EQ: bc.type = byte_code_enum::JMP_E; break;
		case tkn_type2::T_COND_NE: bc.type = byte_code_enum::JMP_NE; break;
		case tkn_type2::T_LESSER_THAN: bc.type = byte_code_enum::JMP_B; break;
		case tkn_type2::T_LESSER_EQ: bc.type = byte_code_enum::JMP_BE; break;
		case tkn_type2::T_GREATER_THAN: bc.type = byte_code_enum::JMP_A; break;
		case tkn_type2::T_GREATER_EQ: bc.type = byte_code_enum::JMP_AE; break;
		}
	}
	else
	{
		switch(t)
		{
		case tkn_type2::T_EXCLAMATION:
		case tkn_type2::T_COND_EQ: bc.type = byte_code_enum::JMP_E; break;
		case tkn_type2::T_COND_NE: bc.type = byte_code_enum::JMP_NE; break;
		case tkn_type2::T_LESSER_THAN: bc.type = byte_code_enum::JMP_L; break;
		case tkn_type2::T_LESSER_EQ: bc.type = byte_code_enum::JMP_LE; break;
		case tkn_type2::T_GREATER_THAN: bc.type = byte_code_enum::JMP_G; break;
		case tkn_type2::T_GREATER_EQ: bc.type = byte_code_enum::JMP_GE; break;
		}
	}
	bc.jmp_rel = offset;
	out->bcodes.emplace_back(bc);
}
#define CREATE_LEA(reg_base, reg_dst, offset) (long long)((reg_base & 0xff) | ((reg_dst & 0xff) << 8) | ((offset & 0xffffffff)<16))

void EmplaceDataSectReloc(char reg_dst, int offset, descend_func_ret *ret)
{
	ret->bcodes.emplace_back(byte_code(rel_type::REL_DATA, (char*)nullptr, (int)offset, reg_dst));
}

void MovStrLitToReg(lang_state *lang_stat, char reg, own_std::string str, descend_func_ret *ret)
{
	int str_sz = str.size();
	char *str_data = (char *)str.data();

	ASSERT((str_sz + 1) < 256)
	
	
	char buffer[256];
	memcpy(buffer, str_data, str_sz);
	buffer[str_sz] = 0;

	EmplaceDataSectReloc(reg, (int)lang_stat->data_sect.size(), ret);
	//ret->bcodes.emplace_back(byte_code(rel_type::REL_DATA, (char*)nullptr, (int)lang_stat->data_sect.size(), (char)reg));
	InsertIntoDataSect(lang_stat, (void *)str_data, str_sz + 1);
}
void MovFloatToSSEReg(lang_state *lang_stat, char reg, float f, descend_func_ret *ret)
{
	char xmm_r = reg | (1 << 6);

	// this will insert a movss instruction to xmm0
	ret->bcodes.emplace_back(byte_code(rel_type::REL_DATA, (char*)nullptr, (int)lang_stat->data_sect.size(), xmm_r));
	InsertIntoDataSect(lang_stat, (void *)&f, sizeof(float));
}
void GenInstFromLhsAndRhs(lang_state *lang_stat, func_byte_code *final_func, node *n, descend_func_ret &lhs_bcode, descend_func_ret &rhs_bcode, descend_func_ret &ret)
{
	auto inst = TokenToInst(n->t->type);


	MaybePushingPramRegsInDivAndMulInsts(lang_stat, final_func, n->t->type, &ret);

	
	if (lhs_bcode.type != R_SSE_REG && lhs_bcode.type != R_FLOAT)
	{
		// div logic
		switch (inst)
		{
		case byte_code_instr::BC_MODULO:
		case byte_code_instr::BC_DIV:
		{
			switch (lhs_bcode.type)
			{
			case fret_type::R_REG:
			{
				// divisor rcx
				// dividend rax
				// remainder rdx

				// and rdx, 0
				EmplaceInstImmToReg(final_func, 2, 0, byte_code_instr::BC_AND, &ret, 8);
				descend_func_ret rcx;
				rcx.type = fret_type::R_REG;
				rcx.reg = 1;
				rcx.reg_sz = 8;
				// mov rcx, rhs
				ret.bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_EQUAL, &rcx, &rhs_bcode));
				// mov rax, lhs
				EmplaceInstRegToReg(final_func, 0, lhs_bcode.reg, lhs_bcode.reg_sz, byte_code_instr::BC_EQUAL, &ret);

				byte_code bc;
				bc.type = byte_code_enum::DIV_R;
				bc.bin.lhs.reg = rcx.reg;
				bc.bin.lhs.reg_sz = rcx.reg_sz;
				// div rcx 
				ret.bcodes.emplace_back(bc);
				if (inst == byte_code_instr::BC_MODULO)
				{
					lhs_bcode.type = fret_type::R_REG;
					lhs_bcode.reg = 2;
				}
			}break;
			default:
				ASSERT(false)
			}
			MaybePoppingPramRegsInDivAndMulInsts(lang_stat, final_func, n->t->type, &ret);
			return;
		}break;
		}
	}
	if (lhs_bcode.type == R_FLOAT)
	{
		char dst_reg = rhs_bcode.reg == 4 ? 5 : 4;
		MovFloatToSSEReg(lang_stat, dst_reg, lhs_bcode.f, &ret);
		lhs_bcode.type = R_SSE_REG;
		lhs_bcode.reg = dst_reg;

		ret.bcodes.emplace_back(ReturnBCode(inst, &lhs_bcode, &rhs_bcode));

	}
	else if (lhs_bcode.type == fret_type::R_REG && rhs_bcode.type == fret_type::R_STR_LIT)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);

		ret.bcodes.emplace_back(byte_code(rel_type::REL_DATA, (char*)nullptr, (int)lang_stat->data_sect.size(), (char)lhs_bcode.reg));
		InsertIntoDataSect(lang_stat, (void *)rhs_bcode.str.data(), rhs_bcode.str.size());

	}
	else if(lhs_bcode.type == fret_type::R_INT)
	{
		if (rhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);

		descend_func_ret correct_lhs_type;
		// we need to assign later the right side to left side, since the lhs is just an INT
		//memcpy(&correct_lhs_type, &rhs_bcode, sizeof(descend_func_ret));
		//memcpy(&lhs_bcode, &rhs_bcode, sizeof(descend_func_ret));

		switch(rhs_bcode.type)
		{
		case fret_type::R_INT:
		{
			lhs_bcode.i = GetExpressionValT(n->t->type, (int)lhs_bcode.i, (int)rhs_bcode.i);
			lhs_bcode.type = fret_type::R_INT;
			return;
		}break;
		case fret_type::R_REG:
		{
			IREG_TO_REG(rhs_bcode.reg, 1, rhs_bcode.reg_sz, ret, BC_EQUAL)
			rhs_bcode.reg = 1;
		}break;
		case fret_type::R_RMEM:
		case fret_type::R_MEM:
		{
			EmplaceInstMemOffsetToReg2(final_func, 2, rhs_bcode.voffset, rhs_bcode.reg, byte_code_instr::BC_EQUAL, &ret, rhs_bcode.var);
			//EmplaceInstMemToReg(final_func, 1, rhs_bcode.var, rhs_bcode.voffset, byte_code_instr::BC_EQUAL, &ret);
			rhs_bcode.reg = 2;
			rhs_bcode.type = fret_type::R_REG;

			
			
		}break;
		default:
			ASSERT(false)
		}
		EmplaceInstImmToReg(final_func, 0, lhs_bcode.val, byte_code_instr::BC_EQUAL, &ret, rhs_bcode.reg_sz);

		

		lhs_bcode.type = fret_type::R_REG;
		lhs_bcode.reg = 0;
		lhs_bcode.reg_sz = rhs_bcode.reg_sz;
		lhs_bcode.voffset = 0;
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));

	}
	else if(lhs_bcode.type == fret_type::R_REG || lhs_bcode.type == fret_type::R_MEM || lhs_bcode.type == fret_type::R_SSE_REG)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		if (rhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);

		bool was_pushed = false;

		switch(rhs_bcode.type)
		{
		case fret_type::R_MEM:
		{
			bool is_float = rhs_bcode.var->type.IsFloat();
			char reg_dst = is_float ? 7 : 1;
			// moving rhs to rcx
			EmplaceInstMemOffsetToReg2(final_func, reg_dst, rhs_bcode.voffset, rhs_bcode.reg, byte_code_instr::BC_EQUAL, &ret, rhs_bcode.var);
			rhs_bcode.reg = reg_dst;
			rhs_bcode.reg_sz = 8;
			rhs_bcode.type = rhs_bcode.var->type.IsFloat() ? R_SSE_REG : fret_type::R_REG;
		}break;
		case fret_type::R_SSE_REG:
		case fret_type::R_REG:
		{
			if (rhs_bcode.reg != 3)
			{
				bool is_sse = rhs_bcode.type == R_SSE_REG;
				char reg_dst = is_sse ? 7 : 32;
				// transfering reg to rcx
				EmplaceInstRegToReg(final_func, reg_dst, rhs_bcode.reg, 8, byte_code_instr::BC_EQUAL, &ret, is_sse);
				rhs_bcode.reg = reg_dst;
				rhs_bcode.reg_sz = 8;
			}
		}break;
		case fret_type::R_INT:
		{
		}break;
		case fret_type::R_STR_LIT:
		{
			MovStrLitToReg(lang_stat, 0, rhs_bcode.str, &ret);
			rhs_bcode.reg  = 0;
			rhs_bcode.type = fret_type::R_REG;
		}break;
		case fret_type::R_FLOAT:
		{
			char dst_reg = 6;//lhs_bcode.reg == 0 ? 1 : 0;
			MovFloatToSSEReg(lang_stat, dst_reg, rhs_bcode.f, &ret);
			rhs_bcode.type = fret_type::R_SSE_REG;
			rhs_bcode.reg = dst_reg;
			rhs_bcode.reg_sz = 4;
		}break;
		default:
			ASSERT(false)
		}

		//lhs_bcode.type = fret_type::R_MEM;
		//lhs_bcode.var_size = 8;
		//lhs_bcode.reg_sz = 8;
		//EmplaceInstMemOffsetToReg(final_func, 0, lhs_bcode.voffset, byte_code_instr::BC_EQUAL, &ret);

		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));

	}
	else if (lhs_bcode.type == fret_type::R_REG && (rhs_bcode.type == fret_type::R_MEM || rhs_bcode.type == fret_type::R_REG || rhs_bcode.type == fret_type::R_RMEM))
	{
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));
    }
	/*
	else if (lhs_bcode.type == fret_type::R_REG && (rhs_bcode.type == fret_type::R_FLO))
	{
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));
	}
	*/
	else if (lhs_bcode.type == fret_type::R_MEM && rhs_bcode.type == fret_type::R_SSE_REG)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		if (rhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));
	}
	else if ((lhs_bcode.type == fret_type::R_PARAM_REG || lhs_bcode.type == fret_type::R_RMEM || lhs_bcode.type == fret_type::R_SSE_REG || lhs_bcode.type == fret_type::R_MEM)
		&& rhs_bcode.type == fret_type::R_FLOAT)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);

		MovFloatToSSEReg(lang_stat, 4, rhs_bcode.f32, &ret);

		// storing r0 to mem
		descend_func_ret reg;
		reg.type = fret_type::R_SSE_REG;
		reg.reg  = 4;
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &reg));

	}
	else if (lhs_bcode.type == fret_type::R_PARAM_REG && rhs_bcode.type == fret_type::R_STR_LIT)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);

		ret.bcodes.emplace_back(byte_code(rel_type::REL_DATA, (char*)nullptr, (int)lang_stat->data_sect.size(), (char)lhs_bcode.reg));
		InsertIntoDataSect(lang_stat, (void *)rhs_bcode.str.data(), rhs_bcode.str.size());

		// storing r0 to param reg
		descend_func_ret reg;
		CREATE_REG(reg, 0);
		ret.bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_EQUAL, &lhs_bcode, &reg));

	}
	else if (lhs_bcode.type == fret_type::R_MEM && rhs_bcode.type == fret_type::R_STR_LIT)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		//ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));
		// putting str ptr from the data section to r0
		ret.bcodes.emplace_back(byte_code(rel_type::REL_DATA, (char*)nullptr, (int)lang_stat->data_sect.size(), (char)0));

		// storing r0 to mem
		descend_func_ret reg;
		descend_func_ret var;
		CREATE_VAR(var, lhs_bcode.var)
		var.var_size = 8;
		CREATE_REG(reg, 0);
		ret.bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_EQUAL, &var, &reg));

		InsertIntoDataSect(lang_stat, (void *)rhs_bcode.str.data(), rhs_bcode.str.size());

		// storig str size to len, which is at offset 8
		descend_func_ret imm;
		CREATE_VAR(var, lhs_bcode.var)
		var.var_size = 8;
		CREATE_IMM(imm, rhs_bcode.str.size());
		var.voffset += 8;
		ret.bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_EQUAL, &var, &imm));
	}
	else if ((lhs_bcode.type == fret_type::R_MEM || lhs_bcode.type == fret_type::R_REG)
		&& rhs_bcode.type == fret_type::R_INT)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));

	}
	
	else if ((lhs_bcode.type == fret_type::R_RMEM)
		&& rhs_bcode.type == fret_type::R_INT)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));

	}
	else if ((lhs_bcode.type == fret_type::R_RMEM)
		&& rhs_bcode.type == fret_type::R_MEM)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		// moving the mem to reg, and then multiplying it by the var size
		EmplaceInstMemToReg(final_func, 0, rhs_bcode.var, byte_code_instr::BC_EQUAL, &ret);
		rhs_bcode.type = fret_type::R_REG;
		rhs_bcode.val = 0;

		descend_func_ret imm;
		CREATE_IMM(imm, lhs_bcode.lea.size);
		// multipying th  reg by the rmsize size
		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &rhs_bcode, &imm));

		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));

	}
	else if ((lhs_bcode.type == fret_type::R_MEM  || lhs_bcode.type == fret_type::R_RMEM)&& (rhs_bcode.type == fret_type::R_REG || rhs_bcode.type == fret_type::R_RMEM))
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		if (rhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);

		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));
	}
	else if (lhs_bcode.type == fret_type::R_MEM && rhs_bcode.type == fret_type::R_RMEM)
	{
		if (rhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);

		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));
	}
	else if (lhs_bcode.type == fret_type::R_MEM && rhs_bcode.type == fret_type::R_MEM)
	{
		if (lhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, lhs_bcode.bcodes);
		if (rhs_bcode.bcodes.size() > 0)
			INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);
		// transfering the mem to r0
		EmplaceInstMemToReg(final_func, 0, lhs_bcode.var, byte_code_instr::BC_EQUAL, &ret);
		lhs_bcode.type = fret_type::R_REG;
		lhs_bcode.reg = 0;



		ret.bcodes.emplace_back(ReturnBCode(TokenToInst(n->t->type), &lhs_bcode, &rhs_bcode));

	}
	else
	{
		// not handled operation
		ASSERT(false)
	}
	MaybePoppingPramRegsInDivAndMulInsts(lang_stat, final_func, n->t->type, &ret);
}

void CondsAndScopeGenBCode(lang_state *lang_stat, func_byte_code *final_func, node *cond_n, node *scp_n, descend_func_ret *ret, scope *scp, int *cond_bc_sz, node *else_block, int *scope_bc_sz, int inst_to_add_to_scp = 0)
{
	descend_func_ret conds;
	descend_func_ret scope_bc;

	if (cond_n )
	{
		int wasm_begin_block_idx = ret->bcodes.size();
        if(lang_stat->gen_wasm)
        {
			ret->bcodes.emplace_back(byte_code(byte_code_enum::X64_WASM_BEGIN_BLOCK));
			ret->bcodes.emplace_back(byte_code(byte_code_enum::X64_WASM_BEGIN_BLOCK));
        }
		if(cond_n->type != N_STMNT)
			conds = DescendFunc(lang_stat, final_func, cond_n, scp);
		else
			conds = DescendFunc(lang_stat, final_func, cond_n->r, scp);

		if (conds.type == fret_type::R_INT)
		{
			// infinite loop
			if (conds.i == 1)
			{

			}
			else
			{
				//conds.false_bool_bcode_idx.emplace_back((int)conds.bcodes.size());
				//EmplaceCondJmpInst(tkn_type2::T_COND_NE, 1, &conds);
			}

			//conds.false_bool_bcode_idx.emplace_back((int)conds.bcodes.size());
			//EmplaceCondJmpInst(tkn_type2::T_COND_NE, 1, &conds);
		}
		
		else if (conds.type == fret_type::R_MEM)
		{
			descend_func_ret imm;
			CREATE_IMM(imm, 0);
			conds.bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_CMP, &conds, &imm));

			conds.false_bool_bcode_idx.emplace_back((int)conds.bcodes.size());
			EmplaceCondJmpInst(tkn_type2::T_COND_EQ, 0, &conds, false);
		}
		
		else if (conds.type == fret_type::R_BOOL )
		{
			conds.false_bool_bcode_idx.emplace_back((int)conds.bcodes.size());
			EmplaceCondJmpInst(OppositeCondCmp(cond_n->t->type), 0, &conds, cond_n->is_unsigned);
		}
		else if(conds.type == fret_type::R_REG)
		{
			conds.type = fret_type::R_REG;
			descend_func_ret imm;
			CREATE_IMM(imm, 1);
			conds.bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_CMP, &conds, &imm));

			conds.false_bool_bcode_idx.emplace_back((int)conds.bcodes.size());
			EmplaceCondJmpInst(tkn_type2::T_COND_NE, 0, &conds, false);
		}
		else if(conds.type == fret_type::R_COND)
		{

		}
		else
		{
			ASSERT(false)
		}

        if(lang_stat->gen_wasm)
        {
			conds.bcodes.emplace_back(byte_code(byte_code_enum::X64_WASM_END_BLOCK));
			auto wasm_bc = ret->bcodes.begin() + wasm_begin_block_idx + 1;
			wasm_bc->val = conds.bcodes.size();
        }
		if(scp_n)
			scope_bc = DescendFunc(lang_stat, final_func, scp_n, scp);
		
		if(else_block)
			scope_bc.bcodes.emplace_back(byte_code(byte_code_enum::JMP));

        if(lang_stat->gen_wasm)
        {
			scope_bc.bcodes.emplace_back(byte_code(byte_code_enum::X64_WASM_END_BLOCK));
			auto wasm_bc = ret->bcodes.begin() + wasm_begin_block_idx;
			wasm_bc->val = conds.bcodes.size() + scope_bc.bcodes.size() + 1;
			int a = 0;
        }
		// assigning true conds to inside the scope
		FOR_VEC(c, conds.true_bool_bcode_idx)
		{
			auto bc_ptr = &conds.bcodes[*c];
			bc_ptr->jmp_rel = conds.bcodes.size() - (*c + 1) ;
		}
		// assigning true conds to after the scope
		FOR_VEC(c, conds.false_bool_bcode_idx)
		{
			auto bc_ptr = &conds.bcodes[*c];
			int diff_to_end_cond = conds.bcodes.size() - (*c + 1);
			bc_ptr->jmp_rel = diff_to_end_cond + scope_bc.bcodes.size() + inst_to_add_to_scp;
			int t = 0;
		}

		INSERT_VEC(ret->bcodes, conds.bcodes);
		INSERT_VEC(ret->bcodes, scope_bc.bcodes);
	}

	if(scope_bc_sz)
		*scope_bc_sz = scope_bc.bcodes.size();

	if(scope_bc_sz)
		*cond_bc_sz = conds.bcodes.size();
}
void EmlaceInstPopParamReg(func_byte_code *final_func, func_decl *fdecl, char reg, descend_func_ret &ret)
{
	ASSERT((final_func->cur_call_size / 8) > 0)
	final_func->cur_call_size -= 8;
	int stack_offset = final_func->fdecl->saved_regs_offset + final_func->cur_call_size;
	byte_code bc;
	bc.type = POP_REG_PARAM;
	bc.bin.rhs.reg = 5;
	bc.bin.rhs.reg_sz = 8;
	bc.bin.rhs.voffset = stack_offset;
	bc.bin.lhs.reg = reg;
	bc.bin.lhs.reg_sz = 8;

	ret.bcodes.emplace_back(bc);
	// kepping track of how many regs were  already "pushed" during the function call
	// so that, if any other func calls are inside ano the call,
	// we can save the previous reg's call to the stack
	
	
	
}
void EmlaceInstPushParamReg(func_byte_code *final_func, func_decl *fdecl, char reg, descend_func_ret &ret)
{
	int stack_offset = final_func->fdecl->saved_regs_offset + final_func->cur_call_size;

	byte_code bc;
	bc.type = PUSH_REG_PARAM;
	bc.bin.lhs.reg = 5;
	bc.bin.lhs.reg_sz = 8;
	bc.bin.lhs.voffset = stack_offset;
	bc.bin.rhs.reg = reg;
	bc.bin.rhs.reg_sz = 8;

	ret.bcodes.emplace_back(bc);
	// kepping track of how many regs were already "pushed" during the function call
	// so that, if any other func calls are inside ano the call,
	// we can save the previous reg's call to the stack
	final_func->cur_call_size += 8;
	int sz = final_func->cur_call_size / 8;
	ASSERT(fdecl->biggest_saved_regs > sz)
}
char *IdxToArgStr(int idx)
{
	switch(idx)
	{
		case 0:
		return "// arg 0\n";

		case 1:
		return "// arg 1\n";

		case 2:
		return "// arg 2\n";

		case 3:
		return "// arg 3\n";

		case 4:
		return "// arg 4\n";
		
		default:
		return "// arg ..\n";
	}
}
#define GEN_CALL_IS_PTR 1
#define GEN_CALL_IS_PTR_FAR 2

void GenCall(lang_state *lang_stat, func_byte_code *final_func, node *n, scope *scp, func_decl *fdecl, 
	descend_func_ret &ret,
	descend_func_ret &lhs_bcode, 
	descend_func_ret &rhs_bcode,
	int flags = 0,
	void *data = nullptr,
	void *data2 = nullptr
	)
{
}
void EmplaceLeaInst(char reg_dst, char reg_base, int offset, char reg_sz, descend_func_ret *out)
{
	if (reg_dst == reg_base && offset == 0)
		return;
	byte_code lea;
	lea.type = byte_code_enum::INST_LEA;
	lea.bin.lhs.reg = reg_dst;
	lea.bin.rhs.lea.reg_dst = reg_dst;
	lea.bin.rhs.lea.reg_base = reg_base;
	lea.bin.rhs.lea.size = 8;
	lea.bin.rhs.lea.offset = offset;
	out->bcodes.emplace_back(lea);
}

#define DONT_DEREF_PTR 1
#define HAS_PARENT_COND    2

#define DFR_IS_STRUCT_VAL(r) (r.type == fret_type::R_MEM  && r.var && r.var->type.type == enum_type2::TYPE_STRUCT && r.ptr ==0)

decl2 *FromBuiltinTypeToDecl(lang_state *lang_stat, enum_type2 tp)
{
	switch (tp)
	{
	case enum_type2::TYPE_FUNC_PTR:
	{
		return lang_stat->u64_decl;
	}break;
	case enum_type2::TYPE_S64:
		return lang_stat->s64_decl;

	case enum_type2::TYPE_S32:
		return lang_stat->s32_decl;

	case enum_type2::TYPE_S16:
		return lang_stat->s16_decl;

	case enum_type2::TYPE_S8:
		return lang_stat->s8_decl;

	case enum_type2::TYPE_U64:
		return lang_stat->u64_decl;

	case enum_type2::TYPE_ENUM:
	case enum_type2::TYPE_U32:
		return lang_stat->u32_decl;

	case enum_type2::TYPE_U16:
		return lang_stat->u16_decl;

	case enum_type2::TYPE_U8:
		return lang_stat->u8_decl;
	case enum_type2::TYPE_F64:
		return lang_stat->f64_decl;
	case enum_type2::TYPE_F32:
		return lang_stat->f32_decl;

	case enum_type2::TYPE_BOOL:
		return lang_stat->bool_decl;
	case enum_type2::TYPE_CHAR:
		return lang_stat->char_decl;
	case enum_type2::TYPE_VOID:
		return lang_stat->void_decl;


	default:
		ASSERT(false)
	}
}

decl2 *FromTypeToDecl(lang_state *lang_stat, type2 *tp)
{
    decl2 *decl;
	if(tp->IsStrct(&decl))
		return decl;
	else
		return FromBuiltinTypeToDecl(lang_stat, tp->type);

}

void DerefDFR(func_byte_code *final_func, descend_func_ret *ret)
{
	// the reason for the second check is because the ptr is already in the register,
	// if we dereference it until we hit zero, we'll get to its value.
	// so we still want to maintain the address in reg, and to to that
	// we check just until we hit 1
	bool was_on_stack = ret->reg == 5 || ret->reg == 34;
	while((was_on_stack && ret->ptr > 0) || (!was_on_stack && ret->ptr > 1))
	{
		EmplaceInstMemOffsetToReg2(final_func, 0, ret->voffset, ret->reg, byte_code_instr::BC_EQUAL, ret, ret->var);
		ret->reg = 0;
		ret->voffset = 0;
		ret->ptr--;
	}
	ret->ptr = 1;
}
#define DEREF_DFR(ret, side)\
				DerefDFR(final_func, &side);\
				\
				ret.type = fret_type::R_MEM;\
				ret.reg = 0;\
				ret.ptr = 0;\
				ret.reg_sz = 8;\
				ret.voffset = 0;\
				\
				final_voffset = 0;
own_std::string TypeToRelType(type2 *tp)
{
	own_std::string ret = "$$";

	switch(tp->type)
	{
	case enum_type2::TYPE_U64:
	case enum_type2::TYPE_U32:
	case enum_type2::TYPE_U16:
	case enum_type2::TYPE_U8:
	case enum_type2::TYPE_S64:
	case enum_type2::TYPE_S32:
	case enum_type2::TYPE_S16:
	case enum_type2::TYPE_S8:
		ret += "s32";
		break;
	case enum_type2::TYPE_STRUCT:
		ret += tp->strct->name;
		break;
	default:
		ASSERT(false)
	}
	return ret;
}
void MaybePoppingPramRegsInDivAndMulInsts(lang_state *lang_stat, func_byte_code *final_func, tkn_type2 tp, descend_func_ret *ret)
{
	func_decl *fdecl = final_func->fdecl;
	if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_ON_FUNC_CALL))
	{
		EmplaceComment(ret, "// popping param regs");
		// popping param registers
		if (tp == tkn_type2::T_DIV )
			EmlaceInstPopParamReg(final_func, fdecl, 6 + 1, *ret);

		// popping param registers
		if (tp == tkn_type2::T_MUL || tp == tkn_type2::T_DIV || tp == tkn_type2::T_PERCENT)
			EmlaceInstPopParamReg(final_func, fdecl, 6, *ret);
	}
}
void MaybePushingPramRegsInDivAndMulInsts(lang_state *lang_stat, func_byte_code *final_func, tkn_type2 tp, descend_func_ret *ret)
{
	func_decl *fdecl = final_func->fdecl;
	if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_ON_FUNC_CALL))
	{
		EmplaceComment(ret, "// pushing param regs");
		// pushing param registers
		if (tp == tkn_type2::T_MUL || tp == tkn_type2::T_DIV || tp == tkn_type2::T_PERCENT)
			EmlaceInstPushParamReg(final_func, fdecl, 6, *ret);
		// pushing param registers
		if (tp == tkn_type2::T_DIV)
			EmlaceInstPushParamReg(final_func, fdecl, 6 + 1, *ret);
	}
}

void EmplaceComment(descend_func_ret* ret, char* comment)
{
	byte_code comment_bc;
	comment_bc.type = COMMENT;
	comment_bc.name = comment;
	ret->bcodes.emplace_back(comment_bc);
}

void CommonBinOpStuff(func_byte_code *final_func, descend_func_ret *side, descend_func_ret *ret, int reg = 0)
{
	if (side->ptr > 0 && side->reg != 5)
		side->type = fret_type::R_REG;
	else if (side->ptr > 0 && side->reg == 5)
	{
		IMEM_TO_REG(reg, *ret, side->voffset, side->var, 8, 5);
		side->reg  = reg;
		side->type = fret_type::R_REG;
	}
	else if(side->type == fret_type::R_REG && side->voffset != 0)
	{
		EmplaceLeaInst(reg, side->reg, side->voffset, 8, ret);
		side->reg     = reg;
		side->voffset = 0;
	}
}
void PushDFRToMem(lang_state *lang_stat, func_byte_code *final_func, descend_func_ret &ret, descend_func_ret &lhs_bcode, bool &is_sse)
{
	EmplaceComment(&ret, "// pushing lhs");
	// if lhs is rm wee need to only store its value, not the ptr to the value
	
	int offset = final_func->fdecl->saved_lhs_offset + lang_stat->lhs_saved * 8;
	EmplaceInstRegToMemOffset(final_func, lhs_bcode.reg, 8, offset, 8, byte_code_instr::BC_EQUAL, &ret, is_sse);
	//lhs_bcode.type = is_sse ? fret_type::R_REG;
	lhs_bcode.voffset = offset;
	//lhs_bcode.var_size = 8;
	//lhs_bcode.reg_sz = 8;
	lhs_bcode.bcodes.clear();

	lang_stat->lhs_saved++;
	//ASSERT(lang_stat->lhs_saved <=final_func->fdecl->biggest_saved_lhs)
}

void PopDFR(func_byte_code *final_func, bool &is_sse, descend_func_ret &ret, descend_func_ret &lhs_bcode, descend_func_ret &rhs_bcode)
{
	//transfering the rhs reg to rcx, because we want rax to be for the rm
	if (rhs_bcode.reg == lhs_bcode.reg)
	{
		if ((rhs_bcode.type == fret_type::R_REG || rhs_bcode.type == fret_type::R_MEM) && rhs_bcode.reg != 5)
		{
			EmplaceLeaInst(33, rhs_bcode.reg, rhs_bcode.voffset, 8, &ret);
			rhs_bcode.reg = 33;
		}
		else if (rhs_bcode.type == R_SSE_REG)
		{

			EmplaceInstRegToReg(final_func, 1, rhs_bcode.reg, 4, BC_EQUAL, &ret, true);
			rhs_bcode.reg = 1;
		}
	}
	char reg_dst = 0;
	reg_dst = is_sse ? 2 : reg_dst;
	EmplaceComment(&ret, "// popping lhs");
	EmplaceInstMemOffsetToReg(final_func, reg_dst, lhs_bcode.voffset, byte_code_instr::BC_EQUAL, &ret, is_sse, 5);
	//lhs_bcode.type = fret_type::R_REG;
	lhs_bcode.voffset = 0;
	lhs_bcode.reg = reg_dst;
}
void InsertFalseTrueConds(descend_func_ret &dst, descend_func_ret &src)
{
	// was already added to ret
	int sz = dst.bcodes.size();
	FOR_VEC(c, src.true_bool_bcode_idx)
	{
		dst.true_bool_bcode_idx.emplace_back(*c + sz);
	}
	FOR_VEC(c, src.false_bool_bcode_idx)
	{
		dst.false_bool_bcode_idx.emplace_back(*c + sz);
	}
	//INSERT_VEC(ret.bcodes, rhs_bcode.bcodes);
}

// $DescendFunc
descend_func_ret DescendFunc(lang_state *lang_stat, func_byte_code *final_func, node *n, scope *scp, int flags)
{
	descend_func_ret ret;
	
	return ret;
}
own_std::vector<decl2 *> GetScopeDecls(scope *scp)
{
	own_std::vector<decl2 *> ret;
	return ret;
}
void ParametersToStack(func_byte_code *final_func, func_decl *fdecl, descend_func_ret *out)
{
	int cur_arg = 6;
	descend_func_ret sp;
	
	// byte code stack
	if (false)
	{
		FOR_VEC(arg, fdecl->args)
		{
			auto a = *arg;

			descend_func_ret var;
			descend_func_ret reg;
			CREATE_VAR(var, a);
			CREATE_REG(reg, cur_arg);

			if(a->type.IsFloat() && a->type.ptr == 0)
				reg.type = R_SSE_REG;
			else
				reg.type = fret_type::R_PARAM_REG;

			// bool param is if the reg is lhs or not
			out->bcodes.emplace_back(ReturnBCode(byte_code_instr::BC_EQUAL, &var, &reg));
			//EmplaceInstRegToMem(final_func, cur_arg, a, byte_code_instr::BC_EQUAL, out);

			cur_arg++;
			ASSERT(cur_arg < 14)
		}
	}
	// win32 stack
	else
	{
		int offset = 32;

		
		//if(final_func->fdecl->name == "entry")
			//offset -= 8;
			
		for(int i = 3; i >= 0; i--)
		{
			
			decl2* a = nullptr;
			if(i < fdecl->args.size())
				a = fdecl->args[i];

			byte_code bc;
			bc.type = byte_code_enum::STORE_REG_PARAM;
			bc.bin.lhs.reg = 5;
			bc.bin.lhs.voffset = offset;
			bc.bin.lhs.reg_sz = 8;
			bc.bin.lhs.var_size= 8;
			bc.bin.rhs.reg = 6 + i;

			if(a && (a->type.IsFloat() && a->type.ptr == 0))
			{
				bc.bin.rhs.reg = i;
				bc.bin.rhs.reg |= 1 << 4;
			}

			bc.bin.rhs.reg_sz = 8;
			// bool param is if the reg is lhs or not
			out->bcodes.emplace_back(bc);

			// bool param is if the reg is lhs or not
			//out->bcodes.emplace_back(byte_code(STORE_REG_PARAM, false, (char) 6 + i, (char)8, (int)offset, (int)8));
			offset -= 8;
		}
	}
	
}
func_byte_code *GetFunc(lang_state *lang_stat, func_decl *fdecl)
{
	lang_stat->global_funcs.emplace_back(fdecl);

	ASSERT(IS_FLAG_ON(fdecl->flags, FUNC_DECL_IS_DONE))
	auto final_func = new func_byte_code();
	memset(final_func, 0, sizeof(func_byte_code));
	final_func->fdecl = fdecl;
    fdecl->func_bcode = final_func;

	descend_func_ret ret_bcode;
	memset(&ret_bcode, 0, sizeof(descend_func_ret));
	/*
	byte_code bc;
	bc.type  = BEGIN_FUNC;
	bc.fdecl = fdecl;;
	ret_bcode.bcodes.emplace_back(bc);
	*/

	EmplaceComment(&ret_bcode, fdecl->func_node->t->line_str);
	own_std::string fname = own_std::string("// func name ") + fdecl->name;
	EmplaceComment(&ret_bcode, std_str_to_heap(lang_stat, &fname));

	
	
	// moving rsp to rax
	IREG_TO_REG(5, 0, 8, ret_bcode, BC_EQUAL);

	
	
	ParametersToStack(final_func, fdecl, &ret_bcode);

	// making the stack 16 byte aligned
	EmplaceInstImmToReg(final_func, 5, 0xf0, byte_code_instr::BC_AND, &ret_bcode, 1);
	//EmplaceInstImmToReg(final_func, 5, 16, byte_code_instr::BC_SUB, &ret_bcode, 1);
	//

	// subing the stack pointer
	EmplaceInstImmToReg(final_func, 5, fdecl->stack_size, byte_code_instr::BC_SUB, &ret_bcode);
	//EmplacePushStackSize(final_func, fdecl->stack_size, &ret_bcode);

	
	// pushing rbx
	int offset = final_func->fdecl->saved_lhs_offset;
	EmplaceInstRegToMemOffset(final_func, 3, 8, offset, 8, byte_code_instr::BC_EQUAL, &ret_bcode);

	lang_stat->lhs_saved++;
	// pushing rbp
	EmplaceInstRegToMemOffset(final_func, 34, 8, offset + 8, 8, byte_code_instr::BC_EQUAL, &ret_bcode);
	lang_stat->lhs_saved++;

	// moving rax to rbp
	IREG_TO_REG(0, 34, 8, ret_bcode, BC_EQUAL);

	
	descend_func_ret func_bcodes;
	if(fdecl->func_node->r)
		func_bcodes = DescendFunc(lang_stat, final_func, fdecl->func_node->r->r, fdecl->scp);

	lang_stat->lhs_saved--;
	lang_stat->lhs_saved--;

	ret_bcode.bcodes.insert(ret_bcode.bcodes.end(), func_bcodes.bcodes.begin(), func_bcodes.bcodes.end());


	EmplaceRetGroup(final_func, fdecl, nullptr, &ret_bcode);
	ret_bcode.bcodes.emplace_back(byte_code(byte_code_enum::ASSIGN_FUNC_SIZE, (long long)fdecl));

	/*
	bc.type = END_FUNC;
	ret_bcode.bcodes.emplace_back(bc);
	*/

	//ret_bcode.bcodes.insert(ret_bcode.bcodes.end(), ret_bcode.bcodes.begin(), ret_bcode.bcodes.end());
	ASSIGN_VEC(final_func->bcodes, ret_bcode.bcodes);
    fdecl->flags |= FUNC_DECL_CODE_WAS_GENERATED;
	return final_func;
}

	
void GenFuncByteCode(lang_state *lang_stat, func_decl *fdecl, own_std::vector<func_byte_code *> &ret)
{

    fdecl->biggest_saved_lhs = 6;

    bool is_entry = fdecl->name == "entry";

    int shadow_stack_size = 32 + 8;
    // we already account for 4 args anyway in the shadow stack, so we're checking if the 
    // func call has more than 4 args so that we can add the reamining args to the size of the stack
    if (fdecl->biggest_call_args > 4)
        shadow_stack_size += (fdecl->biggest_call_args - 4) * 8;
    

    int dummy_biggest_size = 0;
    // local vars stack size
    int stack_size = SetVariablesAddress(&fdecl->vars, fdecl->args.size(), &dummy_biggest_size, shadow_stack_size);

    stack_size = get_even_address_with(16, stack_size);
    // getting the offset of the saved regs, this is the place where we store call regs
    // that need to be saved after a func call inside another func call
    // 
    // I dont know if we need to add this 8 here
    stack_size += 8;
    fdecl->biggest_saved_regs += 4;
    fdecl->saved_regs_offset = stack_size;
    stack_size += fdecl->biggest_saved_regs * 8;
    
    // skiping the an saved reg
    stack_size += 8;
    fdecl->saved_rsp_offset = stack_size;
    stack_size += 8;

    

    // strct vals per stmnt
    fdecl->per_stmnt_strct_val_offset = stack_size;
    stack_size += fdecl->per_stmnt_strct_val_sz;

    // making space for lhs saved regs
    fdecl->saved_lhs_offset = stack_size;
    fdecl->biggest_saved_lhs += 4;
    stack_size += fdecl->biggest_saved_lhs * 8;

    // making space for strct_vals
    fdecl->strct_vals_offset = stack_size;

    // offset for the struct ret val
    fdecl->strct_val_ret_offset = stack_size + fdecl->strct_val_ret_offset;
    fdecl->strct_vals_sz += 32;
    stack_size += fdecl->strct_vals_sz;

    // offset to call strct vals
    fdecl->call_strcts_val_offset = stack_size;
    fdecl->call_strcts_val_sz += 16;
    stack_size += fdecl->call_strcts_val_sz;


    // offset for the array literal instantiation
    fdecl->array_literal_offset = stack_size;
    stack_size += fdecl->array_literal_sz;

    // making the satck 16 byte aligned
    //if (fdecl->name == "entry" || fdecl->name == "main")
        stack_size = get_even_address_with(16, stack_size);
    //else
    //	stack_size |= 8;

    /*
    if (fdecl->name == "entry" || fdecl->name == "main")
        stack_size += 16 - (stack_size % 16);
    else
        stack_size += 8 - (stack_size % 8);
    */
    fdecl->stack_size = stack_size;

    // we adding more 8 bytes because we want to skip the ret address

    //int arg_start_addr = stack_size + (is_entry ? 0 : 8);
    int arg_start_addr =  8;
    // getting this func parameters offset
    FOR_VEC(arg, fdecl->args)
    {
        auto a = *arg;
        a->flags |= DECL_IS_ARG;
        a->offset = arg_start_addr;

        arg_start_addr += 8;
        
    }
    auto ret_func = GetFunc(lang_stat, fdecl);
    bool is_test = IS_FLAG_ON(fdecl->flags, FUNC_DECL_TEST);
    ret_func->is_test = is_test;
    ret_func->name = fdecl->name.substr();
    ret.emplace_back(ret_func);
}
own_std::vector<func_byte_code *> GetFuncs(lang_state *lang_stat, scope *scp)
{
	own_std::vector<func_byte_code *> ret;
	FOR_VEC(var, scp->vars)
	{
		auto v = *var;
		switch(v->type.type)
		{
		case enum_type2::TYPE_FUNC_EXTERN:
		{

			auto new_f = new func_byte_code();
			memset(new_f, 0, sizeof(func_byte_code));
			new_f->name = v->name.substr();
			new_f->fdecl = v->type.fdecl;

			bool is_link_name = IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_LINK_NAME);
			bool is_outsider = IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_IS_OUTSIDER);
			bool is_test = IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_TEST);
			new_f->is_outsider  = is_link_name | is_outsider;
			new_f->is_link_name = is_link_name;
			

			if(is_link_name)
				new_f->name = new_f->fdecl->link_name;


			ret.emplace_back(new_f);
			continue;
		}break;
		case enum_type2::TYPE_FUNC:
		{
			if(v->type.fdecl->templates.size() == 0)
			{
				if (IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_TEMPLATED)
					|| IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_INTERNAL) ||IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_MACRO) )
				{
					continue;
				}
				else if (IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_IS_OUTSIDER))
				{
					auto new_f = new func_byte_code();
					memset(new_f, 0, sizeof(func_byte_code));
					new_f->name = v->name;
					new_f->fdecl = v->type.fdecl;
					new_f->is_outsider = true;
					ret.emplace_back(new_f);
					continue;
				}
                GenFuncByteCode(lang_stat, v->type.fdecl, ret);

                continue;

				v->type.fdecl->biggest_saved_lhs = 6;

				bool is_entry = v->name == "entry";

				int shadow_stack_size = 32 + 8;
				// we already account for 4 args anyway in the shadow stack, so we're checking if the 
				// func call has more than 4 args so that we can add the reamining args to the size of the stack
				if (v->type.fdecl->biggest_call_args > 4)
					shadow_stack_size += (v->type.fdecl->biggest_call_args - 4) * 8;
				

				int dummy_biggest_size = 0;
				// local vars stack size
				int stack_size = SetVariablesAddress(&v->type.fdecl->vars, v->type.fdecl->args.size(), &dummy_biggest_size, shadow_stack_size);

				stack_size = get_even_address_with(16, stack_size);
				// getting the offset of the saved regs, this is the place where we store call regs
				// that need to be saved after a func call inside another func call
				// 
				// I dont know if we need to add this 8 here
				stack_size += 8;
				v->type.fdecl->biggest_saved_regs += 4;
				v->type.fdecl->saved_regs_offset = stack_size;
				stack_size += v->type.fdecl->biggest_saved_regs * 8;
				
				// skiping the an saved reg
				stack_size += 8;
				v->type.fdecl->saved_rsp_offset = stack_size;
				stack_size += 8;

				

				// strct vals per stmnt
				v->type.fdecl->per_stmnt_strct_val_offset = stack_size;
				stack_size += v->type.fdecl->per_stmnt_strct_val_sz;

				// making space for lhs saved regs
				v->type.fdecl->saved_lhs_offset = stack_size;
				v->type.fdecl->biggest_saved_lhs += 4;
				stack_size += v->type.fdecl->biggest_saved_lhs * 8;

				// making space for strct_vals
				v->type.fdecl->strct_vals_offset = stack_size;

				// offset for the struct ret val
				v->type.fdecl->strct_val_ret_offset = stack_size + v->type.fdecl->strct_val_ret_offset;
				v->type.fdecl->strct_vals_sz += 32;
				stack_size += v->type.fdecl->strct_vals_sz;

				// offset to call strct vals
				v->type.fdecl->call_strcts_val_offset = stack_size;
				v->type.fdecl->call_strcts_val_sz += 16;
				stack_size += v->type.fdecl->call_strcts_val_sz;


				// offset for the array literal instantiation
				v->type.fdecl->array_literal_offset = stack_size;
				stack_size += v->type.fdecl->array_literal_sz;

				// making the satck 16 byte aligned
				//if (v->type.fdecl->name == "entry" || v->type.fdecl->name == "main")
					stack_size = get_even_address_with(16, stack_size);
				//else
				//	stack_size |= 8;

				/*
				if (v->type.fdecl->name == "entry" || v->type.fdecl->name == "main")
					stack_size += 16 - (stack_size % 16);
				else
					stack_size += 8 - (stack_size % 8);
				*/
				v->type.fdecl->stack_size = stack_size;

				// we adding more 8 bytes because we want to skip the ret address

				//int arg_start_addr = stack_size + (is_entry ? 0 : 8);
				int arg_start_addr =  8;
				// getting this func parameters offset
				FOR_VEC(arg, v->type.fdecl->args)
				{
					auto a = *arg;
					a->flags |= DECL_IS_ARG;
					a->offset = arg_start_addr;

					arg_start_addr += 8;
					
				}
				auto ret_func = GetFunc(lang_stat, v->type.fdecl);
				bool is_test = IS_FLAG_ON(v->type.fdecl->flags, FUNC_DECL_TEST);
				ret_func->is_test = is_test;
				ret_func->name = v->name.substr();
				ret.emplace_back(ret_func);
			}
		}break;
		}
	}
	return ret;
}

long long *interpreter::GetRegValPtr(char idx)
{
	long long *r = ((long long *)&regs.r0) + idx;
	return r;
}
long long interpreter::GetRegVal(char idx)
{
	long long* r = (long long *)(((interpreter::reg*) & regs.r0) + idx);
	return *r;
}
void interpreter::NextInst()
{
}

int interpreter::GetCurStackSize()
{
	ASSERT(stack_sizes.size() > 0)
	return *(stack_sizes.end() - 1);
}
long long *interpreter::GetMem(int offset, char reg_idx = 5)
{
	long long reg_val = GetRegVal(reg_idx);
	long long *cur_ptr = (long long *)((reg_val - GetCurStackSize()) + offset);

	return cur_ptr;
}
void interpreter::SetRegFromMem(int offset, char out_reg_idx, char mem_size, char base_reg)
{
	auto mem_ptr = GetMem(offset, base_reg);

	auto out_reg = GetRegValPtr(out_reg_idx);
	ASSERT(mem_size <= 8)
	memcpy(out_reg, mem_ptr, mem_size);
}
void interpreter::SetMemFromReg(char reg_idx, int offset)
{
	auto mem_ptr = GetMem(offset, 5);

	auto reg_val = GetRegVal(reg_idx);
	// getting to the base of the stack and adding the var offset
	

	*mem_ptr = reg_val;
}
#define CREATE_BIN_OP(sgn)\
		if(is_unsigned)\
		{\
			switch(mem_size)\
			{\
			case 1: return ((unsigned char) lhs) sgn ((unsigned char) rhs);\
			case 2: return ((unsigned short) lhs) sgn ((unsigned short) rhs);\
			case 4: return ((unsigned int) lhs) sgn ((unsigned int) rhs);\
			case 8: return ((unsigned long long) lhs) sgn ((unsigned long long) rhs);\
			}\
		}\
		else\
		{\
			switch(mem_size)\
			{\
			case 1: return ((char) lhs) sgn ((char) rhs);\
			case 2: return ((short) lhs) sgn ((short) rhs);\
			case 4: return ((int) lhs) sgn ((int) rhs);\
			case 8: return ((long long) lhs) sgn ((long long) rhs);\
			}\
		}

byte_code_instr bcode_enm_to_bcode_inst(byte_code_enum enm)
{
	switch(enm)
	{
		case byte_code_enum::ADD_I_2_R:
		case byte_code_enum::ADD_I_2_M: 
		case byte_code_enum::ADD_R_2_R:
		case byte_code_enum::ADD_R_2_M:
		case byte_code_enum::ADD_M_2_R:
			return byte_code_instr::BC_PLUS;
		case byte_code_enum::SUB_I_2_R:
		case byte_code_enum::SUB_I_2_M: 
		case byte_code_enum::SUB_R_2_R:
		case byte_code_enum::SUB_R_2_M:
		case byte_code_enum::SUB_M_2_R:
			return byte_code_instr::BC_SUB;
		default: ASSERT(false)
	}
		return byte_code_instr::BC_SUB;
}
long long PerformBinOp(long long lhs, long long rhs, char mem_size, bool is_unsigned, byte_code_instr inst)
{
	
	switch(inst)
	{
	case byte_code_instr::BC_PLUS:
	{
		CREATE_BIN_OP(+)
	}break;
	case byte_code_instr::BC_SUB:
	{
		CREATE_BIN_OP(-)
	}break;
	default:
	{
		ASSERT(false)
	}break;
	}
	return 0;
}
void interpreter::ExecInst()
{

	
}
void *interpreter::GetOutsiderFunc(own_std::string name)
{
	FOR_VEC(f, outsider_funcs)
	{
		if(name == f->name)
			return f->addr;
	}
	return nullptr;
}
void interpreter::CmpSetFlags(long long lhs, long long rhs, char size, bool is_unsigned)
{
	int res = PerformBinOp(lhs, rhs,
		size,
		is_unsigned,
		byte_code_instr::BC_SUB
	);

	regs.flags.i64 &= ~(ZERO_FLAG | SIGN_FLAG);

	if (res == 0)
		regs.flags.i64 |= ZERO_FLAG;
	if (res < 0)
		regs.flags.i64 |= SIGN_FLAG;
}

