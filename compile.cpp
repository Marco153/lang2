#include "compile.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <chrono> 
#include <thread>
#include <vector>
#include <unordered_map>

#include "Array.cpp"
#include "rel_utils.h"

#define ANSI_RESET   "\033[0m"
#define ANSI_BLACK     "\033[30m"
#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_WHITE   "\033[37m"

#define ANSI_BG_BLACK   "\033[40m"
#define ANSI_BG_RED     "\033[41m"
#define ANSI_BG_GREEN   "\033[42m"
#define ANSI_BG_YELLOW  "\033[43m"
#define ANSI_BG_BLUE    "\033[44m"
#define ANSI_BG_MAGENTA "\033[45m"
#define ANSI_BG_CYAN    "\033[46m"
#define ANSI_BG_WHITE   "\033[47m"

struct comp_time_type_info
{
	//var arg struct, defined at base.lng
	long long val;
	char ptr;
	void* ptr_to_type_data;

};

struct global_variables_lang
{
	char* main_buffer;
	bool string_use_page_allocator = false;

	void* data;
	void* (*alloc)(void*, int, void*);
	void (*free)(void*, void*, int);


	//LangArray<type_struct2> *structs;
	//LangArray<type_struct2> *template_strcts;

	//page_allocator *cur_page_allocator = nullptr;
	//sub_systems_enum string_sub_system_operations = sub_systems_enum::STRING;

}__lang_globals;

namespace own_std
{
	template<typename T, int method = 0>
	struct vector
	{
		LangArray<T> ar;
		void Init(int len)
		{
			if (__lang_globals.data == nullptr)
				return;

			// 
			T* b = (T*)__lang_globals.alloc(__lang_globals.data, (len + 1) * sizeof(T), (void*)method);
			memset(b, 0, ar.length * sizeof(T));

			ar.start = b;
			ar.end = b;
			ar.count = 0;
			ar.length = len;
		}
		vector(int len)
		{
			memset(this, 0, sizeof(*this));
			Init(len);
		}
		vector()
		{
			memset(this, 0, sizeof(*this));
			Init(1);
		}

		T& operator [](int idx)
		{
			return *ar[idx];
		}

		vector(vector& a)
		{
			memset(this, 0, sizeof(*this));
			this->assign(a.begin(), a.end());
		}
		vector& operator=(vector& a)
		{
			this->assign(a.begin(), a.end());
			return *this;

		}

		void assign(T* start, T* end)
		{
			int count = end - start;

			regrow(count);
			ar.count = count;
			ar.end += count;

			memcpy(ar.start, start, count * sizeof(T));
		}

		T* end()
		{
			return ar.start + ar.count;
		}
		T* begin()
		{
			return ar.start;
		}
		void make_count(int new_size)
		{
			regrow(new_size);
			ar.count = new_size;
		}
		void regrow(int new_size)
		{
			if (new_size < ar.length)
			{
				//TODO: call destructor for stuff
				//ar.count = new_size;
			}
			else
			{
				T* aux = ar.start;
				int before = ar.count;

				int prev_len = ar.length;
				Init(new_size);
				memcpy(ar.start, aux, prev_len * sizeof(T));

				ar.count = before;
				ar.end += before;

				if (aux != nullptr)
					__lang_globals.free(__lang_globals.data, aux, method);
			}
		}
		T& back()
		{
			return *ar[ar.count - 1];
		}
		void clear()
		{
			ar.Clear();
		}
		void pop_back()
		{
			ar.Pop();
		}
		bool empty()
		{
			return ar.count == 0;
		}
		void resize(unsigned int len, T&& val)
		{
			regrow(len);
		}
		unsigned int size()
		{
			return ar.count;
		}

		void reserve(int len)
		{
			Init(len);
		}

		T* data()
		{
			return ar.start;
		}

		void insert(T* at, T* start, T* end)
		{
			int a = at - ar.start;
			ASSERT(a >= 0 && a <= this->ar.count)
				int other_len = end - start;

			if ((ar.count + other_len) >= ar.length)
			{
				regrow(ar.count + other_len);
			}
			ar.count += other_len;
			ar.end += other_len;

			int diff = ar.count - a;
			char* aux_buffer = (char*)__lang_globals.alloc(__lang_globals.data, diff * sizeof(T), (void*)method);
			memcpy(aux_buffer, ar.start + a, sizeof(T) * diff);

			T* t = ar[a];

			memcpy(t + other_len, aux_buffer, sizeof(T) * (diff - other_len));
			memcpy(t, start, sizeof(T) * other_len);

			if (aux_buffer != nullptr)
				__lang_globals.free(__lang_globals.data, aux_buffer, method);
		}

		void insert(T* at, int count, T&& arg)
		{
			int a = at - ar.start;
			ASSERT(a >= 0 && a <= this->ar.count)

				for (int i = 0; i < count; i++)
				{
					T val = arg;
					insert(a, val);
				}
		}
		T* insert(unsigned int a, T& arg)
		{
			ASSERT(a >= 0 && a <= this->ar.count)
				ar.count += 1;
			TestSizeAndRegrow();


			int diff = ar.count - a;
			char* aux_buffer = (char*)__lang_globals.alloc(__lang_globals.data, diff * sizeof(T), (void*)method);
			memcpy(aux_buffer, ar.start + a, sizeof(T) * diff);

			T* t = this->ar.start + a;

			if (diff > 0)
			{
				diff = this->ar.count - a - 1;
				memcpy(t + 1, aux_buffer, sizeof(T) * diff);
				__lang_globals.free(__lang_globals.data, aux_buffer, method);

			}

			memcpy(t, &arg, sizeof(T));

			//ar.count += 1;
			return t;

		}

		void push_back(T& arg)
		{
			emplace_back(arg);
		}
		void TestSizeAndRegrow()
		{
			if ((ar.count) >= ar.length)
			{
				if (ar.length == 0)
				{
					regrow(1);
				}
				regrow(ar.length * 2);
			}
		}
		void emplace_back(T& arg)
		{
			TestSizeAndRegrow();
			ar.Add(&arg);
		}

		void push_back(T&& arg)
		{
			emplace_back(arg);
		}
		void emplace_back(T&& arg)
		{
			TestSizeAndRegrow();
			ar.Add(&arg);
		}
		void free()
		{
			if (ar.start != nullptr)
				__lang_globals.free(__lang_globals.data, ar.start, method);
		}
		~vector()
		{
			if (ar.start != nullptr)
				__lang_globals.free(__lang_globals.data, ar.start, method);
		}
	};
}


/*
#include "udis86.h"
#include "libudis86/types.h"
#include "libudis86/udis86.c"
#include "libudis86/decode.c"
#include "libudis86/syn.c"
//#include "libudis86/syn-att.c"
#include "libudis86/syn-intel.c"
//#include "libudis86/itab.c"
*/

#include "debugger.cpp"
//#include "FileIO.cpp"


#define FOR_LESS(var_name, start_val, cond_val) for(int var_name = start_val; var_name < (cond_val); var_name++)

#ifndef IS_FLAG_ON
#define IS_FLAG_ON(a, flag) (((a) & (flag)) != 0)
#define IS_FLAG_OFF(a, flag) (((a) & (flag)) == 0)
#endif

#define COMPILER

struct scope;
struct node;
struct token2;
struct unit_file;
struct type_struct2;
struct func_decl;
struct decl2;
struct machine_reloc;
struct machine_sym;
struct lang_state;
char* AllocMiscData(lang_state*, int sz);

void split(const std::string& s, char separator, own_std::vector<std::string>& ret, std::string* aux)
{
	int start = 0;
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == separator)
		{
			*aux = s.substr(start, i - start);
			start = i + 1;
			ret.emplace_back(*aux);
		}
	}
	*aux = s.substr(start, s.size() - start);
	ret.emplace_back(*aux);
}
/*
#include "../memory.h"
#include "../sub_system.h"
#include "../LangArray.cpp"
#include "../memory.cpp"
*/
#define MAX_ARGS 32
struct lang_state
{
	int cur_idx;
	int flags;

	bool something_was_declared;
	scope* root;

	node* not_found_nd;
#ifdef DEBUG_NAME
	own_std::vector<node*> not_founds;
#endif
	bool gen_wasm;

	bool ir_in_stmnt;
	func_decl* cur_func;

	int cur_strct_constrct_size_per_statement;

	int cur_spilled_offset;

	unsigned short regs[32];
	unsigned short arg_regs[MAX_ARGS];

	node* cur_stmnt;
	own_std::vector<scope**> scope_stack;
	own_std::vector<func_decl*> func_ptrs_decls;
	own_std::vector<func_decl*> outsider_funcs;
	own_std::vector<own_std::vector<token2>*> allocated_vectors;
	func_decl* plugins_for_func;
	std::unordered_map<func_decl*, func_decl*> comp_time_funcs;
	std::unordered_map<std::string, unsigned int> symbols_offset_on_type_sect;

	std::unordered_map<std::string, char*> internal_funcs_addr;




	own_std::vector<unit_file*> files;

	scope* funcs_scp;

	unit_file* cur_file;

	int parentheses_opened;

	own_std::vector<char> data_sect;
	own_std::vector<char> type_sect;
	own_std::vector<unsigned char, 1> code_sect;

	int already_inserted_of_type_sect_in_code_sect;

	own_std::vector<char> type_sect_str_table;
	own_std::vector<own_std::vector<char>> type_sect_str_table2;


	std::vector<std::string> struct_scope;


	int type_sect_str_tbl_sz;

	decl2* base_lang;


	own_std::vector<machine_reloc> type_sect_rels;

	own_std::vector<machine_sym> type_sect_syms;

	own_std::vector<func_decl*> global_funcs;

	std::string work_dir;

	int call_regs_used;
	int max_bytes_in_ar_lit;
	int lhs_saved;
	int cur_ar_lit_offset;
	int cur_ar_lit_sz;
	int cur_per_stmnt_strct_val_sz;
	int cur_per_stmnt_strct_val_offset;

	std::string linker_options;

	//for_it_desugared fit_dsug;
	char deref_reg;

	decl2* i64_decl;
	decl2* s64_decl;
	decl2* u64_decl;
	decl2* s32_decl;
	decl2* u32_decl;
	decl2* s16_decl;
	decl2* u16_decl;
	decl2* s8_decl;
	decl2* u8_decl;

	decl2* bool_decl;
	decl2* f32_decl;
	decl2* f64_decl;

	decl2* void_decl;
	decl2* char_decl;


	node* node_arena;
	int cur_nd;
	int max_nd;

	char* misc_arena;
	int cur_misc;
	int max_misc;

	char* GetCodeAddr(int offset)
	{
		return (char*)&code_sect[offset + type_sect.size()];
	}

};

struct type_struct2;


char* ReadEntireFileLang(char* name, int* read)
{
	HANDLE file = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	ASSERT(file != INVALID_HANDLE_VALUE);
	LARGE_INTEGER file_size;
	GetFileSizeEx(file, &file_size);
	char* f = (char*)__lang_globals.alloc(__lang_globals.data, file_size.QuadPart + 1, 0);

	int bytes_read;
	ReadFile(file, (void*)f, file_size.QuadPart, (LPDWORD)&bytes_read, 0);
	f[file_size.QuadPart] = 0;

	*read = bytes_read;

	CloseHandle(file);
	return f;
}
void WriteFileLang(char* name, void* data, int size)
{
	HANDLE file = CreateFile(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	int written;
	WriteFile(file, data, size, (LPDWORD)&written, nullptr);

	CloseHandle(file);
}
struct node;
struct scope;

void DescendStmntMode(node* stmnt, scope* scp, int mode, void* data);
// Descend Mode DescendNode
#define DMODE_DNODE 1
#define DMODE_DNAME 2
#define DMODE_DFUNC 3

//#include "../sub_system.cpp"
#define STRING_IMPLEMENTATION
//#include "../tokenizer.cpp"
#include "token.cpp"
#include "node.cpp"
#include "bytecode.cpp"
#include "obj_generator.h"
#include "IR.cpp"

#include "error_report.cpp"

struct macro_call
{
	//string macro_name;
	//string inner;
	int start_ch;
	int end_ch;
};
enum wasm_bc_enum
{
	WASM_INST_F32_CONST,
	WASM_INST_F32_STORE,
	WASM_INST_F32_ADD,
	WASM_INST_F32_LOAD,
	WASM_INST_I32_CONST,
	WASM_INST_I32_LOAD,
	WASM_INST_I32_STORE,
	WASM_INST_I32_STORE8,
	WASM_INST_I32_SUB,
	WASM_INST_I32_ADD,
	WASM_INST_I32_MUL,
	WASM_INST_I32_GE_U,
	WASM_INST_I32_GE_S,
	WASM_INST_BLOCK,
	WASM_INST_END,
	WASM_INST_LOOP,
	WASM_INST_BREAK,
	WASM_INST_BREAK_IF,
	WASM_INST_NOP,
	WASM_INST_CALL,
	WASM_INST_INDIRECT_CALL,
	WASM_INST_RET
};
enum wasm_reg
{
	R0_WASM = 0,
	R1_WASM = 1 * 8,
	R2_WASM = 2 * 8,
	R3_WASM = 3 * 8,
	R4_WASM = 4 * 8,
	R5_WASM = 5 * 8,
	RS_WASM = STACK_PTR_REG * 8,
	RBS_WASM = BASE_STACK_PTR_REG * 8,
};
struct wasm_bc
{
	wasm_bc_enum type;
	bool dbg_brk;
	bool one_time_dbg_brk;
	int start_code;
	int end_code;
	union
	{
		wasm_bc* block_end;
		wasm_reg reg;
		int i;
		float f32;
	};
};
struct wasm_func
{
	int idx;
	own_std::vector<wasm_bc> bcs;
};
// setting and compiling the macros.cpp file

// forward decls
bool AreIRValsEqual(ir_val* lhs, ir_val* rhs);
std::string GetTypeInfoLangArrayStr(std::string str)
{
	std::string final_ret;
	final_ret.reserve(64);

	for (auto c : str)
	{

		char aux[16];
		sprintf_s(aux, 16, "\'\\x%x\', ", (unsigned char)c);
		final_ret.append(aux);
	}
	return final_ret;
}

void MapMacro(node* n, node* marco_inner, node* stmnt, void* out)
{
	int a = 0;
}
void ttt()
{
}
void call(int a, int b, int c, int d, int e, int f, int g)
{
	int gss = 9;
	int dss = 10;
	d = d + e;
	ttt();
	return;
}

char* std_str_to_heap(lang_state*, std::string* str);

void NewTypeToSection(lang_state* lang_stat, char* type_name, enum_type2 idx)
{
	std::string final_name = std::string("$$") + type_name;
	lang_stat->type_sect_syms.push_back(
		machine_sym(lang_stat, SYM_TYPE_DATA, (unsigned int)lang_stat->type_sect.size() + sizeof(type_data), std_str_to_heap(lang_stat, &final_name))
	);

	auto& buffer = lang_stat->type_sect;
	int sz = buffer.size();
	buffer.insert(buffer.begin(), sizeof(type_data), 0);
	type_data* strct_ptr = (type_data*)((char*)buffer.data());
	strct_ptr->name = 0;
	strct_ptr->tp = idx;
}
int CreateDbgScope(std::vector<byte_code>* byte_codes, int start)
{

	int i = 0;
	for (auto bc = byte_codes->begin() + start; bc < byte_codes->end(); bc++)
	{
		i++;
	}
	return i;
}
struct dbg_reloc
{
	int type;
	char* to_fill;
	union
	{
		enum_type2 tp;
		char* strct_name;
	};
};
/*
struct dbg_state
{
	dbg_scp_info *scp_info;
	dbg_func_info *cur_func;

	std::vector<dbg_reloc> rels;
	std::vector<dbg_scp> scps;
	std::vector<dbg_decl> decls;
	std::vector<dbg_type> types;
	std::vector<dbg_func> funcs;


};
*/


/*
void FillDbgScpInfo(dbg_scp_info *info, scope *ref_scp, dbg_state *state)
{
	int decls_sz = ref_scp->vars.size() * sizeof(dbg_decl);
	int name_sz = 0;

	FOR_VEC(d, ref_scp->vars)
	{
		name_sz += (*d)->name.length();
	}

	info->data           = AllocMiscData(lang_stat, decls_sz + name_sz);
	info->decls_offset   = (dbg_decl *)info->data;
	info->str_tbl_offset = info->data + decls_sz;

	//info->total_sz = total_sz;

	int cur_decl = 0;

	// assigning the decls to buffer
	dbg_decl *d_decl = (dbg_decl *)info->decls_offset;
	FOR_VEC(dd, ref_scp->vars)
	{
		auto d = *dd;
		//gettind the dst name addr
		char *name_offset = (char *)(info->str_tbl_offset + info->str_tbl_sz);

		d_decl->name = (char *)(info->str_tbl_offset + info->str_tbl_sz);

		int name_sz = d->name.length();

		memcpy(name_offset, d->name.data(), name_sz);

		// one is because the string is null-terminated
		info->str_tbl_sz += name_sz + 1;

		dbg_reloc rel;

		if(d->type.type == TYPE_STRUCT)
		{
			rel.type = 1;
			rel.strct_name = (char *)d->type.strct->name.data();
		}
		else
		{
			rel.type = 0;
			rel.tp   = d->type.type;
		}

		if(IS_FLAG_ON(d->flags, DECL_IS_ARG))
			d_decl->reg = 5;
		else
			d_decl->reg = 4;

		d_decl->offset = d->offset;


		rel.to_fill = (char *)&d_decl->type;
		state->rels.push_back(rel);

		d_decl++;
		cur_decl++;
	}
}
dbg_file *GetDbgFile(char *name)
{
	return nullptr;
}

int CreateDbg(std::vector<byte_code> *byte_codes, int start, dbg_state *state)
{
	std::vector<dbg_line> lines;

	int i = 0;

	dbg_scp_info *last_scp;

	while(i < byte_codes->size())
	{
		auto bc = &(*byte_codes)[i];

		switch(bc->type)
		{
		case BEGIN_SCP:
		{
			last_scp = state->scp_info;
			auto new_scp = (dbg_scp_info *)AllocMiscData(lang_stat, sizeof(dbg_scp_info));;

			FillDbgScpInfo(new_scp, bc->scp, state);

			state->scp_info = new_scp;

			i += CreateDbg(byte_codes, i + 1, state);
		}break;
		case END_SCP:
		{
			state->scp_info = last_scp;
		}break;
		case BEGIN_FUNC:
		{
			last_scp = state->scp_info;
			auto new_scp  = (dbg_scp_info *)AllocMiscData(lang_stat, sizeof(dbg_scp_info));;
			auto new_func = (dbg_func_info *)AllocMiscData(lang_stat, sizeof(dbg_func_info));


			FillDbgScpInfo(new_scp, bc->fdecl->scp, state);

			state->scp_info = new_scp;
			state->cur_func = new_func;
			i += CreateDbg(byte_codes, i + 1, state);
		}break;
		case END_FUNC:
		{
			state->cur_func = nullptr;
			state->scp_info = last_scp;
		}break;
		case NEW_LINE:
		{
			dbg_line ln;
			ln.code_idx = bc->machine_code_idx;
			ln.ln_num  = bc->line;

			state->cur_func->lines.push_back(ln);
		}break;
		}

		i++;
	}

	return i;
}
*/

int ForeignFuncAdd(char a)
{
	return a + 2;
}

bool GetTypeInTypeSect(lang_state* lang_stat, machine_reloc* c, own_std::vector<machine_sym>& symbols, int data_sect_start, unsigned int& sym_idx)
{
	std::string cur_rel_name = std::string(c->name);
	FOR_VEC(sym, symbols)
	{
		if (cur_rel_name == std::string(sym->name))
		{
			sym_idx = sym->idx;

			if (c->type == machine_rel_type::TYPE_DATA)
				sym_idx = lang_stat->type_sect.size() - sym_idx;
			else
			{
				sym_idx = sym_idx + data_sect_start;
			}
			return true;
		}
	}

	return false;
}


char* CompleteMachineCode(lang_state* lang_stat, machine_code& code)
{
	ResolveJmpInsts(&code);

	int& offset = lang_stat->already_inserted_of_type_sect_in_code_sect;

	lang_stat->code_sect.insert(lang_stat->code_sect.begin(), (unsigned char*)lang_stat->type_sect.begin() + offset, (unsigned char*)lang_stat->type_sect.end());

	offset += lang_stat->type_sect.size() - offset;

	code.executable = lang_stat->code_sect.size() - lang_stat->type_sect.size();

	lang_stat->code_sect.insert(lang_stat->code_sect.end(), code.code.begin(), code.code.end());

	auto base_addr = (char*)lang_stat->code_sect.data() + lang_stat->type_sect.size();

	// resolving calls
	FOR_VEC(c, code.call_rels)
	{
		auto to_fill = (int*)&base_addr[code.executable + c->call_idx];
		long long end_call_inst = (long long)((char*)to_fill) + 4;

		long long dst = (long long)&base_addr[c->fdecl->code_start_idx];
		*to_fill = dst - end_call_inst;
	}
	// resolving calls to internal funcs as if they were dll funcs
	FOR_VEC(c, code.rels)
	{
		char* addr = nullptr;
		auto name = std::string(c->name);
		if (lang_stat->internal_funcs_addr.find(name) != lang_stat->internal_funcs_addr.end())
		{
			addr = (char*)lang_stat->internal_funcs_addr[name];

		}
		if (addr != nullptr)
		{
			auto to_fill = (long long*)&base_addr[code.executable + c->idx];
			long long end_call_inst = (long long)((char*)to_fill) + 4;

			long long dst = (long long)addr;
			*to_fill = dst - end_call_inst;
		}
	}
	//code.symbols.insert(code.symbols.end(), lang_stat->type_sect_syms.begin(), plang_stat->type_sect_syms.end());



	auto data = code.code.data();

	char msg_hdr[256];
	//auto exec_funcs = (char*)VirtualAlloc(0, code.code.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	//memcpy(exec_funcs, code.code.data(), code.code.size());


	int data_sect_start = lang_stat->code_sect.size();
	lang_stat->code_sect.insert(lang_stat->code_sect.end(), (unsigned char*)lang_stat->data_sect.begin(), (unsigned char*)lang_stat->data_sect.end());
	lang_stat->data_sect.clear();

	char* exec_funcs = (char*)lang_stat->code_sect.data() + code.executable + lang_stat->type_sect.size();



	// resolving data rels
	FOR_VEC(c, code.rels)
	{
		if (c->type != machine_rel_type::TYPE_DATA && c->type != machine_rel_type::DATA)
			continue;

		int dst_offset = 0;

		int* to_fill = (int*)&exec_funcs[c->idx];
		long long end_call_inst = (long long)((char*)to_fill) + 4;

		bool found = false;
		unsigned int sym_idx = 0;
		unsigned int sym_idx_aux = 0;
		if (!GetTypeInTypeSect(lang_stat, c, code.symbols, data_sect_start, sym_idx))
		{
			ASSERT(GetTypeInTypeSect(lang_stat, c, lang_stat->type_sect_syms, data_sect_start, sym_idx));
		}

		long long dst = (long long)&lang_stat->code_sect[sym_idx];
		*to_fill = dst - end_call_inst;
	}
	return exec_funcs;
}
void SetNodeIdent(lang_state* lang_stat, node* n, char* data, int len)
{
	n->type = N_IDENTIFIER;
	if (n->t == nullptr)
	{
		n->t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
	}
	n->t->str = std::string(data, len);
	n->flags |= NODE_FLAGS_WAS_MODIFIED;
}
node* GetNodeR(node* n)
{
	return n->r;
}
void* GetProcessedNodeType(comp_time_type_info* tp)
{
	return &tp->val;
}
bool IsNodeType(node* n, node_type tp)
{
	return n->type == tp;
}
node* GetNodeL(node* n)
{
	return n->l;
}
node_type GetNodeType(node* n)
{
	return n->type;
}

void CreateBaseFileCode(lang_state* lang_stat)
{
	auto f = lang_stat->base_lang->type.imp->fl;
	lang_stat->cur_file = f;
	lang_stat->lhs_saved = 0;
	lang_stat->call_regs_used = 0;
	DescendNode(lang_stat, f->s, f->global);

	lang_stat->call_regs_used = 0;
	own_std::vector<func_byte_code*>all_funcs = GetFuncs(lang_stat, f->global);
	machine_code code;
	FromByteCodeToX64(lang_stat, &all_funcs, code);

	auto exec_funcs = CompleteMachineCode(lang_stat, code);
}

struct wasm_interp;
struct dbg_state;
typedef  void(*outsider_func_ptr)(dbg_state*);
struct wasm_interp
{
	dbg_state *dbg;
	own_std::vector<func_decl*> funcs;
	std::unordered_map<std::string, outsider_func_ptr> outsiders;
};
struct web_assembly_state
{
	own_std::vector<decl2*> types;
	own_std::vector<func_decl*> funcs;
	own_std::vector<decl2*> imports;
	own_std::vector<decl2*> exports;

	std::unordered_map<std::string, outsider_func_ptr> outsiders;

	own_std::vector<unsigned char> final_buf;

	lang_state* lang_stat;
};
struct wasm_gen_state
{
	own_std::vector<ir_val*> similar;
	int advance_ptr;
	int strcts_construct_stack_offset;

	func_decl* cur_func;
	web_assembly_state* wasm_state;

	int cur_line;
};
void WasmPushIRVal(wasm_gen_state* gen_state, ir_val* val, own_std::vector<unsigned char>& code_sect, bool = false);

func_decl* WasmGetFuncAtIdx(wasm_interp* state, int idx)
{
	return (state->funcs[idx]);
}
bool FuncAddedWasm(web_assembly_state* state, std::string name, int* idx = nullptr)
{
	int i = 0;
	bool found = false;
	FOR_VEC(decl, state->imports)
	{
		decl2* d = (*decl);
		if (d->type.type == TYPE_FUNC && d->type.fdecl->name == name)
		{
			found = true;
			break;
		}
		i++;
	}
	if (!found)
	{
		i = state->imports.size();
		FOR_VEC(func, state->funcs)
		{
			func_decl* f = *func;
			if (f->name == name)
			{
				found = true;
				break;
			}
			i++;
		}
	}
	if (idx)
		*idx = i;

	return found;;
}
void AddFuncToWasm(web_assembly_state* state, func_decl* func, bool export_this = true)
{
	type2 dummy;
	decl2* decl = FindIdentifier(func->name, state->lang_stat->funcs_scp, &dummy);
	func->this_decl = decl;
	ASSERT(decl)
		state->types.emplace_back(decl);
	state->funcs.emplace_back(func);

	if (export_this)
		state->exports.emplace_back(decl);
}
// https://llvm.org/doxygen/LEB128_8h_source.html
/// Utility function to decode a SLEB128 value.
///
/// If \p error is non-null, it will point to a static error message,
/// if an error occured. It will not be modified on success.
inline int64_t decodeSLEB128(const uint8_t* p, unsigned* n = nullptr,
	const uint8_t* end = nullptr,
	const char** error = nullptr) {
	const uint8_t* orig_p = p;
	int64_t Value = 0;
	unsigned Shift = 0;
	uint8_t Byte;
	do {
		if (p == end) {
			if (error)
				*error = "malformed sleb128, extends past end";
			if (n)
				*n = (unsigned)(p - orig_p);
			return 0;
		}
		Byte = *p;
		uint64_t Slice = Byte & 0x7f;
		if (Shift >= 63 &&
			((Shift == 63 && Slice != 0 && Slice != 0x7f) ||
				(Shift > 63 && Slice != (Value < 0 ? 0x7f : 0x00)))) {
			if (error)
				*error = "sleb128 too big for int64";
			if (n)
				*n = (unsigned)(p - orig_p);
			return 0;
		}
		Value |= Slice << Shift;
		Shift += 7;
		++p;
	} while (Byte >= 128);
	// Sign extend negative numbers if needed.
	if (Shift < 64 && (Byte & 0x40))
		Value |= UINT64_MAX << Shift;
	if (n)
		*n = (unsigned)(p - orig_p);
	return Value;
}

// adapted from https://llvm.org/doxygen/LEB128_8h_source.html
inline unsigned int encodeSLEB128(own_std::vector<unsigned char>* OS, int64_t value,
	unsigned PadTo = 0) {
	bool more;
	unsigned count = 0;
	do {
		uint8_t byte = value & 0x7f;
		// NOTE: this assumes that this signed shift is an arithmetic right shift.
		value >>= 7;
		more = !((((value == 0) && ((byte & 0x40) == 0)) ||
			((value == -1) && ((byte & 0x40) != 0))));
		count++;
		if (more || count < PadTo)
			byte |= 0x80; // Mark this byte to show that more bytes will follow.
		OS->emplace_back(byte);
	} while (more);

	// Pad with 0x80 and emit a terminating byte at the end.
	if (count < PadTo) {
		uint8_t PadValue = value < 0 ? 0x7f : 0x00;
		for (; count < PadTo - 1; ++count)
		{
			OS->emplace_back(char(PadValue | 0x80));
		}
		OS->emplace_back(char(PadValue | 0x80));
		count++;
	}
	return count;
}

int GenUleb128(own_std::vector<unsigned char>* out, unsigned int val)
{
	int count = 0;
	bool new_loop = false;
	do {
		new_loop = false;
		unsigned char byte = val & 0x7f;
		if ((val & 0x40) != 0)
			new_loop = true;
		val >>= 7;

		if (val != 0 || new_loop)
			byte |= 0x80;  // mark this byte to show that more bytes will follow

		out->emplace_back(byte);
		count++;
	} while (val != 0 || new_loop);

	return count;
}

#define WASM_TYPE_INT 0
#define WASM_TYPE_FLOAT 1

#define WASM_LOAD_INT 0
#define WASM_LOAD_FLOAT 1



#define WASM_I32_CONST 0x41

#define WASM_STORE_OP 0x36
#define WASM_STORE_F32_OP 0x38
#define WASM_LOAD_OP  0x28
#define WASM_LOAD_F32_OP  0x2a

void WasmPushImm(int imm, own_std::vector<unsigned char>* out)
{
	own_std::vector<unsigned char> uleb;
	encodeSLEB128(&uleb, imm);
	out->insert(out->end(), uleb.begin(), uleb.end());
}
void WasmPushConst(char type_int_or_float, char size, int offset, own_std::vector<unsigned char>* out)
{
	if (type_int_or_float == WASM_TYPE_INT)
	{
		out->emplace_back(WASM_I32_CONST + size);
		own_std::vector<unsigned char> uleb;
		encodeSLEB128(&uleb, offset);
		out->insert(out->end(), uleb.begin(), uleb.end());
	}
	else if (type_int_or_float == WASM_TYPE_FLOAT)
	{
		out->emplace_back(WASM_I32_CONST + size + 2);
		out->emplace_back(offset >> 24);
		out->emplace_back(offset >> 16);
		out->emplace_back(offset >> 8);
		out->emplace_back(offset & 0xff);

	}
	else
	{
		ASSERT(0);
	}

}
// if it is 64 bit load, make size 1

void WasmPushLoadOrStore(char size, char type, char op_code, int offset, own_std::vector<unsigned char>* out)
{
	WasmPushConst(WASM_TYPE_INT, 0, offset, out);
	if (type == WASM_LOAD_INT)
	{
		out->emplace_back(op_code + size);
	}
	else if (WASM_LOAD_FLOAT)
	{
		out->emplace_back(op_code + 2 + size);
	}
	else
	{
		ASSERT(0);
	}
	if (size == 1)
		// 8-byte aligned
		out->emplace_back(0x3);
	else if (size == 0)
		out->emplace_back(0x2);
	else
		ASSERT(0);

	// offset
	out->emplace_back(0x0);
}
void WasmStoreInst(own_std::vector<unsigned char>& code_sect, int size, char inst = WASM_STORE_OP)
{
	int final_inst = 0;
	switch (inst)
	{
	case WASM_LOAD_F32_OP:
	{
		final_inst = WASM_LOAD_F32_OP;
	}break;
	case WASM_STORE_F32_OP:
	{
		final_inst = WASM_STORE_F32_OP;
	}break;
	case WASM_LOAD_OP:
	{
		switch (size)
		{
		case 1:
			final_inst = 0x2c;
			break;
		case 2:
			final_inst = 0x2e;
			break;
		case 0:
		case 4:
		case 8:
			final_inst = 0x28;
			break;
		default:
			ASSERT(0);
		}
	}break;
	case WASM_STORE_OP:
	{
		switch (size)
		{
		case 1:
			final_inst = 0x3a;
			break;
		case 2:
			final_inst = 0x3b;
			break;
		case 0:
		case 4:
		case 8:
			final_inst = 0x36;
			break;
		default:
			ASSERT(0);
		}
	}break;
	default:
		ASSERT(0);
	}
	code_sect.emplace_back(final_inst);
	char alignment = 0;
	switch (size)
	{
	case 1:
		alignment = 0;
		break;
	default:
		alignment = 2;
	}
	/*
	if (size == 1)
		// 8-byte aligned
		code_sect.emplace_back(0x3);
	else if (size == 0)
		code_sect.emplace_back(0x2);
	else
		ASSERT(0);
		*/
	code_sect.emplace_back(alignment);
	// offset
	code_sect.emplace_back(0x0);
}

void WasmGenBinOpImmToReg(int reg, char reg_sz, int imm, own_std::vector<unsigned char>& code_sect, char inst_32bit)
{
	ASSERT(reg_sz >= 4)
		int dst_reg = reg;

	int size = reg_sz <= 4 ? 0 : 1;

	// pushing reg dst offset
	WasmPushConst(WASM_TYPE_INT, 0, dst_reg * 8, &code_sect);

	WasmPushLoadOrStore(size, WASM_TYPE_INT, WASM_LOAD_OP, dst_reg * 8, &code_sect);
	WasmPushConst(WASM_TYPE_INT, size, imm, &code_sect);
	// if the size is 1 (64 bit), we will get the 64 bit instruction, which
	// is the 32bit one plus 0x12
	// TODO: this a pattern i noticed, but i dont know if is valid for all of them, so i have to check that
	// 
	code_sect.emplace_back(inst_32bit + (size * 0x12));

	WasmStoreInst(code_sect, size);
}
void WasmGenRegToMem(byte_code* bc, own_std::vector<unsigned char>& code_sect)
{
	int src_reg = FromAsmRegToWasmReg(bc->bin.rhs.reg);
	int dst_reg = FromAsmRegToWasmReg(bc->bin.lhs.reg);

	int size = 0;

	// pushing dst reg offset in mem
	WasmPushConst(WASM_TYPE_INT, size, bc->bin.lhs.voffset, &code_sect);

	// getting dst reg val and summing with offset
	WasmPushLoadOrStore(size, WASM_TYPE_INT, WASM_LOAD_OP, dst_reg * 8, &code_sect);
	// 64bit add inst
	code_sect.emplace_back(0x6a);

	// src reg val to mem
	WasmPushLoadOrStore(size, WASM_TYPE_INT, WASM_LOAD_OP, src_reg * 8, &code_sect);

	WasmStoreInst(code_sect, size);
}

static std::string base64_encode(const std::string& in) {

	std::string out;

	int val = 0, valb = -6;
	for (u_char c : in) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4) out.push_back('=');
	return out;
}

static std::string base64_decode(const std::string& in) {

	std::string out;

	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) {
		T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
	}

	int val = 0, valb = -8;
	for (u_char c : in) {
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}

struct block_linked
{
	bool used;
	int bc_idx;
	block_linked* parent;
	union
	{
		byte_code* bc;
		wasm_bc* wbc;

		ir_rep* ir;
		int begin;
	};
};
void FreeBlock(block_linked* block)
{
	__lang_globals.free(__lang_globals.data, (void*)block, 0);
}
block_linked* NewBlock(block_linked* parent)
{
	auto ret = (block_linked*)__lang_globals.alloc(__lang_globals.data, sizeof(block_linked), nullptr);
	memset(ret, 0, sizeof(block_linked));
	ret->parent = parent;
	return ret;

}
void WesmGenMovR(int src_reg, int dst_reg, own_std::vector<unsigned char>& code_sect)
{

	// pushing dst reg offset in mem
	WasmPushConst(WASM_TYPE_INT, 0, dst_reg * 8, &code_sect);
	// loading rhs 
	WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, src_reg * 8, &code_sect);


	WasmStoreInst(code_sect, 0);
}
int GetDepthBreak(block_linked* cur, int cur_bc_idx, int jmp_rel)
{
	int depth_to_break = -1;
	int dst_bc_idx = cur_bc_idx + jmp_rel;

	block_linked* b = cur;
	bool found = false;
	while (b->parent && !found && b->bc)
	{
		// the bc is a block inst, which contains the size of the block
		int min = b->bc_idx;
		int max = b->bc_idx + b->bc->val;

		if (dst_bc_idx >= min && dst_bc_idx <= max)
		{
			found = true;
		}
		depth_to_break++;
		b = b->parent;
	}

	ASSERT(depth_to_break > -1);

	return depth_to_break;
}
void GenConditionalJump(own_std::vector<unsigned char>& code_sect, char inst, int cur_bc_idx, int jmp_rel, block_linked* cur)
{
	int eflags = FromAsmRegToWasmReg(13);
	WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, eflags * 8, &code_sect);
	WasmPushConst(WASM_TYPE_INT, 0, 0, &code_sect);

	// not equal (ne) inst
	code_sect.emplace_back(inst);
	int depth_to_break = GetDepthBreak(cur, cur_bc_idx - 1, jmp_rel);

	// br if
	code_sect.emplace_back(0xd);
	WasmPushImm(depth_to_break, &code_sect);
}

void WasmPushLocalSet(own_std::vector<unsigned char>& code_sect, int idx)
{
	code_sect.emplace_back(0x21);
	WasmPushImm(idx, &code_sect);
}
void WasmPushLocalGet(own_std::vector<unsigned char>& code_sect, int idx)
{
	code_sect.emplace_back(0x20);
	WasmPushImm(idx, &code_sect);
}

void WasmPushWhateverIRValIs(std::unordered_map<decl2*, int>& decl_to_local_idx, ir_val* val, ir_val* last_on_stack, own_std::vector<unsigned char>& code_sect)
{
	switch (val->type)
	{
	case IR_TYPE_REG:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, val->reg * 8, &code_sect);
	}break;
	case IR_TYPE_INT:
	{
		WasmPushConst(WASM_TYPE_INT, 0, val->i, &code_sect);
	}break;
	case IR_TYPE_DECL:
	{
		int idx = decl_to_local_idx[val->decl];

		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->decl->offset, &code_sect);
		code_sect.emplace_back(0x6a);
		WasmStoreInst(code_sect, 0, WASM_LOAD_OP);

		//WasmPushLocalGet(code_sect, idx);
	}break;
	default:
		ASSERT(0)
	}
}
enum wstack_val_type
{
	WSTACK_VAL_INT,
	WSTACK_VAL_F32,
};

void WasmPushInst(tkn_type2 op, bool is_unsigned, own_std::vector<unsigned char>& code_sect, wstack_val_type type = WSTACK_VAL_INT)
{
	if (type == WSTACK_VAL_INT)
	{
		switch (op)
		{
		case T_GREATER_EQ:
		{

			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x4f);
			// signed
			else
				code_sect.emplace_back(0x4e);
		}break;
		case T_MUL:
		{
			code_sect.emplace_back(0x6c);
		}break;
		case T_COND_NE:
		{
			code_sect.emplace_back(0x47);
		}break;
		case T_COND_EQ:
		{
			code_sect.emplace_back(0x46);
		}break;
		case T_POINT:
		case T_PLUS:
		{
			code_sect.emplace_back(0x6a);
		}break;
		default:
			ASSERT(0);
		}
	}
	else
	{
		switch (op)
		{
		case T_PLUS:
		{
			code_sect.emplace_back(0x92);
		}break;
		default:
			ASSERT(0);
		}

	}
}

void WasmPushMultiple(wasm_gen_state* gen_state, bool only_lhs, ir_val& lhs, ir_val& rhs, ir_val* last_on_stack, tkn_type2 op, own_std::vector<unsigned char>& code_sect, bool deref = false)
{
	if (only_lhs)
	{
		WasmPushIRVal(gen_state, &lhs, code_sect, deref);
		//WasmPushWhateverIRValIs(decl_to_local_idx, &lhs, last_on_stack, code_sect);
	}
	else
	{
		wstack_val_type type = WSTACK_VAL_INT;
		if (lhs.is_float)
			type = WSTACK_VAL_F32;

		if (op == T_POINT)
		{
			WasmPushIRVal(gen_state, &lhs, code_sect, false);
			WasmPushConst(WASM_LOAD_INT, 0, rhs.decl->offset, &code_sect);
			WasmPushInst(op, lhs.is_unsigned, code_sect);

			if (deref)
				WasmStoreInst(code_sect, 0, WASM_LOAD_OP);
		}
		else
		{
			WasmPushIRVal(gen_state, &lhs, code_sect, deref);
			WasmPushIRVal(gen_state, &rhs, code_sect, deref);
			WasmPushInst(op, lhs.is_unsigned, code_sect, type);

		}
		//WasmPushWhateverIRValIs(decl_to_local_idx, &lhs, last_on_stack, code_sect);
		//WasmPushWhateverIRValIs(decl_to_local_idx, &rhs, last_on_stack, code_sect);

		bool is_decl = lhs.type == IR_TYPE_DECL;
		bool is_struct = is_decl && lhs.decl->type.IsStrct(nullptr);
		bool is_float = is_decl && lhs.decl->type.type == TYPE_F32;
		ASSERT(lhs.is_unsigned == rhs.is_unsigned || is_struct || is_float);

	}
}

ir_type GetOppositeCondIR(ir_type type)
{
	switch (type)
	{
	case IR_CMP_EQ:
		return IR_CMP_NE;
	case IR_CMP_NE:
		return IR_CMP_EQ;
	default:
		ASSERT(0)
	}
	return IR_CMP_EQ;
}

void WasmBeginStack(own_std::vector<unsigned char>& code_sect, int stack_size)
{
	WesmGenMovR(STACK_PTR_REG, BASE_STACK_PTR_REG, code_sect);
	// sub inst
	WasmGenBinOpImmToReg(STACK_PTR_REG, 4, stack_size, code_sect, 0x6b);
}
void WasmEndStack(own_std::vector<unsigned char>& code_sect, int stack_size)
{
	// sum inst
	WasmGenBinOpImmToReg(STACK_PTR_REG, 4, stack_size, code_sect, 0x6a);
	WesmGenMovR(STACK_PTR_REG, BASE_STACK_PTR_REG, code_sect);
}

bool AreIRValsEqual(ir_val* lhs, ir_val* rhs)
{
	if (lhs->type != rhs->type)
		return false;

	switch (lhs->type)
	{
	case IR_TYPE_ON_STACK:
		return lhs->i == rhs->i;

	case IR_TYPE_DECL:
		return lhs->decl == rhs->decl;
	case IR_TYPE_REG:
	case IR_TYPE_PARAM_REG:
	case IR_TYPE_ARG_REG:
	case IR_TYPE_RET_REG:
		return lhs->reg == rhs->reg;
	default:
		ASSERT(0)
	}

}



void WasmPushIRAddress(wasm_gen_state *gen_state, ir_val *val, own_std::vector<unsigned char> &code_sect)
{
	switch (val->type)
	{
	case IR_TYPE_ON_STACK:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, gen_state->strcts_construct_stack_offset + val->i, &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_REG:
	{
		WasmPushConst(WASM_LOAD_INT, 0, val->reg * 8, &code_sect);
	}break;
	case IR_TYPE_RET_REG:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, RET_1_REG * 8, &code_sect);
	}break;
	case IR_TYPE_ARG_REG:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->reg * 8, &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_PARAM_REG:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->reg * 8, &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_DECL:
	{
		//int idx = decl_to_local_idx[val->decl];
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->decl->offset, &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	default:
		ASSERT(0)
	}
}
void WasmPushIRVal(wasm_gen_state *gen_state, ir_val *val, own_std::vector<unsigned char> &code_sect, bool deref)
{
	switch (val->type)
	{
	case IR_TYPE_F32:
	{
		WasmPushConst(WASM_LOAD_FLOAT, 0, *(int *) & val->f32, &code_sect);
		//code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_INT:
	{
		WasmPushConst(WASM_LOAD_INT, 0, val->i, &code_sect);
		//code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_ON_STACK:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
		int base_offset = gen_state->strcts_construct_stack_offset;
		int offset = val->i;
		WasmPushConst(WASM_LOAD_INT, 0, -(base_offset + offset), &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_REG:
	{
		//WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, val->reg * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->reg * 8, &code_sect);
	}break;
	case IR_TYPE_RET_REG:
	{
		WasmPushConst(WASM_LOAD_INT, 0, (RET_1_REG + val->reg) * 8, &code_sect);
	}break;
	case IR_TYPE_ARG_REG:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->reg * 8, &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_PARAM_REG:
	{
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
		WasmPushConst(WASM_LOAD_INT, 0, val->reg * 8, &code_sect);
		code_sect.emplace_back(0x6a);
	}break;
	case IR_TYPE_DECL:
	{
		if (val->decl->type.type == TYPE_FUNC)
		{
			int idx = 0;
			ASSERT(FuncAddedWasm(gen_state->wasm_state, val->decl->name, &idx));
			WasmPushConst(WASM_LOAD_INT, 0, idx, &code_sect);

		}
		else
		{
			//int idx = decl_to_local_idx[val->decl];
			WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
			WasmPushConst(WASM_LOAD_INT, 0, val->decl->offset, &code_sect);
			code_sect.emplace_back(0x6a);
		}
		// address of, so we dont want te load whats inside whe variable
		if (val->ptr == -1)
			deref = false;
	}break;
	default:
		ASSERT(0)
	}
	int ptr = val->ptr;
	while (((ptr > 0 || deref) && val->type != IR_TYPE_INT && val->type != IR_TYPE_F32) && ptr > -1)
	{

		int inst = WASM_LOAD_OP;
		if (IsIrValFloat(val))
			inst = WASM_LOAD_F32_OP;
		WasmStoreInst(code_sect, 0, inst);
		if(!deref)
			ptr--;
		deref = false;
	}
}

void WasmPopToRegister(wasm_gen_state* state, int reg_dst, own_std::vector<unsigned char>& code_sect)
{
	WasmPushConst(WASM_TYPE_INT, 0, BASE_STACK_PTR_REG * 8, &code_sect);
	WasmPushConst(WASM_TYPE_INT, 0, STACK_PTR_REG * 8, &code_sect);
	WasmStoreInst(code_sect, 0, WASM_LOAD_OP);
	WasmStoreInst(code_sect, 0, WASM_LOAD_OP);
	WasmStoreInst(code_sect, 0, WASM_STORE_OP);
	WasmGenBinOpImmToReg(STACK_PTR_REG, 4, 8, code_sect, 0x6a);
}
void WasmPushRegister(wasm_gen_state* state, int reg, own_std::vector<unsigned char>& code_sect)
{
	WasmGenBinOpImmToReg(STACK_PTR_REG, 4, 8, code_sect, 0x6b);

	WasmPushConst(WASM_TYPE_INT, 0, STACK_PTR_REG * 8, &code_sect);
	WasmStoreInst(code_sect, 0, WASM_LOAD_OP);
	WasmPushConst(WASM_TYPE_INT, 0, BASE_STACK_PTR_REG * 8, &code_sect);
	WasmStoreInst(code_sect, 0, WASM_LOAD_OP);
	WasmStoreInst(code_sect, 0, WASM_STORE_OP);

}
void WasmFromSingleIR(std::unordered_map<decl2*, int> &decl_to_local_idx, 
					  int &total_of_args, int &total_of_locals, 
				      ir_rep *cur_ir, own_std::vector<unsigned char> &code_sect, 
					  int *stack_size, own_std::vector<ir_rep> *irs, block_linked **cur,
					  ir_val *last_on_stack, wasm_gen_state *gen_state)
{
	cur_ir->idx = cur_ir - irs->begin();
	gen_state->advance_ptr = 0;
	cur_ir->start = code_sect.size();
	switch (cur_ir->type)
	{
	case IR_NOP:
	{
		code_sect.emplace_back(0x1);
	}break;
	case IR_RET:
	{
        WasmPushMultiple(gen_state, cur_ir->assign.only_lhs, cur_ir->assign.lhs, cur_ir->assign.rhs,  last_on_stack, cur_ir->assign.op, code_sect);
		
		WasmEndStack(code_sect, *stack_size);
		// adding stack_ptr
		//WasmGenBinOpImmToReg(BASE_STACK_PTR_REG, 4, *stack_size, code_sect, 0x6a);

        code_sect.emplace_back(0xf);
	}break;
	case IR_BREAK_IF:
    {
		int depth = 0;

		block_linked *aux = *cur;
		while (aux->parent)
		{
			if (aux->ir->type == IR_BEGIN_IF_BLOCK)
                break;
			depth++;
			aux = aux->parent;
		}
		code_sect.emplace_back(0xc);
		WasmPushImm(depth, &code_sect);
    }break;
	case IR_BEGIN_STMNT:
	{
		cur_ir->block.stmnt.code_start = code_sect.size();
		gen_state->cur_line = cur_ir->block.stmnt.line;

	}break;
	case IR_END_STMNT:
	{
		stmnt_dbg st;
		ir_rep* begin = irs->begin() + cur_ir->block.other_idx;
		ASSERT(begin->type == IR_BEGIN_STMNT);
		st.start = begin->block.stmnt.code_start;
		st.end = code_sect.size() - 1;
		st.line = begin->block.stmnt.line;

		//if(st.start != st.end)
		gen_state->cur_func->wasm_stmnts.emplace_back(st);

	}break;
	case IR_BEGIN_LOOP_BLOCK:
	{
		code_sect.emplace_back(0x3);
		// some blocks have to return void?
		code_sect.emplace_back(0x40);

		*cur = NewBlock(*cur);
		(*cur)->ir = cur_ir;
	}break;
	case IR_END_LOOP_BLOCK:
	{
		code_sect.emplace_back(0xc);
		// some blocks have to return void?
		code_sect.emplace_back(0x0);
		code_sect.emplace_back(0xb);
		FreeBlock(*cur);
		*cur = (*cur)->parent;
	}break;
	case IR_BEGIN_SUB_IF_BLOCK:
	case IR_BEGIN_AND_BLOCK:
	case IR_BEGIN_OR_BLOCK:
	case IR_BEGIN_COND_BLOCK:
	case IR_BEGIN_IF_BLOCK:
	{
		code_sect.emplace_back(0x2);
		// some blocks have to return void?
		code_sect.emplace_back(0x40);

		*cur = NewBlock(*cur);
		(*cur)->ir = cur_ir;
	}break;
	case IR_END_AND_BLOCK:
	case IR_END_OR_BLOCK:
	case IR_END_COND_BLOCK:
	case IR_END_SUB_IF_BLOCK:
	case IR_END_IF_BLOCK:
	{
		FreeBlock(*cur);
		*cur = (*cur)->parent;
		code_sect.emplace_back(0xb);
	}break;
	case IR_CMP_NE:
	case IR_CMP_GE:
	case IR_CMP_EQ:
	{
		ir_rep* next = cur_ir + 1;
		bool val = cur_ir->bin.it_is_jmp_if_true;

		if (cur_ir->bin.it_is_jmp_if_true)
		{
			if (next->type == IR_END_COND_BLOCK || next->type == IR_END_AND_BLOCK)
			{
				val = false;
			}
		}
		else
		{
			if (next->type == IR_END_OR_BLOCK)
			{
				val = true;
			}
		}

		if (cur_ir->bin.it_is_jmp_if_true != val)
			cur_ir->bin.op = OppositeCondCmp(cur_ir->bin.op);


		int depth = 0;

		block_linked* aux = *cur;
		while (aux->parent)
		{
			if (!val && (aux->ir->type == IR_BEGIN_OR_BLOCK || aux->ir->type == IR_BEGIN_SUB_IF_BLOCK || aux->ir->type == IR_BEGIN_IF_BLOCK))
				break;
			if (val && (aux->ir->type == IR_BEGIN_AND_BLOCK || aux->ir->type == IR_BEGIN_COND_BLOCK))
				break;
			depth++;
			aux = aux->parent;
		}

		WasmPushMultiple(gen_state, cur_ir->bin.only_lhs, cur_ir->bin.lhs, cur_ir->bin.rhs, last_on_stack, cur_ir->bin.op, code_sect, true);
		//WasmPushConst(WASM_TYPE_INT, 0, 7, &code_sect);
		// br if
		code_sect.emplace_back(0xd);
		WasmPushImm(depth, &code_sect);

	}break;
	case IR_ASSIGNMENT:
	{

		gen_state->similar.clear();
		ir_val* lhs = &cur_ir->assign.lhs;;
		ir_val* rhs = &cur_ir->assign.rhs;;

		gen_state->similar.emplace_back(lhs);
		gen_state->similar.emplace_back(rhs);

		ir_rep* next = cur_ir + 1;

		/*
		while(cur_ir->type == next->type && AreIRValsEqual(&cur_ir->assign.to_assign, &next->assign.to_assign)
			&& cur_ir->assign.to_assign.type != IR_TYPE_PARAM_REG)
		{
			if(!AreIRValsEqual(&cur_ir->assign.to_assign, &next->assign.lhs))
				gen_state->similar.emplace_back(&next->assign.lhs);

			gen_state->similar.emplace_back(&next->assign.rhs);
			next++;
			gen_state->advance_ptr++;
		}
		*/


		WasmPushIRVal(gen_state, &cur_ir->assign.to_assign, code_sect);

		WasmPushMultiple(gen_state, cur_ir->assign.only_lhs, *lhs, *rhs, last_on_stack, cur_ir->assign.op, code_sect, true);

		ASSERT(cur_ir->assign.to_assign.is_float == cur_ir->assign.lhs.is_float);
		/*
		for(int i = 2; i < gen_state->similar.size();i++)
		{
			ir_val * r = gen_state->similar[i];
			WasmPushWhateverIRValIs(decl_to_local_idx, r, last_on_stack, code_sect);
			WasmPushInst(cur_ir->assign.op, lhs->is_unsigned, code_sect);
		}
		*/

		int r_sz = cur_ir->assign.to_assign.reg_sz;
		ASSERT(r_sz > 0 && r_sz <= 8);

		switch (cur_ir->assign.to_assign.type)
		{
		case IR_TYPE_REG:
		case IR_TYPE_ON_STACK:
		case IR_TYPE_PARAM_REG:
		case IR_TYPE_ARG_REG:
		case IR_TYPE_RET_REG:
		case IR_TYPE_DECL:
		{
			int inst = WASM_STORE_OP;
			if (IsIrValFloat(&cur_ir->assign.to_assign))
				inst = WASM_STORE_F32_OP;
			WasmStoreInst(code_sect, r_sz, inst);
		}break;
		default:
			ASSERT(0)
		}



		*last_on_stack = cur_ir->assign.to_assign;
		//WasmPushConst(WASM_TYPE_INT, 0, decl_to_local_idx[]);
	}break;
	case IR_END_CALL:
	{
        code_sect.emplace_back(0x10);
        WasmPushImm(gen_state->wasm_state->imports.size() + cur_ir->call.fdecl->wasm_func_sect_idx, &code_sect);
		// sum inst
		WasmGenBinOpImmToReg(STACK_PTR_REG, 4, cur_ir->call.fdecl->args.size() * 8, code_sect, 0x6a);
		WasmPopToRegister(gen_state, BASE_STACK_PTR_REG, code_sect);

	}break;
	case IR_CALL:
	{
		int idx = 0;
		ASSERT(FuncAddedWasm(gen_state->wasm_state, cur_ir->call.fdecl->name, &idx));
		WasmPushRegister(gen_state, BASE_STACK_PTR_REG, code_sect);

        code_sect.emplace_back(0x10);
        WasmPushImm(idx, &code_sect);

		WasmPopToRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
	}break;
	case IR_BEGIN_CALL:
	{
		ASSERT(FuncAddedWasm(gen_state->wasm_state, cur_ir->call.fdecl->name));
		// sub inst
		WasmPushRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
		WasmGenBinOpImmToReg(STACK_PTR_REG, 4, cur_ir->call.fdecl->args.size() * 8, code_sect, 0x6b);

	}break;
	case IR_INDIRECT_CALL:
	{
		int idx = 0;
		ASSERT(FuncAddedWasm(gen_state->wasm_state, cur_ir->call.fdecl->name, &idx));
		WasmPushConst(WASM_TYPE_INT, 0, idx, &code_sect);
		code_sect.emplace_back(0x11);
		code_sect.emplace_back(0x0);
		code_sect.emplace_back(0x0);
	}break;
	case IR_STACK_END:
	{
		//*stack_size = cur_ir->num;
		// sub inst

		WasmEndStack(code_sect, *stack_size);
		code_sect.emplace_back(0xf);
	}break;
	case IR_STACK_BEGIN:
	{
		//*stack_size = cur_ir->num;
		// sub inst
		gen_state->strcts_construct_stack_offset = *stack_size;
		cur_ir->fdecl->strct_constrct_at_offset = *stack_size;
		*stack_size += cur_ir->fdecl->strct_constrct_size_per_statement;
		*stack_size += cur_ir->fdecl->biggest_call_args * 8;

		// 8 bytes for saving rbs
		//*stack_size += 8;

		WasmBeginStack(code_sect, *stack_size);
	}break;
	case IR_DECLARE_LOCAL:
	{
		decl_to_local_idx[cur_ir->decl] = total_of_args + total_of_locals;
		total_of_locals++;
		int to_sum = GetTypeSize(&cur_ir->decl->type);
		*stack_size += to_sum <= 4 ? 4 : to_sum;
		cur_ir->decl->offset = -*stack_size;
		
        //gen_state->cur_func->wasm_scp->vars.emplace_back(cur_ir->decl);

	}break;
	case IR_DECLARE_ARG:
	{
		decl_to_local_idx[cur_ir->decl] = total_of_args;
	
		int offset_to_args = 8;
		cur_ir->decl->offset = offset_to_args + (total_of_args * 8);
		total_of_args++;

		//gen_state->cur_func->wasm_all_vars.emplace_back(cur_ir->decl);
        //gen_state->cur_func->wasm_scp->vars.emplace_back(cur_ir->decl);
		/*
		cur_ir->decl->offset = *stack_size;
		int to_sum = GetTypeSize(&cur_ir->decl->type);
		*stack_size += to_sum <= 4 ? 4 : to_sum;
		*/
	}break;

	case IR_BEGIN_COMPLEX:
	{
		WasmPushIRAddress(gen_state, &cur_ir->complx.dst, code_sect);

		ir_rep* original_ir = cur_ir;

		cur_ir++;
		
		// only trying to push on the stack instead of doing assignments
		while(cur_ir->type != IR_END_COMPLEX)
		{
			// we're not handling complex blocks inside another
			ASSERT(cur_ir->type != IR_BEGIN_COMPLEX);

			switch (cur_ir->type)
			{
			case IR_ASSIGNMENT:
			{
				if (AreIRValsEqual(&cur_ir->assign.to_assign, &original_ir->assign.to_assign))
					break;
				bool to_assign_reg = cur_ir->assign.to_assign.type == IR_TYPE_REG;
				bool lhs_reg = cur_ir->assign.lhs.type == IR_TYPE_REG;
				bool rhs_reg = cur_ir->assign.rhs.type == IR_TYPE_REG;
				bool only_lhs = cur_ir->assign.only_lhs;

				char val = (char)to_assign_reg | (((char)lhs_reg) << 1) | (((char)rhs_reg) << 2);

				ir_val* lhs = &cur_ir->assign.lhs;;
				ir_val* rhs = &cur_ir->assign.rhs;;

				switch (val)
				{
				// everything is reg, meaning they're the stack
				case 7:
				{
					WasmPushInst(cur_ir->assign.op, lhs->is_unsigned, code_sect);
					// make sure that the lhs reg is the same as the reg to assign
					//ASSERT(AreIRValsEqual(&cur_ir->assign.to_assign, &cur_ir->assign.lhs));
				}break;
				// only assign is reg 
				case 5:
				{
					WasmPushMultiple(gen_state, cur_ir->assign.only_lhs, *lhs, *rhs, last_on_stack, cur_ir->assign.op, code_sect);
				}break;
				case 1:
				{
					WasmPushMultiple(gen_state, cur_ir->assign.only_lhs, *lhs, *rhs, last_on_stack, cur_ir->assign.op, code_sect);
				}break;
				default:
					ASSERT(0);
				}

			}break;
			default:
				ASSERT(0);
			}
			gen_state->advance_ptr++;
			cur_ir++;
		}
		ASSERT(cur_ir->type == IR_END_COMPLEX);


		WasmStoreInst(code_sect, 0, WASM_STORE_OP);
		gen_state->advance_ptr++;

	}break;
	default:
		ASSERT(0);
	}
	cur_ir->end = code_sect.size();
}



struct wasm_stack_val
{
	union
	{
		float f32;
		unsigned int u32;
		int s32;
		unsigned long long u64;
		long long s64;
		wasm_reg reg;
	};
	wstack_val_type type;
};
scope *FindScpWithLine(func_decl* func, int line)
{
	scope* cur_scp = func->scp;
	bool can_break = false;
	while (line >= cur_scp->line_start && line <= cur_scp->line_end && !can_break)
	{
		can_break = true;
		FOR_VEC(scp, cur_scp->children)
		{
			scope* s = *scp;
			if (line >= s->line_start && line <= s->line_end)
			{
				cur_scp = s;
				can_break = false;
				break;
			}
		}
	}
	return cur_scp;
}

bool FindVarWithOffset(func_decl *func, int offset, decl2 **out, int line)
{
	scope* cur_scp = FindScpWithLine(func, line);

	FOR_VEC(d, cur_scp->vars)
	{
		if ((*d)->offset == offset)
		{
			*out = *d;
			return true;
		}
	}

	return false;
}

enum dbg_break_type
{
	DBG_NO_BREAK,   
	DBG_BREAK_ON_DIFF_STAT,
	DBG_BREAK_ON_NEXT_STAT,
	DBG_BREAK_ON_NEXT_BC,
	DBG_BREAK_ON_NEXT_IR   
};
enum dbg_expr_type
{
	DBG_EXPR_SHOW_SINGLE_VAL,
	DBG_EXPR_SHOW_VAL_X_TIMES,
};
enum dbg_print_numbers_format
{
	DBG_PRINT_DECIMAL,
	DBG_PRINT_HEX,
};
struct dbg_expr
{
	dbg_expr_type type;
	own_std::vector<ast_rep *> expr;
	own_std::vector<ast_rep *> x_times;

	func_decl* from_func;
	std::string exp_str;
};
struct dbg_state
{
	dbg_break_type break_type;
	dbg_print_numbers_format print_numbers_format;
	wasm_bc **cur_bc;
	ir_rep *cur_ir;
	char* mem_buffer;
	int mem_size;
	func_decl* cur_func;
	stmnt_dbg* cur_st;
	own_std::vector<func_decl*> func_stack;
	own_std::vector<block_linked *> block_stack;
	own_std::vector<wasm_bc*> return_stack;
	own_std::vector<wasm_bc> bcs;
	own_std::vector<wasm_stack_val> wasm_stack;
	own_std::vector<dbg_expr *> exprs;

	lang_state* lang_stat;
	wasm_interp* wasm_state;


	bool some_bc_modified;
};
void WasmModifyCurBcPtr(dbg_state* dbg, wasm_bc* to)
{
	dbg->some_bc_modified = true;
	(*dbg->cur_bc) = to;
}

std::string WasmNumToString(dbg_state* dbg, int num, char limit = -1)
{
	std::string ret = "";
	char buffer[32];
	switch (dbg->print_numbers_format)
	{
	case DBG_PRINT_DECIMAL:
	{
		if(limit == -1)
			snprintf(buffer, 32, "%d", num);
		else 
			snprintf(buffer, 32, "%0*d", limit, num);

	}break;
	case DBG_PRINT_HEX:
	{
		if(limit == -1)
			snprintf(buffer, 32, "0x%x", num);
		else 
			snprintf(buffer, 32, "0x%0*x", limit, num);
		
	}break;
	default:
		ASSERT(0);
	}
	ret = buffer;
	return ret;
}

std::string WasmGetBCString(dbg_state *dbg, func_decl* func, wasm_bc *bc, own_std::vector<wasm_bc>* bcs)
{
	char buffer[64];
	std::string ret = "       ";
	switch (bc->type)
	{
	case WASM_INST_I32_GE_U:
	{
		ret += "i32.ge_u";
	}break;
	case WASM_INST_I32_GE_S:
	{
		ret += "i32.ge_s";
	}break;
	case WASM_INST_RET:
	{
		ret += "ret";
	}break;
	case WASM_INST_LOOP:
	{
		ret += "loop";
	}break;
	case WASM_INST_I32_STORE8:
	{
		ret += "i32.store8";
	}break;
	case WASM_INST_F32_STORE:
	{
		ret += "f32.store";
	}break;
	case WASM_INST_I32_STORE:
	{
		ret += "i32.store";
	}break;
	case WASM_INST_F32_LOAD:
	{
		ret += "f32.load";
	}break;
	case WASM_INST_I32_LOAD:
	{
		ret += "i32.load";
	}break;
	case WASM_INST_END:
	{
		ret += "end";
	}break;
	case WASM_INST_I32_SUB:
	{
		ret += "i32.sub";
	}break;
	case WASM_INST_F32_ADD:
	{
		ret += "f32.add";
	}break;
	case WASM_INST_I32_ADD:
	{
		ret += "i32.add";
	}break;
	case WASM_INST_INDIRECT_CALL:
	{
		std::string func_name = WasmGetFuncAtIdx(dbg->wasm_state, bc->i)->name;
		ret += "indirect call ";
		ret += "\t \\" + func_name;
	}break;
	case WASM_INST_CALL:
	{
		std::string func_name = WasmGetFuncAtIdx(dbg->wasm_state, bc->i)->name;
		ret += "call "+ WasmNumToString(dbg, bc->i);
		ret += "\t \\" + func_name;
	}break;
	case WASM_INST_BREAK_IF:
	{
		ret += "br_if "+ WasmNumToString(dbg, bc->i);;
	}break;
	case WASM_INST_BREAK:
	{
		ret += "br " + WasmNumToString(dbg, bc->i);
	}break;
	case WASM_INST_I32_MUL:
	{
		ret += "i32.mul";
	}break;
	case WASM_INST_BLOCK:
	{
		ret += "block";
	}break;
	case WASM_INST_NOP:
	{
		ret += "nop";
	}break;
	case WASM_INST_F32_CONST:
	{
		snprintf(buffer, 64, "f32.const %.3f", bc->f32);
		ret += buffer;
	}break;
	case WASM_INST_I32_CONST:
	{
		bool is_base_ptr = bc->reg == RBS_WASM;
		bool is_n1_load = (bc + 1)->type == WASM_INST_I32_LOAD;
		bool is_n2_i32_const = (bc + 2)->type == WASM_INST_I32_CONST;
		bool is_n3_i32_add = (bc + 3)->type == WASM_INST_I32_ADD;
		
		bool loading_var_address = is_base_ptr && is_n1_load && is_n2_i32_const && is_n3_i32_add;

		wasm_bc* probable_var_offset = bc + 2;
		std::string loaded_var = "";
		decl2* var;
		if (loading_var_address && FindVarWithOffset(func, probable_var_offset->i, &var, dbg->cur_st->line))
		{
			loaded_var = std::string("\t//pushing $") + var->name + " on the stack";
		}
		ret += "i32.const " + WasmNumToString(dbg, bc->i) + loaded_var;
	}break;
	default:
		ASSERT(0);
	}
	return ret;
}
int WasmGetRegVal(dbg_state* dbg, int reg)
{
	return *(int*)&dbg->mem_buffer[reg * 8];

}
std::string WasmIrValToString(dbg_state* dbg, ir_val* val)
{
	std::string ret = "";
	char buffer[32];
	int ptr = 0;
	if(ptr == -1)
	{
		ret += "&";
	}
	while (ptr < val->ptr && val->type != IR_TYPE_INT)
	{
		ret += "*";
		ptr++;
	}
	switch (val->type)
	{
	case IR_TYPE_REG:
	{
		int r_sz = val->reg_sz;
		std::string reg_sz_str = "";
		if (r_sz > 0)
		{
			r_sz = max(r_sz, 4);
			r_sz = 2 << r_sz;
		}
		std::string float_str = "";
		if(val->is_float)
			float_str = "_f32_";
		snprintf(buffer, 32, "reg%s%d[%s]", float_str.c_str(), r_sz, WasmNumToString(dbg, val->reg).c_str());
		ret += buffer;
	}break;
	case IR_TYPE_ON_STACK:
	{
		int base_ptr = WasmGetRegVal(dbg, BASE_STACK_PTR_REG);
		base_ptr -= dbg->cur_func->strct_constrct_at_offset;
		snprintf(buffer, 32, "struct constr (address %s)", WasmNumToString(dbg, base_ptr).c_str());
		ret += buffer;
	}break;
	case IR_TYPE_PARAM_REG:
	{
		snprintf(buffer, 32, "param_reg[%s]", WasmNumToString(dbg, val->reg).c_str());
		ret += buffer;
	}break;
	case IR_TYPE_DECL:
	{
		snprintf(buffer, 32, "%s", val->decl->name.c_str());
		ret += buffer;
	}break;
	case IR_TYPE_RET_REG:
	{
		snprintf(buffer, 32, "ret_reg[%s]", WasmNumToString(dbg, val->reg).c_str());
		ret += buffer;
	}break;
	case IR_TYPE_ARG_REG:
	{
		snprintf(buffer, 32, "arg_reg[%s]", WasmNumToString(dbg, val->reg).c_str());
		ret += buffer;
	}break;
	case IR_TYPE_F32:
	{
		snprintf(buffer, 32, "%.4f", val->f32);
		ret += buffer;
	}break;
	case IR_TYPE_INT:
	{
		ret += WasmNumToString(dbg, val->i);
	}break;
	default:
		ASSERT(0);
	}
	return ret;
}

std::string WasmGetBinIR(dbg_state* dbg, ir_rep* ir)
{
	char buffer[64];
	std::string lhs = WasmIrValToString(dbg, &ir->bin.lhs);
	std::string rhs = WasmIrValToString(dbg, &ir->bin.rhs);
	std::string op = OperatorToString(ir->bin.op);
	snprintf(buffer, 64, "%s %s %s", lhs.c_str(), op.c_str(), rhs.c_str());
	return buffer;
}

std::string WasmIrToString(dbg_state* dbg, ir_rep *ir)
{
	char buffer[128];
	std::string ret="";
	switch (ir->type)
	{
	case IR_END_CALL:
	{
		ret = "calling "+ir->call.fdecl->name + " \n";
	}break;
	case IR_INDIRECT_CALL:
	{
		ret = "indirect calling to " + ir->call.fdecl->name + " \n";
	}break;
	case IR_CALL:
	{
		ret = "calling to "+ir->call.fdecl->name + " \n";
	}break;
	case IR_BEGIN_CALL:
	{
		ret = "beginning call to "+ir->call.fdecl->name + " \n";
	}break;
	case IR_CMP_GE:
	{
		ret = WasmGetBinIR(dbg, ir) + "\n";
	}break;
	case IR_END_LOOP_BLOCK:
	{
		ret = "end loop\n";
	}break;
	case IR_BEGIN_LOOP_BLOCK:
	{
		ret = "begin loop\n";
	}break;
	case IR_END_IF_BLOCK:
	{
		ret = "end cond if\n";
	}break;
	case IR_BEGIN_IF_BLOCK:
	{
		ret = "begin cond if\n";
	}break;
	case IR_STACK_BEGIN:
	case IR_STACK_END:
	case IR_BEGIN_STMNT:
	case IR_END_STMNT:
	case IR_NOP:
	{

	}break;
	case IR_ASSIGNMENT:
	{
		std::string to_assign = WasmIrValToString(dbg, &ir->assign.to_assign);
		std::string lhs = WasmIrValToString(dbg, &ir->assign.lhs);
		if (ir->assign.only_lhs)
			snprintf(buffer, 128, "%s = %s", to_assign.c_str(), lhs.c_str());
		else
		{
			std::string rhs = WasmIrValToString(dbg, &ir->assign.rhs);
			std::string op = OperatorToString(ir->assign.op);
			snprintf(buffer, 128, "%s = %s %s %s", to_assign.c_str(), lhs.c_str(), op.c_str(), rhs.c_str());
		}
		ret = buffer;
		ret += "\n";
	}break;
	default:
		ASSERT(0);
	}
	return ret;
}

void WasmGetIrWithIdx(dbg_state* dbg, int idx, ir_rep **ir_start, int *len_of_same_start, int start = -1, int end = -1)
{

	if (start == -1)
		dbg->cur_st->start;
	if (end == -1)
		dbg->cur_st->end;
	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &dbg->cur_func->ir;
	ir_rep* ir = ir_ar->begin();
	for (int i = 0; i < ir_ar->size(); i++)
	{
		if (ir->start == idx)
		{
			*ir_start = ir;
			(*len_of_same_start) = 0;
			while (ir->start == idx)
			{
				(*len_of_same_start)++;
				ir++;
			}

			return;
		}
		ir++;
	}
	*ir_start = nullptr;
}

std::string WasmPrintCodeGranular(dbg_state* dbg, int max = -1, bool center_cur = true)
{
	std::string ret = "";
	int i = 0;
	//int offset = dbg->

	int start_bc = dbg->cur_st->start;
	int end_bc = dbg->cur_st->end;

	if (center_cur)
	{
		int window = 8;
		int half_window = window / 2;
		int cur_bc_idx = *dbg->cur_bc - dbg->bcs.begin();
		start_bc = max(dbg->cur_st->start, cur_bc_idx - half_window);
		end_bc = min(cur_bc_idx + half_window, dbg->bcs.size());
	}
	wasm_bc* stat_begin = dbg->bcs.begin() + start_bc;
	wasm_bc* cur = stat_begin;

	char buffer[512];

	if (max == -1)
	{
		max = start_bc - end_bc;
		max++;
	}
	//max = max(max, 16);

	while (i < max && cur < dbg->bcs.end())
	{
		int bc_on_stat_rel_idx = cur - stat_begin;
		int bc_on_stat_abs_idx = bc_on_stat_rel_idx + start_bc;
		ir_rep* ir = nullptr;
		int len_ir = 0;
		WasmGetIrWithIdx(dbg, bc_on_stat_abs_idx, &ir, &len_ir, start_bc, start_bc + max);
		std::string ir_str = "";
		if (ir)
		{
			while (len_ir > 0)
			{
				std::string ir_str = WasmIrToString(dbg, ir);
				if (ir_str != "")
				{
					std::string ir_idx_str = WasmNumToString(dbg, ir->idx);
					snprintf(buffer, 512, ANSI_YELLOW "%s: %s" ANSI_RESET, ir_idx_str.c_str(), ir_str.c_str());
					ret += buffer;
				}
				len_ir--;
				ir++;
			}
		}
		std::string bc_str = WasmGetBCString(dbg, dbg->cur_func, cur, &dbg->bcs);
		std::string bc_rel_idx = WasmNumToString(dbg, i, 3);

		if (cur == *dbg->cur_bc)
			sprintf(buffer, ANSI_BG_WHITE ANSI_BLACK "%s:%s" ANSI_RESET, bc_rel_idx.c_str(), bc_str.c_str());
		else if (cur->dbg_brk)
			sprintf(buffer, ANSI_RED "%s:%s" ANSI_RESET, bc_rel_idx.c_str(), bc_str.c_str());
		else
			sprintf(buffer, "%s:%s", bc_rel_idx.c_str(), bc_str.c_str());
		ret += buffer;
		ret += "\n";
		i++;
		cur++;
	}
	return ret;
}
std::string WasmPrintVars(dbg_state *dbg)
{
	std::string ret = "";
	char* mem_buffer = dbg->mem_buffer;
	func_decl* func = dbg->cur_func;
	stmnt_dbg* stmnt = dbg->cur_st;
	own_std::vector<wasm_bc>* bcs = &dbg->bcs;

	int base_stack_ptr = *(int *)&mem_buffer[BASE_STACK_PTR_REG * 8];
	printf("**vars and params\n");
	scope* cur_scp = FindScpWithLine(func, dbg->cur_st->line);
	while(cur_scp->parent)
	{
		FOR_VEC(decl, cur_scp->vars)
		{
			decl2* d = *decl;
			if (d->type.type == TYPE_FUNC || d->type.type == TYPE_STRUCT_TYPE || d->type.type == TYPE_FUNC_PTR)
				continue;

			int val = *(int*)&mem_buffer[base_stack_ptr + d->offset];

			std::string type;

			if (IS_FLAG_ON(d->flags, DECL_IS_ARG))
				type = "param";
			else
				type = "local";

			std::string offset_str = WasmNumToString(dbg, d->offset);
			std::string val_str = WasmNumToString(dbg, val);

			char buffer[512];
			sprintf(buffer, "%s: %s, value: %s, offset: %s\n", type.c_str(), d->name.c_str(), val_str.c_str(), offset_str.c_str());
			ret += buffer;
		}
		cur_scp = cur_scp->parent;
	}
	//printf("wasm:\n %s", WasmPrintCode(mem_buffer, func, stmnt, bcs).c_str());
	ret += "\n\n";
	return ret;
}
long long WasmInterpGetReVal(char* mem_buffer, int reg, bool deref)
{
	auto ptr = *(long long *)&mem_buffer[reg * 8];
	if (deref)
		return *(long long*)&mem_buffer[ptr];
	return ptr;
}

ir_rep *GetIrBasedOnOffset(dbg_state *dbg, int offset)
{
	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &dbg->cur_func->ir;
	FOR_VEC(ir, (*ir_ar))
	{
		if (offset >= ir->start && offset <= ir->end)
			return ir;
	}
	return nullptr;
}
stmnt_dbg* GetStmntBasedOnOffset(own_std::vector<stmnt_dbg>* ar, int offset)
{
	FOR_VEC(st, (*ar))
	{
		if (offset >= st->start && offset <= st->end)
			return st;
	}
	return nullptr;
}

#define DBG_STATE_ON_LANG 0
#define DBG_STATE_ON_WASM 1



std::string WasmGetStack(dbg_state* dbg)
{
	std::string ret = "stack: \n";
	char buffer[512];
	int i = 0;
	FOR_VEC(s, dbg->wasm_stack)
	{
		std::string type = "";
		std::string val_str = "";
		switch (s->type)
		{
			// f32
		case WSTACK_VAL_F32:
		{
			snprintf(buffer, 512, "%.3f", s->f32);
			val_str = buffer;
			type = "f32";
		}break;
		case WSTACK_VAL_INT:
		{
			val_str = WasmNumToString(dbg, s->s32);
			type = "i32";
		}break;
		default:
			ASSERT(0);
		}
		sprintf(buffer, "%d: %s %s\n", i, type.c_str(), val_str.c_str());
		ret += buffer;
		i++;
	}

	return ret;
}

struct typed_stack_val
{
	ast_rep* a;
	bool decl;
	int offset;
	type2 type;
};

int WasmGetMemOffsetVal(dbg_state* dbg, unsigned int offset)
{
	return *(int*)&dbg->mem_buffer[offset];
}
int WasmGetDeclVal(dbg_state* dbg, int offset)
{
	int mem = *(int*)&dbg->mem_buffer[BASE_STACK_PTR_REG * 8];
	return *(int*)&dbg->mem_buffer[mem + offset];
}
void WasmFromAstArrToStackVal(dbg_state* dbg, own_std::vector<ast_rep *> expr, typed_stack_val* out)
{
	own_std::vector<typed_stack_val> expr_stack;
	FOR_VEC(ast, expr)
	{
		ast_rep* a = *ast;
		typed_stack_val val = {};
		val.a = a;

		switch (a->type)
		{
		case AST_UNOP:
		{
			typed_stack_val *top = &expr_stack.back();
			ASSERT(top->decl == true)
			expr_stack.pop_back();
			val.type = top->type;
			val.type.ptr++;
			val.offset = top->offset;
		}break;
		case AST_CAST:
		{
			typed_stack_val *top = &expr_stack.back();
			expr_stack.pop_back();
			val.a = top->a;
			val.offset = top->offset;
			val.type = a->cast.type;
			//val.type.type = FromTypeToVarType(a->cast.type.type)
			
			val.type = a->cast.type;

			val.offset = top->offset;
		}break;
		case AST_INT:
		{
			val.type.type = TYPE_S32;
			val.offset = a->num;
		}break;
		case AST_IDENT:
		{
			val.type = a->decl->type;
			val.type.e_decl = a->decl;
			val.decl = true;
			val.offset = WasmGetRegVal(dbg, BASE_STACK_PTR_REG) + a->decl->offset;
			//int decl_val = WasmGetDeclVal(dbg, a->decl->offset);

			//val.type.u32 = decl_val;
		}break;
		default:
			ASSERT(0);
		}
		expr_stack.emplace_back(val);
	}
	ASSERT(expr_stack.size() == 1);
	*out = expr_stack.back();

}
std::string WasmGenRegStr(dbg_state* dbg, int reg, int reg_sz, std::string reg_name)
{
	unsigned int reg_val = WasmGetMemOffsetVal(dbg, reg);
	char buffer[64];
	std::string reg_idx = WasmNumToString(dbg, reg);
	std::string reg_val_str = WasmNumToString(dbg, reg_val);
	std::string ptr_val_str;
	std::string reg_size_str;

	if (reg_sz > 4)
		reg_sz = 4;

	reg_sz = 2 << reg_sz;

	if (reg_sz != 0)
		reg_size_str = std::to_string(reg_sz);


	if (reg_val < 0 || reg_val >dbg->mem_size)
		ptr_val_str = "out of bounds";
	else
	{
		int ptr_deref_val = WasmGetMemOffsetVal(dbg, reg_val);
		ptr_val_str = WasmNumToString(dbg, ptr_deref_val);
	}

	snprintf(buffer, 64, "%s%s[%s] = %s {%s}", reg_name.c_str(), reg_size_str.c_str(), reg_idx.c_str(), reg_val_str.c_str(), ptr_val_str.c_str());
	return buffer;

}
void WasmPrintRegisters(dbg_state* dbg)
{
	std::string str = "";
	char buffer[64];
	for (int i = 0; i < 16; i++)
	{
		auto reg = (wasm_reg)(i * 8);

		switch (reg)
		{
		case wasm_reg::RS_WASM:
		{

			str += WasmGenRegStr(dbg, reg, 4, "stack");
			str += "\n";
		}break;
		case RBS_WASM:
		{
			str += WasmGenRegStr(dbg, reg, 4, "base");
			str += "\n";
		}break;
		case R5_WASM:
		case R4_WASM:
		case R3_WASM:
		case R2_WASM:
		case R1_WASM:
		case R0_WASM:
		{
			str += WasmGenRegStr(dbg, reg, 4, "reg");
			str += "\n";
		}break;
		default:
			break;
		}
	}
	int stack_ptr = WasmGetMemOffsetVal(dbg, STACK_PTR_REG * 8);
	for (int i = 0; i < 4; i++)
	{
		str += WasmGenRegStr(dbg, stack_ptr + (i * 8), 4, "arg_reg");
		str += "\n";
	}
	printf("%s", str.c_str());
}
std::string WasmVarToString(dbg_state* dbg, char indent, decl2* d, int struct_offset)
{
	char buffer[512];
	std::string ret;
	if (d->type.type == TYPE_STRUCT || d->type.type == TYPE_STRUCT_TYPE)
	{
		indent++;
		ret += "{";
		FOR_VEC(struct_d, d->type.strct->scp->vars)
		{
			decl2* ds = *struct_d;
			std::string var_str = WasmVarToString(dbg, indent, ds, struct_offset);
			snprintf(buffer, 512, "%s, ", var_str.c_str());
			ret += buffer;
		}

		ret += "}";
	}
	else
	{
		std::string val_str = WasmNumToString(dbg, WasmGetMemOffsetVal(dbg, struct_offset + d->offset));
		snprintf(buffer, 512, "%s: %s", d->name.c_str(), val_str.c_str());
		ret += buffer;
	}

	return ret;
}
std::string WasmGetSingleExprStr(dbg_state* dbg, dbg_expr* exp)
{
	std::string ret = "expression: " + exp->exp_str;

	//if(exp->)
	typed_stack_val expr_val;
	WasmFromAstArrToStackVal(dbg, exp->expr, &expr_val);
	char buffer[512];
	if (expr_val.type.type == TYPE_STRUCT)
	{
		decl2* d = expr_val.type.e_decl;
		//std::string WasmVarToString(dbg_state* dbg, char indent, decl2* d, int struct_offset)

		std::string struct_str = WasmVarToString(dbg, 0, d, expr_val.offset);
		sprintf(buffer, " addr(%s) (%s) %s ", WasmNumToString(dbg, expr_val.offset).c_str(), TypeToString(expr_val.type).c_str(), struct_str.c_str());
	}
	else
	{
		int val = WasmGetMemOffsetVal(dbg, expr_val.offset);
		sprintf(buffer, " (%s) %s ", TypeToString(expr_val.type).c_str(), WasmNumToString(dbg, val).c_str());
	}
	ret += buffer;

	switch (exp->type)
	{
	case DBG_EXPR_SHOW_SINGLE_VAL:
	{
		if (expr_val.type.ptr > 0)
		{

			int ptr_deref_val = WasmGetMemOffsetVal(dbg, expr_val.offset);

			sprintf(buffer, "{%s}", WasmNumToString(dbg, ptr_deref_val).c_str());
			ret += buffer;

		}
	}break;
	case DBG_EXPR_SHOW_VAL_X_TIMES:
	{
		typed_stack_val x_times;
		WasmFromAstArrToStackVal(dbg, exp->x_times, &x_times);
		ASSERT(expr_val.type.ptr > 0);
		ASSERT(x_times.type.ptr == 0);

		void *ptr_buffer = &dbg->mem_buffer[expr_val.offset];
		std::string show = "";
		switch (expr_val.type.type)
		{
		case TYPE_CHAR:
		{
			for (int i = 0; i < x_times.offset; i++)
			{
				show += *(((char*)ptr_buffer) + i);
			}
		}break;
		default:
		{
			ASSERT(0);
		}break;
		}
		snprintf(buffer, 512, " {%s} ", show.c_str());
		ret += buffer;

	}break;
	default:
		ASSERT(0);
	}

	return ret;
}

void WasmPrintExpressions(dbg_state* dbg)
{
	FOR_VEC(e, dbg->exprs)
	{
		printf("%s\n", WasmGetSingleExprStr(dbg, *e).c_str());
	}
}

void WasmPrintStackAndBcs(dbg_state* dbg, int max_bcs = -1)
{
	printf("%s\n", WasmPrintCodeGranular(dbg, max_bcs).c_str());
	printf("%s\n", WasmGetStack(dbg).c_str());
}

void WasmBreakOnNextStmnt(dbg_state* dbg, bool *args_break)
{
	stmnt_dbg* next = dbg->cur_st;
	stmnt_dbg* prev = next;
	next++;
	while (next->line == prev->line && next <= &dbg->cur_func->wasm_stmnts.back())
	{
		prev = next;
		next++;
	}
	if (next <= &dbg->cur_func->wasm_stmnts.back())
	{
		wasm_bc* next_bc = dbg->bcs.begin() + next->start;
		next_bc->one_time_dbg_brk = true;
		dbg->break_type = DBG_BREAK_ON_NEXT_STAT;
	}
	else
		dbg->break_type = DBG_BREAK_ON_DIFF_STAT;

	*args_break = true;

}
void WasmOnArgs(dbg_state* dbg)
{
	bool args_break = false;
	bool first_timer = true;

	while (!args_break)
	{

		if (first_timer)
		{
			if ((dbg->break_type == DBG_BREAK_ON_NEXT_BC)
				|| (*dbg->cur_bc)->dbg_brk || (*dbg->cur_bc)->one_time_dbg_brk)
			{
				WasmPrintStackAndBcs(dbg, 16);
			}
			dbg->break_type = DBG_BREAK_ON_DIFF_STAT;
			WasmPrintExpressions(dbg);
			(*dbg->cur_bc)->one_time_dbg_brk = false;
		}
		first_timer = false;

		func_decl* func = dbg->cur_func;
		stmnt_dbg* stmnt = dbg->cur_st;

		char* cur_line = func->from_file->lines[stmnt->line - 1];
		printf("%s\n", cur_line);

		std::string input;
		std::getline(std::cin, input);
		//std::cin >> input;
		own_std::vector<std::string> args;
		/*
		if (dbg_state == DBG_STATE_ON_LANG)
			printf("WASM:");
		else
			printf("LANG:");
			*/

		std::string aux;
		split(input, ' ', args, &aux);

		if (args[0] == "p")
		{
			if (args.size() == 1)
				continue;

			if (args[1] == "wasm")
			{
				int quantity = 1;
				if (args.size() == 2)
				{
					quantity = max(1, dbg->cur_st->end - dbg->cur_st->start);
				}
				if (args.size() > 2)
					quantity = atof(args[2].c_str());
				std::string ret = WasmPrintCodeGranular(dbg, quantity);
				printf("%s\n", ret.c_str());
				printf("%s\n", WasmGetStack(dbg).c_str());
			}
			else if (args[1] == "locals")
			{
				printf("%s\n", WasmPrintVars(dbg).c_str());
			}
			else if (args[1] == "exs")
			{
				WasmPrintExpressions(dbg);

			}
			else if (args[1] == "stack")
			{
				printf("%s\n", WasmGetStack(dbg).c_str());
			}
			else if (args[1] == "regs")
			{
				WasmPrintRegisters(dbg);
			}
		}
		else if (args[0] == "con")
		{
			dbg->break_type = DBG_NO_BREAK;
			args_break = true;
		}
		else if (args[0] == "brelw")
		{
			if (args.size() == 1)
			{
				continue;
			}
			int offset = atof(args[1].c_str());

			((*dbg->cur_bc) + offset)->dbg_brk = true;


		}
		else if (args[0] == "set")
		{
			if (args.size() == 1)
				continue;

			if (args[1] == "numformat")
			{
				if (args.size() == 2)
					continue;
				if (args[2] == "hex")
				{
					dbg->print_numbers_format = DBG_PRINT_HEX;
				}
				else if (args[2] == "dec")
				{
					dbg->print_numbers_format = DBG_PRINT_DECIMAL;
				}
			}
		}
		else if (args[0] == "new")
		{
			if (args.size() == 2)
				continue;
			if (args[1] == "ex")
			{
				std::string exp_str = "";
				for (int i = 2; i < args.size(); i++)
				{
					exp_str += args[i];
				}
				node* n = ParseString(dbg->lang_stat, exp_str);
				DescendNode(dbg->lang_stat, n, dbg->cur_func->wasm_scp);

				ast_rep* ast = AstFromNode(dbg->lang_stat, n, dbg->cur_func->scp);

				n->FreeTree();
				int last_idx = dbg->lang_stat->allocated_vectors.size() - 1;
				dbg->lang_stat->allocated_vectors[last_idx]->~vector();
				dbg->lang_stat->allocated_vectors.pop_back();

				auto exp = (dbg_expr*)AllocMiscData(dbg->lang_stat, sizeof(dbg_expr));
				exp->exp_str = exp_str.substr();
				exp->from_func = dbg->cur_func;

				//exp_str = "";
				if (ast->type == AST_BINOP && ast->op == T_COMMA)
				{
					ASSERT(ast->expr.size() == 2);

					exp->type = DBG_EXPR_SHOW_VAL_X_TIMES;

					PushAstsInOrder(dbg->lang_stat, ast->expr[0], &exp->expr);
					PushAstsInOrder(dbg->lang_stat, ast->expr[1], &exp->x_times);
					//exp_str = WasmGetSingleExprStr(dbg, &exp);
				}
				else
				{
					exp->type = DBG_EXPR_SHOW_SINGLE_VAL;

					PushAstsInOrder(dbg->lang_stat, ast, &exp->expr);
					//exp_str = WasmGetSingleExprStr(dbg, &exp);
				}


				dbg->exprs.emplace_back(exp);
				int a = 0;
			}
		}
		else if (args[0] == "ns")
		{ 
			WasmBreakOnNextStmnt(dbg, &args_break);
		}
		// step into func
		else if (args[0] == "si")
		{
			wasm_bc* cur = *dbg->cur_bc;
			wasm_bc* end = dbg->bcs.begin() + dbg->cur_st->end;
			while (cur <= end && (cur->type != WASM_INST_CALL && cur->type != WASM_INST_INDIRECT_CALL))
			{
				cur++;
			}
			// found call
			if (cur <= end)
			{
				func_decl* first_func = dbg->wasm_state->funcs[cur->i];

				stmnt_dbg* first_st = &first_func->wasm_stmnts[0];

				wasm_bc* first_bc = dbg->bcs.begin() + first_st->start;
				args_break = true;
				first_bc->one_time_dbg_brk = true;
				
			}
			else
			{
				WasmBreakOnNextStmnt(dbg, &args_break);
			}

		}
		// previous ir
		else if (args[0] == "pi")
		{
			args_break = true;
			ir_rep* cur = dbg->cur_ir;
			while (cur->start == cur->end)
				cur->start--;
			cur--;

			wasm_bc *to = dbg->bcs.begin() + cur->start;

			WasmModifyCurBcPtr(dbg, to);
			to->one_time_dbg_brk = true;
		}
		// next ir
		else if (args[0] == "ni")
		{
			args_break = true;
			ir_rep* cur = dbg->cur_ir;
			while (cur->start == cur->end)
				cur->start++;
			cur++;

			wasm_bc* cur_ir_bc = dbg->bcs.begin() + cur->start;
			cur_ir_bc->one_time_dbg_brk = true;
			dbg->some_bc_modified = true;

			dbg->break_type = DBG_NO_BREAK;
		}
		else if (args[0] == "nw")
		{
			args_break = true;
			dbg->break_type = DBG_BREAK_ON_NEXT_BC;

		}
	}
}

void JsPrint(dbg_state* dbg)
{
	*(int*)&dbg->mem_buffer[RET_1_REG * 8] = 7;
}
void WasmDoCallInstruction(dbg_state *dbg, wasm_bc **bc, block_linked **cur, func_decl *call_f)
{
	dbg->cur_func = call_f;
	dbg->func_stack.emplace_back(call_f);
	dbg->block_stack.emplace_back(*cur);
	dbg->return_stack.emplace_back(*bc);
	*bc = dbg->bcs.begin() + call_f->wasm_code_sect_idx - 1;
	*cur = nullptr;

}
void WasmSerializePushU32IntoVector(own_std::vector<unsigned char>* out, int val)
{
	out->emplace_back(val & 0xff);
	out->emplace_back((val & 0xff00)>>8);
	out->emplace_back((val & 0xff0000)>>16);
	out->emplace_back((val & 0xff000000)>>24);
}
struct str_dbg
{
	int name_on_string_sect;
	int name_len;
};
struct struct_dbg
{
	str_dbg name;
	int vars_offset;
	int num_of_vars;
};
struct var_dbg
{
	str_dbg name;
	enum_type2 type;
	int type_idx;
	int offset;
	char ptr;
};
struct type_dbg
{
	str_dbg name;
	int vars_offset;
	int num_of_vars;
};
enum scp_dbg_type
{
	SCP_DBG_TP_UNDEFINED,
	SCP_DBG_TP_FUNC,
	SCP_DBG_TP_STRUCT,
};
struct scope_dbg
{
	int vars_offset;
	int num_of_vars;
	unsigned int parent;
	int children;
	int children_len;

	int line_start;
	int line_end;

	scp_dbg_type type;
	union
	{
		int type_idx;
		int func_idx;
	};
};
struct file_dbg
{
	str_dbg name;
};
struct func_dbg
{
	str_dbg name;
	int scope;
	int code_start;

	int stmnts_offset;
	int stmnts_len;

	int ir_offset;
	int ir_len;

	int file_idx;

};
struct serialize_state
{
	own_std::vector<unsigned char> func_sect;
	own_std::vector<unsigned char> stmnts_sect;
	own_std::vector<unsigned char> string_sect;
	own_std::vector<unsigned char> scopes_sect;
	own_std::vector<unsigned char> types_sect;
	own_std::vector<unsigned char> vars_sect;
	own_std::vector<unsigned char> ir_sect;
	own_std::vector<unsigned char> file_sect;
};
enum dbg_code_type
{
	DBG_CODE_WASM,
};
struct dbg_file
{
	int func_sect;
	int stmnts_sect;
	int string_sect;
	int scopes_sect;
	int types_sect;
	int vars_sect;
	int code_sect;
	int ir_sect;
	int files_sect;
	int total_files;

	dbg_code_type code_type;
};
void WasmSerializePushString(serialize_state* ser_state, std::string* name, str_dbg *);

void WasmSerializeSimpleVar(web_assembly_state* wasm_state, serialize_state* ser_state, decl2* var, int var_idx)
{
	auto var_ser = (var_dbg*)(ser_state->vars_sect.begin() + var_idx);
	memset(var_ser, 0, sizeof(var_dbg));
	if (var->type.type == TYPE_STRUCT || var->type.type == TYPE_STRUCT_TYPE)
	{
		type_struct2* strct = var->type.strct;
		ASSERT(IS_FLAG_ON(strct->flags, TP_STRCT_STRUCT_SERIALIZED));
		var_ser->type_idx = strct->serialized_type_idx;

	}
	var_ser->type = var->type.type;
	var_ser->ptr = var->type.ptr;
	var_ser->offset = var->offset;
	WasmSerializePushString(ser_state, &var->name, &var_ser->name);
}
void WasmSerializeStructType(web_assembly_state* wasm_state, serialize_state *ser_state, type_struct2* strct)
{
	
	FOR_VEC(decl, strct->vars)
	{
		decl2* d = *decl;
		switch (d->type.type)
		{
		case TYPE_STRUCT:
		case TYPE_STRUCT_TYPE:
		{
			if(IS_FLAG_OFF(d->type.strct->flags, TP_STRCT_STRUCT_SERIALIZED))
				WasmSerializeStructType(wasm_state, ser_state, d->type.strct);
		}break;
		}
	}

	int type_offset = ser_state->types_sect.size();
	int var_offset = ser_state->vars_sect.size();

	strct->serialized_type_idx = type_offset;

	ser_state->vars_sect.make_count(ser_state->vars_sect.size() + strct->vars.size() * sizeof(var_dbg));
	ser_state->types_sect.make_count(ser_state->types_sect.size() + sizeof(type_dbg));

	auto type = (type_dbg*)ser_state->types_sect.begin() + type_offset;
	type->num_of_vars = strct->vars.size();
	type->vars_offset = var_offset;
	WasmSerializePushString(ser_state, &strct->name, &type->name);

	/*
	int i = 0; 
	FOR_VEC(decl, strct->vars)
	{
		decl2* d = *decl;
		
		WasmSerializeSimpleVar(wasm_state, ser_state, d, var_offset + i * sizeof(var_dbg));
		i++;
	}
	*/
	strct->flags |= TP_STRCT_STRUCT_SERIALIZED;
}
void WasmSerializeFuncIr(serialize_state *ser_state, func_decl *fdecl)
{
	auto ir_ar = (own_std::vector<ir_rep> *) &fdecl->ir;
	FOR_VEC(ir, (*ir_ar))
	{
		switch (ir->type)
		{
		case IR_CALL:
		{
			func_decl* call = ir->call.fdecl;
			ir->call.i = call->func_dbg_idx;
		}break;
		}
	}
	unsigned char* start = (unsigned char *)ir_ar->begin();
	unsigned char* end = (unsigned char *)ir_ar->end();
	ser_state->ir_sect.insert(ser_state->ir_sect.begin(), start, end);
}
void WasmSerializeScope(web_assembly_state* wasm_state, serialize_state *ser_state, scope *scp, unsigned int parent)
{
	int scp_offset = ser_state->scopes_sect.size();
	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_STRUCT))
	{
		int a = 0;
	}

	int vars_offset = ser_state->vars_sect.size();
	ser_state->scopes_sect.make_count(ser_state->scopes_sect.size() + sizeof(scope_dbg));
	ser_state->vars_sect.make_count(ser_state->vars_sect.size() + scp->vars.size() * sizeof(var_dbg));

	int i = 0;
	FOR_VEC(decl, scp->vars)
	{
		decl2* d = *decl;
		switch (d->type.type)
		{
		case TYPE_FUNC:
		{
			func_decl* f = d->type.fdecl;
			if (IS_FLAG_ON(f->flags, FUNC_DECL_IS_OUTSIDER | FUNC_DECL_INTERNAL | FUNC_DECL_SERIALIZED))
				break;
			int func_offset = ser_state->func_sect.size();
			int stmnt_offset = ser_state->stmnts_sect.size();

			ser_state->func_sect.make_count(ser_state->func_sect.size() + sizeof(func_dbg));
			ser_state->stmnts_sect.insert(ser_state->stmnts_sect.end(), (unsigned char*)f->wasm_stmnts.begin(), (unsigned char*)f->wasm_stmnts.end());

			auto fdbg = (func_dbg*)(ser_state->func_sect.begin() + func_offset);
			fdbg->scope = f->scp->serialized_offset;
			fdbg->code_start = f->wasm_code_sect_idx;
			fdbg->stmnts_offset = stmnt_offset;
			fdbg->stmnts_len = f->wasm_stmnts.size();
			WasmSerializePushString(ser_state, &f->name, &fdbg->name);

			f->func_dbg_idx = func_offset;
			f->scp->type = SCP_TYPE_FUNC;
			f->flags |= FUNC_DECL_SERIALIZED;
		}break;
		case TYPE_STRUCT_TYPE:
		{
			d->type.strct->scp->type = SCP_TYPE_STRUCT;
			WasmSerializeStructType(wasm_state, ser_state, d->type.strct);
		}break;
		case TYPE_S32:
		case TYPE_U32:
		case TYPE_S64:
		case TYPE_U64:
		case TYPE_FUNC_TYPE:
		
		break;
		default:
			ASSERT(0);
		}
		WasmSerializeSimpleVar(wasm_state, ser_state, d, vars_offset + i * sizeof (var_dbg));
		i++;

	}
	FOR_VEC(ch, scp->children)
	{
		scope* c = *ch;
		WasmSerializeScope(wasm_state, ser_state, c, scp_offset);
	}

	auto s = (scope_dbg*)(ser_state->scopes_sect.begin() + scp_offset);
	s->children = scp_offset + sizeof(scope_dbg);
	s->children_len = scp->children.size();
	s->num_of_vars  = scp->vars.size();
	s->line_start = scp->line_start;
	s->line_end = scp->line_end;
	s->vars_offset  = vars_offset;
	s->type = SCP_DBG_TP_UNDEFINED;


	switch(scp->type)
	{
	case SCP_TYPE_FUNC:
	{
		ASSERT(IS_FLAG_ON(scp->fdecl->flags, FUNC_DECL_SERIALIZED));
		s->type = SCP_DBG_TP_FUNC;
		s->func_idx = scp->fdecl->func_dbg_idx;
	}break;
	case SCP_TYPE_STRUCT:
	{
		ASSERT(IS_FLAG_ON(scp->tstrct->flags, TP_STRCT_STRUCT_SERIALIZED));
		s->type = SCP_DBG_TP_STRUCT;
		s->type_idx = scp->tstrct->serialized_type_idx;
	}break;
	}
	scp->serialized_offset = scp_offset;
	s->parent = parent;
}
void WasmSerializePushString(serialize_state* ser_state, std::string* name, str_dbg *out)
{
	out->name_on_string_sect = ser_state->string_sect.size();
	out->name_len = name->size();
	ser_state->string_sect.insert(ser_state->string_sect.end(), (unsigned char *)name->data(), (unsigned char *)(name->data() + name->size()));
}
void WasmSerialize(web_assembly_state* wasm_state, own_std::vector<unsigned char>& code)
{
	own_std::vector<unsigned char> final_buffer;
	serialize_state ser_state;
	//printf("\nscop1: \n%s\n", wasm_state->lang_stat->root->Print(0).c_str());
	
	WasmSerializeScope(wasm_state, &ser_state, wasm_state->lang_stat->root, -1);

	//printf("\nscop2: \n%s\n", wasm_state->lang_stat->root->Print(0).c_str());

	ser_state.file_sect.make_count(wasm_state->lang_stat->files.size() * sizeof(file_dbg));
	int i = 0;
	FOR_VEC(file, wasm_state->lang_stat->files)
	{
		unit_file* f = *file;
		
		int file_offset = i * sizeof(file_dbg);

		f->file_dbg_idx = file_offset;

		auto cur_dbg_f = (file_dbg*)(ser_state.file_sect.begin() + file_offset);

		WasmSerializePushString(&ser_state, &f->name, &cur_dbg_f->name);
		i++;

	}
	FOR_VEC(func, wasm_state->lang_stat->global_funcs)
	{
		func_decl* f = *func;
		auto fdbg = (func_dbg *)(ser_state.func_sect.begin() + f->func_dbg_idx);
		fdbg->ir_offset = ser_state.ir_sect.size();
		fdbg->ir_len = f->ir.size();
		fdbg->file_idx = f->from_file->file_dbg_idx;
		WasmSerializeFuncIr(&ser_state, f);

	}
	/*
	ser_state.func_sect.make_count(ser_state.func_sect.size() + wasm_state->funcs.size() * sizeof(func_dbg));

	int i = 0;
	FOR_VEC(func, wasm_state->lang_stat->global_funcs)
	{
		func_decl* f = *func;
		auto fdbg = (func_dbg *) (ser_state.func_sect.begin() + i * sizeof(func_dbg));
		int stmnt_offset = ser_state.stmnts_sect.size();

		ser_state.stmnts_sect.insert(ser_state.stmnts_sect.end(), (unsigned char*)f->wasm_stmnts.begin(), (unsigned char*)f->wasm_stmnts.end());

		fdbg->scope = f->scp->serialized_offset;
		fdbg->stmnts_offset = stmnt_offset;
		fdbg->stmnts_len = f->wasm_stmnts.size();

		i++;
	}
	*/
	//final_buffer.make_count(sizeof(dbg_file));
	dbg_file file;
	file.code_type = DBG_CODE_WASM;

	file.func_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.func_sect);

	file.vars_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.vars_sect);

	file.scopes_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.scopes_sect);

	file.stmnts_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.stmnts_sect);

	file.string_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.string_sect);

	file.types_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.types_sect);
	
	file.code_sect = final_buffer.size();
	INSERT_VEC(final_buffer, code);

	file.ir_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.ir_sect);

	file.files_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.file_sect);

	file.total_files = wasm_state->lang_stat->files.size();

	final_buffer.insert(final_buffer.begin(), (unsigned char*)&file, ((unsigned char*)&file) + sizeof(dbg_file));
	
	WriteFileLang("../../wabt/dbg_wasm.dbg", final_buffer.begin(), final_buffer.size());

}
std::string WasmInterpNameFromOffsetAndLen(unsigned char* data, dbg_file* file, str_dbg *name)
{
	unsigned char* string_sect = data + file->string_sect;
	return std::string((const char *)string_sect + name->name_on_string_sect, name->name_len);
}

decl2 *WasmInterpBuildSimplaVar(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope* scp_final, var_dbg* var)
{
	std::string name = WasmInterpNameFromOffsetAndLen(data, file, &var->name);
	type2 tp;
	tp.type = var->type;
	decl2* ret = NewDecl(lang_stat, name, tp);
	return ret;

}
decl2 *WasmInterpBuildStruct(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, struct_dbg *dstrct)
{
	unsigned char* string_sect = data + file->string_sect;
	auto cur_var = (var_dbg *)(data + file->vars_sect + dstrct->vars_offset);

	auto final_struct = (type_struct2* )AllocMiscData(lang_stat, sizeof(type_struct2));
	type2 tp;
	tp.type = TYPE_STRUCT_TYPE;
	tp.strct = final_struct;
	std::string name = WasmInterpNameFromOffsetAndLen(data, file, &dstrct->name);

	decl2* d = NewDecl(lang_stat, name, tp);
	return d;

}
void WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
{
	unsigned char* string_sect = data + file->string_sect;
	auto cur_var = (var_dbg*)(data + file->vars_sect + scp_pre->vars_offset);
	for (int i = 0; i < scp_pre->num_of_vars; i++)
	{
		type2 tp = {};
		if (cur_var->type == TYPE_STRUCT)
		{
			auto type_strct = (type_dbg*)(data + file->types_sect + cur_var->type_idx);
			std::string name = WasmInterpNameFromOffsetAndLen(data, file, &type_strct->name);
			decl2 *d = FindIdentifier(name, scp_final, &tp);
			ASSERT(d);
			tp.type = TYPE_STRUCT;
			tp.strct = d->type.strct;

		}
		tp.type = cur_var->type;

		std::string name = std::string((const char *)string_sect + cur_var->name.name_on_string_sect, cur_var->name.name_len);
		decl2 *d = FindIdentifier(name, scp_final, &tp);
		if (!d)
		{
			decl2* d = NewDecl(lang_stat, name, tp);
			d->offset = cur_var->offset;

			scp_final->vars.emplace_back(d);
		}

		cur_var++;
	}

}
unit_file *WasmInterpSearchFile(lang_state* lang_stat, std::string *name)
{
	FOR_VEC(file, lang_stat->files)
	{
		if ((*file)->name == *name)
			return *file;
	}
	return nullptr;
}
scope *WasmInterpBuildScopes(wasm_interp *winterp, unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file *file, scope *parent, scope_dbg *scp_pre)
{
	scope* ret_scp = NewScope(lang_stat, parent);
	ret_scp->line_start = scp_pre->line_start;
	ret_scp->line_end = scp_pre->line_end;

	int child_offset = file->scopes_sect + scp_pre->children;
	auto cur_scp = (scope_dbg*)(data + child_offset);
	for (int i = 0; i < scp_pre->children_len; i++)
	{
		//scope* new_scp = NewScope(lang_stat, ret_scp);
		scope *child_scp = WasmInterpBuildScopes(winterp, data, len, lang_stat, file, ret_scp, cur_scp);

		if (cur_scp->type == SCP_DBG_TP_FUNC)
		{
			auto fdbg = (func_dbg *)(data + file->func_sect + cur_scp->func_idx);

			auto fdecl = (func_decl* )AllocMiscData(lang_stat, sizeof(func_decl));
			fdecl->code_start_idx = fdbg->code_start;

			std::string fname = WasmInterpNameFromOffsetAndLen(data, file, &fdbg->name);
			fdecl->scp = child_scp;
			fdecl->name = std::string(fname);
			type2 tp;
			tp.type = TYPE_FUNC;
			tp.fdecl = fdecl;
			decl2* d = NewDecl(lang_stat, fname, tp);

			ret_scp->vars.emplace_back(d);

			winterp->funcs.emplace_back(fdecl);

			own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &fdecl->ir;
			auto start_ir = (ir_rep *)(data + file->ir_sect + fdbg->ir_offset);
			ir_ar->insert(ir_ar->begin(), start_ir, start_ir + fdbg->ir_len);

			auto start_st = (stmnt_dbg*)(data + file->stmnts_sect + fdbg->stmnts_offset);
			fdecl->wasm_stmnts.insert(fdecl->wasm_stmnts.begin(), start_st, start_st + fdbg->stmnts_len);

			auto fl_dbg = (file_dbg*)(data + file->files_sect + fdbg->file_idx);
			std::string file_name = WasmInterpNameFromOffsetAndLen(data, file, &fl_dbg->name);
			fdecl->from_file = WasmInterpSearchFile(lang_stat, &file_name);
			ASSERT(fdecl->from_file);


			//WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
		}
		if (cur_scp->type == SCP_DBG_TP_STRUCT)
		{
			auto dstrct = (struct_dbg*)(data + file->types_sect + cur_scp->type_idx);

			decl2 *d = WasmInterpBuildStruct(data, len, lang_stat, file, dstrct);

			d->type.strct->scp = child_scp;

			ret_scp->vars.emplace_back(d);

			//WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
		}
		cur_scp++;
	}
	WasmInterpBuildVarsForScope(data, len, lang_stat, file, scp_pre, ret_scp);



	return ret_scp;
}
void WasmInterpRun(wasm_interp* winterp, unsigned char* mem_buffer, unsigned int len, std::string func_start, long long* args, int total_args)
{
	dbg_state& dbg = *winterp->dbg;
	own_std::vector<wasm_bc>& bcs = winterp->dbg->bcs;


	own_std::vector<func_decl*> &func_stack = dbg.func_stack;
	FOR_VEC(func, winterp->funcs)
	{
		func_decl* f = *func;
		if (f->name == func_start)
		{
			func_stack.emplace_back(f);
			break;
		}
	}
	ASSERT(func_stack.size() > 0);
	func_decl* cur_func = func_stack.back();

	block_linked* cur = nullptr;
	
	int a = 0;



	own_std::vector<wasm_stack_val> &wasm_stack = dbg.wasm_stack;
	wasm_stack.reserve(16);

	//FOR_VEC(bc, wf->bcs)
	wasm_bc *bc = &bcs[cur_func->wasm_code_sect_idx];
	//bc->one_time_dbg_brk = true;

	int stack_offset = 1000;
	*(int*)&mem_buffer[STACK_PTR_REG * 8] = stack_offset;

	auto stack_ptr_val = (long long *)&mem_buffer[stack_offset];

	for (int i = 0; i < total_args; i++)
	{
		*stack_ptr_val = args[i];
		stack_ptr_val++;
	}

	dbg.cur_bc = &bc;
	dbg.cur_func = cur_func;
	dbg.mem_buffer = (char *)mem_buffer;
	dbg.wasm_state = winterp;

	int first_start_offset = dbg.cur_func->wasm_stmnts[0].start;
	bcs[first_start_offset].one_time_dbg_brk = true;
	
	// WASM BYTECODE

	bool can_break = false;
	dbg.break_type = DBG_BREAK_ON_NEXT_STAT;

	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &cur_func->ir;
	//ir_rep* cur_ir = ir_ar->begin();

	while(!can_break)
	{
		int bc_idx = (long long)(bc - &bcs[0]);
		wasm_stack_val val = {};
		stmnt_dbg* cur_st = GetStmntBasedOnOffset(&dbg.cur_func->wasm_stmnts, bc_idx);
		ir_rep* cur_ir = GetIrBasedOnOffset(&dbg, bc_idx);
		bool is_different_stmnt = cur_st && dbg.cur_st && dbg.break_type == DBG_BREAK_ON_DIFF_STAT && cur_st->line != dbg.cur_st->line;

		if ( dbg.break_type == DBG_BREAK_ON_NEXT_BC || is_different_stmnt || bc->dbg_brk || bc->one_time_dbg_brk)
		{
			if (!cur_st)
				cur_st = &dbg.cur_func->wasm_stmnts[0];
			dbg.cur_st = cur_st;
			dbg.cur_ir = cur_ir;
			WasmOnArgs(&dbg);
			if (dbg.some_bc_modified)
			{
				dbg.some_bc_modified = false;
				continue;
			}
			bc->one_time_dbg_brk = false;
		}
		switch (bc->type)
		{
		case WASM_INST_NOP:
		{
			//WasmPrintVars(&dbg);
			int a = 0;
		}break;
		case WASM_INST_BREAK:
		{
			int i = 0;
			wasm_bc *label;
			while (i < bc->i)
			{
				cur = cur->parent;
				i++;
			}
			if (!cur)
			{
				can_break = true;
				break;
			}
			if (cur->wbc->type == WASM_INST_LOOP)
			{
				bc = cur->wbc + 1;
				continue;
			}
			bc = cur->wbc->block_end;
			continue;
			
		}break;
		case WASM_INST_BREAK_IF:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			if (top.s32 != 1)
				break;
			wasm_bc *label;
			int i = 0;
			while (i < bc->i)
			{
				cur = cur->parent;
				i++;
			}
			if (!cur)
			{
				can_break = true;
				break;
			}
			bc = cur->wbc->block_end;
			continue;
		}break;
		case WASM_INST_INDIRECT_CALL:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			func_decl* call_f = dbg.wasm_state->funcs[top.u32];
			WasmDoCallInstruction(&dbg, &bc, &cur, call_f);
		}break;
		case WASM_INST_CALL:
		{
			func_decl* call_f = dbg.wasm_state->funcs[bc->i];
			if (IS_FLAG_ON(call_f->flags, FUNC_DECL_IS_OUTSIDER))
			{
				if (winterp->outsiders.find(call_f->name) != winterp->outsiders.end())
				{
					outsider_func_ptr func_ptr = winterp->outsiders[call_f->name];
					func_ptr(&dbg);
				}
				else
					ASSERT(0);

			}
			else
			{
				WasmDoCallInstruction(&dbg, &bc, &cur, call_f);
			}

			// assert is 32bit
			int a = 0;
		}break;
		case WASM_INST_RET:
		{
			dbg.func_stack.pop_back();
			if (dbg.func_stack.size() == 0)
			{
				can_break = true;
				break;
			}
			dbg.cur_func = dbg.func_stack.back();
			
			bc = dbg.return_stack.back();
			dbg.return_stack.pop_back();

			cur = dbg.block_stack.back();
			dbg.block_stack.pop_back();
		}break;
		case WASM_INST_I32_GE_U:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 = (int) (penultimate->u32 >= top.u32);
			int a = 0;
		}break;
		case WASM_INST_I32_STORE8:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			auto penultimate = wasm_stack.back();
			wasm_stack.pop_back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate.type == 0);
			*(char*)&mem_buffer[penultimate.s32] = top.s32;
			int a = 0;
		}break;
		case WASM_INST_F32_STORE:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			auto penultimate = wasm_stack.back();
			wasm_stack.pop_back();
			// assert is 32bit
			ASSERT(top.type == WSTACK_VAL_F32 && penultimate.type == 0);
			*(int*)&mem_buffer[penultimate.s32] = top.s32;
			int a = 0;
		}break;
		case WASM_INST_I32_STORE:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			auto penultimate = wasm_stack.back();
			wasm_stack.pop_back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate.type == 0);
			*(int*)&mem_buffer[penultimate.s32] = top.s32;
			int a = 0;
		}break;
		case WASM_INST_F32_CONST:
		{
			auto top = wasm_stack.back();
			val.type = WSTACK_VAL_F32;
			val.f32 = bc->f32;
			wasm_stack.emplace_back(val);
		}break;
		case WASM_INST_F32_LOAD:
		{
			auto w = &wasm_stack.back();
			// assert is 32bit
			ASSERT(w->type == WSTACK_VAL_INT);
			w->f32 = *(float*)&mem_buffer[w->s32];
			w->type = WSTACK_VAL_F32;
			int a = 0;
		}break;
		case WASM_INST_I32_LOAD:
		{
			auto w = &wasm_stack.back();
			// assert is 32bit
			ASSERT(w->type == WSTACK_VAL_INT);
			w->s32 = *(int*)&mem_buffer[w->s32];
			int a = 0;
		}break;
		case WASM_INST_I32_CONST:
		{
			val.type = WSTACK_VAL_INT;
			val.s32 = bc->i;
			wasm_stack.emplace_back(val);
		}break;
		case WASM_INST_LOOP:
		case WASM_INST_BLOCK:
		{
			cur = NewBlock(cur);
			cur->wbc = bc;
		}break;
		case WASM_INST_I32_MUL:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 *= top.s32;
		}break;
		case WASM_INST_F32_ADD:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == WSTACK_VAL_F32 && penultimate->type == WSTACK_VAL_F32);
			penultimate->f32 += top.f32;
		}break;
		case WASM_INST_I32_ADD:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 += top.s32;
		}break;
		case WASM_INST_I32_SUB:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 -= top.s32;
		}break;
		case WASM_INST_END:
		{
			if (cur)
			{
				FreeBlock(cur);
				cur = cur->parent;
			}

		}break;
		default:
			ASSERT(0)
		}
		bc++;
	}
}
func_decl* WasmInterpFindFunc(wasm_interp* winterp, std::string func_name)
{
	FOR_VEC(func, winterp->funcs)
	{
		func_decl* f = *func;
		if (f->name == func_name)
		{
			return f;
		}
	}
	return nullptr;
}

void WasmInterpPatchIr(own_std::vector<ir_rep>* ir_ar, wasm_interp* winterp, dbg_file* file)
{
	unsigned char* start_f = (unsigned char*)(file)+file->func_sect;
	FOR_VEC(ir, (*ir_ar))
	{
		switch (ir->type)
		{
		case IR_CALL:
		{
			auto fdbg = (func_dbg*)(start_f + ir->call.i);
			std::string name = WasmInterpNameFromOffsetAndLen((unsigned char*)(file + 1), file, &fdbg->name);
			func_decl* fdecl = WasmInterpFindFunc(winterp, name);
			ir->call.fdecl = fdecl;
		}break;
		}
	}
}
std::string PrintScpPre(unsigned char * start, scope_dbg *s)
{

	std::string ret = "{";
	char buffer[512];
	snprintf(buffer, 512, "type: %d", s->type);
	ret += buffer;

	for (int i = 0; i < s->children_len;i++)
	{
		ret += PrintScpPre(start, (scope_dbg *)(start + s->children + i * sizeof(scope_dbg)));
		ret += ", ";
		//snprintf(buffer, 512,"")
	}
	if (s->children_len > 0)
	{
		ret.pop_back();
		ret.pop_back();
	}
	ret += " }";
	return ret;
}

void WasmInterpInit(wasm_interp *winterp, unsigned char *data, unsigned int len, lang_state *lang_stat)
{

	auto file = (dbg_file*) data;
	data = (unsigned char*)(file + 1);

	unsigned char* code = data + file->code_sect;

	lang_stat->files.clear();

	for (int i = 0; i < file->total_files; i++)
	{
		auto new_f = (unit_file*)AllocMiscData(lang_stat, sizeof(unit_file));

		auto cur_file = (file_dbg*)(data + file->files_sect);

		std::string name = WasmInterpNameFromOffsetAndLen(data, file, &cur_file->name);

		new_f->name = std::string(name);
		int read = 0;
		new_f->contents = ReadEntireFileLang((char *)new_f->name.c_str(), &read);
		char* cur_str = new_f->contents;

		for (int j = 0; j < read; j++)
		{
			if (new_f->contents[j] == '\n')
			{
				*(char*)&new_f->contents[j] = 0;
				new_f->lines.emplace_back(cur_str);
				cur_str = &new_f->contents[j + 1];

			}
		}
		lang_stat->files.emplace_back(new_f);
	}
	auto scp_pre = (scope_dbg*)(data + file->scopes_sect);
	scope* root = WasmInterpBuildScopes(winterp, data, len, lang_stat, file, nullptr, scp_pre);
	std::string scp_pre_str = PrintScpPre(data + file->scopes_sect, scp_pre);
	//printf("\nscop: \n%s\n****\n%s", root->Print(0).c_str(), scp_pre_str.c_str());


	FOR_VEC(func, winterp->funcs)
	{
		func_decl* f = *func;
		own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &f->ir;
		WasmInterpPatchIr(ir_ar, winterp, file);

	}


	int ptr = 0;
	int sect_code = code[ptr];
	ASSERT(sect_code == 0xa)
		ptr++;
	unsigned int consumed = 0;
	int sect_size = decodeSLEB128(&code[ptr], &consumed);
	ptr += consumed;

	int number_of_functions = decodeSLEB128(&code[ptr], &consumed);
	ptr += consumed;

	int i = 0;

	winterp->dbg = (dbg_state *)AllocMiscData(lang_stat, sizeof(dbg_state));
	dbg_state& dbg = *winterp->dbg;
	//dbg.mem_size = size;
	dbg.lang_stat = lang_stat;
	//dbg.wasm_state = wasm_state;

	own_std::vector<wasm_bc>& bcs = dbg.bcs;
	while (i < winterp->funcs.size())
	{
		func_decl* cur_f = winterp->funcs[i];
		if (IS_FLAG_ON(cur_f->flags, FUNC_DECL_IS_OUTSIDER))
		{
			i++;
			continue;
		}
		int func_sz = decodeSLEB128(&code[ptr], &consumed);
		ptr += consumed;
		int next_func = ptr + func_sz;

		int local_entries = decodeSLEB128(&code[ptr], &consumed);
		ptr += consumed;
		// not handling entries at the moment
		if (local_entries > 0)
		{
			ASSERT(0);
		}

		int fi = ptr;

		cur_f->code_start_idx = bcs.size();

		stmnt_dbg* cur_st = &cur_f->wasm_stmnts[0];



		cur_f->wasm_code_sect_idx = bcs.size();

		while (ptr < next_func)
		{
			wasm_bc bc = {};

			int cur_ptr = (ptr - fi);
			bc.start_code = cur_ptr;
			if (cur_ptr == cur_st->start)
			{
				cur_st->start = bcs.size();
			}


			int op = code[ptr];
			ptr++;
			switch (op)
			{
			case 0x1:
			{
				bc.type = WASM_INST_NOP;
			}break;
			case 0x11:
			{
				bc.type = WASM_INST_INDIRECT_CALL;

				// type idx
				decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
				// table idx idx
				decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0x10:
			{
				bc.type = WASM_INST_CALL;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0x6c:
			{
				bc.type = WASM_INST_I32_MUL;
			}break;
			case 0x4e:
			{
				bc.type = WASM_INST_I32_GE_S;
			}break;
			case 0x4f:
			{
				bc.type = WASM_INST_I32_GE_U;
			}break;
			case 0xd:
			{
				bc.type = WASM_INST_BREAK_IF;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0xc:
			{
				bc.type = WASM_INST_BREAK;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0x3:
			{
				bc.type = WASM_INST_LOOP;
				// block return type
				ptr++;
			}break;
			case 0xb:
			{
				bc.type = WASM_INST_END;
			}break;
			case 0x2:
			{
				bc.type = WASM_INST_BLOCK;
				// block return type
				ptr++;
			}break;
			case 0x43:
			{
				bc.type = WASM_INST_F32_CONST;
				//ptr++;
				int* int_ptr = (int *) & code[ptr];
				*(int *)&bc.f32 = (*int_ptr << 24) | (((*int_ptr) & 0xff00) << 8) | (((*int_ptr) & 0xff0000) >> 8) | (((*int_ptr) >> 24));
				
				ptr += 4;
			}break;
			case 0x92:
			{
				bc.type = WASM_INST_F32_ADD;
			}break;
			case 0x6a:
			{
				bc.type = WASM_INST_I32_ADD;
			}break;
			case 0x6b:
			{
				bc.type = WASM_INST_I32_SUB;
			}break;
			case 0x3a:
			{
				bc.type = WASM_INST_I32_STORE8;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x38:
			{
				bc.type = WASM_INST_F32_STORE;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x36:
			{
				bc.type = WASM_INST_I32_STORE;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0xf:
			{
				bc.type = WASM_INST_RET;
			}break;
			case 0x2a:
			{
				bc.type = WASM_INST_F32_LOAD;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x28:
			{
				bc.type = WASM_INST_I32_LOAD;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x41:
			{
				bc.type = WASM_INST_I32_CONST;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;

			}break;
			default:
				ASSERT(0);
			}
			// decrementing because the statement counts only the end of the instructions, and not the beginning of the next one
			cur_ptr = (ptr - fi) - 1;
			if (cur_ptr == cur_st->end)
			{
				cur_st->end = bcs.size();

				cur_st++;
			}
			bc.end_code = cur_ptr + 1;
			bcs.emplace_back(bc);

		}

		own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) & cur_f->ir;
		ir_rep* cur_ir = ir_ar->begin();
		for (int i = cur_f->wasm_code_sect_idx; i < bcs.size(); i++)
		{
			while (cur_ir->start == cur_ir->end)
			{
				cur_ir->start = i;
				cur_ir->end = i;
				cur_ir++;
			}
			wasm_bc* cur_bc = bcs.begin() + i;
			if (cur_ir->start == cur_bc->start_code)
				cur_ir->start = i;
			if (cur_ir->end == cur_bc->end_code)
			{
				cur_ir->end = max(cur_ir->start, max(i - 1, 0));
				cur_ir++;
			}
		}

		ptr = next_func;
		
		//funcs[i] = wf;
		i++;
	}

	//wasm_func *wf = &funcs[func_idx];
	
	/*
	own_std::vector<func_decl*> &func_stack = dbg.func_stack;
	FOR_VEC(func, winterp->funcs)
	{
		func_decl* f = *func;
		if (f->name == func_start_name)
		{
			func_stack.emplace_back(f);
			break;
		}
	}
	ASSERT(func_stack.size() > 0);
	func_decl* cur_func = func_stack.back();
	*/

	block_linked* cur = nullptr;
	FOR_VEC(bc, bcs)
	{
		switch (bc->type)
		{
		case WASM_INST_LOOP:
		case WASM_INST_BLOCK:
		{
			cur = NewBlock(cur);
			cur->wbc = bc;
		}break;
		case WASM_INST_END:
		{
			if (cur)
			{
				FreeBlock(cur);
				cur->wbc->block_end = bc;
				cur = cur->parent;
			}

		}break;
		}
	}
}
void WasmInterp(own_std::vector<unsigned char>& code, char* mem_buffer, int size, std::string func_start_name, web_assembly_state* wasm_state, long long* args, int total_args)
{
	wasm_state->outsiders["js_print"] = JsPrint;


	int ptr = 0;
	int sect_code = code[ptr];
	ASSERT(sect_code == 0xa)
		ptr++;
	unsigned int consumed = 0;
	int sect_size = decodeSLEB128(&code[ptr], &consumed);
	ptr += consumed;

	int number_of_functions = decodeSLEB128(&code[ptr], &consumed);
	ptr += consumed;

	int i = 0;

	dbg_state dbg = {};
	dbg.mem_size = size;
	dbg.lang_stat = wasm_state->lang_stat;
	//dbg.wasm_state = wasm_state;

	own_std::vector<wasm_bc>& bcs = dbg.bcs;
	while (i < wasm_state->funcs.size())
	{
		func_decl* cur_f = wasm_state->funcs[i];
		if (IS_FLAG_ON(cur_f->flags, FUNC_DECL_IS_OUTSIDER))
		{
			i++;
			continue;
		}
		wasm_func wf = {};
		int func_sz = decodeSLEB128(&code[ptr], &consumed);
		ptr += consumed;
		int next_func = ptr + func_sz;

		int local_entries = decodeSLEB128(&code[ptr], &consumed);
		ptr += consumed;
		// not handling entries at the moment
		if (local_entries > 0)
		{
			ASSERT(0);
		}
		wf.idx = ptr;

		int fi = ptr;

		cur_f->code_start_idx = bcs.size();

		stmnt_dbg* cur_st = &cur_f->wasm_stmnts[0];



		cur_f->wasm_code_sect_idx = bcs.size();

		while (ptr < next_func)
		{
			wasm_bc bc = {};

			int cur_ptr = (ptr - fi);
			bc.start_code = cur_ptr;
			if (cur_ptr == cur_st->start)
			{
				cur_st->start = bcs.size();
			}


			int op = code[ptr];
			ptr++;
			switch (op)
			{
			case 0x1:
			{
				bc.type = WASM_INST_NOP;
			}break;
			case 0x11:
			{
				bc.type = WASM_INST_INDIRECT_CALL;

				// type idx
				decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
				// table idx idx
				decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0x10:
			{
				bc.type = WASM_INST_CALL;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0x6c:
			{
				bc.type = WASM_INST_I32_MUL;
			}break;
			case 0x4e:
			{
				bc.type = WASM_INST_I32_GE_S;
			}break;
			case 0x4f:
			{
				bc.type = WASM_INST_I32_GE_U;
			}break;
			case 0xd:
			{
				bc.type = WASM_INST_BREAK_IF;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0xc:
			{
				bc.type = WASM_INST_BREAK;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;
			}break;
			case 0x3:
			{
				bc.type = WASM_INST_LOOP;
				// block return type
				ptr++;
			}break;
			case 0xb:
			{
				bc.type = WASM_INST_END;
			}break;
			case 0x2:
			{
				bc.type = WASM_INST_BLOCK;
				// block return type
				ptr++;
			}break;
			case 0x43:
			{
				bc.type = WASM_INST_F32_CONST;
				//ptr++;
				int* int_ptr = (int *) & code[ptr];
				*(int *)&bc.f32 = (*int_ptr << 24) | (((*int_ptr) & 0xff00) << 8) | (((*int_ptr) & 0xff0000) >> 8) | (((*int_ptr) >> 24));
				
				ptr += 4;
			}break;
			case 0x92:
			{
				bc.type = WASM_INST_F32_ADD;
			}break;
			case 0x6a:
			{
				bc.type = WASM_INST_I32_ADD;
			}break;
			case 0x6b:
			{
				bc.type = WASM_INST_I32_SUB;
			}break;
			case 0x3a:
			{
				bc.type = WASM_INST_I32_STORE8;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x38:
			{
				bc.type = WASM_INST_F32_STORE;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x36:
			{
				bc.type = WASM_INST_I32_STORE;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0xf:
			{
				bc.type = WASM_INST_RET;
			}break;
			case 0x2a:
			{
				bc.type = WASM_INST_F32_LOAD;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x28:
			{
				bc.type = WASM_INST_I32_LOAD;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x41:
			{
				bc.type = WASM_INST_I32_CONST;
				bc.i = decodeSLEB128(&code[ptr], &consumed);
				ptr += consumed;

			}break;
			default:
				ASSERT(0);
			}
			// decrementing because the statement counts only the end of the instructions, and not the beginning of the next one
			cur_ptr = (ptr - fi) - 1;
			if (cur_ptr == cur_st->end)
			{
				cur_st->end = bcs.size();

				cur_st++;
			}
			bc.end_code = cur_ptr + 1;
			bcs.emplace_back(bc);

		}

		own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) & cur_f->ir;
		ir_rep* cur_ir = ir_ar->begin();
		for (int i = cur_f->wasm_code_sect_idx; i < bcs.size(); i++)
		{
			while (cur_ir->start == cur_ir->end)
			{
				cur_ir->start = i;
				cur_ir->end = i;
				cur_ir++;
			}
			wasm_bc* cur_bc = bcs.begin() + i;
			if (cur_ir->start == cur_bc->start_code)
				cur_ir->start = i;
			if (cur_ir->end == cur_bc->end_code)
			{
				cur_ir->end = max(cur_ir->start, max(i - 1, 0));
				cur_ir++;
			}
		}

		ptr = next_func;
		
		//funcs[i] = wf;
		i++;
	}

	//wasm_func *wf = &funcs[func_idx];
	
	own_std::vector<func_decl*> &func_stack = dbg.func_stack;
	FOR_VEC(func, wasm_state->funcs)
	{
		func_decl* f = *func;
		if (f->name == func_start_name)
		{
			func_stack.emplace_back(f);
			break;
		}
	}
	ASSERT(func_stack.size() > 0);
	func_decl* cur_func = func_stack.back();

	block_linked* cur = nullptr;
	FOR_VEC(bc, bcs)
	{
		switch (bc->type)
		{
		case WASM_INST_LOOP:
		case WASM_INST_BLOCK:
		{
			cur = NewBlock(cur);
			cur->wbc = bc;
		}break;
		case WASM_INST_END:
		{
			if (cur)
			{
				FreeBlock(cur);
				cur->wbc->block_end = bc;
				cur = cur->parent;
			}

		}break;
		}
	}
	
	int a = 0;


	own_std::vector<wasm_stack_val> &wasm_stack = dbg.wasm_stack;
	wasm_stack.reserve(16);

	//FOR_VEC(bc, wf->bcs)
	wasm_bc *bc = &bcs[cur_func->wasm_code_sect_idx];
	//bc->one_time_dbg_brk = true;

	int stack_offset = 1000;
	*(int*)&mem_buffer[STACK_PTR_REG * 8] = stack_offset;

	auto stack_ptr_val = (long long *)&mem_buffer[stack_offset];

	for (int i = 0; i < total_args; i++)
	{
		*stack_ptr_val = args[i];
		stack_ptr_val++;
	}

	dbg.cur_bc = &bc;
	dbg.cur_func = cur_func;
	dbg.mem_buffer = mem_buffer;

	int first_start_offset = dbg.cur_func->wasm_stmnts[0].start;
	bcs[first_start_offset].one_time_dbg_brk = true;
	
	// WASM BYTECODE

	bool can_break = false;
	dbg.break_type = DBG_BREAK_ON_NEXT_STAT;

	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &cur_func->ir;
	//ir_rep* cur_ir = ir_ar->begin();

	while(!can_break)
	{
		int bc_idx = (long long)(bc - &bcs[0]);
		wasm_stack_val val = {};
		stmnt_dbg* cur_st = GetStmntBasedOnOffset(&dbg.cur_func->wasm_stmnts, bc_idx);
		ir_rep* cur_ir = GetIrBasedOnOffset(&dbg, bc_idx);
		bool is_different_stmnt = cur_st && dbg.cur_st && dbg.break_type == DBG_BREAK_ON_DIFF_STAT && cur_st->line != dbg.cur_st->line;

		if ( dbg.break_type == DBG_BREAK_ON_NEXT_BC || is_different_stmnt || bc->dbg_brk || bc->one_time_dbg_brk)
		{
			if (!cur_st)
				cur_st = &dbg.cur_func->wasm_stmnts[0];
			dbg.cur_st = cur_st;
			dbg.cur_ir = cur_ir;
			WasmOnArgs(&dbg);
			if (dbg.some_bc_modified)
			{
				dbg.some_bc_modified = false;
				continue;
			}
			bc->one_time_dbg_brk = false;
		}
		switch (bc->type)
		{
		case WASM_INST_NOP:
		{
			//WasmPrintVars(&dbg);
			int a = 0;
		}break;
		case WASM_INST_BREAK:
		{
			int i = 0;
			wasm_bc *label;
			while (i < bc->i)
			{
				cur = cur->parent;
				i++;
			}
			if (!cur)
			{
				can_break = true;
				break;
			}
			if (cur->wbc->type == WASM_INST_LOOP)
			{
				bc = cur->wbc + 1;
				continue;
			}
			bc = cur->wbc->block_end;
			continue;
			
		}break;
		case WASM_INST_BREAK_IF:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			if (top.s32 != 1)
				break;
			wasm_bc *label;
			i = 0;
			while (i < bc->i)
			{
				cur = cur->parent;
				i++;
			}
			if (!cur)
			{
				can_break = true;
				break;
			}
			bc = cur->wbc->block_end;
			continue;
		}break;
		case WASM_INST_INDIRECT_CALL:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			func_decl* call_f = dbg.wasm_state->funcs[top.u32];
			WasmDoCallInstruction(&dbg, &bc, &cur, call_f);
		}break;
		case WASM_INST_CALL:
		{
			func_decl* call_f = dbg.wasm_state->funcs[bc->i];
			if (IS_FLAG_ON(call_f->flags, FUNC_DECL_IS_OUTSIDER))
			{
				if (wasm_state->outsiders.find(call_f->name) != wasm_state->outsiders.end())
				{
					outsider_func_ptr func_ptr = wasm_state->outsiders[call_f->name];
					func_ptr(&dbg);
				}
				else
					ASSERT(0);

			}
			else
			{
				WasmDoCallInstruction(&dbg, &bc, &cur, call_f);
			}

			// assert is 32bit
			int a = 0;
		}break;
		case WASM_INST_RET:
		{
			dbg.func_stack.pop_back();
			if (dbg.func_stack.size() == 0)
			{
				can_break = true;
				break;
			}
			dbg.cur_func = dbg.func_stack.back();
			
			bc = dbg.return_stack.back();
			dbg.return_stack.pop_back();

			cur = dbg.block_stack.back();
			dbg.block_stack.pop_back();
		}break;
		case WASM_INST_I32_GE_U:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 = (int) (penultimate->u32 >= top.u32);
			int a = 0;
		}break;
		case WASM_INST_I32_STORE8:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			auto penultimate = wasm_stack.back();
			wasm_stack.pop_back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate.type == 0);
			*(char*)&mem_buffer[penultimate.s32] = top.s32;
			int a = 0;
		}break;
		case WASM_INST_F32_STORE:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			auto penultimate = wasm_stack.back();
			wasm_stack.pop_back();
			// assert is 32bit
			ASSERT(top.type == WSTACK_VAL_F32 && penultimate.type == 0);
			*(int*)&mem_buffer[penultimate.s32] = top.s32;
			int a = 0;
		}break;
		case WASM_INST_I32_STORE:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();

			auto penultimate = wasm_stack.back();
			wasm_stack.pop_back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate.type == 0);
			*(int*)&mem_buffer[penultimate.s32] = top.s32;
			int a = 0;
		}break;
		case WASM_INST_F32_CONST:
		{
			auto top = wasm_stack.back();
			val.type = WSTACK_VAL_F32;
			val.f32 = bc->f32;
			wasm_stack.emplace_back(val);
		}break;
		case WASM_INST_F32_LOAD:
		{
			auto w = &wasm_stack.back();
			// assert is 32bit
			ASSERT(w->type == WSTACK_VAL_INT);
			w->f32 = *(float*)&mem_buffer[w->s32];
			w->type = WSTACK_VAL_F32;
			int a = 0;
		}break;
		case WASM_INST_I32_LOAD:
		{
			auto w = &wasm_stack.back();
			// assert is 32bit
			ASSERT(w->type == WSTACK_VAL_INT);
			w->s32 = *(int*)&mem_buffer[w->s32];
			int a = 0;
		}break;
		case WASM_INST_I32_CONST:
		{
			val.type = WSTACK_VAL_INT;
			val.s32 = bc->i;
			wasm_stack.emplace_back(val);
		}break;
		case WASM_INST_LOOP:
		case WASM_INST_BLOCK:
		{
			cur = NewBlock(cur);
			cur->wbc = bc;
		}break;
		case WASM_INST_I32_MUL:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 *= top.s32;
		}break;
		case WASM_INST_F32_ADD:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == WSTACK_VAL_F32 && penultimate->type == WSTACK_VAL_F32);
			penultimate->f32 += top.f32;
		}break;
		case WASM_INST_I32_ADD:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 += top.s32;
		}break;
		case WASM_INST_I32_SUB:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->s32 -= top.s32;
		}break;
		case WASM_INST_END:
		{
			if (cur)
			{
				FreeBlock(cur);
				cur = cur->parent;
			}

		}break;
		default:
			ASSERT(0)
		}
		bc++;
	}

}

void WasmPushNameIntoArray(own_std::vector<unsigned char>* out, std::string name)
{
	own_std::vector<unsigned char> uleb;
	int name_len = name.size();
	uleb.clear();
	encodeSLEB128(&uleb, name_len);
	out->insert(out->end(), uleb.begin(), uleb.end());
	unsigned char* name_ptr = (unsigned char*)name.data();
	out->insert(out->end(), name_ptr, name_ptr + name_len);

}
void WasmInsertSectSizeAndType(own_std::vector<unsigned char>* out, char type)
{
	own_std::vector<unsigned char> uleb;
	uleb.clear();
	//encodeSLEB128(&uleb, out->size());
	GenUleb128(&uleb, out->size());

	// first inserting the size of the sect and then the type
	// so that, in memory, the type will come fircs
	out->insert(out->begin(), uleb.begin(), uleb.end());
	unsigned char sect_type = type;
	out->insert(0, sect_type);
}
void GenWasm(web_assembly_state* wasm_state)
{
	//ASSERT(func->func_bcode);
	own_std::vector<unsigned char>* ret = &wasm_state->final_buf;
	own_std::vector<unsigned char> type_sect;
	own_std::vector<unsigned char> func_sect;
	own_std::vector<unsigned char> import_sect;
	own_std::vector<unsigned char> element_sect;
	own_std::vector<unsigned char> table_sect;
	own_std::vector<unsigned char> exports_sect;
	own_std::vector<unsigned char> memory_sect;

	// magic number
	ret->emplace_back(0);
	ret->emplace_back(0x61);
	ret->emplace_back(0x73);
	ret->emplace_back(0x6d);
	// version
	ret->emplace_back(01);
	ret->emplace_back(0);
	ret->emplace_back(0);
	ret->emplace_back(0);

	// number of elem segmentns
	element_sect.emplace_back(1);
	// segment flags
	element_sect.emplace_back(0);
	// number of func ptrs
	WasmPushConst(WASM_TYPE_INT, 0, wasm_state->funcs.size(), &element_sect);
	element_sect.emplace_back(0xb);
	own_std::vector<unsigned char> uleb;
	FOR_VEC(func, wasm_state->funcs)
	{
		func_decl* f = *func;
		int idx = 0;
		FuncAddedWasm(wasm_state, f->name, &idx);
		
		uleb.clear();
		GenUleb128(&uleb, idx);
		INSERT_VEC(element_sect, uleb);

	}
	WasmInsertSectSizeAndType(&element_sect, 9);



	// ****
	// type of the section (type section)
	// ****
	int inserted = 0;
	//type sect
	FOR_VEC(t, wasm_state->types)
	{
		decl2* tp = *t;
		switch (tp->type.type)
		{
		case TYPE_FUNC_PTR:
		case TYPE_FUNC:
		{
			// func type
			type_sect.emplace_back(0x60);

			func_decl* fdecl = tp->type.fdecl;
			int num_of_args = 0;//fdecl->args.size();

			type_sect.emplace_back(num_of_args);
			/*
			FOR_VEC(arg, fdecl->args)
			{
				decl2* a = *arg;
				// for now we pushing all args as int
				type_sect.emplace_back(0x7f);
			}
			*/
			// no return type
			type_sect.emplace_back(0);
			/*
			if (fdecl->ret_type.type != TYPE_VOID)
			{
				// only one return type
				type_sect.emplace_back(0x01);
				// an int
				type_sect.emplace_back(0x7f);
			}
			*/
		}break;
		default:
			ASSERT(0)
		}
		tp->wasm_type_idx = inserted;
		inserted++;
	}

	uleb.clear();
	encodeSLEB128(&uleb, wasm_state->types.size());
	type_sect.insert(type_sect.begin(), uleb.begin(), uleb.end());

	uleb.clear();
	encodeSLEB128(&uleb, type_sect.size());

	// first inserting the size of the sect and then the type
	// so that, in memory, the type will come fircs
	type_sect.insert(type_sect.begin(), uleb.begin(), uleb.end());
	unsigned char sect_type = 1;
	type_sect.insert(0, sect_type);

	//****
	// import section
	//****
	inserted = 0;
	FOR_VEC(decl, wasm_state->imports)
	{
		decl2* d = *decl;
		switch (d->type.type)
		{
		case TYPE_FUNC:
		{
			func_decl* f = d->type.fdecl;
			WasmPushNameIntoArray(&import_sect, "funcs_import");
			WasmPushNameIntoArray(&import_sect, f->name);
			// import kind
			import_sect.emplace_back(0);
			// signature idx
			import_sect.emplace_back(0);

			inserted++;
		}break;
		default:
			ASSERT(0)
		}
	}
	uleb.clear();
	encodeSLEB128(&uleb, inserted);
	import_sect.insert(import_sect.begin(), uleb.begin(), uleb.end());
	WasmInsertSectSizeAndType(&import_sect, 2);

	//import_sect.emplace_back(2);

	//****
	//func section
	//****
	int funcs_inserted = 0;
	FOR_VEC(f, wasm_state->funcs)
	{
		decl2* d = (*f)->this_decl;

		// inserting idx of the type section
		uleb.clear();
		encodeSLEB128(&uleb, d->wasm_type_idx);

		func_sect.insert(func_sect.end(), uleb.begin(), uleb.end());

		(*f)->wasm_func_sect_idx = funcs_inserted;
		funcs_inserted++;

	}
	uleb.clear();
	encodeSLEB128(&uleb, funcs_inserted);
	// same scheme as the type sect, but here we have to insert
	// to maintain the order in memory: first type, bytes then total funcs
	func_sect.insert(func_sect.begin(), uleb.begin(), uleb.end());

	uleb.clear();
	encodeSLEB128(&uleb, func_sect.size());

	func_sect.insert(func_sect.begin(), uleb.begin(), uleb.end());

	sect_type = 3;
	func_sect.insert(0, sect_type);


	// ******
	// export section
	// ******

	uleb.clear();
	encodeSLEB128(&uleb, wasm_state->exports.size());
	exports_sect.insert(exports_sect.begin(), uleb.begin(), uleb.end());

	FOR_VEC(exp, wasm_state->exports)
	{
		decl2* ex = *exp;

		WasmPushNameIntoArray(&exports_sect, ex->name);
		/*
		int name_len = ex->name.size();
		uleb.clear();
		encodeSLEB128(&uleb, name_len);
		exports_sect.insert(exports_sect.end(), uleb.begin(), uleb.end());
		unsigned char* name_ptr = (unsigned char*)ex->name.data();
		exports_sect.insert(exports_sect.end(), name_ptr, name_ptr + name_len);
		*/

		switch (ex->type.type)
		{
		case TYPE_WASM_MEMORY:
		{
			exports_sect.emplace_back(2);
			// func type
			exports_sect.emplace_back(0);
		}break;
		case TYPE_FUNC:
		{
			uleb.clear();
			encodeSLEB128(&uleb, ex->type.fdecl->wasm_func_sect_idx);
			// func type
			exports_sect.emplace_back(0);
			// idx, i stiil dont know if this is in the type sect. or in the funct sect
			exports_sect.insert(exports_sect.end(), uleb.begin(), uleb.end());
		}break;
		default:
			ASSERT(0);
		}
	}
	// sect size
	uleb.clear();
	encodeSLEB128(&uleb, exports_sect.size());
	exports_sect.insert(exports_sect.begin(), uleb.begin(), uleb.end());
	sect_type = 7;
	exports_sect.insert(0, sect_type);


	table_sect.emplace_back(4);
	table_sect.emplace_back(5);
	table_sect.emplace_back(1);
	table_sect.emplace_back(0x70);
	table_sect.emplace_back(0x1);
	table_sect.emplace_back(0x0);
	table_sect.emplace_back(0x1);

	// ******
	// memory sect
	// ******
	memory_sect.emplace_back(5);
	memory_sect.emplace_back(3);
	// num of memories
	memory_sect.emplace_back(1);
	// flags
	memory_sect.emplace_back(0);
	// initial
	memory_sect.emplace_back(16);


	auto cur = NewBlock(nullptr);

	own_std::vector<unsigned char> final_code_sect;
	FOR_VEC(f, wasm_state->funcs)
	{
		own_std::vector<unsigned char> code_sect;
		func_decl* func = *f;
		int prev_size = code_sect.size();

		own_std::vector<ir_rep>* ir = (own_std::vector<ir_rep> *) & func->ir;
		bool use_ir = true;
		int total_of_locals = 0;

		func->wasm_code_sect_idx = final_code_sect.size();

		std::unordered_map<decl2*, int> decl_to_local_idx;
		int total_of_args = 0;
		int stack_size = 0;
		ir_val last_on_stack;
		wasm_gen_state gen;
		gen.wasm_state = wasm_state;
		gen.cur_func = func;
		gen.similar.reserve(8);
		ir_rep* cur_ir = ir->begin();
		while (cur_ir < ir->end())
		{
			WasmFromSingleIR(decl_to_local_idx, total_of_args, total_of_locals, cur_ir, code_sect, &stack_size, ir, &cur, &last_on_stack, &gen);

			cur_ir = cur_ir + (1 + (gen.advance_ptr));
		}
		func->stack_size = stack_size;
		//WasmPushConst(WASM_TYPE_INT, 0, 3, &code_sect);
		code_sect.emplace_back(0x0b);
		// end
		total_of_locals = 0;
		if (total_of_locals == 0)
		{
			// total of locals
			sect_type = total_of_locals;
			code_sect.insert(0, sect_type);
		}
		else
		{
			sect_type = 1;
			own_std::vector<unsigned char> aux;

			// only i32 type
			aux.insert(0, sect_type);
			//number of locals of type 32
			WasmPushImm(total_of_locals, &aux);
			// i32 type
			aux.emplace_back(0x7f);

			code_sect.insert(code_sect.begin(), aux.begin(), aux.end());
		}
		uleb.clear();
		// function size
		encodeSLEB128(&uleb, code_sect.size() - prev_size);
		code_sect.insert(code_sect.begin(), uleb.begin(), uleb.end());
		INSERT_VEC(final_code_sect, code_sect);

	}
	uleb.clear();
	// total funcs size
	encodeSLEB128(&uleb, wasm_state->funcs.size());
	final_code_sect.insert(final_code_sect.begin(), uleb.begin(), uleb.end());

	uleb.clear();
	// sect size
	encodeSLEB128(&uleb, final_code_sect.size());
	final_code_sect.insert(final_code_sect.begin(), uleb.begin(), uleb.end());

	sect_type = 10;
	final_code_sect.insert(0, sect_type);

	/*
	own_std::vector<unsigned char> type_sect;
	own_std::vector<unsigned char> code_sect;
	own_std::vector<unsigned char> func_sect;
	own_std::vector<unsigned char> exports_sect;
	own_std::vector<unsigned char> memory_sect;
	*/

	INSERT_VEC((*ret), type_sect);
	INSERT_VEC((*ret), import_sect);
	INSERT_VEC((*ret), func_sect);
	INSERT_VEC((*ret), table_sect);
	INSERT_VEC((*ret), memory_sect);
	INSERT_VEC((*ret), exports_sect);
	INSERT_VEC((*ret), element_sect);
	INSERT_VEC((*ret), final_code_sect);
	WriteFileLang("../../wabt/test.wasm", ret->begin(), ret->size());
	std::string code_str((char*)ret->begin(), ret->size());
	std::string encoded_str = base64_encode(code_str);

	std::string page =
		std::string(
			"\
<!DOCTYPE html>\
<html>\
<script>\
window.onload = start\n\
async function start(){\n\
	let inst = WebAssembly.instantiateStreaming(await fetch(\"test.wasm\")).then((inst)=>{\n\
	let {func_sum} = inst.exports.func_sum\n\
	let a = 0;\
	}).catch((err)=> console.log(err))\n\
}\n\
</script>;\
</html>\
");

	FOR_VEC(decl, wasm_state->imports)
	{
		decl2* d = *decl;

		if(d->type.type == TYPE_FUNC)
			wasm_state->funcs.insert(0, d->type.fdecl);
	}


	WasmSerialize(wasm_state, final_code_sect);
	int read;
	auto file = (unsigned char *)ReadEntireFileLang("../../wabt/dbg_wasm.dbg", &read);
	auto dfile = (dbg_file*)(file);
	wasm_interp winterp;
	WasmInterpInit(&winterp, file, read, wasm_state->lang_stat);


	int mem_size = 16000;
	auto buffer = (unsigned char*)malloc(mem_size);
	int src = 2000;
	int dst = 1500;
	long long args[] = { dst, src, 3 };
	*(int*)&buffer[src] = 0x12345678;
	WasmInterpRun(&winterp, buffer, mem_size, "main", args, 3);

	//WasmInterp(final_code_sect, buffer, mem_size, "wasm_test_func_ptr", wasm_state, args, 3);

	WriteFileLang("../../wabt/test.html", (void*)page.data(), page.size());
	//void FromIRToAsm()
}

void CreateAstFromFunc(lang_state *lang_stat, web_assembly_state *wasm_state, func_decl *f)
{
	ast_rep *ast = AstFromNode(lang_stat, f->func_node, f->scp);
	ASSERT(ast->type == AST_FUNC);
	own_std::vector<ir_rep>* ir = (own_std::vector<ir_rep> *) &f->ir;
	GetIRFromAst(lang_stat, ast, ir);

	//auto func = (int(*)(int)) lang_stat->GetCodeAddr(f->code_start_idx);
	//auto ret =func(2);
	//f->wasm_scp = NewScope(lang_stat, f->scp->parent);
	//AddFuncToWasm(f);

}

int InitLang(lang_state *lang_stat, void *alloc_addr, void *free_addr, void *page_alloc_addr)
{
    __lang_globals.alloc = (void *(*)(void*, int, void *)) alloc_addr;
    __lang_globals.data = page_alloc_addr;
    __lang_globals.free = (void(*)(void *, void *, int)) free_addr;
	auto test = lang_state();
	*lang_stat = test;
	lang_stat->code_sect.reserve(256);

	lang_stat->internal_funcs_addr["ForeignFuncAdd"] = (char *) ForeignFuncAdd;
	lang_stat->internal_funcs_addr["get_node_type"] = (char *) GetNodeType;
	lang_stat->internal_funcs_addr["is_node_type"] = (char *) IsNodeType;
	lang_stat->internal_funcs_addr["get_node_l"] = (char *) GetNodeL;
	lang_stat->internal_funcs_addr["get_node_r"] = (char *) GetNodeR;
	lang_stat->internal_funcs_addr["set_node_ident"] = (char *) SetNodeIdent;
	lang_stat->internal_funcs_addr["free"] = (char *) free;
	lang_stat->internal_funcs_addr["malloc"] = (char *) malloc;
	lang_stat->internal_funcs_addr["memcpy"] = (char *) memcpy;
	lang_stat->internal_funcs_addr["memset"] = (char *) memset;
	lang_stat->internal_funcs_addr["sqrtf"] = (char *) sqrtf;
	lang_stat->internal_funcs_addr["pow"] = (char*)powf;
	lang_stat->internal_funcs_addr["cosf"] = (char *) cosf;

	lang_stat->internal_funcs_addr["get_node_type_checked2"] = (char *) GetProcessedNodeType;
	//CreateWindowEx()
	//glfwInit();
	//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | BACKGROUND_BLUE);
	//printf("hello");

    
	lang_stat->max_nd = 20000 * 2;

	//lang_stat->node_arena = InitSubSystems(64 * 1024 * 1024);
	lang_stat->max_misc = 16 * 1024 * 1024;
	lang_stat->misc_arena = (char*)VirtualAlloc(0, lang_stat->max_misc, MEM_COMMIT, PAGE_READWRITE);
	lang_stat->node_arena = (node*)VirtualAlloc(0, lang_stat->max_nd * sizeof(node), MEM_COMMIT, PAGE_READWRITE);
    
	/*
	lang_stat->structs = (LangLangArray<type_struct2> *)malloc(sizeof(LangLangArray<int>));
	lang_stat->template_strcts = (LangLangArray<type_struct2> *)malloc(sizeof(LangLangArray<int>));
	memset(lang_stat->structs, 0, sizeof(LangArray<int>));
	memset(lang_stat->template_strcts, 0, sizeof(LangArray<int>));
	*/


	auto a = ParseString(lang_stat, "a = a * a + a * a + a * a");

	auto dummy_decl = new decl2();
	memset(dummy_decl, 0, sizeof(decl2));

	// adding sizeof builtin
	lang_stat->root = NewScope(lang_stat, nullptr);
	lang_stat->funcs_scp = NewScope(lang_stat, nullptr);

	type2 tp;
	tp.type = enum_type2::TYPE_FUNC;
	func_decl* sz_of_fdecl = (func_decl*)AllocMiscData(lang_stat, sizeof(func_decl));
	memset(sz_of_fdecl, 0, sizeof(func_decl));
	sz_of_fdecl->ret_type.type = enum_type2::TYPE_INT;
	tp.fdecl = sz_of_fdecl;
	tp.fdecl->flags |= FUNC_DECL_INTERNAL;
	tp.fdecl->name = std::string("sizeof");
	tp.fdecl->args.push_back(dummy_decl);

	lang_stat->root->vars.push_back(NewDecl(lang_stat, "sizeof", tp));

	sz_of_fdecl = (func_decl*)AllocMiscData(lang_stat, sizeof(func_decl));
	memset(sz_of_fdecl, 0, sizeof(func_decl));
	tp.fdecl = sz_of_fdecl;
	sz_of_fdecl->flags |= FUNC_DECL_INTERNAL;
	tp.fdecl->name = std::string("enum_count");
	lang_stat->root->vars.push_back(NewDecl(lang_stat, "enum_count", tp));
	lang_stat->root->vars.push_back(NewDecl(lang_stat, "get_type_data", tp));

	tp.type = enum_type2::TYPE_VOID;
	lang_stat->void_decl = NewDecl(lang_stat, "void", tp);

	tp.type = enum_type2::TYPE_U64;
	lang_stat->i64_decl = NewDecl(lang_stat, "i64", tp);

	tp.type = enum_type2::TYPE_U64;
	lang_stat->u64_decl = NewDecl(lang_stat, "u64", tp);

	tp.type = enum_type2::TYPE_S64;
	lang_stat->s64_decl = NewDecl(lang_stat, "s64", tp);

	tp.type = enum_type2::TYPE_U32;
	lang_stat->u32_decl = NewDecl(lang_stat, "u32", tp);

	tp.type = enum_type2::TYPE_S32;
	lang_stat->s32_decl = NewDecl(lang_stat, "s32", tp);

	tp.type = enum_type2::TYPE_U16;
	lang_stat->s16_decl = NewDecl(lang_stat, "u16", tp);

	tp.type = enum_type2::TYPE_S16;
	lang_stat->u16_decl = NewDecl(lang_stat, "s16", tp);

	tp.type = enum_type2::TYPE_U8;
	lang_stat->u8_decl = NewDecl(lang_stat, "u8", tp);

	tp.type = enum_type2::TYPE_S8;
	lang_stat->s8_decl = NewDecl(lang_stat, "s8", tp);

	tp.type = enum_type2::TYPE_BOOL;
	lang_stat->bool_decl = NewDecl(lang_stat, "bool", tp);

	tp.type = enum_type2::TYPE_F32;
	lang_stat->f32_decl = NewDecl(lang_stat, "f32", tp);

	tp.type = enum_type2::TYPE_F64;
	lang_stat->f64_decl = NewDecl(lang_stat, "f64", tp);

	tp.type = enum_type2::TYPE_CHAR;
	lang_stat->char_decl = NewDecl(lang_stat, "char", tp);

	// inserting builtin types
	{
		NewTypeToSection(lang_stat, "s64", TYPE_S64);
		NewTypeToSection(lang_stat, "s32", TYPE_S32);
		NewTypeToSection(lang_stat, "s16", TYPE_S16);
		NewTypeToSection(lang_stat, "s8", TYPE_S8);
		NewTypeToSection(lang_stat, "u64", TYPE_U64);
		NewTypeToSection(lang_stat, "u32", TYPE_U32);
		NewTypeToSection(lang_stat, "u16", TYPE_U16);
		NewTypeToSection(lang_stat, "u8", TYPE_U8);
		NewTypeToSection(lang_stat, "bool", TYPE_BOOL);
		NewTypeToSection(lang_stat, "void", TYPE_VOID);
		NewTypeToSection(lang_stat, "str_lit", TYPE_STR_LIT);
	}
	
	srand(time(0));
	// getting the compiler exe path 
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);

	// addind the file name to the path
	std::string exe_dir = buffer;
	int last_bar_comp = exe_dir.find_last_of('\\');
	/*
	std::string file_name_dir = std::string(argv[1]);
	int last_bar_main = file_name_dir.find_last_of('/');
	*/
	//std::string dir = exe_dir.substr(0, last_bar_comp) + "\\" + file_name_dir.substr(0, last_bar_main);
	std::string dir = exe_dir.substr(0, last_bar_comp);
	lang_stat->work_dir = dir;

	//std::string file_name = file_name_dir.substr(last_bar_main+1);

	lang_stat->gen_wasm = true;
	//auto base_fl = AddNewFile(lang_stat, "Core/base.lng");
	//tp.type = enum_type2::TYPE_IMPORT;
	//tp.imp = NewImport(lang_stat, import_type::IMP_IMPLICIT_NAME, "", base_fl);
	//lang_stat->base_lang = NewDecl(lang_stat, "base", tp);

	AddNewFile(lang_stat, "game.lng");
	//AddNewFile(lang_stat, "Core/tests.lng");
	//AddNewFile(lang_stat, "Core/player.lng");
	//AddNewFile(file_name);
	

	int cur_f = 0;

	int iterations = 0;
	while(true)
	{
		lang_stat->something_was_declared = false;
		
		for(cur_f = 0; cur_f < lang_stat->files.size(); cur_f++)
		{
			auto f = lang_stat->files[cur_f];
			lang_stat->cur_file = f;
			DescendNameFinding(lang_stat, f->s, f->global);
			int a = 0;
			iterations++;
		}
		if(!lang_stat->something_was_declared)
			break;
	}
	lang_stat->flags = PSR_FLAGS_REPORT_UNDECLARED_IDENTS;

	for(cur_f = 0; cur_f < lang_stat->files.size(); cur_f++)
	{
		auto f = lang_stat->files[cur_f];
		lang_stat->cur_file = f;
		DescendNameFinding(lang_stat, f->s, f->global);
	}
	//DescendIndefinedIdents(s, &global);
	
	if(IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_ERRO_REPORTED))
		ExitProcess(1);

	lang_stat->flags |= PSR_FLAGS_ASSIGN_SAVED_REGS;
	lang_stat->flags |= PSR_FLAGS_AFTER_TYPE_CHECK;

	//CreateBaseFileCode(lang_stat);

    // comp time function 
    for ( auto it = lang_stat->comp_time_funcs.begin(); it != lang_stat->comp_time_funcs.end(); ++it )
    {

        func_decl *fdecl = it->second;
		auto f = fdecl->from_file;
		lang_stat->cur_file = f;
		lang_stat->lhs_saved = 0;
		lang_stat->call_regs_used = 0;
		DescendNode(lang_stat, fdecl->func_node, fdecl->scp);

        lang_stat->call_regs_used = 0;
        own_std::vector<func_byte_code*> func_byte_code;
        GenFuncByteCode(lang_stat, fdecl, func_byte_code);

        fdecl->code = (machine_code *)__lang_globals.alloc(__lang_globals.data, sizeof(machine_code), nullptr);
		memset(fdecl->code, 0, sizeof(machine_code));
        FromByteCodeToX64(lang_stat, &func_byte_code, *fdecl->code);


        CompleteMachineCode(lang_stat, *fdecl->code);
        int a = 0;

        //print_key_value(key, value);
    }
	
	for(cur_f = 0; cur_f < lang_stat->files.size(); cur_f++)
	{
		auto f = lang_stat->files[cur_f];
		lang_stat->cur_file = f;
		lang_stat->lhs_saved = 0;
		lang_stat->call_regs_used = 0;
		//std::string scp_str = lang_stat->root->Print(0);
		//printf("file: %s, scp: \n %s", f->name.c_str(), scp_str.c_str());
		DescendNode(lang_stat, f->s, f->global);

		/*
		TypeCheckTree(f->s, f->global, []()
		{
			while (true)
			{
				auto msg = works.GetMessageA();
				if (!msg) continue;
				if (msg->type == msg_type::MSG_DONE) break;
			}
		});
		*/
		lang_stat->call_regs_used = 0;
		own_std::vector<func_byte_code*>all_funcs = GetFuncs(lang_stat, lang_stat->funcs_scp);
		machine_code code;
		FromByteCodeToX64(lang_stat, &all_funcs, code);

		auto exec_funcs = CompleteMachineCode(lang_stat, code);
	}

	

	char msg_hdr[256];
    web_assembly_state wasm_state;
	wasm_state.lang_stat = lang_stat;

    auto mem_decl = (decl2 *)AllocMiscData(lang_stat, sizeof(decl2));
    mem_decl->name = std::string("mem");
    mem_decl->type.type = TYPE_WASM_MEMORY;
    wasm_state.exports.emplace_back(mem_decl);

	FOR_VEC(f_ptr, lang_stat->outsider_funcs)
	{
		auto f = *f_ptr;

		if (f->name != "js_print")
			continue;
		auto fdecl = (decl2 *)AllocMiscData(lang_stat, sizeof(decl2));
		fdecl->type.type = TYPE_FUNC;
		fdecl->name = f->name;
		fdecl->type.fdecl = f;
		f->this_decl = fdecl;
		wasm_state.imports.emplace_back(f->this_decl);
	}
	FOR_VEC(f_ptr, lang_stat->func_ptrs_decls)
	{
		AddFuncToWasm(&wasm_state, *f_ptr, false);
		CreateAstFromFunc(lang_stat, &wasm_state, *f_ptr);
	}
	FOR_VEC(f_ptr, lang_stat->global_funcs)
	{
		auto f = *f_ptr;
        //if(f->name == "wasm_test_func_ptr")
        //{
			if (FuncAddedWasm(&wasm_state, f->name))
				continue;

			AddFuncToWasm(&wasm_state, f);
			CreateAstFromFunc(lang_stat, &wasm_state, f);
        //}

		if (IS_FLAG_ON(f->flags, NODE_FLAGS_FUNC_TEST))
		{
			auto func = (int(*)(char*, long long*)) lang_stat->GetCodeAddr(f->code_start_idx);
			GetFuncBasedOnAddr(lang_stat, (unsigned long long)func);
			GetFuncAddrBasedOnName(lang_stat, "_own_memset");
			char ret_success = 0;
			long long add_info = 0;
			int reached = func(&ret_success, &add_info);

			lang_stat->cur_file = f->from_file;

			if (ret_success == 0)
			{
				token2* t = f->func_node->t;
				REPORT_ERROR(t->line, t->line_offset, VAR_ARGS("test failed! reached %d, info %Ix\n", reached, add_info))
                ASSERT(0);
			}
		}
	}

    GenWasm(&wasm_state);

    return 0;
}
