#include "compile.h"
#include <iostream>
#include <string>
#include <cstring>
#include <chrono> 
#include <thread>
#include <vector>
#include <math.h>
#include <unordered_map>
#include <setjmp.h>

#include "Array.cpp"
#define MAX_PATH 256
#define TCHAR char


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

#define HEAP_START 19000
//#define MEM_ALLOC_ADDR 17000
#define MEM_PTR_CUR_ADDR 18000
#define MEM_PTR_MAX_ADDR 18008

#define DATA_SECT_MAX 2048
#define DATA_SECT_OFFSET 180000
#define BUFFER_MEM_MAX (DATA_SECT_OFFSET + DATA_SECT_MAX)

//#define DEBUG_GLOBAL_NOT_FOUND 

struct comp_time_type_info
{
	//var arg struct, defined at base.lng
	long long val;
	char ptr;
	void* ptr_to_type_data;

};

struct dbg_state;
struct block_linked;

typedef void* (*FreeTypeFunc)(void* this_ptr, void* ptr);
typedef void* (*AllocTypeFunc)(void* this_ptr, int sz);

struct global_variables_lang
{
	char* main_buffer;
	bool string_use_page_allocator = false;

	void* data;
	AllocTypeFunc alloc;
	FreeTypeFunc free;

	block_linked* blocks;
	int total_blocks;
	int cur_block;


	//LangArray<type_struct2> *structs;
	//LangArray<type_struct2> *template_strcts;

	//page_allocator *cur_page_allocator = nullptr;
	//sub_systems_enum string_sub_system_operations = sub_systems_enum::STRING;

}__lang_globals;

#include "own_vector.cpp" 


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

#include "memory.cpp"
#include "rel_utils.h"
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

void split(own_std::string& s, char separator, own_std::vector<own_std::string>& ret, own_std::string* aux)
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
struct web_assembly_state;
struct wasm_interp;
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
	bool release;


	
	jmp_buf jump_buffer;
	bool ir_in_stmnt;
	func_decl* cur_func;

	int cur_strct_constrct_size_per_statement;
	int cur_strct_ret_size_per_statement;

	int cur_spilled_offset;

	unsigned short regs[32];
	unsigned short arg_regs[MAX_ARGS];

	node* cur_stmnt;
	own_std::vector<node*> global_decl_not_found;
	own_std::vector<scope**> scope_stack;
	own_std::vector<func_decl*> func_ptrs_decls;
	own_std::vector<func_decl*> outsider_funcs;
	own_std::vector<own_std::vector<token2>*> allocated_vectors;
	func_decl* plugins_for_func;
	std::unordered_map<func_decl*, func_decl*> comp_time_funcs;
	//std::unordered_map<std::string *, unsigned int> symbols_offset_on_type_sect;

	std::unordered_map<std::string, char*> internal_funcs_addr;

	own_std::string liz_file_path;

	wasm_interp* winterp;


	own_std::vector<unit_file*> files;

	scope* funcs_scp;

	unit_file* cur_file;

	int parentheses_opened;
	int scopes_opened;

	own_std::vector<char> data_sect;
	own_std::vector<char> type_sect;
	own_std::vector<unsigned char, 1> code_sect;

	int already_inserted_of_type_sect_in_code_sect;

	own_std::vector<char> type_sect_str_table;
	own_std::vector<own_std::vector<char>> type_sect_str_table2;


	std::vector<own_std::string> struct_scope;


	int type_sect_str_tbl_sz;

	decl2* base_lang;


	own_std::vector<machine_reloc> type_sect_rels;

	own_std::vector<machine_sym> type_sect_syms;

	own_std::vector<func_decl*> global_funcs;

	own_std::string work_dir;
	own_std::string exe_dir;

	int call_regs_used;
	int max_bytes_in_ar_lit;
	int lhs_saved;
	int cur_ar_lit_offset;
	int cur_ar_lit_sz;
	int cur_per_stmnt_strct_val_sz;
	int cur_per_stmnt_strct_val_offset;

	own_std::string linker_options;
	web_assembly_state* wasm_state;

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

	node* zero_int_node;

	char* GetCodeAddr(int offset)
	{
		return (char*)&code_sect[offset + type_sect.size()];
	}

};

struct type_struct2;


char* ReadEntireFileLang(char* name, unsigned int* read_out)
{
#ifdef LINUX
	int fh, n;
	struct stat v;

	fh = open(name, O_RDONLY);
	if (fh == -1) {
		perror("open");
		return nullptr;
	}

	/* first find the size of file .. use stat() system call */
	stat(name, &v);

	char* string = (char*)__lang_globals.alloc(__lang_globals.data, v.st_size + 1);
	ASSERT(string);
	int res=read(fh, string, v.st_size);
	if (res == -1) {
		perror("read");
		return nullptr;
	}
	close(fh);

	string[v.st_size + 1] = 0;
	*read_out = v.st_size + 1;
	return string;
#else
	HANDLE file = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (file == nullptr )
	{
		printf("file \"%s\" not found", name);
		ExitProcess(0);
	}
	if (file == INVALID_HANDLE_VALUE)
	{
		TCHAR buffer[MAX_PATH] = { 0 };
		GetCurrentDirectory(MAX_PATH, buffer);
		//TCHAR full_file_path[MAX_PATH] = { 0 };
		//GetFullPathName(filename, MAX_PATH, fullFilename, nullptr);


		printf("file \"%s\" was invalid handle, last error %d, full exe path %s", name, GetLastError(), buffer);

		ExitProcess(0);
	}

	LARGE_INTEGER file_size;
	GetFileSizeEx(file, &file_size);
	char* f = (char*)__lang_globals.alloc(__lang_globals.data, file_size.QuadPart + 1);

	int bytes_read;
	ReadFile(file, (void*)f, file_size.QuadPart, (LPDWORD)&bytes_read, 0);
	f[file_size.QuadPart] = 0;

	*read = bytes_read;

	CloseHandle(file);
	return f;
#endif
}
void WriteFileLang(char* name, void* data, int size)
{
#ifdef LINUX
	int fd = open(name, O_CREAT | O_WRONLY );
	if( fd == -1 ) {
		printf( "Error opening file: errno: %d - %s\n", errno, strerror( errno ) );
		return;
	}
	write(fd, data, size);
#else
	HANDLE file = CreateFile(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	int written;
	WriteFile(file, data, size, (LPDWORD)&written, nullptr);

	CloseHandle(file);
#endif
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
#include "memory.h"

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
	WASM_INST_F32_SUB,
	WASM_INST_F32_LOAD,
	WASM_INST_I32_CONST,
	WASM_INST_I32_LOAD,
	WASM_INST_I32_LOAD_8_S,
	WASM_INST_I32_STORE,
	WASM_INST_I32_STORE8,
	WASM_INST_I32_SUB,
	WASM_INST_I32_REMAINDER_U,
	WASM_INST_I32_REMAINDER_S,
	WASM_INST_I32_ADD,
	WASM_INST_I32_AND,
	WASM_INST_I32_OR,
	WASM_INST_I32_MUL,


	WASM_INST_I64_LOAD,
	WASM_INST_I64_STORE,

	WASM_INST_F32_GE,
	WASM_INST_F32_LE,
	WASM_INST_F32_LT,
	WASM_INST_F32_GT,
	WASM_INST_F32_DIV,
	WASM_INST_F32_MUL,
	WASM_INST_F32_NE,
	WASM_INST_F32_EQ,

	WASM_INST_CAST_S64_2_F32,
	WASM_INST_CAST_S32_2_F32,
	WASM_INST_CAST_U32_2_F32,
	WASM_INST_CAST_F32_2_S32,

	WASM_INST_I32_GE_U,
	WASM_INST_I32_GE_S,
	WASM_INST_I32_LE_U,
	WASM_INST_I32_LE_S,
	WASM_INST_I32_LT_U,
	WASM_INST_I32_LT_S,
	WASM_INST_I32_GT_U,
	WASM_INST_I32_GT_S,
	WASM_INST_I32_DIV_U,
	WASM_INST_I32_DIV_S,
	WASM_INST_I32_NE,
	WASM_INST_I32_EQ,
	WASM_INST_BLOCK,
	WASM_INST_END,
	WASM_INST_LOOP,
	WASM_INST_BREAK,
	WASM_INST_BREAK_IF,
	WASM_INST_NOP,
	WASM_INST_CALL,
	WASM_INST_INDIRECT_CALL,
	WASM_INST_RET,
	WASM_INST_DBG_BREAK
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
own_std::string GetTypeInfoLangArrayStr(own_std::string str)
{
	ASSERT(false);
	/*
	own_std::string final_ret;
	final_ret.reserve(64);

	for (auto c : str)
	{

		char aux[16];
		snprintf(aux, 16, "\'\\x%x\', ", (unsigned char)c);
		final_ret.append(aux);
	}
	*/
	return "";
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

char* std_str_to_heap(lang_state*, own_std::string* str);

void NewTypeToSection(lang_state* lang_stat, char* type_name, enum_type2 idx)
{
	char char_buffer[512];
	snprintf(char_buffer, 512, "$$%s", type_name);
	own_std::string final_name = char_buffer;
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
	own_std::string cur_rel_name = own_std::string(c->name);
	FOR_VEC(sym, symbols)
	{
		if (cur_rel_name == own_std::string(sym->name))
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
	n->t->str = own_std::string(data, len);
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
struct wasm_interp
{
	dbg_state *dbg;
	own_std::vector<func_decl*> funcs;
	std::unordered_map<std::string, OutsiderFuncType> outsiders;
};
struct web_assembly_state
{
	own_std::vector<decl2*> types;
	own_std::vector<func_decl*> funcs;
	own_std::vector<decl2*> imports;
	own_std::vector<decl2*> exports;

	std::unordered_map<std::string, OutsiderFuncType> outsiders;

	own_std::vector<unsigned char> final_buf;
	own_std::string wasm_dir;
	own_std::string folder_name;

	lang_state* lang_stat;
};
struct wasm_gen_state
{
	own_std::vector<ir_val*> similar;
	int advance_ptr;
	int strcts_construct_stack_offset;
	int strcts_ret_stack_offset;
	int to_spill_offset;

	func_decl* cur_func;
	web_assembly_state* wasm_state;

	int cur_line;
};
void WasmPushIRVal(wasm_gen_state* gen_state, ir_val* val, own_std::vector<unsigned char>& code_sect, bool = false);

func_decl* WasmGetFuncAtIdx(wasm_interp* state, int idx)
{
	return (state->funcs[idx]);
}
func_decl *FuncAddedWasmInterp(wasm_interp* interp, own_std::string name)
{
	int i = 0;
	bool found = false;
	FOR_VEC(func, interp->funcs)
	{
		func_decl* f = *func;
		if (f->name == name)
		{
			return f;
		}
	}

	return nullptr;
}
bool FuncAddedWasm(web_assembly_state* state, own_std::string name, int* idx = nullptr)
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
		int f_inst = WASM_I32_CONST + size + 2;
		out->emplace_back(f_inst);
		out->emplace_back(offset & 0xff);
		out->emplace_back(offset >> 8);
		out->emplace_back(offset >> 16);
		out->emplace_back(offset >> 24);

	}
	else
	{
		ASSERT(0);
	}

}
// if it is 64 bit load, make size 1

void WasmStoreInst(lang_state *lang_stat, own_std::vector<unsigned char>& code_sect, int size, char inst = WASM_STORE_OP)
{
	int final_inst = 0;
	if (lang_stat->release && size == 8)
		size = 4;
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
			final_inst = 0x28;
			break;
		case 8:
			final_inst = 0x29;
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
			final_inst = 0x36;
			break;
		case 8:
			final_inst = 0x37;
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
void WasmPushLoadOrStore(char size, char type, char op_code, int offset, own_std::vector<unsigned char>* out)
{
	WasmPushConst(WASM_TYPE_INT, 0, offset, out);
	ASSERT(size != 8);
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

void WasmGenBinOpImmToReg(lang_state *lang_stat, int reg, char reg_sz, int imm, own_std::vector<unsigned char>& code_sect, char inst_32bit)
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

	WasmStoreInst(lang_stat, code_sect, size);
}
void WasmGenRegToMem(lang_state *lang_stat, byte_code* bc, own_std::vector<unsigned char>& code_sect)
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

	WasmStoreInst(lang_stat, code_sect, size);
}

static own_std::string base64_encode(const own_std::string& in) {

	own_std::string out;

	int val = 0, valb = -6;
	for (int i = 0; i < in.size(); i++)
	{
		unsigned char c = in.data_[i];
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

static own_std::string base64_decode(const own_std::string& in) {

	own_std::string out;

	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) {
		T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
	}

	int val = 0, valb = -8;
	for (int i = 0; i < in.size(); i++)
	{
		unsigned char c = in.data_[i];
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
	__lang_globals.cur_block--; (__lang_globals.data, (void*)block);
}
block_linked* NewBlock(block_linked* parent)
{
	ASSERT((__lang_globals.cur_block + 1) < __lang_globals.total_blocks)
	auto ret = __lang_globals.blocks + __lang_globals.cur_block;
	memset(ret, 0, sizeof(block_linked));
	__lang_globals.cur_block++;
	ret->parent = parent;
	return ret;

}
void WesmGenMovR(lang_state *lang_stat, int src_reg, int dst_reg, own_std::vector<unsigned char>& code_sect)
{

	// pushing dst reg offset in mem
	WasmPushConst(WASM_TYPE_INT, 0, dst_reg * 8, &code_sect);
	// loading rhs 
	WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, src_reg * 8, &code_sect);


	WasmStoreInst(lang_stat, code_sect, 0);
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

void WasmPushWhateverIRValIs(lang_state *lang_stat, std::unordered_map<decl2*, int>& decl_to_local_idx, ir_val* val, ir_val* last_on_stack, own_std::vector<unsigned char>& code_sect)
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
		WasmStoreInst(lang_stat, code_sect, 0, WASM_LOAD_OP);

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
		case T_COND_AND:
		case T_AMPERSAND:
		{
			code_sect.emplace_back(0x71);
		}break;
		case T_COND_OR:
		{
			code_sect.emplace_back(0x72);
		}break;
		case T_DIV:
		{
			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x6e);
			// signed
			else
				code_sect.emplace_back(0x6d);
		}break;
		case T_GREATER_THAN:
		{
			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x4b);
			// signed
			else
				code_sect.emplace_back(0x4a);
		}break;
		case T_LESSER_EQ:
		{
			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x4d);
			// signed
			else
				code_sect.emplace_back(0x4c);
		}break;
		case T_LESSER_THAN:
		{
			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x49);
			// signed
			else
				code_sect.emplace_back(0x48);
		}break;
		case T_GREATER_EQ:
		{

			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x4f);
			// signed
			else
				code_sect.emplace_back(0x4e);
		}break;
		case T_PIPE:
		{
			code_sect.emplace_back(0x72);
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
		case T_MINUS:
		{
			code_sect.emplace_back(0x6b);
		}break;
		case T_POINT:
		case T_PLUS:
		{
			code_sect.emplace_back(0x6a);
		}break;
		case T_PERCENT:
		{
			// unsigned
			if (is_unsigned)
				code_sect.emplace_back(0x70);
			// signed
			else
				code_sect.emplace_back(0x6f);

		}break;
		default:
			ASSERT(0);
		}
	}
	else
	{
		switch (op)
		{
		case T_GREATER_THAN:
		{
			code_sect.emplace_back(0x5e);
		}break;
		case T_LESSER_EQ:
		{
			code_sect.emplace_back(0x5f);
		}break;
		case T_LESSER_THAN:
		{
			code_sect.emplace_back(0x5d);
		}break;
		case T_GREATER_EQ:
		{
			code_sect.emplace_back(0x60);
		}break;
		case T_COND_NE:
		{
			code_sect.emplace_back(0x5c);
		}break;
		case T_COND_EQ:
		{
			code_sect.emplace_back(0x5b);
		}break;
		case T_MUL:
		{
			code_sect.emplace_back(0x94);
		}break;
		case T_DIV:
		{
			code_sect.emplace_back(0x95);
		}break;
		case T_MINUS:
		{
			code_sect.emplace_back(0x93);
		}break;
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

		if (lhs.type == IR_TYPE_REG)
		{
			if(lhs.ptr == -1)
				lhs.ptr = 1;
			deref = true;
		}
		if (op == T_POINT)
		{
			WasmPushIRVal(gen_state, &lhs, code_sect, deref);
			WasmPushConst(WASM_LOAD_INT, 0, rhs.decl->offset, &code_sect);
			WasmPushInst(op, lhs.is_unsigned, code_sect);

			//if (deref && rhs.ptr != -1)
				//WasmStoreInst(code_sect, 0, WASM_LOAD_OP);
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
		ASSERT( lhs.is_unsigned == rhs.is_unsigned || is_struct || is_float);

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

void WasmBeginStack(lang_state *lang_stat, own_std::vector<unsigned char>& code_sect, int stack_size)
{
	WesmGenMovR(lang_stat, STACK_PTR_REG, BASE_STACK_PTR_REG, code_sect);
	// sub inst
	WasmGenBinOpImmToReg(lang_stat, STACK_PTR_REG, 4, stack_size, code_sect, 0x6b);
}
void WasmEndStack(lang_state *lang_stat, own_std::vector<unsigned char>& code_sect, int stack_size)
{
	// sum inst
	WasmGenBinOpImmToReg(lang_stat, STACK_PTR_REG, 4, stack_size, code_sect, 0x6a);
	WesmGenMovR(lang_stat, STACK_PTR_REG, BASE_STACK_PTR_REG, code_sect);
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
		WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, (RET_1_REG + val->reg) * 8 , &code_sect);
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
	case IR_TYPE_STR_LIT:
	{

		lang_state* lang_stat = gen_state->wasm_state->lang_stat;
		int data_offset = lang_stat->data_sect.size();
		InsertIntoDataSect(lang_stat, val->str, strlen(val->str) + 1);
		WasmPushConst(WASM_LOAD_INT, 0, DATA_SECT_OFFSET + data_offset , &code_sect);
		val->on_data_sect_offset = data_offset;
	}break;
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
		int base_offset = 0;
		switch (val->on_stack_type)
		{
		case ON_STACK_STRUCT_RET:
		{
			base_offset = gen_state->strcts_ret_stack_offset;
		}break;
		case ON_STACK_STRUCT_CONSTR:
		{
			base_offset = gen_state->strcts_construct_stack_offset;
		}break;
		case ON_STACK_SPILL:
		{
			base_offset = gen_state->to_spill_offset;
		}break;
		default:
			ASSERT(0)
		}
		
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
			if (IS_FLAG_ON(val->decl->flags, DECL_ABSOLUTE_ADDRESS))
			{
				WasmPushConst(WASM_LOAD_INT, 0, val->decl->offset, &code_sect);
			}
			else
			{
				//int idx = decl_to_local_idx[val->decl];
				WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
				WasmPushConst(WASM_LOAD_INT, 0, val->decl->offset, &code_sect);
				code_sect.emplace_back(0x6a);
			}
		}
		deref = false;
	}break;
	default:
		ASSERT(0)
	}
	int deref_times = val->deref;
	while ((deref_times > 0) && val->type != IR_TYPE_INT && val->type != IR_TYPE_F32 && val->type != IR_TYPE_STR_LIT)
	{

		int reg_sz = val->reg_sz;
		int inst = WASM_LOAD_OP;
		if (IsIrValFloat(val) && deref_times == 1)
			inst = WASM_LOAD_F32_OP;
		if (deref_times > 1)
		{
			reg_sz = 8;
			if(gen_state->wasm_state->lang_stat->release)
				reg_sz = 4;
		}
		WasmStoreInst(gen_state->wasm_state->lang_stat, code_sect, reg_sz, inst);
		//if(!deref)
			deref_times--;
		deref = false;
	}
}

void WasmPopToRegister(wasm_gen_state* state, int reg_dst, own_std::vector<unsigned char>& code_sect)
{
	lang_state* lang_stat = state->wasm_state->lang_stat;
	WasmPushConst(WASM_TYPE_INT, 0, BASE_STACK_PTR_REG * 8, &code_sect);
	WasmPushConst(WASM_TYPE_INT, 0, STACK_PTR_REG * 8, &code_sect);
	WasmStoreInst(lang_stat, code_sect, 0, WASM_LOAD_OP);
	WasmStoreInst(lang_stat, code_sect, 0, WASM_LOAD_OP);
	WasmStoreInst(lang_stat, code_sect, 0, WASM_STORE_OP);
	WasmGenBinOpImmToReg(lang_stat, STACK_PTR_REG, 4, 8, code_sect, 0x6a);
}
void WasmPushRegister(wasm_gen_state* state, int reg, own_std::vector<unsigned char>& code_sect)
{
	lang_state* lang_stat = state->wasm_state->lang_stat;
	WasmGenBinOpImmToReg(lang_stat, STACK_PTR_REG, 4, 8, code_sect, 0x6b);

	WasmPushConst(WASM_TYPE_INT, 0, STACK_PTR_REG * 8, &code_sect);
	WasmStoreInst(lang_stat, code_sect, 0, WASM_LOAD_OP);
	WasmPushConst(WASM_TYPE_INT, 0, BASE_STACK_PTR_REG * 8, &code_sect);
	WasmStoreInst(lang_stat, code_sect, 0, WASM_LOAD_OP);
	WasmStoreInst(lang_stat, code_sect, 0, WASM_STORE_OP);
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
	lang_state* lang_stat = gen_state->wasm_state->lang_stat;
	switch (cur_ir->type)
	{
	case IR_SPILL:
	{

		code_sect.emplace_back(0x1);
	}break;
	case IR_NOP:
	{
		code_sect.emplace_back(0x1);
	}break;
	case IR_RET:
	{
		if (!cur_ir->ret.no_ret_val)
		{
			WasmPushConst(WASM_TYPE_INT, 0, (RET_1_REG + cur_ir->ret.assign.to_assign.reg) * 8, &code_sect);
			WasmPushMultiple(gen_state, cur_ir->ret.assign.only_lhs, cur_ir->ret.assign.lhs, cur_ir->ret.assign.rhs, last_on_stack, cur_ir->ret.assign.op, code_sect);
			//WasmPushLoadOrStore(0, WASM_TYPE_INT, WASM_LOAD_OP, BASE_STACK_PTR_REG * 8, &code_sect);
			int inst = WASM_STORE_OP;
			if(cur_ir->ret.assign.to_assign.is_float)
				inst = WASM_STORE_F32_OP;
			WasmStoreInst(lang_stat, code_sect, cur_ir->ret.assign.to_assign.reg_sz, inst);

		}
		WasmEndStack(lang_stat, code_sect, *stack_size);
		// adding stack_ptr
		//WasmGenBinOpImmToReg(BASE_STACK_PTR_REG, 4, *stack_size, code_sect, 0x6a);

        code_sect.emplace_back(0xf);
	}break;
	case IR_BREAK_OUT_IF_BLOCK:
    {
		int depth = 0;

		block_linked *aux = *cur;
		while (aux->parent)
		{
			if (aux->ir->type == IR_BEGIN_IF_BLOCK || aux->ir->type == IR_BEGIN_LOOP_BLOCK)
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
	case IR_CMP_GT:
	case IR_CMP_LE:
	case IR_CMP_LT:
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
			if (!val && (aux->ir->type == IR_BEGIN_LOOP_BLOCK))
				depth++;
				//break;
			if (!val && (aux->ir->type == IR_BEGIN_OR_BLOCK || aux->ir->type == IR_BEGIN_SUB_IF_BLOCK || aux->ir->type == IR_BEGIN_IF_BLOCK || aux->ir->type == IR_BEGIN_LOOP_BLOCK))
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

		int prev_reg_sz = cur_ir->assign.to_assign.reg_sz;
		if (cur_ir->assign.to_assign.type == IR_TYPE_REG && cur_ir->assign.to_assign.deref > 0)
			cur_ir->assign.to_assign.reg_sz = 8;
			
		WasmPushIRVal(gen_state, &cur_ir->assign.to_assign, code_sect);

		cur_ir->assign.to_assign.reg_sz = prev_reg_sz;

		WasmPushMultiple(gen_state, cur_ir->assign.only_lhs, *lhs, *rhs, last_on_stack, cur_ir->assign.op, code_sect, true);

		if(!cur_ir->assign.only_lhs && cur_ir->assign.op != T_POINT)
			ASSERT(cur_ir->assign.rhs.is_float == cur_ir->assign.lhs.is_float);
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
			if (cur_ir->assign.lhs.is_float)
				inst = WASM_STORE_F32_OP;
			WasmStoreInst(lang_stat, code_sect, r_sz, inst);
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
		WasmGenBinOpImmToReg(lang_stat, STACK_PTR_REG, 4, cur_ir->call.fdecl->args.size() * 8, code_sect, 0x6a);
		WasmPopToRegister(gen_state, BASE_STACK_PTR_REG, code_sect);

	}break;
	case IR_CALL:
	{
		int idx = 0;
		ASSERT(FuncAddedWasm(gen_state->wasm_state, cur_ir->call.fdecl->name, &idx));
		WasmPushRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
		//WasmPushRegister(gen_state, 0, code_sect);

        code_sect.emplace_back(0x10);
        WasmPushImm(idx, &code_sect);

		//WasmPopToRegister(gen_state, 0, code_sect);
		WasmPopToRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
	}break;
	case IR_BEGIN_CALL:
	{
		ASSERT(FuncAddedWasm(gen_state->wasm_state, cur_ir->call.fdecl->name));
		// sub inst
		WasmPushRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
		WasmGenBinOpImmToReg(lang_stat, STACK_PTR_REG, 4, cur_ir->call.fdecl->args.size() * 8, code_sect, 0x6b);

	}break;
	case IR_INDIRECT_CALL:
	{
		int idx = 0;
		//ASSERT(FuncAddedWasm(gen_state->wasm_state, cur_ir->call.fdecl->name, &idx));
		//WasmPushConst(WASM_TYPE_INT, 0, idx, &code_sect);
		WasmPushRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
		//WasmPushRegister(gen_state, 0, code_sect);
		WasmPushIRVal(gen_state, &cur_ir->bin.lhs, code_sect);
		code_sect.emplace_back(0x11);
		code_sect.emplace_back(0x0);
		code_sect.emplace_back(0x0);
		WasmPopToRegister(gen_state, BASE_STACK_PTR_REG, code_sect);
	}break;
	case IR_STACK_END:
	{
		//*stack_size = cur_ir->num;
		// sub inst

		WasmEndStack(lang_stat, code_sect, *stack_size);
		code_sect.emplace_back(0xf);
	}break;
	case IR_STACK_BEGIN:
	{
		//*stack_size = cur_ir->num;
		// sub inst
		gen_state->strcts_construct_stack_offset = *stack_size;
		cur_ir->fdecl->strct_constrct_at_offset = *stack_size;
		*stack_size += cur_ir->fdecl->strct_constrct_size_per_statement;

		gen_state->to_spill_offset = *stack_size;
		cur_ir->fdecl->to_spill_offset = *stack_size;
		*stack_size += cur_ir->fdecl->to_spill_size * 8;

		gen_state->strcts_ret_stack_offset = *stack_size;
		cur_ir->fdecl->strct_ret_size_per_statement_offset = *stack_size;
		*stack_size += cur_ir->fdecl->strct_ret_size_per_statement;

		*stack_size += cur_ir->fdecl->biggest_call_args * 8;

		// 8 bytes for saving rbs
		//*stack_size += 8;

		WasmBeginStack(lang_stat, code_sect, *stack_size);
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
	case IR_CAST_F32_TO_INT:
	{
		WasmPushIRVal(gen_state, &cur_ir->bin.lhs, code_sect);
		WasmPushIRVal(gen_state, &cur_ir->bin.rhs, code_sect, true);

		if (gen_state->wasm_state->lang_stat->release && cur_ir->bin.rhs.reg_sz == 8)
			cur_ir->bin.rhs.reg_sz = 4;

		code_sect.emplace_back(0xa8);

		WasmStoreInst(lang_stat, code_sect, 4, WASM_STORE_OP);
	}break;
	case IR_CAST_INT_TO_F32:
	{
		WasmPushIRVal(gen_state, &cur_ir->bin.lhs, code_sect);
		WasmPushIRVal(gen_state, &cur_ir->bin.rhs, code_sect, true);

		if (gen_state->wasm_state->lang_stat->release && cur_ir->bin.rhs.reg_sz == 8)
			cur_ir->bin.rhs.reg_sz = 4;
		if (cur_ir->bin.rhs.is_unsigned)
		{
			switch (cur_ir->bin.rhs.reg_sz)
			{
			case 4:
			case 8:
			{
				code_sect.emplace_back(0xb3);
			}break;
			/*
			{
				code_sect.emplace_back(0xb8);
			}break;
			*/
			default:
				ASSERT(0)
			}
		}
		else
		{
			switch (cur_ir->bin.rhs.reg_sz)
			{
			case 4:
			{
				code_sect.emplace_back(0xb2);
			}break;
			case 8:
			{
				code_sect.emplace_back(0xb7);
			}break;
			default:
				ASSERT(0)
			}
		}

		WasmStoreInst(lang_stat, code_sect, 4, WASM_STORE_F32_OP);
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
	case IR_DBG_BREAK:
	{
		code_sect.emplace_back(0xff);
	}break;
	case IR_BEGIN_BLOCK:
	{
		code_sect.emplace_back(0x2);
		code_sect.emplace_back(0x40);
	}break;
	case IR_END_BLOCK:
	{

		code_sect.emplace_back(0xb);
	}break;
	case IR_BREAK:
	{
		int depth = 0;

		block_linked *aux = *cur;
		while (aux->parent)
		{
			if (aux->ir->type == IR_BEGIN_LOOP_BLOCK)
                break;
			depth++;
			aux = aux->parent;
		}
		depth++;
		code_sect.emplace_back(0xc);
		WasmPushImm(depth, &code_sect);
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


		WasmStoreInst(lang_stat, code_sect, 0, WASM_STORE_OP);
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
	DBG_BREAK_ON_DIFF_STAT_BUT_SAME_FUNC,
	DBG_BREAK_ON_NEXT_STAT,
	DBG_BREAK_ON_NEXT_BC,
	DBG_BREAK_ON_NEXT_IR   
};
enum dbg_expr_type
{
	DBG_EXPR_SHOW_SINGLE_VAL,
	DBG_EXPR_SHOW_VAL_X_TIMES,
	DBG_EXPR_FILTER,
};
enum dbg_print_numbers_format
{
	DBG_PRINT_DECIMAL,
	DBG_PRINT_HEX,
};
struct dbg_expr
{
	dbg_expr_type type;
	own_std::vector<ast_rep*> expr;
	own_std::vector<ast_rep*> x_times;
	own_std::vector<ir_rep > filter_cond;

	func_decl* from_func;
	own_std::string exp_str;
};
struct command_info_args
{
	own_std::string incomplete_str;
	int cur_tkn;
	own_std::vector<token2>* tkns;

};
typedef own_std::string(*CommandFunc)(dbg_state* dbg, command_info_args *info);
struct command_info
{
	own_std::vector<own_std::string> names;
	own_std::vector<command_info *> cmds;
	CommandFunc func;
	CommandFunc execute;
	bool end;
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
	func_decl* next_stat_break_func;
	stmnt_dbg* cur_st;
	own_std::vector<func_decl*> func_stack;
	own_std::vector<block_linked *> block_stack;
	own_std::vector<wasm_bc*> return_stack;
	own_std::vector<wasm_bc> bcs;
	own_std::vector<wasm_stack_val> wasm_stack;
	own_std::vector<dbg_expr *> exprs;
	//own_std::vector<command_info> cmds;

	command_info* global_cmd;

	lang_state* lang_stat;
	wasm_interp* wasm_state;


	bool some_bc_modified;

	void* data;
};
void WasmModifyCurBcPtr(dbg_state* dbg, wasm_bc* to)
{
	dbg->some_bc_modified = true;
	(*dbg->cur_bc) = to;
}
enum print_num_type
{
	PRINT_INT,
	PRINT_FLOAT,
	PRINT_CHAR,
};

own_std::string WasmNumToString(dbg_state* dbg, int num, char limit = -1, print_num_type num_type = PRINT_INT)
{
	own_std::string ret = "";
	char buffer[32];
	if (num_type == PRINT_FLOAT)
	{
		float num_f = *(float*)&num;
		if(limit == -1)
			snprintf(buffer, 32, "%.3f", num_f);
		else 
			// search how to arbitrary number of decimals in float
			ASSERT(0)
	}
	else if (num_type == PRINT_CHAR)
	{
		char num_ch = (char)num;
		snprintf(buffer, 32, "0x%x %c", num_ch, num_ch);

	}
	else
	{
		switch (dbg->print_numbers_format)
		{
		case DBG_PRINT_DECIMAL:
		{
			if (limit == -1)
				snprintf(buffer, 32, "%d", num);
			else
				snprintf(buffer, 32, "%0*d", limit, num);

		}break;
		case DBG_PRINT_HEX:
		{
			if (limit == -1)
				snprintf(buffer, 32, "0x%x", num);
			else
				snprintf(buffer, 32, "0x%0*x", limit, num);

		}break;
		default:
			ASSERT(0);
		}
	}
	ret = buffer;
	return ret;
}

own_std::string WasmGetBCString(dbg_state *dbg, func_decl* func, wasm_bc *bc, own_std::vector<wasm_bc>* bcs)
{
	char buffer[64];
	own_std::string ret = "       ";
	switch (bc->type)
	{
	case WASM_INST_DBG_BREAK:
	{
		ret += "dbg_break";
	}break;
	case WASM_INST_I32_NE:
	{
		ret += "i32.ne";
	}break;
	case WASM_INST_I32_LT_S:
	{
		ret += "i32.lt_s";
	}break;
	case WASM_INST_I32_LE_S:
	{
		ret += "i32.le_u";
	}break;
	case WASM_INST_F32_DIV:
	{
		ret += "f32.div";
	}break;
	case WASM_INST_F32_LE:
	{
		ret += "f32.le";
	}break;
	case WASM_INST_F32_GT:
	{
		ret += "f32.gt";
	}break;
	case WASM_INST_F32_LT:
	{
		ret += "f32.lt";
	}break;
	case WASM_INST_F32_GE:
	{
		ret += "f32.ge";
	}break;
	case WASM_INST_I32_GT_U:
	{
		ret += "i32.gs_u";
	}break;
	case WASM_INST_I32_GT_S:
	{
		ret += "i32.gs_t";
	}break;
	case WASM_INST_I32_LT_U:
	{
		ret += "i32.lt_u";
	}break;
	case WASM_INST_I32_LE_U:
	{
		ret += "i32.le_u";
	}break;
	case WASM_INST_I32_GE_S:
	{
		ret += "i32.ge_s";
	}break;
	case WASM_INST_I32_GE_U:
	{
		ret += "i32.ge_u";
	}break;
	case WASM_INST_I32_EQ:
	{
		ret += "i32.eq";
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
	case WASM_INST_I64_STORE:
	{
		ret += "i64.store";
	}break;
	case WASM_INST_I32_STORE:
	{
		ret += "i32.store";
	}break;
	case WASM_INST_F32_LOAD:
	{
		ret += "f32.load";
	}break;
	case WASM_INST_I32_LOAD_8_S:
	{
		ret += "i32.load_8_s";
	}break;
	case WASM_INST_I64_LOAD:
	{
		ret += "i64.load";
	}break;
	case WASM_INST_I32_LOAD:
	{
		ret += "i32.load";
	}break;
	case WASM_INST_END:
	{
		ret += "end";
	}break;
	case WASM_INST_I32_DIV_S:
	{
		ret += "i32.div_s";
	}break;
	case WASM_INST_I32_DIV_U:
	{
		ret += "i32.div_u";
	}break;
	case WASM_INST_I32_SUB:
	{
		ret += "i32.sub";
	}break;
	case WASM_INST_F32_SUB:
	{
		ret += "f32.sub";
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
		own_std::string func_name = WasmGetFuncAtIdx(dbg->wasm_state, bc->i)->name;
		ret += "indirect call ";
		ret += own_std::string("\t \\") + func_name;
	}break;
	case WASM_INST_CALL:
	{
		own_std::string func_name = WasmGetFuncAtIdx(dbg->wasm_state, bc->i)->name;
		ret += own_std::string("call ") + WasmNumToString(dbg, bc->i);
		ret += own_std::string("\t \\") + func_name;
	}break;
	case WASM_INST_BREAK_IF:
	{
		ret += own_std::string("br_if ") + WasmNumToString(dbg, bc->i);;
	}break;
	case WASM_INST_BREAK:
	{
		ret += own_std::string("br ") + WasmNumToString(dbg, bc->i);
	}break;
	case WASM_INST_I32_OR:
	{
		ret += "i32.or";
	}break;
	case WASM_INST_CAST_S32_2_F32:
	{
		ret += "f32.cast_s32";
	}break;
	case WASM_INST_CAST_S64_2_F32:
	{
		ret += "f32.cast_s64";
	}break;
	case WASM_INST_F32_MUL:
	{
		ret += "f32.mul";
	}break;
	case WASM_INST_CAST_F32_2_S32:
	{
		ret += "i32.cast_f32";
	}break;
	case WASM_INST_CAST_U32_2_F32:
	{
		ret += "f32.cast_u32";
	}break;
	case WASM_INST_I32_REMAINDER_U:
	{
		ret += "i32.rem_u";
	}break;
	case WASM_INST_I32_REMAINDER_S:
	{
		ret += "i32.rem_s";
	}break;
	case WASM_INST_I32_AND:
	{
		ret += "i32.and";
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
		own_std::string loaded_var = "";
		decl2* var;
		if (loading_var_address && FindVarWithOffset(func, probable_var_offset->i, &var, dbg->cur_st->line))
		{
			loaded_var = own_std::string("\t//pushing $") + var->name + " on the stack";
		}
		ret += own_std::string("i32.const ") + WasmNumToString(dbg, bc->i) + loaded_var;
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
own_std::string WasmIrValToString(dbg_state* dbg, ir_val* val)
{
	own_std::string ret = "";
	char buffer[32];
	int ptr = 0;
	if(ptr == -1)
	{
		ret += "&";
	}
	while (ptr < val->deref && val->type != IR_TYPE_INT)
	{
		ret += "*";
		ptr++;
	}
	switch (val->type)
	{
	case IR_TYPE_REG:
	{
		int r_sz = val->reg_sz;
		own_std::string reg_sz_str = "";
		if (r_sz > 0)
		{
			r_sz = max(r_sz, 4);
			r_sz = 2 << r_sz;
		}
		own_std::string float_str = "";
		if(val->is_float)
			float_str = "_f32_";
		snprintf(buffer, 32, "reg%s%d[%s]", float_str.c_str(), r_sz, WasmNumToString(dbg, val->reg).c_str());
		ret += buffer;
	}break;
	case IR_TYPE_ON_STACK:
	{
		int base_ptr = WasmGetRegVal(dbg, BASE_STACK_PTR_REG);
		own_std::string stack_type_name = "";
		switch (val->on_stack_type)
		{
		case ON_STACK_STRUCT_RET:
		{
			stack_type_name = "struct ret";
			base_ptr -= dbg->cur_func->strct_ret_size_per_statement_offset;
		}break;
		case ON_STACK_STRUCT_CONSTR:
		{
			stack_type_name = "struct constr";
			base_ptr -= dbg->cur_func->strct_constrct_at_offset;
		}break;
		case ON_STACK_SPILL:
		{
			stack_type_name = "spill";
			base_ptr -= dbg->cur_func->to_spill_offset;
		}break;
		default:
			ASSERT(0);
		}
		snprintf(buffer, 32, "%s (address %s)", stack_type_name.c_str(), WasmNumToString(dbg, base_ptr).c_str());
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
	case IR_TYPE_STR_LIT:
	{
		char* data = dbg->lang_stat->data_sect.begin() + val->i;
		snprintf(buffer, 32, "str \"%s\"", data);
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

own_std::string WasmGetBinIR(dbg_state* dbg, ir_rep* ir)
{
	char buffer[64];
	own_std::string lhs = WasmIrValToString(dbg, &ir->bin.lhs);
	own_std::string rhs = WasmIrValToString(dbg, &ir->bin.rhs);
	own_std::string op = OperatorToString(ir->bin.op);
	snprintf(buffer, 64, "%s %s %s", lhs.c_str(), op.c_str(), rhs.c_str());
	return buffer;
}

own_std::string WasmIrAssignment(dbg_state* dbg, assign_info* assign)
{
	char buffer[128];
	own_std::string ret = "";
	own_std::string to_assign = WasmIrValToString(dbg, &assign->to_assign);
	own_std::string lhs = WasmIrValToString(dbg, &assign->lhs);
	if (assign->only_lhs)
		snprintf(buffer, 128, "%s = %s", to_assign.c_str(), lhs.c_str());
	else
	{
		own_std::string rhs = WasmIrValToString(dbg, &assign->rhs);
		own_std::string op = OperatorToString(assign->op);
		snprintf(buffer, 128, "%s = %s %s %s", to_assign.c_str(), lhs.c_str(), op.c_str(), rhs.c_str());
	}
	ret = buffer;
	ret += "\n";
	return ret;

}
own_std::string WasmIrToString(dbg_state* dbg, ir_rep *ir)
{
	char buffer[128];
	own_std::string ret="";
	switch (ir->type)
	{
	case IR_RET:
	{
		ret += "ret: ";
		if(!ir->ret.no_ret_val)
			ret += WasmIrAssignment(dbg, &ir->ret.assign) + "\n";
		else
			ret += " no return val\n";
	}break;
	case IR_DBG_BREAK:
	{
		ret = "dbg_break\n";
	}break;
	case IR_END_CALL:
	{
		ret = own_std::string("calling ") +ir->call.fdecl->name + " \n";
	}break;
	case IR_INDIRECT_CALL:
	{
		ret = own_std::string("indirect calling to ") + ir->call.fdecl->name + " \n";
	}break;
	case IR_CALL:
	{
		ret = own_std::string("calling to ") +ir->call.fdecl->name + " \n";
	}break;
	case IR_BEGIN_CALL:
	{
		ret = own_std::string("beginning call to ") +ir->call.fdecl->name + " \n";
	}break;
	case IR_CMP_NE:
	case IR_CMP_GT:
	case IR_CMP_LT:
	case IR_CMP_EQ:
	case IR_CMP_LE:
	case IR_CMP_GE:
	{
		ret = WasmGetBinIR(dbg, ir) + "\n";
	}break;
	case IR_BEGIN_BLOCK:
	{
		ret = "begin block\n";
	}break;
	case IR_END_BLOCK:
	{
		ret = "end block\n";
	}break;
	case IR_BEGIN_SUB_IF_BLOCK:
	{
		ret = "begin sub if loop\n";
	}break;
	case IR_END_SUB_IF_BLOCK:
	{
		ret = "end sub if loop\n";
	}break;
	case IR_BREAK_OUT_IF_BLOCK:
	{
		ret = "break if block\n";
	}break;
	case IR_BEGIN_OR_BLOCK:
	{
		ret = "begin or block\n";
	}break;
	case IR_END_OR_BLOCK:
	{
		ret = "end or block\n";
	}break;
	case IR_END_LOOP_BLOCK:
	{
		ret = "end loop\n";
	}break;
	case IR_BEGIN_LOOP_BLOCK:
	{
		ret = "begin loop\n";
	}break;
	case IR_BEGIN_COND_BLOCK:
	{
		ret = "begin cond block\n";
	}break;
	case IR_BREAK:
	{
		ret = "break";
	}break;
	case IR_CAST_INT_TO_F32:
	{
		ret = "cast int to f32\n";
	}break;
	case IR_END_COND_BLOCK:
	{
		ret = "end cond block\n";
	}break;
	case IR_CAST_F32_TO_INT:
	{
		ret = "cast f32 to int\n";
	}break;
	case IR_END_IF_BLOCK:
	{
		ret = "end if block\n";
	}break;
	case IR_BEGIN_IF_BLOCK:
	{
		ret = "begin if block\n";
	}break;
	case IR_STACK_BEGIN:
	case IR_STACK_END:
	case IR_BEGIN_STMNT:
	case IR_DECLARE_LOCAL:
	case IR_DECLARE_ARG:
	case IR_END_STMNT:
	case IR_NOP:
	{

	}break;
	case IR_ASSIGNMENT:
	{
		ret+= WasmIrAssignment(dbg, &ir->assign);
		ret += "\n";
	}break;
	default:
		ASSERT(0);
	}
	return ret;
}

void WasmGetIrWithIdx(dbg_state* dbg, func_decl *func, int idx, ir_rep **ir_start, int *len_of_same_start, int start = -1, int end = -1)
{

	if (start == -1)
		dbg->cur_st->start;
	if (end == -1)
		dbg->cur_st->end;
	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &func->ir;
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

own_std::string WasmPrintCodeGranular(dbg_state* dbg, func_decl *fdecl, wasm_bc *cur_bc, int code_start, int code_end, int max_ir = -1, bool center_cur = true, bool print_bc = true)
{
	own_std::string ret = "";
	int i = 0;
	//int offset = dbg->

	int start_bc = code_start;
	int end_bc = code_end;

	if (center_cur)
	{
		int window = 8;
		int half_window = window / 2;
		int cur_bc_idx = cur_bc - dbg->bcs.begin();
		start_bc = max(code_start, cur_bc_idx - half_window);
		end_bc = min(cur_bc_idx + half_window, dbg->bcs.size());
	}
	wasm_bc* stat_begin = dbg->bcs.begin() + start_bc;
	wasm_bc* cur = stat_begin;

	char buffer[512];

	if (max_ir == -1)
	{
		max_ir = start_bc - end_bc;
		max_ir++;
	}
	//max = max(max, 16);

	while (i < max_ir && cur < dbg->bcs.end())
	{
		int bc_on_stat_rel_idx = cur - stat_begin;
		int bc_on_stat_abs_idx = bc_on_stat_rel_idx + start_bc;
		ir_rep* ir = nullptr;
		int len_ir = 0;
		WasmGetIrWithIdx(dbg, fdecl, bc_on_stat_abs_idx, &ir, &len_ir, start_bc, start_bc + max_ir);
		own_std::string ir_str = "";
		if (ir)
		{
			while (len_ir > 0)
			{
				own_std::string ir_str = WasmIrToString(dbg, ir);
				if (ir_str != "")
				{
					own_std::string ir_idx_str = WasmNumToString(dbg, ir->idx);
					snprintf(buffer, 512, ANSI_YELLOW "%s: %s" ANSI_RESET, ir_idx_str.c_str(), ir_str.c_str());
					ret += buffer;
				}
				len_ir--;
				ir++;
			}
		}
		if (print_bc)
		{
			own_std::string bc_str = WasmGetBCString(dbg, fdecl, cur, &dbg->bcs);
			own_std::string bc_rel_idx = WasmNumToString(dbg, i, 3);

			if (cur == cur_bc)
				snprintf(buffer, 512, ANSI_BG_WHITE ANSI_BLACK "%s:%s" ANSI_RESET, bc_rel_idx.c_str(), bc_str.c_str());
			else if (cur->dbg_brk)
				snprintf(buffer, 512, ANSI_RED "%s:%s" ANSI_RESET, bc_rel_idx.c_str(), bc_str.c_str());
			else
				snprintf(buffer, 512, "%s:%s", bc_rel_idx.c_str(), bc_str.c_str());
			ret += buffer;
			ret += "\n";
		}
		i++;
		cur++;
	}
	return ret;
}
own_std::string WasmPrintVars(dbg_state *dbg)
{
	own_std::string ret = "";
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
			if (d->type.type == TYPE_FUNC || d->type.type == TYPE_FUNC_TYPE || d->type.type == TYPE_STRUCT_TYPE || d->type.type == TYPE_FUNC_PTR)
				continue;

			int val = *(int*)&mem_buffer[base_stack_ptr + d->offset];

			own_std::string type;

			if (IS_FLAG_ON(d->flags, DECL_IS_ARG))
				type = "param";
			else
				type = "local";

			own_std::string offset_str = WasmNumToString(dbg, d->offset);
			own_std::string val_str = WasmNumToString(dbg, val);

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
	ir_rep* ir = ir_ar->begin();
	ir_rep* end = ir_ar->end();
	while(ir < end)
	{
		if (offset >= ir->start && offset <= ir->end)
			return ir;
		ir++;
	}
	return nullptr;
}
stmnt_dbg* GetStmntBasedOnOffset(own_std::vector<stmnt_dbg>* ar, int offset)
{
	stmnt_dbg* st = ar->begin();
	stmnt_dbg* end = ar->end();
	while(st < end)
	{
		if (offset >= st->start && offset <= st->end)
			return st;
		st++;
	}
	return nullptr;
}

#define DBG_STATE_ON_LANG 0
#define DBG_STATE_ON_WASM 1



own_std::string WasmGetStack(dbg_state* dbg)
{
	own_std::string ret = "stack: \n";
	char buffer[512];
	int i = 0;
	FOR_VEC(s, dbg->wasm_stack)
	{
		own_std::string type = "";
		own_std::string val_str = "";
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
	if (offset > dbg->mem_size)
		return -1;
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
		case AST_DEREF:
		{
			typed_stack_val *top = &expr_stack.back();
			expr_stack.pop_back();

			int deref_times = a->deref.times;
			while (deref_times > 0)
			{
				top->offset = WasmGetMemOffsetVal(dbg, top->offset);
				top->type.ptr--;
				deref_times--;
			}
			val = *top;
		}break;
		case AST_BINOP:
		{
			switch (a->op)
			{
			case T_POINT:
			{
				typed_stack_val *top = expr_stack.end();
				typed_stack_val* first = top - a->points.size();

				expr_stack.pop_back();
				typed_stack_val* punultimate = first;
				for (int i = 1; i < a->points.size(); i++)
				{
					while (punultimate->type.ptr > 0)
					{
						punultimate->offset = WasmGetMemOffsetVal(dbg, punultimate->offset);
						punultimate->type.ptr--;
					}
					typed_stack_val* next = first + i;
					punultimate->offset += next->type.e_decl->offset;
					punultimate->type = next->type;
				}
				for (int i = 0; i < a->points.size(); i++)
				{
					expr_stack.pop_back();
				}
				expr_stack.emplace_back(*punultimate);
			}break;
			default:
				ASSERT(0);
			}
			typed_stack_val *top = &expr_stack.back();
			ASSERT(top->decl == true)
			expr_stack.pop_back();
			val.type = top->type;
			val.type.ptr;
			val.offset = top->offset;
			char ptr = val.type.ptr;
			//val.offset = WasmGetMemOffsetVal(dbg, val.offset);
			//val.type.ptr--;
		}break;
		case AST_UNOP:
		{
			// only handling this un op at the moment
			ASSERT(a->op == T_MUL);
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
			decl2* d = top->type.e_decl;
			val.type = a->cast.type;
			//val.type.type = FromTypeToVarType(a->cast.type.type)
			val.type.e_decl = d;

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
own_std::string WasmGenRegStr(dbg_state* dbg, int reg, int reg_sz, own_std::string reg_name)
{
	unsigned int reg_val = WasmGetMemOffsetVal(dbg, reg);
	char buffer[64];
	own_std::string reg_idx = WasmNumToString(dbg, reg);
	own_std::string reg_val_str = WasmNumToString(dbg, reg_val);
	own_std::string ptr_val_str;
	own_std::string reg_size_str;

	if (reg_sz > 4)
		reg_sz = 4;

	reg_sz = 2 << reg_sz;

	if (reg_sz != 0)
		reg_size_str = std::to_string(reg_sz).c_str();


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
	own_std::string str = "";
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
own_std::string WasmVarToString(dbg_state* dbg, char indent, decl2* d, int struct_offset, char ptr, bool is_self_ref = false)
{

	char buffer[512];
	char indent_buffer[32];

	memset(indent_buffer, ' ', indent);
	indent_buffer[indent] = 0;

	own_std::string ptr_val_str = "";
	if (ptr > 0)
	{
		int ptr_val = WasmGetMemOffsetVal(dbg, struct_offset);
		ptr_val_str = own_std::string("(*") + WasmNumToString(dbg, ptr_val) + own_std::string(")");
	}
	own_std::string ret;
	if (!is_self_ref && (d->type.type == TYPE_STRUCT || d->type.type == TYPE_STRUCT_TYPE))
	{

		//char ptr = ptr;
		while (ptr > 0)
		{
			struct_offset = WasmGetMemOffsetVal(dbg, struct_offset);
			break;
		}

		snprintf(buffer, 512, "%s%s%s : {\n", indent_buffer, d->name.c_str(), ptr_val_str.c_str());
		ret += buffer;
		scope* strct_scp = d->type.strct->scp;

		indent++;
		memset(indent_buffer, ' ', indent);
		indent_buffer[indent] = 0;


		if (ptr > 0 && struct_offset < 128)
		{
		}
		else
		{

			//own_std::string ptr_addr = WasmNumToString(dbg, struct_offset);
			FOR_VEC(struct_d, strct_scp->vars)
			{
				decl2* ds = *struct_d;
				bool is_self_ref = ds->type.type == TYPE_STRUCT && ds->type.strct == d->type.strct;
				if (ds->type.type == TYPE_STRUCT_TYPE)
					continue;
				own_std::string var_str = WasmVarToString(dbg, indent, ds, struct_offset + ds->offset, ds->type.ptr, is_self_ref);
				snprintf(buffer, 512, "%s,\n", var_str.c_str());
				ret += buffer;
			}
			if (strct_scp->vars.size() > 0)
			{
				ret.pop_back();
				ret.pop_back();
			}
		}

		indent--;
		memset(indent_buffer, ' ', indent);
		indent_buffer[indent] = 0;

		snprintf(buffer, 512, "\n%s}", indent_buffer);
		ret += buffer;
	}
	else// if(d->type.type != TYPE_STRUCT_TYPE)
	{

		own_std::string val_str = "";
		if (struct_offset < 128)
			val_str = "0";
		else
		{
			print_num_type ptype = d->type.type == TYPE_F32 ? PRINT_FLOAT : PRINT_INT;
			val_str = WasmNumToString(dbg, WasmGetMemOffsetVal(dbg, struct_offset), -1, ptype);
		}
		snprintf(buffer, 512, "%s%s%s: %s", indent_buffer, d->name.c_str(), ptr_val_str.c_str(), val_str.c_str());
		ret += buffer;
	}

	return ret;
}
own_std::string WasmGetSingleExprToStr(dbg_state* dbg, dbg_expr* exp)
{
	own_std::string ret = own_std::string("expression: ") + exp->exp_str;

	//if(exp->)
	typed_stack_val expr_val;
	WasmFromAstArrToStackVal(dbg, exp->expr, &expr_val);
	char buffer[512];
	own_std::string addr_str = WasmNumToString(dbg, expr_val.offset);

	int ptr = expr_val.type.ptr;

	int in_ptr_addr = WasmGetMemOffsetVal(dbg, expr_val.offset);
	while (ptr > 0)
	{
		if (in_ptr_addr > dbg->mem_size || in_ptr_addr < 0)
			snprintf(buffer, 512, "->out of bounds");
		else
			snprintf(buffer, 512, "->%s", WasmNumToString(dbg, in_ptr_addr).c_str());
		addr_str += buffer;
		in_ptr_addr = WasmGetMemOffsetVal(dbg, in_ptr_addr);
		ptr--;
	}
	if (expr_val.type.type == TYPE_STRUCT)
	{
		decl2* d = expr_val.type.strct->this_decl;
		//own_std::string WasmVarToString(dbg_state* dbg, char indent, decl2* d, int struct_offset)


		ptr = expr_val.type.ptr;
		if (ptr > 0)
		{
			expr_val.offset = WasmGetMemOffsetVal(dbg, expr_val.offset);
			ptr--;
		}
		

		own_std::string struct_str = WasmVarToString(dbg, 0, d, expr_val.offset, ptr);
		snprintf(buffer, 512," addr(%s) (%s) %s ", addr_str.c_str(), TypeToString(expr_val.type).c_str(), struct_str.c_str());
	}
	else
	{
		int val = 0;
		print_num_type print_type = PRINT_INT;
		own_std::string str_val = "";
		if (expr_val.type.ptr > 0)
		{
			val = WasmGetMemOffsetVal(dbg, expr_val.offset);
		}
		else if (expr_val.type.type == TYPE_STR_LIT)
		{
			val = WasmGetMemOffsetVal(dbg, expr_val.offset);
			auto addr = (char*)&dbg->mem_buffer[val];
			snprintf(buffer, 512, "%s", addr);
			str_val = buffer;

			//unsigned int len = strlen(addr);
			//len = 0;
		}
		else
		{
			switch (expr_val.type.type)
			{
			case TYPE_CHAR:
			{
				val = WasmGetMemOffsetVal(dbg, expr_val.offset) & 0xff;
				print_type = PRINT_CHAR;
			}break;
			case TYPE_U8:
			case TYPE_BOOL:
			{
				val = WasmGetMemOffsetVal(dbg, expr_val.offset) & 0xff;

			}break;
			case TYPE_VOID:
			case TYPE_U64:
			case TYPE_S32:
			case TYPE_U32:
			{
				val = WasmGetMemOffsetVal(dbg, expr_val.offset);

			}break;
			case TYPE_F32:
			{
				val = WasmGetMemOffsetVal(dbg, expr_val.offset);
				print_type = PRINT_FLOAT;
			}break;
			default:
				ASSERT(0);
			}
			str_val = WasmNumToString(dbg, val, -1, print_type);
		}


	
		snprintf(buffer, 512, "addr(%s) (%s) %s ", addr_str.c_str(), TypeToString(expr_val.type).c_str(), str_val.c_str());
	}
	ret += buffer;

	switch (exp->type)
	{
	case DBG_EXPR_SHOW_SINGLE_VAL:
	{
		if (expr_val.type.ptr > 0)
		{

			int ptr_deref_val = WasmGetMemOffsetVal(dbg, expr_val.offset);

			snprintf(buffer, 512, "{%s}", WasmNumToString(dbg, ptr_deref_val).c_str());
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
		own_std::string show = "";
		if (expr_val.type.ptr == 1)
		{
			ptr_buffer = &dbg->mem_buffer[*(int*)ptr_buffer];
			switch (expr_val.type.type)
			{
			case TYPE_U8:
			{
			}break;
			case TYPE_F32:
			{
				for (int i = 0; i < x_times.offset; i++)
				{
					unsigned int num = *(((unsigned int*)ptr_buffer) + i);
					show += WasmNumToString(dbg, num, -1, PRINT_FLOAT);
					show += ", ";
				}

				show.pop_back();
				show.pop_back();
			}break;
			case TYPE_U32:
			{
				for (int i = 0; i < x_times.offset; i++)
				{
					unsigned int num = *(((unsigned int*)ptr_buffer) + i);
					show += WasmNumToString(dbg, num);
				}
			}break;
			case TYPE_CHAR:
			{
				for (int i = 0; i < x_times.offset; i++)
				{
					show += *(((char*)ptr_buffer) + i);
				}
			}break;
			case TYPE_STRUCT:
			{
				
				if (x_times.offset > 10)
				{
					show.reserve(1024 * 4);
				}
				for (int i = 0; i < x_times.offset; i++)
				{
					snprintf(buffer, 512, "%d: \n", i);
					show += buffer;
					show += WasmVarToString(dbg, 0, expr_val.type.strct->this_decl, expr_val.offset + i * expr_val.type.strct->size, 0);
				}
			}break;
			default:
			{
				ASSERT(0);
			}break;
			}
		}
		else if (expr_val.type.ptr > 1)
		{
			for (int i = 0; i < x_times.offset; i++)
			{
				show += WasmNumToString(dbg, *(((unsigned int*)ptr_buffer) + i * 8));
				show += ", ";
			}
			if (x_times.offset > 0)
			{
				show.pop_back();
				show.pop_back();
			}
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
		printf("%s\n", WasmGetSingleExprToStr(dbg, *e).c_str());
	}
}

void WasmPrintStackAndBcs(dbg_state* dbg, int max_bcs = -1)
{
	printf("%s\n", WasmPrintCodeGranular(dbg, dbg->cur_func, *dbg->cur_bc, dbg->cur_st->start, dbg->cur_st->end, max_bcs).c_str());
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
		dbg->next_stat_break_func = dbg->cur_func;
		dbg->break_type = DBG_BREAK_ON_DIFF_STAT_BUT_SAME_FUNC;
	}

	*args_break = true;
}
void getConsoleLine(int line, int length) {
	/*
    // Get the console output handle
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Get console screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    // Allocate space for the characters to read
    CHAR_INFO* buffer = new CHAR_INFO[length];
    COORD bufferSize = {length, 1};  // Width x Height
    COORD bufferCoord = {0, 0};
    SMALL_RECT readRegion = {0, line, static_cast<SHORT>(length - 1), line};

    // Read the console output into the buffer
    ReadConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &readRegion);

    // Print out the characters
    std::cout << "Line " << line << ": ";
    for (int i = 0; i < length; ++i) {
        std::cout << buffer[i].Char.AsciiChar;
    }
    std::cout << std::endl;

    // Clean up
    delete[] buffer;
    */
}
void printInput(const own_std::string& input, size_t cursorPos) {
	//std::cout << "\033[2K\r" << input.c_str()<<"\r";  // Clear the current line and print the input
	if(cursorPos > 0)
		std::cout << "\033[" << cursorPos<< "C";  // Move the cursor back

}
command_info* GetSuggestion(command_info *cur, own_std::string incomplete_str)
{

	FOR_VEC(cmd, cur->cmds)
	{
		if (cur->end)
		{
			return cur;
		}
		else
		{
			command_info* c = *cmd;

			bool choose_command = false;
			FOR_VEC(name, c->names)
			{
				own_std::string n = *name;
				bool is_equal = true;
				for (int j = 0; j < incomplete_str.size(); j++)
				{
					if (incomplete_str[j] != n[j])
					{
						is_equal = false;
						break;
					}
				}
				if (is_equal)
					return c;
			}
		}
	}
	return nullptr;
}
void InsertSuggestion(dbg_state* dbg, own_std::string &input, int *cursor_pos)
{
	own_std::vector<own_std::string> args;

	//own_std::string aux;
	//split(input, ' ', args, &aux);
	own_std::vector<token2> tkns;
	Tokenize2((char *)input.c_str(), input.size(), &tkns);

	command_info* cur = dbg->global_cmd;
	command_info* last = nullptr;
	// poping EOF
	tkns.pop_back();
	
	FOR_VEC(t, tkns)
	{
		if (t->type == T_COMMA)
			continue;

		if (cur->end)
		{
			if ((t + 1)->type == T_COMMA)
			{
				cur = last;
				continue;
			}
			if (cur->func)
			{
				command_info_args args;
				args.incomplete_str = t->str;

				ASSERT(cur->func);
				own_std::string final_str = cur->func(dbg, &args);

				ASSERT(0);
				input.erase(t->line_offset, t->str.size());
				input.insert(t->line_offset, final_str);
				*cursor_pos = t->line_offset + final_str.size();
			}
			continue;

		}
		command_info* c = GetSuggestion(cur, t->str);

		if (c)
		{
			input.erase(t->line_offset, t->str.size());
			own_std::string final_str = c->names[0];

			input.insert(t->line_offset,  final_str);
			*cursor_pos = t->line_offset + final_str.size();
			last = cur;
			cur = c;
		}

	}
}

dbg_expr* WasmGetExprFromTkns(dbg_state* dbg, own_std::vector<token2> *tkns)
{
	func_decl* func = dbg->cur_func;
	node_iter niter = node_iter(tkns, dbg->lang_stat);
	node *n = niter.parse_all();

	type_struct2* strct_for_filter;
	//Scope
	scope* cur_scp = FindScpWithLine(func, dbg->cur_st->line);

	if (IS_COMMA(n))
	{
		own_std::vector<node *> commas_val;
		node* cur = n;
		while (IS_COMMA(cur))
		{
			ASSERT(!IS_COMMA(cur->r));
			commas_val.emplace_back(cur->r);
			cur = cur->l;
		}
		commas_val.emplace_back(cur);
		
		switch (commas_val.size())
		{
		case 2:
		case 1:
		{

			dbg->lang_stat->flags |= PSR_FLAGS_REPORT_UNDECLARED_IDENTS;
			DescendNameFinding(dbg->lang_stat, n, cur_scp);

			DescendNode(dbg->lang_stat, n, cur_scp);
		}break;
		// filter vals
		case 3:
		{
			
			dbg->lang_stat->flags |= PSR_FLAGS_REPORT_UNDECLARED_IDENTS;
			DescendNameFinding(dbg->lang_stat, commas_val[2], cur_scp);
			type2 strct = DescendNode(dbg->lang_stat, commas_val[2], cur_scp);

			ASSERT(strct.type == TYPE_STRUCT && strct.ptr == 1)

			DescendNameFinding(dbg->lang_stat, commas_val[1], cur_scp);
			DescendNode(dbg->lang_stat, commas_val[1], cur_scp);

			// creating a new scope to not polute the original one
			scope* scp = NewScope(dbg->lang_stat, cur_scp);
			cur_scp = scp;

			auto _ptr = (decl2*)AllocMiscData(dbg->lang_stat, sizeof(decl2));
			_ptr->name = own_std::string("_ptr");
			_ptr->flags = DECL_ABSOLUTE_ADDRESS;
			_ptr->offset = FILTER_PTR;
			_ptr->type = strct;
			cur_scp->vars.emplace_back(_ptr);
			DescendNameFinding(dbg->lang_stat, commas_val[0], cur_scp);
			DescendNode(dbg->lang_stat, commas_val[0], cur_scp);
		}break;
		default:
			ASSERT(0);
		}
	}
	else
	{
		dbg->lang_stat->flags |= PSR_FLAGS_REPORT_UNDECLARED_IDENTS;
		DescendNameFinding(dbg->lang_stat, n, cur_scp);

		DescendNode(dbg->lang_stat, n, cur_scp);
	}



	ast_rep* ast = AstFromNode(dbg->lang_stat, n, cur_scp);

	n->FreeTree();
	/*
	int last_idx = dbg->lang_stat->allocated_vectors.size() - 1;
	if(last_idx >= 0)
	{
	dbg->lang_stat->allocated_vectors[last_idx]->~vector();
	dbg->lang_stat->allocated_vectors.pop_back();
	*/

	auto exp = (dbg_expr*)AllocMiscData(dbg->lang_stat, sizeof(dbg_expr));
	//exp->exp_str = exp_str.substr();
	exp->from_func = dbg->cur_func;

	//exp_str = "";
	if (ast->type == AST_BINOP && ast->op == T_COMMA)
	{
		ASSERT(ast->expr.size() >= 2);


		PushAstsInOrder(dbg->lang_stat, ast->expr[0], &exp->expr);
		PushAstsInOrder(dbg->lang_stat, ast->expr[1], &exp->x_times);

		if(ast->expr.size() == 2)
			exp->type = DBG_EXPR_SHOW_VAL_X_TIMES;
		else if (ast->expr.size() == 3)
		{
			exp->type = DBG_EXPR_FILTER;
			ast_rep* ret = NewAst();
			ret->type = AST_RET;
			ret->ast = ast->expr[2];
			FreeAllRegs(dbg->lang_stat);
			GetIRFromAst(dbg->lang_stat, ret, &exp->filter_cond);
		}
		else
			ASSERT(0)
		//exp_str = WasmGetSingleExprToStr(dbg, &exp);
	}
	else
	{
		exp->type = DBG_EXPR_SHOW_SINGLE_VAL;

		PushAstsInOrder(dbg->lang_stat, ast, &exp->expr);
		//exp_str = WasmGetSingleExprToStr(dbg, &exp);
	}

	return exp;
}
dbg_expr* WasmGetExprFromStr(dbg_state* dbg, own_std::string exp_str)
{
	func_decl* func = dbg->cur_func;

	own_std::vector<token2> tkns;
	Tokenize2((char *)exp_str.c_str(), exp_str.size(), &tkns);
	return WasmGetExprFromTkns(dbg, &tkns);
}

int WasmIrInterpGetIrVal(dbg_state* dbg, ir_val* val)
{
	int ret = 0;
	char ptr = val->deref;
	switch (val->type)
	{
	case IR_TYPE_INT:
	{
		ret = val->i;
	}break;
	case IR_TYPE_REG:
	{
		ret = WasmGetRegVal(dbg, val->reg);
	}break;
	case IR_TYPE_DECL:
	{

		if (IS_FLAG_ON(val->decl->flags, DECL_ABSOLUTE_ADDRESS))
		{
			ret = WasmGetRegVal(dbg, val->decl->offset);
			ptr--;
		}
		else 
			ASSERT(0);
	}break;
	default:
		ASSERT(0);
	}
	//ptr--;
	if (val->type == IR_TYPE_REG)
		ptr--;
	while (ptr > 0 && val->type != IR_TYPE_INT)
	{
		ret = WasmGetMemOffsetVal(dbg, ret);
		ptr--;
	}
	return ret;
}
void WasmIrInterp(dbg_state* dbg, own_std::vector<ir_rep>* ir_ar)
{
	ir_rep* ir = ir_ar->begin();
	while (ir < ir_ar->end())
	{
		switch (ir->type)
		{
		case IR_RET:
		{
			if (ir->ret.no_ret_val)
			{
				ASSERT(0);
			}
			else
			{
				int final_val = WasmIrInterpGetIrVal(dbg, &ir->ret.assign.lhs);
				int* reg = (int*)&dbg->mem_buffer[(RET_1_REG + ir->ret.assign.to_assign.reg) * 8];
				*reg = final_val;

			}
		}break;
		case IR_ASSIGNMENT:
		{
			int final_val = 0;
			if (ir->assign.only_lhs)
			{
				final_val = WasmIrInterpGetIrVal(dbg, &ir->assign.lhs);
			}
			else
			{
				int lhs_val = WasmIrInterpGetIrVal(dbg, &ir->assign.lhs);
				if (ir->assign.op == T_POINT)
					final_val = lhs_val + ir->assign.rhs.decl->offset;

				else
				{
					int rhs_val = WasmIrInterpGetIrVal(dbg, &ir->assign.rhs);
					final_val = GetExpressionValT(ir->assign.op, lhs_val, rhs_val);
				}



			}

			switch (ir->assign.to_assign.type)
			{
			case IR_TYPE_REG:
			{
				int* reg = (int*)&dbg->mem_buffer[ir->assign.to_assign.reg * 8];
				*reg = final_val;
			}break;
			case IR_TYPE_RET_REG:
			{
				int* reg = (int*)dbg->mem_buffer[(RET_1_REG + ir->assign.to_assign.reg) * 8];
				*reg = final_val;
			}break;
			default:
				ASSERT(0);
			}
		}break;
		default:
			ASSERT(0);
		}
		ir++;

	}
}
void PrintExpressionTkns(dbg_state* dbg, own_std::vector<token2> *tkns)
{
	mem_alloc temp_alloc;
	temp_alloc.chunks_cap = 1024 * 1024;

	InitMemAlloc(&temp_alloc);
	void* prev_alloc = __lang_globals.data;
	dbg_expr* exp = nullptr;
	int val = setjmp(dbg->lang_stat->jump_buffer);
	if (val == 0)
	{
		dbg->lang_stat->flags |= PSR_FLAGS_ON_JMP_WHEN_ERROR;
		exp = WasmGetExprFromTkns(dbg, tkns);
	}
	// error
	else if (val == 1)
	{
		
	}
	if(exp)
		printf("\n%s\n", WasmGetSingleExprToStr(dbg, exp).c_str());

	FreeMemAlloc(&temp_alloc);

	__lang_globals.data = prev_alloc;

}

//void UpdateLastTime(dbg_state* dbg);
void WasmOnArgs(dbg_state* dbg)
{
	bool args_break = false;
	bool first_timer = true;

	printf("cur_block_bc %d\n", __lang_globals.cur_block);
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
			(*dbg->cur_bc)->one_time_dbg_brk = false;
		}
		first_timer = false;

		func_decl* func = dbg->cur_func;
		stmnt_dbg* stmnt = dbg->cur_st;


		for (int i = stmnt->line - 2; i < (stmnt->line + 1); i++)
		{
			i = max(0, i);
			i = min(func->from_file->lines.size(), i);
			char* cur_line = func->from_file->lines[i];

			if(i == (stmnt->line - 1))
				printf(ANSI_BG_WHITE ANSI_BLACK "%s" ANSI_RESET, cur_line);
			else
				printf("%s", cur_line);
			printf("\n", cur_line);
		}

		own_std::string input;

		// chatgt helped me here
		unsigned char ch;
		int cursorPos = 0;
		bool done = false;

		while (!done) {
			ch = 'a';
			//ch = _getch(); // Get a single character

			switch (ch) {
			case '\r': // Enter key
				cursorPos = 0;
				done = true;
			break;
			case '\t': // Tab key
				InsertSuggestion(dbg, input, &cursorPos);

				printInput(input, cursorPos);
			//displaySuggestions(input);
			break;
			case 27: // ESC key
				done = true;
			break;
			case 0:  // Special keys (like arrow keys) start with 0x00
			case 0xE0:  // For extended keys
				ch = 'a';
				//ch = _getch(); // Get the actual special key code
				if (ch == 75) { // Left arrow
					if (cursorPos > 0) {
						std::cout << "\033[" << 1 << "D";  // Move the cursor back
						--cursorPos;
					}
				} else if (ch == 77) { // Right arrow
					if (cursorPos < input.size()) {
						++cursorPos;
						std::cout << "\033[" << 1 << "C";  // Move the cursor back
					}
				} else if (ch == 83) { // DEL key
					if (cursorPos < input.size()) {
						input.erase(cursorPos, 1);
						printInput(input, cursorPos);
					}
				}
			break;
			default:
				if (ch == 8) { // Backspace key
					if (cursorPos > 0) {
						//input.erase(cursorPos - 1, 1);
						--cursorPos;
						printInput(input, cursorPos);
					}
				} else {
					//input.insert(cursorPos, 1, ch);
					++cursorPos;
					printInput(input, cursorPos);
				}
			break;
			}
		}


		//std::cin >> input;
		own_std::vector<own_std::string> args;
		/*
		if (dbg_state == DBG_STATE_ON_LANG)
			printf("WASM:");
		else
			printf("LANG:");
			*/

		own_std::vector<token2> tkns;
		Tokenize2((char*)input.c_str(), input.size(), &tkns);

		if (tkns[0].type == T_WORD && tkns[0].str == "print")
		{
			int i = 1;
			ASSERT(tkns[1].type == T_WORD);
			if (tkns[i].str == "wasm")
			{
				i++;
				bool can_break = false;

				func_decl* fdecl = dbg->cur_func;
				wasm_bc* cur_bc = *dbg->cur_bc;
				stmnt_dbg* cur_st = dbg->cur_st;

				int lines = -1;
				if (tkns[i].type == T_INT)
				{
					lines = tkns[i].i;
					can_break = true;
				}
				bool print_bc = true;
				while (!can_break)
				{
					can_break = true;
					if (tkns[i].str == "func")
					{
						i++;

						ASSERT(tkns[i].type == T_WORD);
						fdecl = FuncAddedWasmInterp(dbg->lang_stat->winterp, tkns[i].str);
						cur_st = fdecl->wasm_stmnts.begin();
						cur_bc = dbg->bcs.begin() + cur_st->start;
						i++;
					}
					else if (tkns[i].str == "ir")
					{
						i++;
						ASSERT(tkns[i].type == T_INT);

						print_bc = false;
						lines = tkns[i].i;
					}
					else if (tkns[i].str == "lines")
					{
						i++;
						ASSERT(tkns[i].type == T_INT);

						lines = tkns[i].i;
					}

					if (tkns[i].type == T_COMMA)
					{
						can_break = false;
						i++;
					}
				}

				own_std::string ret = WasmPrintCodeGranular(dbg, fdecl, cur_bc, cur_st->start, cur_st->end, lines, true, print_bc);
				printf("%s\n", ret.c_str());
			}
			else if (tkns[i].str == "ex")
			{
				i++;
				own_std::string exp_str = "";
				/*
				for (int i = 2; i < tkns.size(); i++)
				{
					//own_std::string cur = own_std::string(args[i]);;
					exp_str += tkns[i].str;
				}
				*/
				tkns.ar.start += i;
				PrintExpressionTkns(dbg, &tkns);
				tkns.ar.start -= i;


			}
			else if (tkns[i].str == "callstack")
			{
				FOR_VEC(func, dbg->func_stack)
				{
					func_decl* f = *func;
					printf("func %s\n", f->name.c_str());
				}
			}
		}
		own_std::string aux;
		split(input, ' ', args, &aux);

		if (args[0] == "print")
		{
			if (args.size() == 1)
				continue;

			else if (args[1] == "locals")
			{
				printf("%s\n", WasmPrintVars(dbg).c_str());
			}
			else if (args[1] == "filter")
			{
				own_std::string exp_str = "";
				for (int i = 2; i < args.size(); i++)
				{
					exp_str += args[i];
				}
				mem_alloc temp_alloc;
				temp_alloc.chunks_cap = 1024 * 1024;

				InitMemAlloc(&temp_alloc);
				void* prev_alloc = __lang_globals.data;
				dbg_expr* exp = nullptr;
				int val = setjmp(dbg->lang_stat->jump_buffer);
				if (val == 0)
				{
					dbg->lang_stat->flags |= PSR_FLAGS_ON_JMP_WHEN_ERROR;
					exp = WasmGetExprFromStr(dbg, exp_str);
				}
				// error
				else if (val == 1)
				{
					
				}
				if (!exp)
					continue;

				typed_stack_val expr_val;
				typed_stack_val x_times;
				WasmFromAstArrToStackVal(dbg, exp->expr, &expr_val);
				WasmFromAstArrToStackVal(dbg, exp->x_times, &x_times);

				type2 dummy_type;
				ASSERT(expr_val.type.type == TYPE_STRUCT && expr_val.type.ptr);

				expr_val.offset = WasmGetMemOffsetVal(dbg, expr_val.offset);
				int start_offset = expr_val.offset;
				char saved_regs[512];

				memcpy(saved_regs, dbg->mem_buffer, 128);

				own_std::string filtered = "";
				decl2* strct_decl = expr_val.type.strct->this_decl;

				for (int i = 0; i < x_times.offset; i++)
				{
					int cur_offset = start_offset + i * expr_val.type.strct->size;
					int* reg = (int*)&dbg->mem_buffer[FILTER_PTR * 8];
					*reg = cur_offset;
					//wasm_bc *bc = exp->f
					WasmIrInterp(dbg, &exp->filter_cond);

					char ret = WasmGetRegVal(dbg, RET_1_REG);
					if (ret == 1)
					{
						filtered += WasmVarToString(dbg, 0, strct_decl, cur_offset, 0);
					}
				}

				printf("%s", filtered.c_str());

				memcpy(dbg->mem_buffer, saved_regs, 128);
				FreeMemAlloc(&temp_alloc);

				__lang_globals.data = prev_alloc;


			}
			else if (args[1] == "onnext")
			{
				own_std::string exp_str = "";
				for (int i = 2; i < args.size(); i++)
				{
					exp_str += args[i];
				}
				mem_alloc temp_alloc;
				temp_alloc.chunks_cap = 1024 * 1024;

				InitMemAlloc(&temp_alloc);
				void* prev_alloc = __lang_globals.data;
				dbg_expr* exp = nullptr;
				int val = setjmp(dbg->lang_stat->jump_buffer);
				if (val == 0)
				{
					dbg->lang_stat->flags |= PSR_FLAGS_ON_JMP_WHEN_ERROR;
					exp = WasmGetExprFromStr(dbg, exp_str);
				}
				// error
				else if (val == 1)
				{
					
				}
				if (!exp)
					continue;
				ASSERT(exp->type == DBG_EXPR_SHOW_SINGLE_VAL);

				typed_stack_val expr_val;
				WasmFromAstArrToStackVal(dbg, exp->expr, &expr_val);

				type2 dummy_type;
				decl2 *next_var = FindIdentifier("next", expr_val.type.strct->scp, &dummy_type);
				ASSERT(expr_val.type.type == TYPE_STRUCT && next_var);
				ASSERT(next_var->type.type == TYPE_STRUCT && next_var->type.strct == expr_val.type.strct);
				ASSERT(next_var->type.ptr == 1);

				//expr_val.offset = WasmGetMemOffsetVal(dbg, expr_val.offset);
				if (expr_val.type.ptr > 0)
				{
					expr_val.offset = WasmGetMemOffsetVal(dbg, expr_val.offset);
				}
				int cur_offset = expr_val.offset;

				own_std::string struct_str = WasmVarToString(dbg, 0, next_var->type.strct->this_decl, cur_offset, 0);
				while (WasmGetMemOffsetVal(dbg, cur_offset + next_var->offset) != 0)
				{
					cur_offset = WasmGetMemOffsetVal(dbg, cur_offset + next_var->offset);
					struct_str += WasmVarToString(dbg, 0, next_var->type.strct->this_decl, cur_offset, 0);

				}

				printf("%s", struct_str.c_str());


				FreeMemAlloc(&temp_alloc);

				__lang_globals.data = prev_alloc;
			}
			else if (args[1] == "ex")
			{
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
				own_std::string exp_str = "";
				for (int i = 2; i < args.size(); i++)
				{
					exp_str += args[i];
				}


				dbg_expr* exp = WasmGetExprFromStr(dbg, exp_str);
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
	//UpdateLastTime(dbg);
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
	int struct_size;
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
	decl2* decl;
};
#define TYPE_DBG_CREATED_TYPE_STRUCT 1
struct type_dbg
{
	enum_type2 type;
	union
	{
		struct
		{
			import_type imp_type;
			str_dbg alias;
			union
			{
				int file_idx;
				unit_file *file;
			};
		}imp;
		str_dbg name;
		type_struct2 *strct;
		func_decl *fdecl;
		unit_file *file;
	};
	char ptr;
	int struct_size;
	int vars_offset;
	int num_of_vars;
	int flags;
};
struct scope_dbg
{
	int vars_offset;
	int num_of_vars;

	int imports;
	int num_of_imports;

	unsigned int parent;
	unsigned int next_sibling;
	int children;
	int children_len;

	int line_start;
	int line_end;

	scp_type type;
	union
	{
		int type_idx;
		int func_idx;
		int file_idx;
	};
	bool created;
	scope* scp;
};
struct file_dbg
{
	union
	{
		str_dbg name;
		unit_file* fl;
	};
	int scp_offset;
};
struct func_dbg
{
	union
	{
		str_dbg name;
		decl2* decl;
	}; 
	int scope;
	int code_start;

	int stmnts_offset;
	int stmnts_len;

	int ir_offset;
	int ir_len;

	int file_idx;

	int flags;
	bool created;

	int strct_constr_offset;
	int strct_ret_offset;
	int to_spill_offset;
};
struct serialize_state
{
	own_std::vector<unsigned char> func_sect;
	own_std::vector<unsigned char> stmnts_sect;
	own_std::vector<unsigned char> string_sect;
	//own_std::vector<unsigned char> ts_sect;
	own_std::vector<unsigned char> scopes_sect;
	own_std::vector<unsigned char> types_sect;
	own_std::vector<unsigned char> vars_sect;
	own_std::vector<unsigned char> ir_sect;
	own_std::vector<unsigned char> file_sect;
	own_std::vector<unsigned char> data_sect;

	unsigned int f32_type_offset;
	unsigned int u32_type_offset;
};
enum dbg_code_type
{
	DBG_CODE_WASM,
};
struct dbg_file_seriealize
{
	int func_sect;
	int total_funcs;
	int stmnts_sect;
	int string_sect;
	int scopes_sect;
	int types_sect;
	int vars_sect;
	int code_sect;
	int ir_sect;
	int files_sect;
	int data_sect;
	int data_sect_size;
	int total_files;

	dbg_code_type code_type;
};
void WasmSerializePushString(serialize_state* ser_state, own_std::string* name, str_dbg *);
void WasmSerializeStructType(web_assembly_state* wasm_state, serialize_state* ser_state, type_struct2* strct);

var_dbg *WasmSerializeSimpleVar(web_assembly_state* wasm_state, serialize_state* ser_state, decl2* var, int var_idx)
{
	auto var_ser = (var_dbg*)(ser_state->vars_sect.begin() + var_idx);
	memset(var_ser, 0, sizeof(var_dbg));
	if (var->type.type == TYPE_STRUCT )
	{
		type_struct2* strct = var->type.strct;
		if(IS_FLAG_OFF(strct->flags, TP_STRCT_STRUCT_SERIALIZED));
		{
			WasmSerializeStructType(wasm_state, ser_state, strct);
		}
		var_ser->type_idx = strct->serialized_type_idx;

	}
	var_ser->type = var->type.type;
	var_ser->ptr = var->type.ptr;
	var_ser->offset = var->offset;
	WasmSerializePushString(ser_state, &var->name, &var_ser->name);
	return var_ser;
}
void WasmSerializeStructType(web_assembly_state* wasm_state, serialize_state* ser_state, type_struct2* strct)
{
	if (IS_FLAG_ON(strct->flags, TP_STRCT_STRUCT_SERIALIZED))
		return;

	int type_offset = ser_state->types_sect.size();
	int var_offset = ser_state->vars_sect.size();

	/*
	FOR_VEC(decl, strct->vars)
	{
		decl2* d = *decl;
		// self
		if (d->type.strct == strct)
			continue;
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
	*/


	strct->serialized_type_idx = type_offset;

	//ser_state->vars_sect.make_count(ser_state->vars_sect.size() + strct->vars.size() * sizeof(var_dbg));
	ser_state->types_sect.make_count(ser_state->types_sect.size() + sizeof(type_dbg));

	auto type = (type_dbg*)(ser_state->types_sect.begin() + type_offset);
	memset(type, 0, sizeof(type_dbg));
	type->num_of_vars = strct->vars.size();
	type->vars_offset = var_offset;
	type->struct_size = strct->size;
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
	auto fdbg = (func_dbg*)(ser_state->func_sect.begin() + fdecl->func_dbg_idx);
	fdbg->ir_offset = ser_state->ir_sect.size();
	fdbg->ir_len = ir_ar->size();
	FOR_VEC(ir, (*ir_ar))
	{
		switch (ir->type)
		{
		case IR_INDIRECT_CALL:
		{
			decl2* decl = ir->bin.lhs.decl;
			ASSERT(IS_FLAG_ON(decl->flags, DECL_IS_SERIALIZED));
			ir->call.i = decl->serialized_type_idx;
		}break;
		case IR_CALL:
		{
			func_decl* call = ir->call.fdecl;
			ir->call.i = call->func_dbg_idx;
		}break;
		}
	}
	unsigned char* start = (unsigned char *)ir_ar->begin();
	unsigned char* end = (unsigned char *)ir_ar->end();
	ser_state->ir_sect.insert(ser_state->ir_sect.end(), start, end);
}
void WasmSerializeFunc(web_assembly_state* wasm_state, serialize_state *ser_state, func_decl *f)
{
	if (IS_FLAG_ON(f->flags, FUNC_DECL_SERIALIZED))
		return;
	bool is_outsider = IS_FLAG_ON(f->flags, FUNC_DECL_IS_OUTSIDER);
	int func_offset = ser_state->func_sect.size();
	int stmnt_offset = ser_state->stmnts_sect.size();


	ser_state->func_sect.make_count(ser_state->func_sect.size() + sizeof(func_dbg));
	auto fdbg = (func_dbg*)(ser_state->func_sect.begin() + func_offset);
	memset(fdbg, 0, sizeof(func_dbg));

	if (!is_outsider)
	{
		ser_state->stmnts_sect.insert(ser_state->stmnts_sect.end(), (unsigned char*)f->wasm_stmnts.begin(), (unsigned char*)f->wasm_stmnts.end());

		own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &f->ir;
	}

	fdbg->scope = f->scp->serialized_offset;
	fdbg->code_start = f->wasm_code_sect_idx;
	fdbg->stmnts_offset = stmnt_offset;
	fdbg->flags = f->flags;
	fdbg->stmnts_len = f->wasm_stmnts.size();
	fdbg->strct_constr_offset = f->strct_constrct_at_offset;
	fdbg->strct_ret_offset = f->strct_ret_size_per_statement_offset;
	fdbg->to_spill_offset = f->to_spill_offset;
	WasmSerializePushString(ser_state, &f->name, &fdbg->name);

	f->func_dbg_idx = func_offset;
	f->scp->type = SCP_TYPE_FUNC;
	f->flags |= FUNC_DECL_SERIALIZED;
}
int WasmSerializeType(web_assembly_state* wasm_state, serialize_state* ser_state, type2 *tp)
{
	int offset = ser_state->types_sect.size();

	ser_state->types_sect.make_count(ser_state->types_sect.size() + sizeof(type_dbg));

	auto type = (type_dbg *)(ser_state->types_sect.begin() + offset);
	if (tp->type == TYPE_IMPORT)
	{
		type->imp.imp_type = tp->imp->type;
		type->imp.file_idx = tp->imp->fl->file_dbg_idx;

		WasmSerializePushString(ser_state, &tp->imp->alias, &type->imp.alias);
		//type->imp.alias = tp->imp->fl->file_dbg_idx;
	}
	else
	{
		type->type = tp->type;
		type->ptr = tp->ptr;
		type->strct = tp->strct;
	}
	return offset;

}
void WasmSerializeScope(web_assembly_state* wasm_state, serialize_state *ser_state, scope *scp, unsigned int parent, unsigned int scp_offset)
{
	if(IS_FLAG_ON(scp->flags, SCOPE_SKIP_SERIALIZATION))
		return;
	struct f_ptr_info
	{
		int var_dbg_idx;
		decl2* decl;
	};
	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_STRUCT))
	{
		int a = 0;
	}

	INSERT_VEC(scp->vars, scp->imports);

	int vars_offset = ser_state->vars_sect.size();
	int children_offset = ser_state->scopes_sect.size();

	//ser_state->scopes_sect.make_count(ser_state->scopes_sect.size() + sizeof(scope_dbg));
	int size_children = scp->children.size() * sizeof(scope_dbg);
	ser_state->scopes_sect.make_count(ser_state->scopes_sect.size() + size_children);
	memset(ser_state->scopes_sect.begin() + children_offset, 0, size_children);
	ser_state->vars_sect.make_count(ser_state->vars_sect.size() + scp->vars.size() * sizeof(var_dbg));

	if (scp->type == SCP_TYPE_FUNC && scp->fdecl->name == "main")
		int a = 0;

	own_std::vector<f_ptr_info> func_ptr_idxs;
	int i = 0;
	FOR_VEC(decl, scp->vars)
	{
		decl2* d = *decl;
		var_dbg* vdbg = nullptr;
		int var_offset = vars_offset + i * sizeof(var_dbg);
		d->serialized_type_idx = var_offset;
		vdbg = WasmSerializeSimpleVar(wasm_state, ser_state, d, var_offset);

		switch (d->type.type)
		{
		case TYPE_TEMPLATE:
		case TYPE_ENUM_TYPE:
		{

		}break;
		case TYPE_FUNC_EXTERN:
		{
			int a = 0;
			goto func_label;
		};
		case TYPE_FUNC:
		{
			func_label:
			func_decl* f = d->type.fdecl;
			bool is_outsider = IS_FLAG_ON(f->flags, FUNC_DECL_IS_OUTSIDER);
			vdbg->type_idx = ser_state->func_sect.size();
			if (IS_FLAG_ON(f->flags, FUNC_DECL_SERIALIZED))
				vdbg->type_idx = f->func_dbg_idx;

			if (IS_FLAG_ON(f->flags, FUNC_DECL_INTERNAL | FUNC_DECL_SERIALIZED | FUNC_DECL_SERIALIZED | FUNC_DECL_TEMPLATED))
				break;

			WasmSerializeFunc(wasm_state, ser_state, f);
		}break;
		case TYPE_MACRO_EXPR:
		{

		}break;
		case TYPE_STRUCT_TYPE:
		{
			d->type.strct->scp->type = SCP_TYPE_STRUCT;
			WasmSerializeStructType(wasm_state, ser_state, d->type.strct);
			vdbg->type_idx = d->type.strct->serialized_type_idx;
		}break;
		case TYPE_FUNC_PTR:
		{
			f_ptr_info info = {  };
			info.decl = d;
			info.var_dbg_idx = var_offset;
			func_ptr_idxs.emplace_back(info);

		}break;
		case TYPE_STRUCT:
		{
			ASSERT(IS_FLAG_ON(d->type.strct->flags, TP_STRCT_STRUCT_SERIALIZED));
			if (IS_FLAG_ON(d->type.flags, TYPE_SELF_REF))
			{
				vdbg->type_idx = d->type.strct->serialized_type_idx;
			}
		}break;
		case TYPE_IMPORT:
		{
			vdbg->type_idx = WasmSerializeType(wasm_state, ser_state, &d->type);
		}break;
		case TYPE_STATIC_ARRAY:
		{
			vdbg->type_idx = WasmSerializeType(wasm_state, ser_state, d->type.tp);
		}break;
		case TYPE_BOOL:
		case TYPE_INT:
		case TYPE_F32:
		case TYPE_U8:
		case TYPE_S32:
		case TYPE_U32:
		case TYPE_S64:
		case TYPE_VOID:
		case TYPE_U64:
		case TYPE_FUNC_TYPE:
		case TYPE_STR_LIT:
		case TYPE_ENUM_IDX_32:
		case TYPE_U32_TYPE:
		case TYPE_CHAR:
		case TYPE_ENUM:
		
		break;
		default:
			ASSERT(0);
		}
		d->flags |= DECL_IS_SERIALIZED;
		i++;

	}
	FOR_VEC(f_ptr, func_ptr_idxs)
	{
		auto vdbg = (var_dbg*)(ser_state->vars_sect.begin() + f_ptr->var_dbg_idx);
		func_decl* f = f_ptr->decl->type.fdecl;
		// will skip the scope because it a func ptr declared as variable, and not outsiders or nomal functions
		if (!IS_FLAG_ON(f->flags, FUNC_DECL_SERIALIZED))
		{
			f->scp->flags |= SCOPE_SKIP_SERIALIZATION;
		}

		bool is_outsider = IS_FLAG_ON(f->flags, FUNC_DECL_IS_OUTSIDER);
		vdbg->type_idx = f->func_dbg_idx;
	}
	i = 0;
	FOR_VEC(ch, scp->children)
	{
		scope* c = *ch;
		int offset = children_offset + sizeof(scope_dbg) * i;
		WasmSerializeScope(wasm_state, ser_state, c, scp_offset, offset );
		auto cur = (scope_dbg*)(ser_state->scopes_sect.begin() + scp_offset);
		auto s = (scope_dbg*)(ser_state->scopes_sect.begin() + offset);
		s->next_sibling = ser_state->scopes_sect.size();
		i++;
	}

	auto s = (scope_dbg*)(ser_state->scopes_sect.begin() + scp_offset);
	memset(s, 0, sizeof(scope_dbg));
	s->children = children_offset;
	s->children_len = scp->children.size();
	s->num_of_vars  = scp->vars.size();
	s->line_start = scp->line_start;
	s->line_end = scp->line_end;
	s->vars_offset  = vars_offset;
	s->type = SCP_TYPE_UNDEFINED;




	switch(scp->type)
	{
	case SCP_TYPE_FUNC:
	{
		if (IS_FLAG_OFF(scp->fdecl->flags, FUNC_DECL_TEMPLATED))
		{
			ASSERT(IS_FLAG_ON(scp->fdecl->flags, FUNC_DECL_SERIALIZED));
		}
		s->type = SCP_TYPE_FUNC;
		s->func_idx = scp->fdecl->func_dbg_idx;
	}break;
	case SCP_TYPE_FILE:
	{

		s->type = SCP_TYPE_FILE;
		s->file_idx = scp->file->file_dbg_idx;
	}break;
	case SCP_TYPE_STRUCT:
	{
		ASSERT(IS_FLAG_ON(scp->tstrct->flags, TP_STRCT_STRUCT_SERIALIZED));
		s->type = SCP_TYPE_STRUCT;
		s->type_idx = scp->tstrct->serialized_type_idx;
	}break;
	}
	scp->serialized_offset = scp_offset;
	s->parent = parent;
}
void WasmSerializeBuiltTypes(serialize_state* ser_state)
{
	int offset = ser_state->types_sect.size();
	ser_state->f32_type_offset = offset;

	ser_state->types_sect.make_count(ser_state->types_sect.size() + sizeof(type_dbg));

	auto f32_type = (type_dbg *)(ser_state->types_sect.begin() + offset);
	f32_type->type = TYPE_F32;
}
void WasmSerializePushString(serialize_state* ser_state, own_std::string* name, str_dbg *out)
{
	out->name_on_string_sect = ser_state->string_sect.size();
	out->name_len = name->size();
	ser_state->string_sect.insert(ser_state->string_sect.end(), (unsigned char *)name->data(), (unsigned char *)(name->data() + name->size()));
}
void WasmSerialize(web_assembly_state* wasm_state, own_std::vector<unsigned char>& code)
{
	own_std::vector<unsigned char> final_buffer;
	serialize_state ser_state;
	//printf("\nscop : \n%s\n", wasm_state->lang_stat->root->Print(0).c_str());
	
	FOR_VEC(func, wasm_state->funcs)
	{
		func_decl* f = *func;
		auto fdbg = (func_dbg *)(ser_state.func_sect.begin() + f->func_dbg_idx);
		WasmSerializeFunc(wasm_state, &ser_state, f);
		fdbg = (func_dbg *)(ser_state.func_sect.begin() + f->func_dbg_idx);
		fdbg->file_idx = f->from_file->file_dbg_idx;
		//WasmSerializeFuncIr(&ser_state, f);

	}
	ser_state.file_sect.make_count(wasm_state->lang_stat->files.size() * sizeof(file_dbg));
	int i = 0;
	FOR_VEC(file, wasm_state->lang_stat->files)
	{
		unit_file* f = *file;
		
		int file_offset = i * sizeof(file_dbg);

		f->file_dbg_idx = file_offset;

		auto cur_dbg_f = (file_dbg*)(ser_state.file_sect.begin() + file_offset);

		WasmSerializePushString(&ser_state, &(f->path + "/" + f->name), &cur_dbg_f->name);
		i++;

	}

	WasmSerializeBuiltTypes(&ser_state);
	ser_state.scopes_sect.make_count(ser_state.scopes_sect.size() + sizeof(scope_dbg));
	WasmSerializeScope(wasm_state, &ser_state, wasm_state->lang_stat->root, -1, 0);

	i = 0;
	//printf("\nscop2: \n%s\n", wasm_state->lang_stat->root->Print(0).c_str());

	FOR_VEC(decl, wasm_state->lang_stat->funcs_scp->vars)
	{
		func_decl* f = (*decl)->type.fdecl;
		auto fdbg = (func_dbg *)(ser_state.func_sect.begin() + f->func_dbg_idx);
		//fdbg->ir_offset = ser_state.ir_sect.size();
		//fdbg->ir_len = f->ir.size();
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
	dbg_file_seriealize file;
	file.code_type = DBG_CODE_WASM;

	file.func_sect = final_buffer.size();
	INSERT_VEC(final_buffer, ser_state.func_sect);

	file.total_funcs = ser_state.func_sect.size() / sizeof(func_dbg);

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

	file.data_sect = final_buffer.size();

	auto casted_ar = (own_std::vector<unsigned char> *) &wasm_state->lang_stat->data_sect;
	final_buffer.insert(final_buffer.end(), casted_ar->begin(), casted_ar->end());
	file.data_sect_size = wasm_state->lang_stat->data_sect.size();

	file.total_files = wasm_state->lang_stat->files.size();

	final_buffer.insert(final_buffer.begin(), (unsigned char*)&file, ((unsigned char*)&file) + sizeof(dbg_file_seriealize));
	
	WriteFileLang((char *)(wasm_state->wasm_dir + wasm_state->folder_name + ".dbg").c_str(), final_buffer.begin(), final_buffer.size());

}
own_std::string WasmInterpNameFromOffsetAndLen(unsigned char* data, dbg_file_seriealize* file, str_dbg *name)
{
	unsigned char* string_sect = data + file->string_sect;
	return own_std::string((const char *)string_sect + name->name_on_string_sect, name->name_len);
}

decl2 *WasmInterpBuildSimplaVar(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file_seriealize* file, scope* scp_final, var_dbg* var)
{
	own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &var->name);
	type2 tp;
	tp.type = var->type;
	decl2* ret = NewDecl(lang_stat, name, tp);
	return ret;

}
decl2 *WasmInterpBuildStruct(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file_seriealize* file, type_dbg *dstrct)
{
	if (IS_FLAG_ON(dstrct->flags, TYPE_DBG_CREATED_TYPE_STRUCT))
		return dstrct->strct->this_decl;

	unsigned char* string_sect = data + file->string_sect;
	auto cur_var = (var_dbg *)(data + file->vars_sect + dstrct->vars_offset);

	auto final_struct = (type_struct2* )AllocMiscData(lang_stat, sizeof(type_struct2));
	type2 tp;
	tp.type = TYPE_STRUCT_TYPE;
	tp.strct = final_struct;
	own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &dstrct->name);
	dstrct->strct = final_struct;
	final_struct->name = own_std::string(name);
	dstrct->flags = TYPE_DBG_CREATED_TYPE_STRUCT;

	decl2* d = NewDecl(lang_stat, name, tp);
	final_struct->this_decl = d;
	return d;

}
unit_file* WasmInterpSearchFile(lang_state* lang_stat, own_std::string* name);
decl2 *WasmInterpBuildFunc(unsigned char *data, wasm_interp *winterp, lang_state *lang_stat, dbg_file_seriealize *file, func_dbg *fdbg)
{
	if (fdbg->created)
		return fdbg->decl;
	auto fdecl = (func_decl* )AllocMiscData(lang_stat, sizeof(func_decl));
	fdecl->code_start_idx = fdbg->code_start;
	fdecl->flags = fdbg->flags;

	own_std::string fname = WasmInterpNameFromOffsetAndLen(data, file, &fdbg->name);
	if (fname == "test_ptr")
		int a = 0;
	fdecl->name = own_std::string(fname);
	type2 tp;
	tp.type = TYPE_FUNC;
	tp.fdecl = fdecl;
	decl2* d = NewDecl(lang_stat, fname, tp);

	//ret_scp->vars.emplace_back(d);


	if (IS_FLAG_OFF(fdecl->flags, FUNC_DECL_MACRO| FUNC_DECL_IS_OUTSIDER))
	{
		own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) & fdecl->ir;
		auto start_ir = (ir_rep*)(data + file->ir_sect + fdbg->ir_offset);
		ir_ar->insert(ir_ar->begin(), start_ir, start_ir + fdbg->ir_len);

		auto start_st = (stmnt_dbg*)(data + file->stmnts_sect + fdbg->stmnts_offset);
		fdecl->wasm_stmnts.insert(fdecl->wasm_stmnts.begin(), start_st, start_st + fdbg->stmnts_len);
	}


	// for some reason macro functions'a from_files arent set, this is hack to make it now check it
	if (IS_FLAG_OFF(fdecl->flags, FUNC_DECL_MACRO))
	{
		auto fl_dbg = (file_dbg*)(data + file->files_sect + fdbg->file_idx);
		//own_std::string file_name = WasmInterpNameFromOffsetAndLen(data, file, &fl_dbg->name);
		fdecl->from_file = fl_dbg->fl;
		ASSERT(fdecl->from_file);
		winterp->funcs.emplace_back(fdecl);
	}
	fdecl->this_decl = d;
	fdecl->strct_constrct_at_offset = fdbg->strct_constr_offset;
	fdecl->strct_ret_size_per_statement_offset = fdbg->strct_ret_offset;
	fdecl->to_spill_offset = fdbg->to_spill_offset;

	fdbg->created = true;
	fdbg->decl = d;


	return d;

}
void WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file_seriealize* file, scope_dbg *scp_pre, scope *scp_final)
{
	unsigned char* string_sect = data + file->string_sect;
	auto cur_var = (var_dbg*)(data + file->vars_sect + scp_pre->vars_offset);
	for (int i = 0; i < scp_pre->num_of_vars; i++)
	{
		type2 tp = {};
		own_std::string name = own_std::string((const char *)string_sect + cur_var->name.name_on_string_sect, cur_var->name.name_len);
		if (name == "main_type")
			int a = 0;
		switch (cur_var->type)
		{
		case TYPE_FUNC_PTR:
		case TYPE_FUNC_EXTERN:
		{
			auto type_strct = (func_dbg*)(data + file->func_sect + cur_var->type_idx);
			tp.fdecl = type_strct->decl->type.fdecl;
			/*
			int a = 0;
			auto fdbg = (func_dbg*)(data + file->func_sect + cur_var->type_idx);
			//WasmInterpBuildFunc(data, lang_stat->winterp, lang_stat, file, fdbg, scp_final);
			int a = 0;
			*/
		}break;
		case TYPE_IMPORT:
		{
			auto tp_dbg = (type_dbg*)(data + file->types_sect + cur_var->type_idx);
			auto fl_dbg = (file_dbg*)(data + file->files_sect + tp_dbg->imp.file_idx);
			//auto fl_dbg = (file_dbg *)(data + file->files_sect + tp_dbg->imp.file_idx)

			//own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &type_strct->name);
			type2 tp;
			tp.type = enum_type2::TYPE_IMPORT;

			own_std::string alias;
			if (tp_dbg->imp.alias.name_len > 0)
			{
				alias = own_std::string((const char*)string_sect + tp_dbg->imp.alias.name_on_string_sect, tp_dbg->imp.alias.name_len);
			}

			tp.imp = NewImport(lang_stat, tp_dbg->imp.imp_type, alias, fl_dbg->fl);
			tp.imp->alias = alias.substr();

			own_std::string name = own_std::string("imp_") + alias;

			scp_final->imports.emplace_back(NewDecl(lang_stat, name, tp));
		}break;
		case TYPE_STRUCT:
		case TYPE_STRUCT_TYPE:
		{
			auto type_strct = (type_dbg*)(data + file->types_sect + cur_var->type_idx);
			ASSERT(IS_FLAG_ON(type_strct->flags, TYPE_DBG_CREATED_TYPE_STRUCT));
			type_struct2 * strct = type_strct->strct;
			strct->size = type_strct->struct_size;
			//own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &type_strct->name);
			//decl2* d = FindIdentifier(name, scp_final, &tp);
			//ASSERT(d);
			tp.type = TYPE_STRUCT;
			tp.strct = strct;

		}break;
		case TYPE_AUTO:
		case TYPE_TEMPLATE:
		case TYPE_FUNC_TYPE:
		case TYPE_FUNC:
		case TYPE_STATIC_ARRAY:
		case TYPE_BOOL:
		case TYPE_INT:
		case TYPE_MACRO_EXPR:
		case TYPE_ENUM_IDX_32:
		case TYPE_ENUM_TYPE:
		case TYPE_STR_LIT:
		case TYPE_CHAR:
		case TYPE_ENUM:
		{
			int a = 0;
		}break;
		case TYPE_F32:
		case TYPE_U8:
		case TYPE_S32:
		case TYPE_U32:
		case TYPE_S64:
		case TYPE_VOID:
		case TYPE_U64:
		case TYPE_U32_TYPE:
			break;
		default:
			ASSERT(0);
		}
		tp.type = cur_var->type;
		tp.ptr = cur_var->ptr;

		decl2 *d = FindIdentifier(name, scp_final, &tp);
		if (!d)
		{
			d = NewDecl(lang_stat, name, tp);
			d->offset = cur_var->offset;

			scp_final->vars.emplace_back(d);
		}
		if (d->type.type == TYPE_STATIC_ARRAY)
		{
			auto type = (type_dbg*)(data + file->types_sect + cur_var->type_idx);
			auto ar_tp = (type2 *)AllocMiscData(lang_stat, sizeof(type2));
			ar_tp->type = type->type;
			ar_tp->ptr = type->ptr;
			ar_tp->strct = type->strct;
			d->type.tp = ar_tp;
		}

		cur_var->decl = d;
		cur_var++;
	}

}
unit_file *WasmInterpSearchFile(lang_state* lang_stat, own_std::string *name)
{
	FOR_VEC(file, lang_stat->files)
	{
		if ((*file)->name == *name)
			return *file;
	}
	return nullptr;
}


scope *WasmInterpBuildScopes(wasm_interp *winterp, unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file_seriealize *file, scope *parent, scope_dbg *scp_pre, bool create_vars = false)
{
	scope* ret_scp = nullptr;
	if (scp_pre->created)
		ret_scp = scp_pre->scp;
	else
	{
		ret_scp = NewScope(lang_stat, parent);
		scp_pre->scp = ret_scp;
		scp_pre->created = true;
	}
	ret_scp->type = scp_pre->type;
	ret_scp->line_start = scp_pre->line_start;
	ret_scp->line_end = scp_pre->line_end;

	int child_offset = file->scopes_sect + scp_pre->children;
	auto cur_scp = (scope_dbg*)(data + child_offset);
	for (int i = 0; i < scp_pre->children_len; i++)
	{
		decl2* d = nullptr;
		if (cur_scp->type == SCP_TYPE_STRUCT)
		{
			auto dstrct = (type_dbg*)(data + file->types_sect + cur_scp->type_idx);

			d = WasmInterpBuildStruct(data, len, lang_stat, file, dstrct);

			//d->type.strct->scp = child_scp;


			//WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
		}
		if (cur_scp->type == SCP_TYPE_FUNC)
		{
			auto fdbg = (func_dbg *)(data + file->func_sect + cur_scp->func_idx);

			d = WasmInterpBuildFunc(data, winterp, lang_stat, file, fdbg);
			if(d->name == "main")
				int a = 0;

			//WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
		}
		//scope* new_scp = NewScope(lang_stat, ret_scp);
		scope *child_scp = WasmInterpBuildScopes(winterp, data, len, lang_stat, file, ret_scp, cur_scp, create_vars);

		if (!create_vars)
		{
			if (cur_scp->type == SCP_TYPE_FUNC)
			{
				ret_scp->vars.emplace_back(d);
				d->type.fdecl->scp = child_scp;

				child_scp->type = SCP_TYPE_FUNC;
				child_scp->fdecl = d->type.fdecl;
				auto tp_dbg = (func_dbg*)(data + file->func_sect + cur_scp->func_idx);
				tp_dbg->decl = d->type.fdecl->this_decl;

				//WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
			}
			if (cur_scp->type == SCP_TYPE_STRUCT)
			{
				//auto dstrct = (struct_dbg*)(data + file->types_sect + cur_scp->type_idx);

				//decl2 *d = WasmInterpBuildStruct(data, len, lang_stat, file, dstrct);
				child_scp->type = SCP_TYPE_STRUCT;
				child_scp->tstrct = d->type.strct;

				d->type.strct->scp = child_scp;

				ret_scp->vars.emplace_back(d);

				//WasmInterpBuildVarsForScope(unsigned char* data, unsigned int len, lang_state* lang_stat, dbg_file* file, scope_dbg *scp_pre, scope *scp_final)
			}
		}
		cur_scp++;
	}
	if(create_vars)
		WasmInterpBuildVarsForScope(data, len, lang_stat, file, scp_pre, ret_scp);

	if (ret_scp->type == SCP_TYPE_FILE)
	{
		auto fl = (file_dbg*)(data + file->files_sect + scp_pre->file_idx);
		fl->fl->global = ret_scp;
	}


	return ret_scp;
}
inline bool WasmBcLogic(wasm_interp* winterp, dbg_state& dbg, wasm_bc** cur_bc, unsigned char* mem_buffer, block_linked** cur, bool &can_break)
{
	own_std::vector<wasm_stack_val> &wasm_stack = dbg.wasm_stack;
	wasm_stack_val val = {};
	switch ((*cur_bc)->type)
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
		while (i < (*cur_bc)->i)
		{
			(*cur) = (*cur)->parent;
			FreeBlock((*cur));
			i++;
		}
		if ((*cur)->wbc->type == WASM_INST_LOOP)
		{
			(*cur_bc) = (*cur)->wbc + 1;
			return true;
		}
		if (!(*cur))
		{
			can_break = true;
			break;
		}
		(*cur_bc) = (*cur)->wbc->block_end;
		return true;
		
	}break;
	case WASM_INST_BREAK_IF:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();

		if (top.s32 != 1)
			break;
		wasm_bc *label;
		int i = 0;
		while (i < (*cur_bc)->i)
		{
			(*cur) = (*cur)->parent;
			FreeBlock((*cur));
			i++;
		}
		if (!(*cur))
		{
			can_break = true;
			break;
		}
		(*cur_bc) = (*cur)->wbc->block_end;
		return true;
	}break;
	case WASM_INST_INDIRECT_CALL:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		func_decl* call_f = dbg.wasm_state->funcs[top.u32];
		WasmDoCallInstruction(&dbg, &(*cur_bc), &(*cur), call_f);
	}break;
	case WASM_INST_CALL:
	{
		func_decl* call_f = dbg.wasm_state->funcs[(*cur_bc)->i];
		if (IS_FLAG_ON(call_f->flags, FUNC_DECL_IS_OUTSIDER))
		{
			if (winterp->outsiders.find(call_f->name.c_str()) != winterp->outsiders.end())
			{
				OutsiderFuncType func_ptr = winterp->outsiders[call_f->name.c_str()];
				func_ptr(&dbg);
			}
			else
				ASSERT(0);

		}
		else
		{
			WasmDoCallInstruction(&dbg, &(*cur_bc), &(*cur), call_f);
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
		while ((*cur))
		{
			FreeBlock((*cur));
			(*cur) = (*cur)->parent;

		}
		dbg.cur_func = dbg.func_stack.back();
		
		(*cur_bc) = dbg.return_stack.back();
		dbg.return_stack.pop_back();

		(*cur) = dbg.block_stack.back();
		dbg.block_stack.pop_back();
	}break;
	case WASM_INST_I32_EQ:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->s32 = (int) (penultimate->u32 == top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_GE_U:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->s32 = (int)(penultimate->u32 >= top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_GT_U:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->u32 > top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_NE:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->u32 != top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_LE_S:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->s32 <= top.s32);
		int a = 0;
	}break;
	case WASM_INST_I32_LT_S:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->s32 < top.s32);
		int a = 0;
	}break;
	case WASM_INST_I32_GT_S:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->s32 > top.s32);
		int a = 0;
	}break;
	case WASM_INST_I32_GE_S:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->s32 >= top.s32);
		int a = 0;
	}break;
	case WASM_INST_I32_LE_U:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->u32 <= top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_LT_U:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (int)(penultimate->u32 < top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_REMAINDER_S:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (penultimate->s32 % top.s32);
		int a = 0;
	}break;
	case WASM_INST_I32_REMAINDER_U:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (penultimate->u32 % top.u32);
		int a = 0;
	}break;
	case WASM_INST_I32_DIV_S:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->s32 = (penultimate->s32 / top.s32);
		int a = 0;
	}break;
	case WASM_INST_I32_DIV_U:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->u32 = (penultimate->u32 / top.u32);
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
		ASSERT(penultimate.u32 < dbg.mem_size);
		*(char*)&mem_buffer[penultimate.u32] = top.s32;
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
		ASSERT(penultimate.u32 < dbg.mem_size);
		*(float*)&mem_buffer[penultimate.u32] = top.f32;
		int a = 0;
	}break;
	case WASM_INST_I64_STORE:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();

		auto penultimate = wasm_stack.back();
		wasm_stack.pop_back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate.type == 0);
		ASSERT(penultimate.u32 < dbg.mem_size);
		*(long long*)&mem_buffer[penultimate.u32] = top.s64;
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
		ASSERT(penultimate.u32 < dbg.mem_size);
		*(int*)&mem_buffer[penultimate.u32] = top.s32;
		int a = 0;
	}break;
	case WASM_INST_F32_CONST:
	{
		auto top = wasm_stack.back();
		val.type = WSTACK_VAL_F32;
		val.f32 = (*cur_bc)->f32;
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
	case WASM_INST_F32_GE:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->u32 = (int)(penultimate->f32 >= top.f32);
		int a = 0;
	}break;
	case WASM_INST_F32_DIV:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->f32 = (penultimate->f32 / top.f32);
		int a = 0;
	}break;
	case WASM_INST_F32_NE:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->u32 = (int)(penultimate->f32 != top.f32);
		int a = 0;
	}break;
	case WASM_INST_F32_EQ:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->u32 = (int)(penultimate->f32 == top.f32);
		int a = 0;
	}break;
	case WASM_INST_F32_LE:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->u32 = (int)(penultimate->f32 <= top.f32);
		int a = 0;
	}break;
	case WASM_INST_F32_GT:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->u32 = (int)(penultimate->f32 > top.f32);
		int a = 0;
	}break;
	case WASM_INST_F32_LT:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == wstack_val_type::WSTACK_VAL_F32 && penultimate->type == wstack_val_type::WSTACK_VAL_F32)
		penultimate->u32 = (int)(penultimate->f32 < top.f32);
		int a = 0;
	}break;
	case WASM_INST_I32_LOAD_8_S:
	{
		auto w = &wasm_stack.back();
		// assert is 32bit
		ASSERT(w->type == WSTACK_VAL_INT);
		w->s32 = *(char*)&mem_buffer[w->s32];
		int a = 0;
	}break;
	case WASM_INST_I64_LOAD:
	{
		auto w = &wasm_stack.back();
		// assert is 32bit
		ASSERT(w->type == WSTACK_VAL_INT);
		w->s64 = *(long long*)&mem_buffer[w->s64];
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
		val.s32 = (*cur_bc)->i;
		wasm_stack.emplace_back(val);
	}break;
	case WASM_INST_LOOP:
	case WASM_INST_BLOCK:
	{
		(*cur) = NewBlock((*cur));
		(*cur)->wbc = (*cur_bc);
	}break;
	case WASM_INST_I32_OR:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->s32 |= top.s32;
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
	case WASM_INST_CAST_F32_2_S32:
	{
		auto top = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top->type == WSTACK_VAL_F32)
		top->s32 = top->f32;
		top->type = WSTACK_VAL_INT;
	}break;
	case WASM_INST_CAST_S64_2_F32:
	{
		auto top = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top->type == WSTACK_VAL_INT)
		top->f32 = top->s64;
		top->type = WSTACK_VAL_F32;
	}break;
	case WASM_INST_CAST_S32_2_F32:
	{
		auto top = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top->type == WSTACK_VAL_INT)
		top->f32 = top->s32;
		top->type = WSTACK_VAL_F32;
	}break;
	case WASM_INST_CAST_U32_2_F32:
	{
		auto top = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top->type == WSTACK_VAL_INT)
		top->f32 = top->u32;
		top->type = WSTACK_VAL_F32;
	}break;
	case WASM_INST_F32_MUL:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == WSTACK_VAL_F32 && penultimate->type == WSTACK_VAL_F32);
		penultimate->f32 *= top.f32;
	}break;
	case WASM_INST_F32_SUB:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == WSTACK_VAL_F32 && penultimate->type == WSTACK_VAL_F32);
		penultimate->f32 -= top.f32;
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
	case WASM_INST_DBG_BREAK:
	{
		printf(ANSI_RED "debug break hit\n" ANSI_RESET);
		dbg.break_type = DBG_BREAK_ON_NEXT_BC;
		return true;

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
	case WASM_INST_I32_AND:
	{
		auto top = wasm_stack.back();
		wasm_stack.pop_back();
		auto penultimate = &wasm_stack.back();
		// assert is 32bit
		ASSERT(top.type == 0 && penultimate->type == 0);
		penultimate->s32 &= top.s32;
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
		if ((*cur))
		{
			FreeBlock((*cur));
			(*cur) = (*cur)->parent;
		}

	}break;
	default:
		ASSERT(0)
	}

	return false;
}
void WasmInterpRun(wasm_interp* winterp, unsigned char* mem_buffer, unsigned int len, own_std::string func_start, long long* args, int total_args)
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

	int stack_offset = 10000;
	*(int*)&mem_buffer[STACK_PTR_REG * 8] = stack_offset;
	*(int*)&mem_buffer[BASE_STACK_PTR_REG * 8] = stack_offset;

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
	//bcs[first_start_offset].one_time_dbg_brk = true;
	
	// WASM BYTECODE

	bool can_break = false;
	dbg.break_type = DBG_BREAK_ON_NEXT_STAT;

	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) &cur_func->ir;
	//ir_rep* cur_ir = ir_ar->begin();

	while(!can_break)
	{
		int bc_idx = (long long)(bc - &bcs[0]);
		wasm_stack_val val = {};
		stmnt_dbg* cur_st = nullptr;
		cur_st = GetStmntBasedOnOffset(&dbg.cur_func->wasm_stmnts, bc_idx);
		ir_rep* cur_ir = nullptr;
		cur_ir = GetIrBasedOnOffset(&dbg, bc_idx);
		bool found_stat = cur_st && dbg.cur_st;
		bool is_different_stmnt =  found_stat && dbg.break_type == DBG_BREAK_ON_DIFF_STAT && cur_st->line != dbg.cur_st->line;
		bool is_different_stmnt_same_func = found_stat && dbg.break_type == DBG_BREAK_ON_DIFF_STAT_BUT_SAME_FUNC && cur_st->line != dbg.cur_st->line && dbg.next_stat_break_func == dbg.cur_func;

		if ( dbg.break_type == DBG_BREAK_ON_NEXT_BC || is_different_stmnt || is_different_stmnt_same_func || bc->dbg_brk || bc->one_time_dbg_brk)
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
			if (bc->type == WASM_INST_DBG_BREAK)
			{
				bc++;
				continue;
			}
			bc->one_time_dbg_brk = false;
		}
		if (WasmBcLogic(winterp, dbg, &bc, mem_buffer, &cur, can_break))
			continue;
		bc++;
	}
}
func_decl* WasmInterpFindFunc(wasm_interp* winterp, own_std::string func_name)
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

void WasmInterpPatchIr(own_std::vector<ir_rep>* ir_ar, wasm_interp* winterp, dbg_file_seriealize* file)
{
	unsigned char* start_f = ((unsigned char*)(file + 1)) + file->func_sect;
	unsigned char* start_vars = ((unsigned char*)(file + 1)) + file->vars_sect;
	unsigned char* start_type = ((unsigned char*)(file + 1)) + file->types_sect;
	FOR_VEC(ir, (*ir_ar))
	{
		switch (ir->type)
		{
		case IR_INDIRECT_CALL:
		{
			auto vdbg = (var_dbg*)(start_vars + ir->call.i);
			decl2* d = vdbg->decl;
			ASSERT(d);
			ir->decl = d;

		}break;
		case IR_CALL:
		{
			auto fdbg = (func_dbg*)(start_f + ir->call.i);
			ASSERT(fdbg->created);
			func_decl* fdecl = fdbg->decl->type.fdecl;
			if (IS_FLAG_ON(fdecl->flags, FUNC_DECL_IS_OUTSIDER))
				ir->call.outsider = winterp->outsiders[fdecl->name.c_str()];

			ASSERT(fdecl);

			ir->call.fdecl = fdecl;
		}break;
		}
	}
}
own_std::string PrintScpPre(unsigned char* start, scope_dbg* s)
{

	own_std::string ret = "{";
	char buffer[512];
	snprintf(buffer, 512, "type: %d", s->type);
	ret += buffer;

	for (int i = 0; i < s->children_len; i++)
	{
		ret += PrintScpPre(start, (scope_dbg*)(start + s->children + i * sizeof(scope_dbg)));
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

own_std::string WasmCmdPrintWasmFuncAutoComplete(dbg_state* dbg, command_info_args *info)
{
	own_std::string ret = "";

	own_std::vector<own_std::string> names_found;
		
	FOR_VEC(func, dbg->lang_stat->winterp->funcs)
	{
		own_std::string n = (*func)->name;
		bool is_equal = true;
		for (int j = 0; j < info->incomplete_str.size(); j++)
		{
			if (info->incomplete_str[j] != n[j])
			{
				is_equal = false;
				break;
			}
		}
		if (is_equal)
		{
			names_found.emplace_back(n);
		}
	}
	if (names_found.size() == 1)
		return names_found[0];

	int shortest_str_idx = -1;
	int shortest_str_count = 60000;

	int second_shortest_str_idx = -1;
	
	int i = 0;
	FOR_VEC(str, names_found)
	{
		if (str->size() <= shortest_str_count)
		{
			second_shortest_str_idx = shortest_str_idx;
			shortest_str_idx = i;
			shortest_str_count = str->size();
		}
		i++;
	}

	own_std::string shortest_str = names_found[shortest_str_idx];
	own_std::string second_shortest_str = names_found[second_shortest_str_idx];

	int j = 0;
	for (; j < shortest_str.size(); j++)
	{
		if (shortest_str[j] != second_shortest_str[j])
		{
			break;
		}
	}

	ret = second_shortest_str.substr(0, j);

	return ret;
}
void WasmInterpInit(wasm_interp* winterp, unsigned char* data, unsigned int len, lang_state* lang_stat)
{

	auto file = (dbg_file_seriealize*)data;
	data = (unsigned char*)(file + 1);

	unsigned char* code = data + file->code_sect;

	//int point_idx = file_name.find_last_of('.');
	//own_std::string data_file_name = file_name.substr(0, point_idx)+"_data.dbg";
	auto file_data_sect = (char*)data + file->data_sect;
	lang_stat->data_sect.insert(lang_stat->data_sect.begin(), file_data_sect, file_data_sect + file->data_sect_size);

	lang_stat->files.clear();

	for (int i = 0; i < file->total_files; i++)
	{
		auto new_f = (unit_file*)AllocMiscData(lang_stat, sizeof(unit_file));

		auto cur_file = (file_dbg*)(data + file->files_sect + i * sizeof(file_dbg));

		own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &cur_file->name);

		new_f->name = own_std::string(name);
		unsigned  int read = 0;
		new_f->contents = ReadEntireFileLang((char*)new_f->name.c_str(), &read);
		char* cur_str = new_f->contents;
		cur_file->fl = new_f;

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
	own_std::vector<char *> strs;

	for (int f = 0; f < file->total_funcs; f++)
	{
		auto fdbg = (func_dbg*)(data + file->func_sect + f * sizeof(func_dbg));
		if (IS_FLAG_ON(fdbg->flags, FUNC_DECL_MACRO))
			continue;
		own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &fdbg->name);
		//printf("\n%d: func_name: %s", (int)strs.size(), name.c_str());
		strs.emplace_back(std_str_to_heap(lang_stat, &name));

		auto d = WasmInterpBuildFunc(data, winterp, lang_stat, file, fdbg);

	}


	auto scp_pre = (scope_dbg*)(data + file->scopes_sect);
	scope* root = WasmInterpBuildScopes(winterp, data, len, lang_stat, file, nullptr, scp_pre, false);
	root = WasmInterpBuildScopes(winterp, data, len, lang_stat, file, nullptr, scp_pre, true);
	own_std::string scp_pre_str = PrintScpPre(data + file->scopes_sect, scp_pre);
	//printf("\nscop 3:\n%s", root->Print(0).c_str());

	for (int i = 0; i < file->total_funcs; i++)
	{
		auto fdbg = (func_dbg*)(data + file->func_sect + i * sizeof(func_dbg));
		if (IS_FLAG_ON(fdbg->flags, FUNC_DECL_IS_OUTSIDER))
		{
			//own_std::string name = WasmInterpNameFromOffsetAndLen(data, file, &fdbg->name);

		}

	}

	FOR_VEC(func, winterp->funcs)
	{
		func_decl* f = *func;
		//if(f->)
		own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) & f->ir;
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

	winterp->dbg = (dbg_state*)AllocMiscData(lang_stat, sizeof(dbg_state));
	dbg_state& dbg = *winterp->dbg;
	//dbg.mem_size = size;
	dbg.lang_stat = lang_stat;

	auto global_cmds = (command_info *)AllocMiscData(lang_stat, sizeof(command_info));
	dbg.global_cmd = global_cmds;

	auto print_command = (command_info *)AllocMiscData(lang_stat, sizeof(command_info));
	print_command->names.emplace_back("print");
	print_command->names.emplace_back("p");

	auto wasm_sub_cmd = (command_info *)AllocMiscData(lang_stat, sizeof(command_info));
	wasm_sub_cmd->names.emplace_back("wasm");

	auto wasm_sub_cmd_func = (command_info *)AllocMiscData(lang_stat, sizeof(command_info));
	wasm_sub_cmd_func->func = WasmCmdPrintWasmFuncAutoComplete;
	wasm_sub_cmd_func->end = true;
	wasm_sub_cmd_func->names.emplace_back("func");

	auto wasm_sub_cmd_lines = (command_info *)AllocMiscData(lang_stat, sizeof(command_info));
	wasm_sub_cmd_lines->end = true;
	wasm_sub_cmd_lines->names.emplace_back("lines");

	wasm_sub_cmd->cmds.emplace_back(wasm_sub_cmd_func);
	wasm_sub_cmd->cmds.emplace_back(wasm_sub_cmd_lines);

	print_command->cmds.emplace_back(wasm_sub_cmd);


	global_cmds->cmds.emplace_back(print_command);


	//print_command_func->names.emplace_back("func");


	//dbg.wasm_state = wasm_state;

	own_std::vector<wasm_bc>& bcs = dbg.bcs;
	while (i < winterp->funcs.size())
	{
		func_decl* cur_f = winterp->funcs[i];
		if (IS_FLAG_ON(cur_f->flags, FUNC_DECL_IS_OUTSIDER | FUNC_DECL_MACRO))
		{
			i++;
			continue;
		}
		int func_sz = decodeSLEB128(&code[ptr], &consumed);
		ASSERT(func_sz > 0);
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
			case 0x47:
			{
				bc.type = WASM_INST_I32_NE;
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
				unsigned int* int_ptr = (unsigned int*)&code[ptr];
				//*(int*)&bc.f32 = (*int_ptr << 24) | (((*int_ptr) & 0xff00) << 8) | (((*int_ptr) & 0xff0000) >> 8) | (((*int_ptr) >> 24));
				*(int*)&bc.f32 = *int_ptr;

				ptr += 4;
			}break;
			case 0x93:
			{
				bc.type = WASM_INST_F32_SUB;
			}break;
			case 0x92:
			{
				bc.type = WASM_INST_F32_ADD;
			}break;
			case 0x6a:
			{
				bc.type = WASM_INST_I32_ADD;
			}break;
			case 0x6f:
			{
				bc.type = WASM_INST_I32_REMAINDER_S;
			}break;
			case 0x70:
			{
				bc.type = WASM_INST_I32_REMAINDER_U;
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
			case 0x37:
			{
				bc.type = WASM_INST_I64_STORE;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0x29:
			{
				bc.type = WASM_INST_I64_LOAD;
				// alignment
				ptr++;
				// offset
				ptr++;

			}break;
			case 0xf:
			{
				bc.type = WASM_INST_RET;
			}break;
			case 0x2c:
			{
				bc.type = WASM_INST_I32_LOAD_8_S;
				// alignment
				ptr++;
				// offset
				ptr++;

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
			case 0x6d:
			{
				bc.type = WASM_INST_I32_DIV_S;

			}break;
			case 0x6e:
			{
				bc.type = WASM_INST_I32_DIV_U;

			}break;
			case 0x46:
			{
				bc.type = WASM_INST_I32_EQ;

			}break;
			case 0x72:
			{
				bc.type = WASM_INST_I32_OR;

			}break;
			case 0x71:
			{
				bc.type = WASM_INST_I32_AND;

			}break;
			case 0x4a:
			{
				bc.type = WASM_INST_I32_GT_S;

			}break;
			case 0x4b:
			{
				bc.type = WASM_INST_I32_GT_U;

			}break;
			case 0x4c:
			{
				bc.type = WASM_INST_I32_LE_S;

			}break;
			case 0x4d:
			{
				bc.type = WASM_INST_I32_LE_U;

			}break;
			case 0x48:
			{
				bc.type = WASM_INST_I32_LT_S;

			}break;
			case 0x49:
			{
				bc.type = WASM_INST_I32_LT_U;

			}break;
			case 0x5c:
			{
				bc.type = WASM_INST_F32_NE;

			}break;
			case 0x5b:
			{
				bc.type = WASM_INST_F32_EQ;

			}break;
			case 0x5f:
			{
				bc.type = WASM_INST_F32_LE;

			}break;
			case 0x5d:
			{
				bc.type = WASM_INST_F32_LT;

			}break;
			case 0x95:
			{
				bc.type = WASM_INST_F32_DIV;

			}break;
			case 0xb2:
			{
				bc.type = WASM_INST_CAST_S32_2_F32;

			}break;
			case 0xa8:
			{
				bc.type = WASM_INST_CAST_F32_2_S32;

			}break;
			case 0xb9:
			{
				bc.type = WASM_INST_CAST_S64_2_F32;

			}break;
			case 0xb3:
			{
				bc.type = WASM_INST_CAST_U32_2_F32;
			}break;
			case 0xba:
			{
				bc.type = WASM_INST_CAST_U32_2_F32;

			}break;
			case 0x60:
			{
				bc.type = WASM_INST_F32_GT;

			}break;
			case 0x94:
			{
				bc.type = WASM_INST_F32_MUL;

			}break;
			case 0x5e:
			{
				bc.type = WASM_INST_F32_GE;

			}break;
			case 0xff:
			{
				bc.type = WASM_INST_DBG_BREAK;
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
			while (cur_ir->start == cur_ir->end && cur_ir < ir_ar->end())
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
void WasmInterp(own_std::vector<unsigned char>& code, char* mem_buffer, int size, own_std::string func_start_name, web_assembly_state* wasm_state, long long* args, int total_args)
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
				int* int_ptr = (int*)&code[ptr];
				*(int*)&bc.f32 = (*int_ptr << 24) | (((*int_ptr) & 0xff00) << 8) | (((*int_ptr) & 0xff0000) >> 8) | (((*int_ptr) >> 24));

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

	own_std::vector<func_decl*>& func_stack = dbg.func_stack;
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


	own_std::vector<wasm_stack_val>& wasm_stack = dbg.wasm_stack;
	wasm_stack.reserve(16);

	//FOR_VEC(bc, wf->bcs)
	wasm_bc* bc = &bcs[cur_func->wasm_code_sect_idx];
	//bc->one_time_dbg_brk = true;

	int stack_offset = 1000;
	*(int*)&mem_buffer[STACK_PTR_REG * 8] = stack_offset;

	auto stack_ptr_val = (long long*)&mem_buffer[stack_offset];

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

	own_std::vector<ir_rep>* ir_ar = (own_std::vector<ir_rep> *) & cur_func->ir;
	//ir_rep* cur_ir = ir_ar->begin();

	while (!can_break)
	{
		int bc_idx = (long long)(bc - &bcs[0]);
		wasm_stack_val val = {};
		stmnt_dbg* cur_st = GetStmntBasedOnOffset(&dbg.cur_func->wasm_stmnts, bc_idx);
		ir_rep* cur_ir = GetIrBasedOnOffset(&dbg, bc_idx);
		bool is_different_stmnt = cur_st && dbg.cur_st && dbg.break_type == DBG_BREAK_ON_DIFF_STAT && cur_st->line != dbg.cur_st->line;

		if (dbg.break_type == DBG_BREAK_ON_NEXT_BC || is_different_stmnt || bc->dbg_brk || bc->one_time_dbg_brk)
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
			wasm_bc* label;
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
			wasm_bc* label;
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
				if (wasm_state->outsiders.find(call_f->name.c_str()) != wasm_state->outsiders.end())
				{
					OutsiderFuncType func_ptr = wasm_state->outsiders[call_f->name.c_str()];
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
			ASSERT(wasm_stack.size() == 0);
		}break;
		case WASM_INST_I32_GE_U:
		{
			auto top = wasm_stack.back();
			wasm_stack.pop_back();
			auto penultimate = &wasm_stack.back();
			// assert is 32bit
			ASSERT(top.type == 0 && penultimate->type == 0);
			penultimate->u32 = (int)(penultimate->u32 >= top.u32);
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
			ASSERT(penultimate.u32 < dbg.mem_size);
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
			ASSERT(penultimate.u32 < dbg.mem_size);
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
			ASSERT(penultimate.u32 < dbg.mem_size);
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

void WasmPushNameIntoArray(own_std::vector<unsigned char>* out, own_std::string name)
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
	WasmPushConst(WASM_TYPE_INT, 0, 0, &element_sect);
	element_sect.emplace_back(0xb);
	own_std::vector<unsigned char> uleb;

	uleb.clear();
	GenUleb128(&uleb, wasm_state->funcs.size() + wasm_state->imports.size());
	INSERT_VEC(element_sect, uleb);
	FOR_VEC(func, wasm_state->imports)
	{
		func_decl* f = (*func)->type.fdecl;
		int idx = 0;
		FuncAddedWasm(wasm_state, f->name, &idx);

		uleb.clear();
		GenUleb128(&uleb, idx);
		INSERT_VEC(element_sect, uleb);

	}
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

		(*f)->wasm_func_sect_idx = funcs_inserted + wasm_state->imports.size();
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
			if (ex->name == "_own_memset")
				auto a = 0;
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
	/*
	uleb.clear();
	//encodeSLEB128(&uleb, out->size());
	GenUleb128(&uleb, 0x90);
	INSERT_VEC(table_sect, uleb);
	uleb.clear();
	//encodeSLEB128(&uleb, out->size());
	GenUleb128(&uleb, 0x90);
	INSERT_VEC(table_sect, uleb);
	*/
	table_sect.emplace_back(0x70);
	table_sect.emplace_back(0x70);

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
	ASSERT(wasm_state->funcs.size() > 0);
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

	lang_state* lang_stat = wasm_state->lang_stat;
	WriteFileLang((char*)(wasm_state->wasm_dir + wasm_state->folder_name + ".wasm").c_str(), ret->begin(), ret->size());
	WriteFileLang((char*)(wasm_state->wasm_dir + wasm_state->folder_name + "_data_sect").c_str(), lang_stat->data_sect.begin(), lang_stat->data_sect.size());

	if (wasm_state->lang_stat->release)
	{
		unsigned int read = 0;
		char* images_data = ReadEntireFileLang("../web/images.data", &read);

		own_std::string code_str((char*)ret->begin(), ret->size());
		own_std::string encoded_str = base64_encode(code_str);
		own_std::string data_sect_str((char*)lang_stat->data_sect.begin(), lang_stat->data_sect.size());
		own_std::string data_sect_encoded_str = base64_encode(data_sect_str);

		own_std::string page =
			own_std::string(
				"<!DOCTYPE html>\n\
<html>\
<script src = \"main.js\"></script>\
<script src = \"https://greggman.github.io/webgl-lint/webgl-lint.js\" crossorigin></script>\
<audio id = \"music\">\
<source src = \"10.mp3\" type = \"audio/mp3\">\
Your browser does not support the audio element.\
</audio>\
<canvas id = \"glcanvas\" width = \"640\" height = \"480\">< / canvas>\
<p id = \"dt\">< / p>\
<p id = \"wasm_module\">") + encoded_str + own_std::string("</p>\
<p id = \"wasm_data_sect\">") + data_sect_encoded_str + own_std::string("</p>\
<p id = \"wasm_images\">") + images_data + own_std::string("</p>\
</html>\
");

		WriteFileLang((char *)(wasm_state->wasm_dir+"index.html").c_str(), (char*)page.data(), page.size());
	}

	for (int i = wasm_state->imports.size() - 1; i >=0 ; i--)
	{
		decl2* d = *(wasm_state->imports.begin() + i);

		if (d->type.type == TYPE_FUNC)
			wasm_state->funcs.insert(0, d->type.fdecl);
	}

	int i = 0;
	FOR_VEC(decl, wasm_state->funcs)
	{
		func_decl* d = *decl;
		
		own_std::string st = d->name;
		//printf("%d: %s\n", i, st.c_str());

		i++;
	}


	if(!wasm_state->lang_stat->release)
		WasmSerialize(wasm_state, final_code_sect);

	//WasmInterp(final_code_sect, buffer, mem_size, "wasm_test_func_ptr", wasm_state, args, 3);

	//WriteFileLang("../../wabt/test.html", (void*)page.data(), page.size());
	//void FromIRToAsm()
}

void CreateAstFromFunc(lang_state* lang_stat, web_assembly_state* wasm_state, func_decl* f)
{
	f->func_node->fdecl = f;
	ast_rep* ast = AstFromNode(lang_stat, f->func_node, f->scp);
	ASSERT(ast->type == AST_FUNC);
	own_std::vector<ir_rep>* ir = (own_std::vector<ir_rep> *) & f->ir;
	GetIRFromAst(lang_stat, ast, ir);

	//auto func = (int(*)(int)) lang_stat->GetCodeAddr(f->code_start_idx);
	//auto ret =func(2);
	//f->wasm_scp = NewScope(lang_stat, f->scp->parent);
	//AddFuncToWasm(f);

}

struct compile_options
{
	own_std::string file;
	own_std::string wasm_dir;
	own_std::string folder_name;
	bool release;
	bool run;
};

void AssignDbgFile(lang_state* lang_stat, own_std::string file_name)
{
	unsigned int read;
	web_assembly_state *wasm_state = lang_stat->wasm_state;
	lang_stat->data_sect.clear();
	wasm_state->lang_stat = lang_stat;
	wasm_state->funcs.clear();
	wasm_state->imports.clear();
	wasm_state->outsiders.clear();
	auto file = (unsigned char *)ReadEntireFileLang((char *)file_name.c_str(), &read);

	

	auto dfile = (dbg_file_seriealize*)(file);
	wasm_interp &winterp = *lang_stat->winterp;

	WasmInterpInit(&winterp, file, read, lang_stat);

}
void RunDbgFunc(lang_state* lang_stat, own_std::string func, long long* args, int total_args)
{


	int mem_size = BUFFER_MEM_MAX;
	auto buffer = (unsigned char*)AllocMiscData(lang_stat, mem_size);
	lang_stat->winterp->dbg->mem_size = mem_size;
	*(int*)&buffer[MEM_PTR_CUR_ADDR] = 20000;
	*(int*)&buffer[MEM_PTR_MAX_ADDR] = 0;

	ASSERT(lang_stat->data_sect.size() < DATA_SECT_MAX);
	memcpy(&buffer[DATA_SECT_OFFSET], lang_stat->data_sect.begin(), lang_stat->data_sect.size());
	WasmInterpRun(lang_stat->winterp, buffer, mem_size, func.c_str(), args, total_args);
	__lang_globals.free(__lang_globals.data, buffer);

}
void AssignOutsiderFunc(lang_state* lang_stat, own_std::string name, OutsiderFuncType func)
{
	lang_stat->winterp->outsiders[name.c_str()] = func;
}

char* heap_alloc(mem_alloc* alloc, int size);//, mem_chunk **out = nullptr)
void AddFolder(lang_state* lang_stat, own_std::string folder)
{
	own_std::vector<own_std::string> file_contents;
	own_std::vector<char *> file_names;
	GetFilesInDirectory(lang_stat->exe_dir+folder, nullptr, &file_names);

	if (file_names.size() == 0)
	{
		printf("no files found in the specified directory");
		ExitProcess(0);
	}


	FOR_VEC(str, file_names)
	{
		AddNewFile(lang_stat, *str);
	}
}
int Compile(lang_state* lang_stat, compile_options *opts)
{
	//own_std::vector<own_std::string> args;
	//own_std::string aux;
	//split(args_str, ' ', args, &aux);
	
	int i = 0;
	own_std::string file = opts->file;
	own_std::string wasm_dir = opts->wasm_dir;
	own_std::string target = "";

	lang_stat->gen_wasm = true;
	lang_stat->release = opts->release;
	new(&lang_stat->wasm_state->folder_name)own_std::string(opts->folder_name);
	new(&lang_stat->wasm_state->wasm_dir) own_std::string(opts->wasm_dir);
	//auto base_fl = AddNewFile(lang_stat, "Core/base.lng");
	//tp.type = enum_type2::TYPE_IMPORT;
	//tp.imp = NewImport(lang_stat, import_type::IMP_IMPLICIT_NAME, "", base_fl);
	//lang_stat->base_lang = NewDecl(lang_stat, "base", tp);

	type2 dummy_type;
	decl2* release = FindIdentifier("RELEASE", lang_stat->root, &dummy_type);
	release->type.i = lang_stat->release;

	

	char buffer[MAX_PATH] = { 0 };
	PlatformGetFileFullPath(file.c_str(), buffer, MAX_PATH);
	//printf("work dir is %s\n", buffer);
	lang_stat->work_dir = buffer;

	AddFolder(lang_stat, file);

	if (lang_stat->files.size() == 0)
	{
		printf("no files to be compiled, will exit\n");
		ExitProcess(0);
	}
	//printf("files added");
	FOR_VEC(i1, lang_stat->files)
	{
		type2 tp;
		tp.type = enum_type2::TYPE_IMPORT;
		tp.imp = NewImport(lang_stat, import_type::IMP_IMPLICIT_NAME, "", *i1);
		FOR_VEC(i2, lang_stat->files)
		{
			if (*i1 == *i2)
				continue;


			(*i2)->global->imports.emplace_back(NewDecl(lang_stat, "__import", tp));
		}
	}
	//AddNewFile(lang_stat, "Core/tests.lng");
	//AddNewFile(lang_stat, "Core/player.lng");
	//AddNewFile(file_name);
	

	int cur_f = 0;

	int iterations = 0;
	bool can_continue = false;
	struct info_not_found
	{
		node* nd;
		scope* scp;
	};
	own_std::vector<info_not_found>names_not_found;
	type2 dummy;
	while(true)
	{
		
		lang_stat->something_was_declared = false;
		
		for(cur_f = 0; cur_f < lang_stat->files.size(); cur_f++)
		{
			can_continue = false;
			lang_stat->flags &= ~PSR_FLAGS_SOMETHING_IN_GLOBAL_NOT_FOUND;
			lang_stat->global_decl_not_found.clear();
			auto f = lang_stat->files[cur_f];
			lang_stat->work_dir = f->path;
			lang_stat->cur_file = f;
			if(!DescendNameFinding(lang_stat, f->s, f->global))
				can_continue = true;


#ifdef DEBUG_GLOBAL_NOT_FOUND
	
			if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_SOMETHING_IN_GLOBAL_NOT_FOUND))
			{
				FOR_VEC(global_nd, lang_stat->global_decl_not_found)
				{
					node* cur = *global_nd;
					info_not_found info;
					info.nd = cur->l;
					info.scp = f->global;
					names_not_found.emplace_back(info);
				}
				can_continue = true;
			}
#endif

			int a = 0;
			iterations++;
			if (iterations >= 99)
			{
				FOR_VEC(cur, names_not_found)
				{
					decl2* d = FindIdentifier(cur->nd->t->str, cur->scp, &dummy);

					printf("\nglobal decl not found line %d\nstr: %s\n", cur->nd->t->line,
						cur->nd->t->line_str);
					if (d == nullptr)
					{
						printf("\nglobal decl not found line %d\nstr: %s\n was found %d", cur->nd->t->line,
							cur->nd->t->line_str, d != nullptr);
					}
					if (d && IS_FLAG_ON(d->flags, DECL_NOT_DONE))
					{
						printf("\nglobal decl not done line %d\nstr: %s\n was found %d", cur->nd->t->line,
							cur->nd->t->line_str, d != nullptr);
					}
					if (d && d->type.type == TYPE_STRUCT_TYPE && IS_FLAG_ON(d->type.strct->flags, TP_STRCT_STRUCT_NOT_NODE))
					{
						printf("\nglobal struct not done line %d\nstr: %s\n was found %d", cur->nd->t->line,
							cur->nd->t->line_str, d != nullptr);
					}

				}
				can_continue = false;
				break;
			}
		}
		if(!lang_stat->something_was_declared && !can_continue)
			break;
	}
#ifdef DEBUG_GLOBAL_NOT_FOUND
	printf("names not found!\n");
	FOR_VEC(cur, names_not_found)
	{
		decl2* d = FindIdentifier(cur->nd->t->str, cur->scp, &dummy);

		if (d == nullptr)
		{
			printf("\nglobal decl not found line %d\nstr: %s\n was found %d", cur->nd->t->line,
				cur->nd->t->line_str, d != nullptr);
		}

	}
#endif
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

		fdecl->code = (machine_code*)__lang_globals.alloc(__lang_globals.data, sizeof(machine_code));
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
		own_std::string scp_str = lang_stat->root->Print(0);
		printf("file: %s, scp: \n %s", f->name.c_str(), scp_str.c_str());
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
		//own_std::vector<func_byte_code*>all_funcs = GetFuncs(lang_stat, lang_stat->funcs_scp);
		machine_code code;
		//FromByteCodeToX64(lang_stat, &all_funcs, code);

		//auto exec_funcs = CompleteMachineCode(lang_stat, code);
	}

	

	char msg_hdr[256];
    web_assembly_state &wasm_state = *lang_stat->wasm_state ;
	wasm_state.lang_stat = lang_stat;
	wasm_state.wasm_dir = opts->wasm_dir;

    auto mem_decl = (decl2 *)AllocMiscData(lang_stat, sizeof(decl2));
    mem_decl->name = own_std::string("mem");
    mem_decl->type.type = TYPE_WASM_MEMORY;
    wasm_state.exports.emplace_back(mem_decl);

	FOR_VEC(f_ptr, lang_stat->outsider_funcs)
	{
		auto f = *f_ptr;
		auto fdecl = (decl2 *)AllocMiscData(lang_stat, sizeof(decl2));
		fdecl->type.type = TYPE_FUNC;
		fdecl->name = own_std::string(f->name);
		fdecl->type.fdecl = f;
		f->this_decl = fdecl;
		wasm_state.imports.emplace_back(f->this_decl);
	}
	FOR_VEC(cur_f, lang_stat->funcs_scp->vars)
	{
		auto f = *cur_f;
		if (f->type.type != TYPE_FUNC)
			continue;
		auto fdecl = f->type.fdecl;
		if (IS_FLAG_ON(fdecl->flags, FUNC_DECL_MACRO | FUNC_DECL_IS_OUTSIDER | FUNC_DECL_TEMPLATED))
			continue;

		if (FuncAddedWasm(&wasm_state, f->name))
			continue;

		AddFuncToWasm(&wasm_state, f->type.fdecl);
		CreateAstFromFunc(lang_stat, &wasm_state, f->type.fdecl);
		int  a = 0;

	}

	/*
	FOR_VEC(f_ptr, lang_stat->func_ptrs_decls)
	{
		AddFuncToWasm(&wasm_state, *f_ptr, false);
		CreateAstFromFunc(lang_stat, &wasm_state, *f_ptr);
	}
	*/
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
	/*
	FOR_VEC(i1, lang_stat->files)
	{
		(*i1)->s->FreeTree();
	}
	*/
}
int InitLang(lang_state *lang_stat, AllocTypeFunc alloc_addr, FreeTypeFunc free_addr, void *data)
{
    __lang_globals.alloc =  alloc_addr;
    __lang_globals.data = data;
    __lang_globals.free = free_addr;
	__lang_globals.total_blocks = 258;
	__lang_globals.blocks = (block_linked*)AllocMiscData(lang_stat, sizeof(block_linked) * __lang_globals.total_blocks);
	__lang_globals.cur_block = 0;
	auto test = lang_state();
	*lang_stat = test;
	lang_stat->code_sect.reserve(256);

	TCHAR buffer[MAX_PATH] = { 0 };
	PlatformExeFullparh(buffer, MAX_PATH);
	own_std::string exe_full = buffer;
	int last_bar = exe_full.find_last_of("\\/");
	own_std::string exe_dir = exe_full.substr(0, last_bar + 1);
	PlatformSetCurretDir((char *)exe_dir.c_str());

	lang_stat->winterp = (wasm_interp *) AllocMiscData(lang_stat, sizeof(wasm_interp));
	new(&lang_stat->winterp->outsiders) std::unordered_map<std::string, OutsiderFuncType>();
	lang_stat->wasm_state = (web_assembly_state *) AllocMiscData(lang_stat, sizeof(web_assembly_state));
	//lang_stat->wasm_state->folder_name = 


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

	lang_stat->max_misc = 16 * 1024 * 1024;
	//lang_stat->misc_arena = (char*)VirtualAlloc(0, lang_stat->max_misc, MEM_COMMIT, PAGE_READWRITE);
	//lang_stat->node_arena = (node*)VirtualAlloc(0, lang_stat->max_nd * sizeof(node), MEM_COMMIT, PAGE_READWRITE);
    

	//auto a = ParseString(lang_stat, "a = a * a + a * a + a * a");

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
	new(&tp.fdecl->name)own_std::string("sizeof");
	tp.fdecl->args.push_back(dummy_decl);

	lang_stat->root->vars.push_back(NewDecl(lang_stat, "sizeof", tp));

	sz_of_fdecl = (func_decl*)AllocMiscData(lang_stat, sizeof(func_decl));
	memset(sz_of_fdecl, 0, sizeof(func_decl));
	tp.fdecl = sz_of_fdecl;
	sz_of_fdecl->flags |= FUNC_DECL_INTERNAL;
	new(&tp.fdecl->name)own_std::string("enum_count");
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

	tp.type = TYPE_INT;
	tp.i = lang_stat->release;
	lang_stat->root->vars.push_back(NewDecl(lang_stat, "RELEASE", tp));
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
	//char buffer[MAX_PATH] = { 0 };
	//GetModuleFileName(NULL, buffer, MAX_PATH);

	//printf("full exe path: %s", buffer);

	// addind the file name to the path

	int last_bar_comp = exe_dir.find_last_of('\\');
	/*
	own_std::string file_name_dir = own_std::string(argv[1]);
	int last_bar_main = file_name_dir.find_last_of('/');
	*/
	//own_std::string dir = exe_dir.substr(0, last_bar_comp) + "\\" + file_name_dir.substr(0, last_bar_main);
	own_std::string dir = exe_dir.substr(0, last_bar_comp + 1);
	lang_stat->exe_dir = dir;

	//own_std::string file_name = file_name_dir.substr(last_bar_main+1);

    return 0;
}
