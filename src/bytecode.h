#pragma once
#include <string>
#include "machine_rel.h"
struct decl2;
struct scope;


#define DISP_BYTE_MAX 0x80

void AddPreMemInsts(char size, char byte, char greater_byte, char, own_std::vector<unsigned char> &code, bool = false);
decl2* FromTypeToDecl(lang_state *, type2* tp);

#define CREATE_REG(v, reg_val) v.type = fret_type::R_REG; v.reg = reg_val; v.reg_sz = 8;
#define CREATE_RM(v, offset, base_reg) v.type = fret_type::R_RMEM; v.lea.base_reg = base_reg; v.lea.offset = offset;
#define CREATE_VAR(v, decl) v.type = fret_type::R_MEM; v.var = decl; \
v.voffset = decl->offset; v.var_size = GetTypeSize(&decl->type);\
v.reg = 5;

#define CREATE_IMM(v, val) v.type = fret_type::R_INT; v.i = val;


#define ADD_OFFSET_RM_OFFSET_TO_ITSELF(out, bcode)\
			if (bcode.voffset!= 0 && bcode.type == fret_type::R_RMEM)\
			{\
				EmplaceInstImmToReg(final_func, bcode.reg, bcode.voffset, byte_code_instr::BC_PLUS, &ret, 8);\
				bcode.voffset = 0;\
			}


enum byte_code_enum
{
	NOP, 

	NEW_LINE,
	BEGIN_FUNC,
	END_FUNC,
	BEGIN_SCP,
	END_SCP,

	ASSIGN_FUNC_SIZE,
		
	RET, 

	BREAK,
	CONTINUE,
	INT3,

	NOT_R,
	NOT_M,

	NEG_R,
	NEG_M,

	XOR_R,
	XOR_M,
	XOR_M_2_M,
	XOR_R_2_M,
	XOR_M_2_R,
	XOR_I_2_R,
	XOR_I_2_M,
	XOR_I_2_RM,
	XOR_R_2_RM,
	XOR_R_2_R,
	XOR_SSE_2_SSE,
	XOR_MEM_2_SSE,
	XOR_SSE_2_MEM,
	XOR_SSE_2_RMEM,

	X64_WASM_BEGIN_BLOCK,
	X64_WASM_END_BLOCK,

	DIV_R,
	DIV_M,
	DIV_M_2_M,
	DIV_R_2_M,
	DIV_M_2_R,
	DIV_I_2_R,
	DIV_I_2_M,
	DIV_I_2_RM,
	DIV_R_2_RM,
	DIV_R_2_R,
	DIV_SSE_2_SSE,
	DIV_MEM_2_SSE,
	DIV_SSE_2_MEM,
	DIV_SSE_2_RMEM,

	OR_M_2_M,
	OR_R_2_M,
	OR_M_2_R,
	OR_I_2_R,
	OR_I_2_M,
	OR_I_2_RM,
	OR_R_2_RM,
	OR_R_2_R,
	OR_SSE_2_SSE,
	OR_MEM_2_SSE,
	OR_SSE_2_MEM,
	OR_SSE_2_RMEM,


	MUL_M_2_M,
	MUL_R_2_M,
	MUL_M_2_R,
	MUL_I_2_R,
	MUL_I_2_M,
	MUL_I_2_RM,
	MUL_R_2_RM,
	MUL_R_2_R,
	MUL_SSE_2_SSE,
	MUL_MEM_2_SSE,
	MUL_SSE_2_MEM,
	MUL_SSE_2_RMEM,
	MUL_SSE_2_REG,

	AND_M_2_M,
	AND_R_2_M,
	AND_M_2_R,
	AND_I_2_R,
	AND_I_2_M,
	AND_I_2_RM,
	AND_R_2_RM,
	AND_R_2_R,
	AND_SSE_2_SSE,
	AND_MEM_2_SSE,
	AND_SSE_2_MEM,
	AND_SSE_2_RMEM,


	SUB_M_2_M,
	SUB_R_2_M,
	SUB_M_2_R,
	SUB_I_2_R,
	SUB_I_2_M,
	SUB_I_2_RM,
	SUB_R_2_RM,
	SUB_R_2_R,
	SUB_SSE_2_SSE,
	SUB_MEM_2_SSE,
	SUB_SSE_2_MEM,
	SUB_SSE_2_RMEM,

	ADD_M_2_M,
	ADD_R_2_M,
	ADD_M_2_R,
	ADD_I_2_R,
	ADD_I_2_M,
	ADD_I_2_RM,
	ADD_R_2_RM,
	ADD_R_2_R,
	ADD_SSE_2_SSE,
	ADD_MEM_2_SSE,
	ADD_SSE_2_MEM,
	ADD_SSE_2_RMEM,

	STORE_M_2_M,
	STORE_R_2_M,
	STORE_I_2_M,
	STORE_RM_2_M,
	STORE_RM_2_R,
	STORE_RM_2_RM,
	STORE_R_2_RM,
	STORE_I_2_RM,

	CVTSS_MEM_2_SD,
	CVTSD_MEM_2_SS,
	CVTSS_REG_2_SD,
	CVTSD_REG_2_SS,

	PUSH_REG_PARAM,
	POP_REG_PARAM,
	// instruction the store the parameter passed
	STORE_REG_PARAM,
	MOV_F_2_REG_PARAM,
	MOV_SSE_2_REG_PARAM,

	MOV_I_2_REG_PARAM,
	MOV_M_2_REG_PARAM,
	MOV_R_2_REG_PARAM,

	MOV_SSE_2_MEM,
	MOV_SSE_2_RM,
	MOV_SSE_2_SSE,
	MOV_M_2_SSE,
	MOV_R_2_SSE,
	MOV_SSE_2_R,

	MOV_I,
	MOV_ABS,
	MOV_R,
	MOV_RM,
	MOV_M,

	PUSH_STACK_SIZE,
	POP_STACK_SIZE,

	PUSH_R,
	POP_R,

	CMP_I_2_I,
	CMP_I_2_R,
	CMP_R_2_R,
	CMP_M_2_R,

	CMP_I_2_M,
	CMP_R_2_M,
	CMP_M_2_M,
	CMP_I_2_RM,
	CMP_R_2_RM,
	CMP_M_2_RM,
	CMP_RM_2_I,
	CMP_RM_2_R,
	CMP_SSE_2_SSE,
	CMP_MEM_2_SSE,
	CMP_SSE_2_MEM,
	CMP_SSE_2_RMEM,

	RELOC,
	INST_CALL,
	INST_CALL_OUTSIDER,
	INST_CALL_REG,
	INST_RET,
	INST_LEA,

	MOVSX_M,
	MOVSX_R,
	MOVZX_R,
	MOVZX_M,

	REP_B,

	JMP,
	JMP_E,
	JMP_NE,
	//unsigned
	JMP_A,
	JMP_AE,
	JMP_B,
	JMP_BE,
	//signed
	JMP_G,
	JMP_GE,
	JMP_L,
	JMP_LE,

	COMMENT,
};
enum rel_type
{
	REL_FUNC,
	REL_GET_FUNC_ADDR,
	REL_DATA,
	REL_TYPE,
};
struct lea_strct
{
	char reg_dst;
	char reg_base;
	int offset;
	char size;
	char is_unsigned;
};
struct reg_strct
{
	char reg;
	char reg_sz;
	char is_rex;
	reg_strct(char r, char rz, char rex)
	{
	}
};
struct byte_code
{
	byte_code_enum type;
	node* nd;
	struct operand
	{
		union
		{
			unsigned char u8;
			unsigned short u16;
			unsigned int u32;
			unsigned long long u64;
			char s8;
			short s16;
			int s32;
			int i;
			long long s64;
			long long i64;
			float f32;
			reg_strct r;
			int mem;
			//decl2 *var;
			struct
			{
				char _reg;
				char _reg_sz;
			};
			struct
			{
				char reg;
				char reg_sz;
				decl2 *var;
				int voffset;
				int var_size;
				//int reg_base;
				int sib;
			};
			lea_strct lea;
		};
	};

	unsigned int machine_code_idx;
	union
	{
		struct
		{
			operand lhs;
			operand rhs;
		}bin;
		struct
		{
			operand val;
		}un;

		struct
		{
			union
			{
				func_decl *fdecl;
				scope *scp;
			};
			int dst_bc;
		};

		int line;

		int address;
		int jmp_rel;
		long long val;
		char *name;
		void *ptr;
		float f32;
		struct
		{
			char *name;
			bool is_link_name;
			char total_args_given;
		}out_func;
		struct
		{
			rel_type type;
			union
			{
				struct
				{
					char *name;
				
					int offset;
					char reg_dst;
					func_decl *call_func;
				};
			};

		}rel;
	};
	byte_code(rel_type tp, char *name, int offset, char reg_dst, func_decl *fdecl = nullptr)
	{
		memset(this, 0, sizeof(*this));
		this->type = byte_code_enum::RELOC;
		this->rel.type = tp;

		switch(tp)
		{
		case rel_type::REL_TYPE:
		{
			this->rel.name = name;
			this->rel.offset = offset;
			this->rel.reg_dst = reg_dst;
		}break;
		case rel_type::REL_GET_FUNC_ADDR:
		{
			this->rel.reg_dst = reg_dst;
			this->rel.call_func = fdecl;
		}break;
		case rel_type::REL_FUNC:
		{
			this->rel.name = name;
			this->rel.call_func = fdecl;
		}break;
		case rel_type::REL_DATA:
		{
			this->rel.offset = offset;
			this->rel.reg_dst = reg_dst;
		}break;
		default:
			ASSERT(false)
		}
	}
	byte_code(byte_code_enum inst, bool reg_is_lhs, char reg, char reg_sz, int offset, int var_size)
	{
		memset(this, 0, sizeof(*this));
		type = inst;
		if(reg_is_lhs)
		{
			bin.lhs.reg = reg;
			bin.lhs.reg_sz = reg_sz;
			bin.rhs.voffset = offset;
			bin.rhs.var_size = var_size;
		}
		else
		{
			bin.rhs.reg = reg;
			bin.rhs.reg_sz = reg_sz;
			bin.lhs.voffset = offset;
			bin.lhs.var_size = var_size;
		}
	}
	byte_code(byte_code_enum inst, void *data)
	{
		memset(this, 0, sizeof(*this));
		this->type = inst;
		this->ptr = data;
	}
	byte_code(byte_code_enum inst, char reg_dst, char reg_base, int offset, int size, bool is_unsigned)
	{
		memset(this, 0, sizeof(*this));
		this->type = inst;
		this->bin.lhs.reg = reg_dst,
		this->bin.rhs.lea.reg_base = reg_base;
		this->bin.rhs.lea.offset = offset;
		this->bin.rhs.lea.is_unsigned = is_unsigned;
		this->bin.rhs.lea.size = size;
	}
	byte_code(byte_code_enum inst, long long lhs, long long rhs)
	{
		memset(this, 0, sizeof(*this));
		this->type = inst;
		this->bin.lhs.s64 = lhs;
		this->bin.rhs.s64 = rhs;
	}
	byte_code(byte_code_enum inst, long long val)
	{
		memset(this, 0, sizeof(*this));
		this->type = inst;
		this->val = val;
	}
	byte_code(byte_code_enum inst)
	{
		memset(this, 0, sizeof(*this));
		this->type = inst;
	}
	byte_code()
	{
		memset(this, 0, sizeof(*this));
	}
	byte_code operator=(const byte_code &other)
	{
		memcpy(this, &other, sizeof(*this));
		return *this;
	}
	byte_code(const byte_code &other)
	{
		memcpy(this, &other, sizeof(*this));
	}
	~byte_code(){}
};

enum fret_type
{
	R_MEM,
	R_RMEM,
	R_SAVED_RM,
	R_INT,
	R_REG,
	R_FLOAT,
	R_SSE_REG,
	R_PARAM_REG,
	R_BOOL,
	R_STR_LIT,
	R_COND,
};
struct bc_reloc
{
	own_std::string name;
	int inst_idx;
	~bc_reloc()
	{
	memset(this, 0, sizeof(*this));
	}
	bc_reloc operator=(const bc_reloc &other)
	{
		memcpy(this, &other, sizeof(*this));
		return *this;
	}
	bc_reloc(const bc_reloc &other)
	{
		memcpy(this, &other, sizeof(*this));
	}
	bc_reloc(){}
	bc_reloc(own_std::string name, int idx){this->name = name; this->inst_idx = idx;}
};
#define DFR_CALL_STRCT_RET_VAL 1
#define DFR_MEM_ALREADY_PULLED 2
#define DFR_STRCT_VAL_ADDRESS_ALREADY_IN_RAX 4
#define DFR_ARG_CODE_ALREADY_INSERTED 8
struct descend_func_ret
{
	fret_type type;
	union
	{
		struct
		{
			char reg;
			char reg_sz;
			decl2 *var;
			long long voffset;
			int var_size;
			int sib;
			char ptr;
		};
		own_std::string str;
		//decl2 *var;
		union
		{
			struct
			{
				char _reg;
				char _reg_sz;
			};
			reg_strct r;
		};
		int i;
		int s32;
		float f32;
		float f;
		long long val;
		lea_strct lea;
	};
	int aux_buffer_start;
	int aux_buffer_end;
	int flags;
	own_std::vector<byte_code> bcodes;
	own_std::vector<int> true_bool_bcode_idx;
	own_std::vector<int> false_bool_bcode_idx;
	own_std::vector<int> continue_bcode_idx;
	own_std::vector<int> break_bcode_idx;
	
	~descend_func_ret()
	{
		memset(this, 0, sizeof(*this));
	}
	descend_func_ret operator=(const descend_func_ret &other)
	{
		memcpy(this, &other, sizeof(*this));
		return *this;
	}
	descend_func_ret(const descend_func_ret &other)
	{
		memcpy(this, &other, sizeof(*this));
	}
	descend_func_ret(){}
};
struct func_byte_code;


#define SIGN_FLAG 2
#define ZERO_FLAG 1
struct interpreter
{
	struct outsider_func
	{
		const char *name;
		void *addr;
		outsider_func(const char *name, void *addr)
		{
			this->name = name;
			this->addr = addr;
		}
	};
	struct reg
	{
		union
		{

			long long i64;
			int i32;
			short i16;
			char i8;
			void *ptr;
		};
	};
	own_std::vector<outsider_func> outsider_funcs;
	own_std::vector<byte_code> bcode;
	own_std::vector<func_byte_code *> *ar;
	struct
	{
		reg r0;
		reg r1;
		reg r2;
		reg r3;
		reg ip;
		reg sp;
		reg r6;
		reg r7;
		reg r8;
		reg r9;
		reg r10;
		reg r11;
		reg r12;
		reg r13;
		reg r14;
		reg flags;
	}regs;

	char regs_info[16];
	char *mem;
	own_std::vector<int> stack_sizes;
	void NextInst();
	void ExecInst();
	void Init();
	int GetCurStackSize();
	void CmpSetFlags(long long lhs, long long rhs, char size, bool is_unsigned);
	
	func_byte_code *SearchFinalFunc(own_std::string name);
	void *GetOutsiderFunc(own_std::string);

	void SetBCode(own_std::vector<func_byte_code *> *ar);
	long long GetRegVal(char idx);
	long long *GetRegValPtr(char idx);
	void PushVal(long long  val);

	long long *GetMem(int offset, char reg_idx);
	void SetRegFromMem(int offset, char out_reg_idx, char mem_size, char base_reg = 5);
	void SetMemFromReg(char reg_idx, int offset);
};

struct func_byte_code
{
	own_std::string name;
	int start_idx;
	struct
	{
		unsigned char is_outsider  : 1;
		unsigned char is_link_name : 1;
		unsigned char is_test : 1;
	};
	own_std::vector<decl2 *> vars;
	own_std::vector<byte_code> bcodes;
	own_std::vector<byte_code> aux_buffer;

	int biggest_call_size;
	int cur_call_size;

	func_decl *fdecl;

	void FlushAuxBuffer()
	{
		bcodes.insert(bcodes.end(), aux_buffer.begin(), aux_buffer.end());
		aux_buffer.clear();
	}
	own_std::vector<bc_reloc> relocs;
};

struct jmp_rel
{
	char jmp_inst_size;
	unsigned int src_inst;
	byte_code *dst_bc;
	jmp_rel(char sz, unsigned int src_idx, byte_code *bc)
	{
		jmp_inst_size = sz;
		src_inst = src_idx;
		dst_bc = bc;
	}
};
struct call_rel
{
	int call_idx;
	func_decl *fdecl;

	call_rel(int idx, func_decl *f)
	{
		call_idx = idx;
		fdecl    = f;
	}
};
struct machine_code
{
	own_std::vector<unsigned char> code;
	own_std::vector<machine_reloc> rels;
	own_std::vector<machine_sym> symbols;
	own_std::vector<jmp_rel> jmp_rels;
	own_std::vector<call_rel> call_rels;

    int executable;

	int generated_data_symbols;

};
