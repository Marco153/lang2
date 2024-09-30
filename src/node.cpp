#include "node.h"
#include "machine_rel.h"
#include "error_report.h"
#include "bytecode.h"
#include <algorithm>
#include <time.h>
//#include "FileIO.cpp"

#define CMP_NTYPE(a, t) ((a)->type == node_type::t)

void AddStructMembersToScopeWithUsing(lang_state*, decl2* decl, scope* scp, node* by_name_nd);
node* parse_expression(own_std::vector<token2>* tkns, int precedence);
node* parse(own_std::vector<token2>* tkns, tkn_type2 target);
bool IsNodeOperator(node* nd, tkn_type2 tkn);
type2 DescendNode(lang_state*, node* n, scope* scp);
enum_type2 FromTypeToVarType(enum_type2 tp);
void ModifyFuncDeclToName(lang_state* lang_stat, func_decl* fdecl, node* n, scope* scp);
own_std::string StringifyNode(node* n);
bool CallNode(lang_state*, node* ncall, scope* scp, type2* ret_type, decl2* = nullptr);
unit_file* AddNewFile(lang_state*, own_std::string name);
bool NameFindingGetType(lang_state*, node* n, scope* scp, type2& ret_type, int = 0);
decl2* PointLogic(lang_state*, node* n, scope* scp, type2* ret_tp);
decl2* DescendNameFinding(lang_state*, node* n, scope* given_scp);
own_std::string FuncNameWithTempls(lang_state*, own_std::string fname, own_std::vector<type2>* types);
node* ParseString(lang_state*, own_std::string str, int = 0);
decl2* DeclareDeclToScopeAndMaybeToFunc(lang_state*, own_std::string name, type2* tp, scope* scp, node* nd = nullptr);
node* NewIdentNode(lang_state*, own_std::string name, token2*);
own_std::string OvrldOpToStr(overload_op op);
void MakeRelPtrDerefFuncCall(lang_state*, func_decl* op_func, node* n);
decl2* FindIdentifier(own_std::string name, scope* scp, type2* ret_type, int = 0);
node* NewBinOpNode(lang_state*, node* lhs, tkn_type2 t, node* rhs);
decl2* FromBuiltinTypeToDecl(lang_state*, enum_type2 tp);
void ReportUndeclaredIdentifier(lang_state*, token2* t);
node* MakeFuncCallArgs(lang_state*, own_std::string op_func, node* ref, own_std::vector<node*>& args, token2*);
enum_type2 FromVarTypeToType(enum_type2 tp);
void CheckStructValToFunc(func_decl* fdecl, type2* type);
void ReportMessage(lang_state*, token2* tkn, char* msg);
decl2* FromTypeToDecl(lang_state*, type2* tp);
bool IsNodeUnop(node* nd, tkn_type2 tkn);
node* NewIntNode(lang_state*, long long i, token2*);
node* NewTypeNode(lang_state*, node* lhs, node_type nd, node* rhs, token2*);
scope* GetScopeFromParent(lang_state*, node* n, scope* given_scp);
node* CreateNodeFromType(lang_state*, type2* tp, token2*);
bool FunctionIsDone(lang_state*, node* n, scope* scp, type2* ret_type, int flags);
void ReportMessageOne(lang_state*, token2* tkn, char* msg, void* data);
void CheckDeclNodeAndMaybeAddEqualZero(lang_state*, node* n, scope* scp);
void DescendComma(lang_state*, node* n, scope* scp, own_std::vector<comma_ret>& ret);
bool CheckOverloadFunction(lang_state*, func_decl* f);

/*
int max(int a, int b)
{
	return a > b ? a : b;
}
int min(int a, int b)
{
	return a < b ? a : b;
}
*/

char* std_str_to_heap(lang_state*, own_std::string* str);
#define PSR_FLAGS_LAMBDA_ARGS 1
#define PSR_FLAGS_IMPLICIT_SEMI_COLON 2
#define PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD 4
#define PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS 8
#define PSR_FLAGS_REPORT_UNDECLARED_IDENTS 0x10
#define PSR_FLAGS_DONT_DECLARE_VARIABLES 0x20
#define PSR_FLAGS_ASSIGN_SAVED_REGS 0x40
#define PSR_FLAGS_INSIDER_FOR_DECL 0x80
#define PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT 0x100
#define PSR_FLAGS_DONT_CHANGE_TEMPL_STRCT_ND_NAME 0x200
#define PSR_FLAGS_ON_FUNC_CALL 0x400
#define PSR_FLAGS_ERRO_REPORTED 0x800
#define PSR_FLAGS_ON_ENUM_DECL 0x1000
#define PSR_FLAGS_AFTER_TYPE_CHECK 0x2000
#define PSR_FLAGS_HAS_PLUGINS 0x4000
#define PSR_FLAGS_ON_FUNC_DECL 0x8000
#define PSR_FLAGS_ON_STRUCT_DECL 0x10000
#define PSR_FLAGS_ON_JMP_WHEN_ERROR 0x20000
#define PSR_FLAGS_SCOPE_WHITHOUT_CURLY 0x40000
#define PSR_FLAGS_COMP_TIME 0x80000
#define PSR_FLAGS_SOMETHING_IN_GLOBAL_NOT_FOUND 0x100000
#define PSR_FLAGS_RET_NIL_EVEN_WHEN_PTR_TO_STRUCT_NOT_DONE 0x200000


#define PREC_SEMI_COLON 0
#define PREC_COMMA 1
#define PREC_EQUAL 4
#define PREC_OR 5
#define PREC_AND 6
#define PREC_GREATER 7
#define PREC_LESSER 7
#define PREC_PLUS 8
#define PREC_MUL 9
#define PREC_OPEN_BRACKETS 10
#define PREC_POINT 11
#define PREC_CLOSE_CURLY -2 
#define PREC_CLOSE_BRACKETS 14
#define PREC_CLOSE_PARENTHESES -3 

#define CREATE_STMNT(n, lhs, rhs) (n)->type = node_type::N_STMNT;\
						      (n)->l = lhs;\
						      (n)->r = rhs;

#define IS_PRS_FLAG_ON(f) IS_FLAG_ON(lang_stat->flags, f)

#define SPREAD_TKN(t) t->line, t->line_offset

#define DONT_DESCEND_SCOPE 1
#define DONT_DESCEND_ARGS  2

own_std::string scope::Print(int indent)
{
	indent += 1;
	char buffer[512];

	char tabs[32];

	indent = max(indent, indent - 1);
	
	memset(tabs, ' ', indent);
	tabs[indent] = 0;

	own_std::string type_name = "";
	if (type == SCP_TYPE_FUNC)
	{
		type_name = own_std::string("func, name ,") + fdecl->name.c_str();
	}
	if (type == SCP_TYPE_STRUCT)
	{
		type_name = own_std::string("struct, name ,") + tstrct->name.c_str();
	}
	snprintf(buffer, 512, "%s{%s lstart: %d, lend: %d\n", tabs, type_name.c_str(), line_start, line_end);


	memset(tabs, ' ', indent + 1);
	tabs[indent + 1] = 0;

	own_std::string ret = buffer;
	FOR_VEC(decl, vars)
	{
		decl2* d = *decl;
		if (d->type.type == TYPE_ENUM_TYPE || d->type.type == TYPE_ENUM)
			continue;
		snprintf(buffer, 512, "%sname: %s, type: %s, offset: %d\n", tabs, d->name.c_str(), TypeToString(d->type).c_str(), d->offset);
		ret += buffer;

	}
	FOR_VEC(scp, children)
	{
		scope* s = *scp;
		ret += s->Print(indent + 1);
	}
	memset(tabs, ' ', indent);
	tabs[indent] = 0;
	snprintf(buffer, 512, "%s}\n", tabs);
	ret += buffer;
	return ret;

}

decl2* type2::GetEnumDecl(own_std::string name)
{
	auto last_prnt = scp->parent;
	scp->parent = nullptr;
	type2 tp;
	return FindIdentifier(name, scp, &tp);
}

template <class T>
struct tag_strct
{
	long long tag;
	T val;

};

template <class T>
T FindTag(own_std::vector<tag_strct<T>>* ar, long long target_tag)
{
	FOR_VEC(t, *ar)
	{
		if (t->tag == target_tag)
			return t->val;
	}
	return nullptr;
}
template <class T>
T GetTagVal(own_std::vector<tag_strct<T>>* ar, long long target_tag)
{
	auto is_added = FindTag(ar, target_tag);
	if (is_added == nullptr)
	{
		is_added = (T)malloc(sizeof(*is_added));
		memset(is_added, 0, sizeof(*is_added));
	}

	return is_added;
}


char* GetFuncBasedOnAddr(unsigned long long offset);
func_decl *IsThereAFunction(lang_state *lang_stat, char *name)
{
	FOR_VEC(it, lang_stat->global_funcs)
	{
		auto f = *it;
		if (f->name == own_std::string(name))
			return f;
	}
	return nullptr;
}
char *GetFuncAddrBasedOnName(lang_state *lang_stat, char *name)
{
	FOR_VEC(it, lang_stat->global_funcs)
	{
		auto f = *it;
		if (f->name == own_std::string(name))
			return lang_stat->GetCodeAddr(f->code_start_idx);
	}
	return nullptr;
}
char* GetFuncBasedOnAddr(lang_state *lang_stat, unsigned long long offset)
{
	FOR_VEC(it, lang_stat->global_funcs)
	{
		auto f = *it;


		auto start_addr = (unsigned long long)lang_stat->GetCodeAddr(f->code_start_idx);
		auto end = (unsigned long long)lang_stat->GetCodeAddr(f->code_start_idx + f->code_size);

		if (offset >= start_addr && offset < end)
		{
			return std_str_to_heap(lang_stat, &f->name);
		}
	}
	return "NOT FOUND";
}
void NewDeclToCurFilseGlobalsScope(lang_state *lang_stat, decl2 *decl)
{
	//lang_stat->cur_file->global->vars.emplace_back(decl);
	lang_stat->funcs_scp->vars.emplace_back(decl);
}
void NewFuncToCompile(lang_state *lang_stat, func_decl *fdecl)
{
	lang_stat->global_funcs.emplace_back(fdecl);
}
void NewTypeDataSymbol(lang_state *lang_stat, int offset, char* name)
{
	//lang_stat->symbols_offset_on_type_sect[std::string(name)] = offset;
}

char* AllocMiscData(lang_state *lang_stat, int sz)
{
	//ASSERT((lang_stat->cur_misc + sz + 4) < lang_stat->max_misc);
	//auto ret = (char *)__lang_globals.alloc(__lang_globals.data, sz + 16, 0);
	auto ret = (char*)__lang_globals.alloc(__lang_globals.data, sz + 16);
	memset(ret, 0, sz);
	//auto ret = lang_stat->misc_arena + lang_stat->cur_misc;
	//lang_stat->cur_misc += sz;
	/*
	lang_stat->cur_misc += sz + 4;
	for (int i = 0; i < 4; i++)
	{
		auto test = ret + sz;
		test[sz + i] = 0xfc;
	}
	*/
	return ret;
}
char* std_str_to_heap(lang_state *lang_stat, own_std::string* str)
{
	int sz = str->size();
	auto buffer = (char*)AllocMiscData(lang_stat, sz + 16);
	memcpy(buffer, str->data(), sz);
	buffer[sz] = 0;
	return buffer;
}
void SwapNodesRhs(node** n)
{
	auto final_top = (*n)->r;
	auto final_bottom = (*n);
	final_bottom->r = final_top->r;
	final_top->r = final_bottom;

	(*n) = final_top;
}
void SwapNodesIf(node** n)
{
	auto n_scp = (*n)->l;
	auto n_scp_r = (*n)->l->r;

	(*n)->l = n_scp_r;
	n_scp->r = (*n);
	(*n) = n_scp;
}
void InsertIntoCharVector(own_std::vector<char>* vec, void* src, int size)
{
	own_std::string data((char*)src, size);
	for (int i = 0; i < size; i++)
	{
		int idx = vec->size();
		vec->insert(idx, (char)data[i]);
	}
}
int AddDataSectSizeRetPrevSize(lang_state *lang_stat, int sz)
{
	int prev_sz = lang_stat->data_sect.size();
	lang_stat->data_sect.resize(lang_stat->data_sect.size() + sz, 0);
	return prev_sz;
}
void InsertIntoDataSect(lang_state *lang_stat, void* src, int size)
{
	InsertIntoCharVector(&lang_stat->data_sect, src, size);
}


node* new_node(lang_state *lang_stat, node* src)
{
	//ASSERT((lang_stat->cur_nd + 1) < lang_stat->max_nd);
	auto cur_node = (node*)AllocMiscData(lang_stat, sizeof(node) * 2);
	cur_node->t = src->t;
	memcpy(cur_node, src, sizeof(node));
	return cur_node;
}
node* new_node(lang_state *lang_stat, token2 *t)
{
	//ASSERT((lang_stat->cur_nd + 1) < lang_stat->max_nd);
	//auto cur_node = lang_stat->node_arena + lang_stat->cur_nd++;
	auto cur_node = (node*)AllocMiscData(lang_stat, sizeof(node) * 2);
	memset(cur_node, 0, sizeof(node));
	ASSERT(t);
	cur_node->t = t;
	return cur_node;
}

bool node_iter::is_operator(token2* tkn, int* precedence)
{

	if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_LAMBDA_ARGS) && tkn->type == tkn_type2::T_PIPE)
	{
		*precedence = PREC_PLUS;
		return true;
	}
	if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_IMPLICIT_SEMI_COLON))
	{
		*precedence = PREC_SEMI_COLON;
		return true;
	}
	if (IsTknWordStr(peek_tkn(), "in"))
	{
		*precedence = PREC_PLUS;
		return true;
	}
	if (IsTknWordStr(peek_tkn(), "rev"))
	{
		*precedence = PREC_PLUS;
		return true;
	}
	if (IsTknWordStr(peek_tkn(), "as"))
	{
		*precedence = PREC_PLUS;
		return true;
	}
	if (tkn->type == tkn_type2::T_EQUAL && (tkn + 1)->type == tkn_type2::T_EQUAL)
	{
		get_tkn();
		*precedence = 1;
		return true;
	}
	switch (tkn->type)
	{
	case tkn_type2::T_POINT:
	{
		*precedence = PREC_POINT;
		return true;
	}break;
	case tkn_type2::T_OPEN_BRACKETS:
	{
		*precedence = PREC_OPEN_BRACKETS;
	}break;
	case tkn_type2::T_DIV:
	case tkn_type2::T_DOLLAR:
	case tkn_type2::T_MUL:
	{
		*precedence = PREC_MUL;
		return true;
	}break;
	case tkn_type2::T_AMPERSAND:
	case tkn_type2::T_MINUS:
	case tkn_type2::T_COLON:
	case tkn_type2::T_PERCENT:
	case tkn_type2::T_IN:
	case tkn_type2::T_PLUS:
	case tkn_type2::T_PIPE:
	case tkn_type2::T_PLUS_PLUS:
	case tkn_type2::T_MINUS_MINUS:
	{
		*precedence = PREC_PLUS;
		return true;
	}break;
	case tkn_type2::T_COND_EQ:
	case tkn_type2::T_COND_NE:
	case tkn_type2::T_LESSER_THAN:
	case tkn_type2::T_LESSER_EQ:
	case tkn_type2::T_GREATER_THAN:
	case tkn_type2::T_GREATER_EQ:
	{
		*precedence = PREC_GREATER;
		return true;
	}break;
	case tkn_type2::T_PLUS_EQUAL:
	case tkn_type2::T_MINUS_EQUAL:
	case tkn_type2::T_EQUAL:
	{
		*precedence = PREC_EQUAL;
		return true;
	}break;
	case tkn_type2::T_SEMI_COLON:
	{
		*precedence = PREC_SEMI_COLON;
		return true;
	}break;
	case tkn_type2::T_COND_OR:
	{
		*precedence = PREC_OR;
		return true;
	}break;
	case tkn_type2::T_COND_AND:
	{
		*precedence = PREC_AND;
		return true;
	}break;
	case tkn_type2::T_COMMA:
	{
		*precedence = PREC_COMMA;
		return true;
	}break;
	case tkn_type2::T_CLOSE_PARENTHESES:
	{
		*precedence = PREC_CLOSE_PARENTHESES;
		return true;
	}break;
	case tkn_type2::T_CLOSE_BRACKETS:
	{
		*precedence = PREC_CLOSE_BRACKETS;
		return true;
	}break;
	case tkn_type2::T_OPEN_CURLY:
	{
		*precedence = PREC_CLOSE_CURLY + 1;
		return true;
	}break;
	case tkn_type2::T_CLOSE_CURLY:
	{
		*precedence = PREC_CLOSE_CURLY;
		return true;
	}break;
	case tkn_type2::T_EOF:
	{
		*precedence = 32;
		return true;
	}break;
	}
	return false;
}
token2* node_iter::peek_tkn()
{
	auto ret = &(*this->tkns)[this->cur_idx];
	return &(*this->tkns)[this->cur_idx];
}
token2* node_iter::get_tkn()
{
	return &(*this->tkns)[this->cur_idx++];
}
void node_iter::SetNodeScopeIdx(lang_state *lang_stat, node** nd, unsigned char val, int scope_start, int scope_end)
{
	auto scp = new_node(lang_stat, (*nd)->t);
	scp->type = N_SCOPE;
	scp->r = *nd;
	scp->t = peek_tkn();
	scp->scope_line_end = scope_end;
	scp->scope_line_start = scope_start;
	*nd = scp;
	//memcpy(*nd, scp, sizeof(node));
}
/*
void SetNodeScopeIdx(int* flags, unsigned char val)
{
	char bit = 24;
	unsigned char cur_idx = ((*flags) >> bit) & 0xff;
	cur_idx = val;
	int mask = ~(0xff << bit);
	*flags |= (*flags & mask) | (cur_idx << bit);
}
*/
bool node_iter::is_unary(tkn_type2* tp)
{
	if (peek_tkn()->type == tkn_type2::T_OPEN_BRACKETS)
	{
		*tp = tkn_type2::T_OPEN_BRACKETS;
		return true;
	}
	if (peek_tkn()->type == tkn_type2::T_MUL)
	{
		*tp = tkn_type2::T_MUL;
		return true;
	}
	if (peek_tkn()->type == tkn_type2::T_MINUS
		&& (peek_tkn() + 1)->type == tkn_type2::T_MINUS)
	{
		*tp = tkn_type2::T_MINUS;
		return true;
	}
	if (peek_tkn()->type == tkn_type2::T_PLUS
		&& (peek_tkn() + 1)->type == tkn_type2::T_PLUS)
	{
		*tp = tkn_type2::T_PLUS;
		return true;
	}
	return false;

}
node* node_iter::parse_all()
{
	ASSERT(lang_stat)
	return parse_stmnts();
}
node* node_iter::parse_stmnts()
{
	return parse_(32, parser_cond::GREATER_EQUAL);
}
bool CompareTypes(type2* lhs, type2* rhs, bool assert = false)
{
	if (rhs->type == enum_type2::TYPE_STR_LIT && lhs->type == enum_type2::TYPE_U8 && lhs->ptr == 1)
		return true;
	bool cond = false;

	if (rhs->type == enum_type2::TYPE_STATIC_ARRAY
		&& rhs->tp->type == lhs->type && (rhs->tp->ptr + 1) == lhs->ptr
		)
		return true;


	cond = (lhs->ptr == rhs->ptr);

	if (IS_FLAG_ON(rhs->flags, TYPE_FLAGS_AUTO_CAST))
		return true;

	if (!cond)
		return false;




	switch (lhs->type)
	{
	case enum_type2::TYPE_FUNC_PTR:
	{
		if (rhs->type != TYPE_FUNC_PTR && rhs->type != TYPE_FUNC)
			return false;

		if (lhs->fdecl->args.size() != rhs->fdecl->args.size())
			return false;

		int cur_param = 0;
		FOR_VEC(f_ptr, lhs->fdecl->args)
		{
			if (!CompareTypes(&(*f_ptr)->type, &rhs->fdecl->args[cur_param]->type))
				return false;
			cur_param++;
		}

		if (!CompareTypes(&lhs->fdecl->ret_type, &rhs->fdecl->ret_type))
			return false;
	}break;
	case enum_type2::TYPE_STATIC_ARRAY:
	{
		bool rhs_type = rhs->type == enum_type2::TYPE_STATIC_ARRAY_TYPE || rhs->type == enum_type2::TYPE_STATIC_ARRAY;
		bool are_arrays_same_type = rhs_type && CompareTypes(lhs->tp, lhs->tp);
		cond =
			(are_arrays_same_type && (rhs->ar_size == lhs->ar_size || rhs->ar_size == -1))
			|| (lhs->tp->type == rhs->type && rhs->ptr == (lhs->tp->ptr + 1));


		if (assert && !cond)
			ASSERT(false)
	}break;
	case enum_type2::TYPE_STR_LIT:
	{
		cond = rhs->type == enum_type2::TYPE_STR_LIT || (rhs->type == enum_type2::TYPE_U8 && rhs->ptr == 1);
		if (assert && !cond)
			ASSERT(false)
	}break;
	case enum_type2::TYPE_ENUM:
	{
		int max_type = (int)enum_type2::TYPE_MAX;

		if (lhs->e_decl->name == "type_enum")
		{
			cond = (int)rhs->type >= 0 && (int)rhs->type <= max_type;
			if (assert && !cond)
				ASSERT(false)
		}
		else
		{
			cond = lhs->e_decl == rhs->from_enum;
		}
	}break;
	case enum_type2::TYPE_CHAR:
	{
		cond = (rhs->type == enum_type2::TYPE_CHAR);
	}break;
	case enum_type2::TYPE_F32:
	{
		cond = rhs->type == enum_type2::TYPE_F32 || rhs->type == enum_type2::TYPE_F32_RAW;
		if (assert && !cond)
			ASSERT(false)
	}break;
	case enum_type2::TYPE_F32_RAW:
	{
		cond = rhs->type == enum_type2::TYPE_F32;
	}break;
	case enum_type2::TYPE_ARRAY:
	{
		cond = (rhs->type == enum_type2::TYPE_STR_LIT || rhs->type == enum_type2::TYPE_STATIC_ARRAY || CompareTypes(lhs->tp, rhs->tp));
	}break;
	case enum_type2::TYPE_INT:
		cond = (rhs->type == enum_type2::TYPE_S64 ||
			rhs->type == enum_type2::TYPE_S32 ||
			rhs->type == enum_type2::TYPE_S16 ||
			rhs->type == enum_type2::TYPE_S8 ||
			rhs->type == enum_type2::TYPE_U64 ||
			rhs->type == enum_type2::TYPE_U32 ||
			rhs->type == enum_type2::TYPE_U16 ||
			rhs->type == enum_type2::TYPE_U8 ||
			rhs->type == enum_type2::TYPE_INT);
		break;
	case enum_type2::TYPE_S64:
		cond = (rhs->type == enum_type2::TYPE_S64 ||
			rhs->type == enum_type2::TYPE_S32 ||
			rhs->type == enum_type2::TYPE_S16 ||
			rhs->type == enum_type2::TYPE_S8 ||
			(rhs->type == enum_type2::TYPE_INT)
			);
		break;
	case enum_type2::TYPE_S32:
		cond = (rhs->type == enum_type2::TYPE_S32 ||
			rhs->type == enum_type2::TYPE_S16 ||
			rhs->type == enum_type2::TYPE_S8 ||
			(rhs->type == enum_type2::TYPE_INT &&
				rhs->s32 >= -2147483646 && rhs->s32 <= 2147483647)
			);
		break;
	case enum_type2::TYPE_S16:
		cond = (rhs->type == enum_type2::TYPE_S16 ||
			rhs->type == enum_type2::TYPE_S8 ||
			(rhs->type == enum_type2::TYPE_INT &&
				rhs->s64 >= -32768 && rhs->s64 <= 32767)
			);
		break;
	case enum_type2::TYPE_S8:
		cond = (rhs->type == enum_type2::TYPE_S8 ||
			(rhs->type == enum_type2::TYPE_INT &&
				rhs->s64 >= -128 && rhs->s64 <= 127)
			);
		break;

	case enum_type2::TYPE_U64:
		cond = rhs->type == enum_type2::TYPE_U64 ||
			rhs->type == enum_type2::TYPE_U32 ||
			rhs->type == enum_type2::TYPE_U16 ||
			rhs->type == enum_type2::TYPE_U8 ||
			rhs->ptr > 0 ||
			(rhs->type == enum_type2::TYPE_INT && rhs->u64 >= 0);

		if (assert && !cond)
			ASSERT(false)

			break;
	case enum_type2::TYPE_U32:
		cond = (rhs->type == enum_type2::TYPE_U32 ||
			rhs->type == enum_type2::TYPE_U16 ||
			rhs->type == enum_type2::TYPE_U8 ||
			(rhs->type == enum_type2::TYPE_INT && rhs->i >= 0 && rhs->u64 <= 0xffffffff)
			);
		break;
	case enum_type2::TYPE_U16:
		cond = (rhs->type == enum_type2::TYPE_U16 ||
			rhs->type == enum_type2::TYPE_U8 ||
			(rhs->type == enum_type2::TYPE_INT && rhs->i >= 0 && rhs->u32 <= 0xffff)
			);
		break;
	case enum_type2::TYPE_U8:
		cond = (rhs->type == enum_type2::TYPE_U8 || rhs->type == enum_type2::TYPE_CHAR ||
			(rhs->type == enum_type2::TYPE_INT && rhs->i >= 0 && rhs->u16 <= 0xff)
			);
		break;
	case enum_type2::TYPE_VOID:
		cond = lhs->ptr > 0 && rhs->ptr > 0;

		break;
	case enum_type2::TYPE_BOOL:
		cond = rhs->type == enum_type2::TYPE_BOOL;

		break;
	default:
	{

		if (assert)
		{
			ASSERT(lhs->type == rhs->type)
		}
		else if (lhs->type != rhs->type)
			return false;

		if (lhs->type == enum_type2::TYPE_STRUCT)
		{
			cond = (lhs->strct->name == rhs->strct->name);
		}
		else if (lhs->type == enum_type2::TYPE_ARRAY || lhs->type == enum_type2::TYPE_ARRAY)
		{
			cond = (CompareTypes(lhs->tp, rhs->tp));
		}
		else
		{
			cond = lhs->type == rhs->type;
		}
	}break;
	}

	return cond;
}
node* parse_expr();
node* parse_(int, parser_cond);

void node_iter::CreateCondAndScope(node** n)
{
	lang_stat->flags |= PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD;
	(*n)->l = parse_(0, parser_cond::LESSER_EQUAL);
	lang_stat->flags &= ~PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD;

	int scope_start = peek_tkn()->line;
	if (peek_tkn()->type != tkn_type2::T_OPEN_CURLY)
	{
		lang_stat->flags |= PSR_FLAGS_SCOPE_WHITHOUT_CURLY;
		(*n)->r = parse_(PREC_SEMI_COLON, parser_cond::LESSER_EQUAL);
		lang_stat->flags &= ~PSR_FLAGS_SCOPE_WHITHOUT_CURLY;

		//(*n)->r->flags = 0;
		if ((*n)->r->type != N_STMNT)
		{
			node* new_nd = new_node(lang_stat, (*n)->t);
			new_nd->l = (*n)->r;
			new_nd->type = N_STMNT;
			(*n)->r = new_nd;
		}
		int scope_end = peek_tkn()->line;
		SetNodeScopeIdx(lang_stat, &(*n)->r, cur_scope_count++, scope_start, scope_end);

		auto peek = peek_tkn();
		bool is_else = peek->type == T_WORD && peek->str == "else";
		if (!is_else)
		{
			ExpectTkn(T_SEMI_COLON);
			//ASSERT(get_tkn()->type == tkn_type2::T_SEMI_COLON)
			get_tkn();
		}
		(*n)->flags |= NODE_FLAGS_IS_SCOPE;
	}
	else
	{
		int last_scope_size = cur_scope_count;
		cur_scope_count = 0;
		(*n)->r = parse_expr();
		if ((*n)->r->r->type != N_STMNT)
		{
			node* new_nd = new_node(lang_stat, (*n)->t);
			new_nd->l = (*n)->r->r;
			new_nd->type = N_STMNT;
			(*n)->r->r = new_nd;
		}
		cur_scope_count = last_scope_size;

		(*n)->flags = (*n)->r->flags;
		(*n)->r->flags = 0;
		// checking if scope is empty

		if (!(*n)->r->r->l && !(*n)->r->r->r)
		{
			//free((*n)->r);
			(*n)->r->r = nullptr;
		}
		//SwapNodesRhs(n);
		//else
		//SetNodeScopeIdx(n, cur_scope_count++);
	}

	lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;

}
void node_iter::ExpectTkn(tkn_type2 t)
{
	char msg_hdr[256];
	auto tkn = peek_tkn();
	if (tkn->type != t)
	{
		REPORT_ERROR(tkn->line, tkn->line_offset, VAR_ARGS("unexpected token\'%s\'\n", tkn->ToString().c_str()))
			ExitProcess(1);
	}
}
node* node_iter::parse_func_like()
{
	auto n = new_node(lang_stat, peek_tkn());
	n->l = new_node(lang_stat,  peek_tkn());
	n->l->l = new_node(lang_stat, peek_tkn());
	n->l->type = node_type::N_SIGNATURE;

	bool is_outsider = false;
	auto tkn = peek_tkn();

	lang_stat->flags |= PSR_FLAGS_ON_FUNC_DECL;
	if (IsTknWordStr(peek_tkn(), "outsider"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_OUTSIDER;
		is_outsider = true;
	}
	else if (IsTknWordStr(peek_tkn(), "test"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_TEST;
	}
	else if (IsTknWordStr(peek_tkn(), "constructor"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_CONSTRUCTOR;
	}
	else if (IsTknWordStr(peek_tkn(), "comp"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_COMP;
	}
	else if (IsTknWordStr(peek_tkn(), "this"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_THIS;
	}
	else if (IsTknWordStr(peek_tkn(), "macro"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_MACRO;
	}
	else if (IsTknWordStr(peek_tkn(), "link_name"))
	{
		get_tkn();
		ASSERT(peek_tkn()->type == tkn_type2::T_STR_LIT);
		n->str = &get_tkn()->str;

		n->flags |= NODE_FLAGS_FUNC_LINK_NAME;
	}
	else if (IsTknWordStr(peek_tkn(), "internal"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_FUNC_INTERNAL;
	}
	if (IsTknWordStr(peek_tkn(), "align_stack_when_call"))
	{
		get_tkn();
		n->flags |= NODE_FLAGS_ALIGN_STACK_WHEN_CALL;
	}
	// template
	if (peek_tkn()->type == tkn_type2::T_OPEN_CURLY)
	{
		n->l->l->l = parse_expr();
		lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;
	}

	ExpectTkn(tkn_type2::T_OPEN_PARENTHESES);
	get_tkn();
	// args
	if(peek_tkn()->type != T_CLOSE_PARENTHESES)
		n->l->l->r = parse_(PREC_CLOSE_PARENTHESES, parser_cond::EQUAL);
		//n->l->l->r = parse_expr();
	ExpectTkn(tkn_type2::T_CLOSE_PARENTHESES);
	get_tkn();


	// return type
	if (peek_tkn()->type == tkn_type2::T_EXCLAMATION)
	{
		get_tkn();
		lang_stat->flags |= PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD;
		n->l->r = parse_(-3, parser_cond::GREATER_EQUAL);
		lang_stat->flags &= ~PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD;

	}
	// getting func plugins
	if (IsTknWordStr(peek_tkn(), "plugin"))
	{
		n->extra = new own_std::vector<node*>();
		get_tkn();
		auto plugin_nd = parse_(-3, parser_cond::GREATER_EQUAL);
		plugin_nd->extra_type = N_PLUGIN;
		n->extra->emplace_back(plugin_nd);

	}

	auto peek = peek_tkn();

	auto t_tp = peek_tkn()->type;
	lang_stat->flags &= ~PSR_FLAGS_ON_FUNC_DECL;
	// scope
	if (t_tp == tkn_type2::T_OPEN_CURLY)
	{
		ASSERT(is_outsider == false)
			n->r = parse_expr();

		n->type = node_type::N_FUNC_DECL;
		int save_flags = n->flags;
		n->flags = n->r->flags | save_flags;
		n->r->flags = 0;

		// being able calling the func decl without having to create a statetemnt
		if(peek_tkn()->type != T_OPEN_PARENTHESES && peek_tkn()->type != T_SEMI_COLON)
			lang_stat->flags |= PSR_FLAGS_IMPLICIT_SEMI_COLON;
		else
			lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;

		// if the scope is empty
		if (n->r->l == nullptr && n->r->r == nullptr)
		{
			//free(n->r);
			n->r = nullptr;
		}
	}
	else
	{
		n->type = node_type::N_FUNC_DEF;
		n->r = new_node(lang_stat, peek_tkn());
		n->flags |= NODE_FLAGS_IS_SCOPE;
		SetNodeScopeIdx(lang_stat, &n->r, cur_scope_count++, n->t->line, n->t->line);
		//SetNodeScopeIdx(&n->flags, )
	}

	return n;
}
node* node_iter::parse_strct_like()
{
	auto before_flags = lang_stat->flags;
	lang_stat->flags |= PSR_FLAGS_ON_STRUCT_DECL;
	node* n = new_node(lang_stat, peek_tkn());
	n->t = peek_tkn() - 1;
	// templates
	if (peek_tkn()->type == tkn_type2::T_OPEN_PARENTHESES)
		n->l = parse_expr();

	ExpectTkn(T_OPEN_CURLY);
	n->r = parse_expr();



	//lang_stat->flags = 1;


	n->flags = n->r->flags;
	n->r->flags = 0;

	lang_stat->flags = before_flags;
	//lang_stat->flags &= ~PSR_FLAGS_ON_STRUCT_DECL;
	if (peek_tkn()->type == T_SEMI_COLON)
	{
		//get_tkn();
	}
	else
	{
		lang_stat->flags |= PSR_FLAGS_IMPLICIT_SEMI_COLON;
	}
	return n;
}
void CheckParenthesesLevel(lang_state *lang_stat, token2* tkn)
{
	if (lang_stat->parentheses_opened > 0)
	{
		ReportMessage(lang_stat, tkn, "Not all parentheses were closed");
		ExitProcess(1);
	}
}
node* node_iter::parse_str(own_std::string& str, int* i)
{
	int end = *i + 1;
	while (end < str.size() && str[end] != '}')
	{

		end++;
	}
	own_std::string new_str = str.substr(*i + 1, end - 1);

	*i = end;

	int sz_tkns = sizeof(own_std::vector<token2>);
	auto new_tkns = (own_std::vector<token2> *) __lang_globals.alloc(__lang_globals.data, sz_tkns);
	memset(new_tkns, 0, sz_tkns);

	Tokenize2((char*)std_str_to_heap(lang_stat, &new_str), new_str.size(), new_tkns);
	node_iter niter(new_tkns, lang_stat);
	return niter.parse_(0, parser_cond::LESSER_EQUAL);
}
// $parse_expr
node* node_iter::parse_expr()
{
	auto n = new_node(lang_stat, peek_tkn());
	auto cur_tkn = get_tkn();
	n->t = cur_tkn;
	printf("tkn is %s\n", cur_tkn->str.c_str());
	switch (cur_tkn->type)
	{
	case tkn_type2::T_HASHTAG:
	{
		n->type = N_HASHTAG;
		n->r = parse_(0, parser_cond::LESSER_EQUAL);
	}break;
	case tkn_type2::T_TWO_POINTS:
	{
		n->type = node_type::N_BINOP;
		if (peek_tkn()->type != tkn_type2::T_CLOSE_BRACKETS)
		{
			n->r = parse_(0, parser_cond::LESSER_EQUAL);
		}
	}break;
	case tkn_type2::T_THREE_POINTS:
	{
		n->type = node_type::N_VAR_ARGS;
	}break;
	case tkn_type2::T_STR_LIT:
	{
		n->type = node_type::N_STR_LIT;
		int i = 0;

		auto peek = peek_tkn();
		while (peek->type == T_STR_LIT)
		{
			cur_tkn->str += peek->str;
			get_tkn();

			peek = peek_tkn();
		}
			
		auto& str = cur_tkn->str;
		own_std::vector<node*> args;

		own_std::string ref_str;
		while (i < str.size())
		{
			int clamped = i - 1;
			if (clamped < 0)
				clamped = 0;

			if (str[i] == '{' && str[clamped] != '//')
			{
				args.emplace_back(parse_str(str, &i));

				ref_str += '%';
			}
			else
				ref_str += str[i];
			i++;
		}

		if (args.size() > 0)
		{
			auto nd_ref_str = NewIdentNode(lang_stat, ref_str, n->t);
			nd_ref_str->type = N_STR_LIT;
			args.insert(0, nd_ref_str);
			auto t = n->t;

			n = MakeFuncCallArgs(lang_stat, "sprintf", nullptr, args, n->t);

			n->t = t;
		}
	}break;
	case tkn_type2::T_FLOAT:
	{
		n->type = node_type::N_FLOAT;
	}break;
	case tkn_type2::T_INT:
	{
		n->type = node_type::N_INT;
	}break;
	case tkn_type2::T_PIPE:
	{
		lang_stat->flags |= PSR_FLAGS_LAMBDA_ARGS;
		n->l = new_node(lang_stat, peek_tkn());
		n->l->type = node_type::N_SIGNATURE;

		// args
		n->l->l = parse_(99, parser_cond::EQUAL);
		get_tkn();

		lang_stat->flags &= ~PSR_FLAGS_LAMBDA_ARGS;

		// return type
		if (peek_tkn()->type == tkn_type2::T_EXCLAMATION)
		{
			get_tkn();
			//n->l->r = parse_expression();

		}

		// scope
		if (peek_tkn()->type == tkn_type2::T_OPEN_CURLY)
		{
			n->r = parse_expr();
			n->flags = n->r->flags;
			n->r->flags = 0;
		}
		n->type = node_type::N_LAMBDA;
	}break;

	case tkn_type2::T_WORD:
	{
		type2 tp;


		if (GetTypeFromTkns(cur_tkn, n->decl_type))
		{
			n->type = node_type::N_TYPE;
		}
		else if (cur_tkn->str == "rev")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_REV;

			if (peek_tkn()->type != T_SEMI_COLON)
				n->r = parse_(0, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "continue")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_CONTINUE;

			if (peek_tkn()->type != T_SEMI_COLON)
				n->r = parse_(0, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "break")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_BREAK;
			if (peek_tkn()->type != tkn_type2::T_SEMI_COLON)
				n->r = parse_(0, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "using")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_USING;
			if (peek_tkn()->type != tkn_type2::T_SEMI_COLON)
				n->r = parse_(PREC_COMMA, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "false")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_FALSE;
		}
		else if (cur_tkn->str == "true")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_TRUE;
		}
		else if (cur_tkn->str == "__dbg_break")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_DBG_BREAK;

			n->l = new_node(lang_stat, cur_tkn);
		}
		else if (cur_tkn->str == "typedef")
		{
			n->type = node_type::N_TYPEDEF;
			n->r = parse_(PREC_SEMI_COLON, parser_cond::LESSER_EQUAL);

			// we probobly declared an inline struct
			if (peek_tkn()->str ==  "as")
			{
				if (n->r->type == N_STRUCT_DECL)
				{
					own_std::string name = own_std::string("unamed") + std::to_string(rand() % 0xffffff).c_str();
					node* name_nd = NewIdentNode(lang_stat, name, n->r->t);
					node* decl_nd = NewBinOpNode(lang_stat, name_nd, tkn_type2::T_COLON, new_node(lang_stat, n->r));
					memcpy(n->r, decl_nd, sizeof(node));
				}

				// we shouldve declared a struct
				ASSERT(IsNodeOperator(n->r, tkn_type2::T_COLON));
				auto as = get_tkn();
				auto name = get_tkn();
				// it should only be name after as
				ASSERT(peek_tkn()->type == tkn_type2::T_SEMI_COLON);
				auto bin = NewBinOpNode(lang_stat, n->r, tkn_type2::T_PLUS, NewIdentNode(lang_stat, name->str, n->r->t));
				bin->t = as;
				n->r = bin;

				lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;
			}
		}
		else if (cur_tkn->str == "return")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_RETURN;
			if (peek_tkn()->type != tkn_type2::T_SEMI_COLON)
				n->r = parse_(0, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "ret_type")
		{
			ASSERT(peek_tkn()->type == tkn_type2::T_OPEN_PARENTHESES)

				n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_RET_TYPE;
			n->r = parse_expr();
		}
		else if (cur_tkn->str == "enum")
		{
			n = parse_strct_like();
			n->type = node_type::N_ENUM_DECL;
		}
		else if (cur_tkn->str == "union")
		{
			n = parse_strct_like();
			n->type = node_type::N_UNION_DECL;
		}
		else if (cur_tkn->str == "struct")
		{
			n = parse_strct_like();
			n->type = node_type::N_STRUCT_DECL;
		}
		else if (cur_tkn->str == "if")
		{
			node* cur_node = n;
			cur_node->type = node_type::N_IF;

			cur_node->l = new_node(lang_stat, cur_tkn);
			cur_node->t = cur_tkn;

			CreateCondAndScope(&cur_node->l);

			cur_node->flags = cur_node->l->flags;
			cur_node->l->flags = 0;

			//SwapNodesIf(&cur_node);
			n = cur_node;
			ASSERT(!cur_node->r);

			/*
			if(IsTknWordStr(peek_tkn(), "else"))
				cur_node->r = new_node(lang_stat, peek_tkn());

			cur_node = cur_node->r;
			*/


			while (IsTknWordStr(peek_tkn(), "else"))
			{
				auto peek = peek_tkn();
				cur_node->r = new_node(lang_stat, cur_tkn);
				cur_node = cur_node->r;
				cur_node->l = new_node(lang_stat, cur_tkn);
				cur_node->t = peek_tkn();

				// else only 
				if (IsTknWordStr(peek_tkn(), "else") && !IsTknWordStr(peek_tkn() + 1, "if"))
				{

					ASSERT(IsTknWordStr(get_tkn(), "else"));
					cur_node->type = node_type::N_ELSE;

					auto else_tkn = peek_tkn();

					int scope_start = peek_tkn()->line;
					if (peek_tkn()->type != tkn_type2::T_OPEN_CURLY)
					{
						lang_stat->flags |= PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD | PSR_FLAGS_SCOPE_WHITHOUT_CURLY;
						cur_node->r = parse_(0, parser_cond::LESSER_EQUAL);
						lang_stat->flags &= ~(PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD | PSR_FLAGS_SCOPE_WHITHOUT_CURLY);

						int scope_end = peek_tkn()->line;
						if (cur_node->r->type != N_STMNT)
						{
							node* new_nd = new_node(lang_stat, cur_node->t);
							new_nd->l = cur_node->r;
							new_nd->type = N_STMNT;
							cur_node->r = new_nd;
						}
						SetNodeScopeIdx(lang_stat, &cur_node->r, cur_scope_count++, scope_start, scope_end);

						ExpectTkn(tkn_type2::T_SEMI_COLON);
						get_tkn();

						cur_node->flags |= NODE_FLAGS_IS_SCOPE;
					}
					else
					{
						cur_node->r = parse_expr();
						//SwapNodesRhs(&cur_node);

						int scope_end = peek_tkn()->line;
						cur_node->r->scope_line_start = scope_start;
						cur_node->r->scope_line_end = scope_end;
						//cur_node = cur_node->r;
					}
					cur_node->t = else_tkn;
				}
				// else if
				else if (IsTknWordStr(peek_tkn(), "else") && IsTknWordStr(peek_tkn() + 1, "if"))
				{
					ASSERT(IsTknWordStr(get_tkn(), "else"));
					get_tkn();
					CreateCondAndScope(&cur_node->l);
					cur_node->type = node_type::N_ELSE_IF;

					cur_node->flags = cur_node->l->flags;
					cur_node->l->flags = 0;
					//SwapNodesIf(&cur_node);
					//cur_node = cur_node->r;
				}
				else
					break;
			}
			lang_stat->flags |= PSR_FLAGS_IMPLICIT_SEMI_COLON;
		}
		else if (cur_tkn->str == "while")
		{
			n->type = node_type::N_WHILE;
			CreateCondAndScope(&n);
			lang_stat->flags |= PSR_FLAGS_IMPLICIT_SEMI_COLON;
			//SwapNodesRhs(&n);
			//SwapNodes(&n);
		}
		else if (cur_tkn->str == "for")
		{
			n->type = node_type::N_FOR;
			n->for_strct.i_var_idx = -1;
			CreateCondAndScope(&n);
			lang_stat->flags |= PSR_FLAGS_IMPLICIT_SEMI_COLON;
			//SwapNodesRhs(&n);
			//SwapNodes(&n);
		}
		else if (cur_tkn->str == "const")
		{
			n->type = node_type::N_CONST;
			n->r = parse_(PREC_MUL, LESSER_EQUAL);
		}
		else if (cur_tkn->str == "import_lib")
		{
			n->type = node_type::N_IMPORT_LIB;
			n->r = parse_(PREC_SEMI_COLON, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "import")
		{
			n->type = node_type::N_IMPORT;
			n->r = parse_(PREC_SEMI_COLON, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "rel")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_REL;
			n->t = cur_tkn;

			if (peek_tkn()->type == tkn_type2::T_OPEN_PARENTHESES)
				n->l = parse_expr();

			n->r = parse_(PREC_COMMA, parser_cond::LESSER_EQUAL);
		}
		else if (cur_tkn->str == "cast")
		{
			n->type = node_type::N_CAST;

			ExpectTkn(tkn_type2::T_OPEN_PARENTHESES);
			n->l = parse_expr();
			n->r = parse_(PREC_MUL, parser_cond::LESSER_EQUAL);

		}
		else if (cur_tkn->str == "operator")
		{
			auto func_tkn_begin = n->t;

			tkn_type2 op_type;
			overload_op ovlrd_op;
			if (peek_tkn()->type == tkn_type2::T_OPEN_BRACKETS)
			{
				get_tkn();
				ASSERT(peek_tkn()->type == tkn_type2::T_CLOSE_BRACKETS)
					op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::INDEX_OP;
				get_tkn()->type;

			}
			else if (peek_tkn()->type == tkn_type2::T_COND_EQ)
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::COND_EQ_OP;
			}
			else if (peek_tkn()->type == tkn_type2::T_MUL)
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::MUL_OP;
			}
			else if (peek_tkn()->type == tkn_type2::T_MINUS)
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::MINUS_OP;
			}
			else if (peek_tkn()->type == tkn_type2::T_PLUS)
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::PLUS_OP;
			}
			else if (peek_tkn()->type == tkn_type2::T_EQUAL)
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::ASSIGN_OP;
			}
			else if (IsTknWordStr(peek_tkn(), "for"))
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::FOR_OP;
			}
			else if (IsTknWordStr(peek_tkn(), "deref"))
			{
				get_tkn();
				op_type = tkn_type2::T_OPEN_BRACKETS;
				ovlrd_op = overload_op::DEREF_OP;
			}
			else
			{
				ASSERT(false)
			}

			n = node_iter::parse_func_like();

			ASSERT(n->type == node_type::N_FUNC_DECL)

				n->type = node_type::N_OP_OVERLOAD;
			n->op_type = op_type;
			n->ovrld_op = ovlrd_op;
			n->t = func_tkn_begin;
		}
		else if (cur_tkn->str == "nil")
		{
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_NIL;
			n->t = cur_tkn;
		}
		else if (cur_tkn->str == "constructor")
		{
			n = node_iter::parse_func_like();
			// asserting we have a return type
			ASSERT(n->l->r)
				n->t = cur_tkn;
		}
		else if (cur_tkn->str == "fn")
		{
			n = node_iter::parse_func_like();
			n->t = cur_tkn;
		}
		else if (cur_tkn->str == "match")
		{
			/*
			n->type = node_type::N_KEYWORD;
			n->kw = keyword::KW_MATCH;
			n->l = parse_sub_expression();
			n->r = parse_sub_expression();
			*/
		}
		else
		{
			n->type = node_type::N_IDENTIFIER;
		}


	}break;
	case tkn_type2::T_APOSTROPHE:
	{
		n->type = node_type::N_APOSTROPHE;
	}break;
	case tkn_type2::T_OPEN_PARENTHESES:
	{
		lang_stat->parentheses_opened++;

		if (peek_tkn()->type != tkn_type2::T_CLOSE_PARENTHESES)
			n = parse_(PREC_CLOSE_PARENTHESES, EQUAL);
		else
			n = nullptr;

		lang_stat->parentheses_opened--;

		if (lang_stat->parentheses_opened < 0)
		{
			ReportMessage(lang_stat, peek_tkn(), "Too many parentheses were closed");
			ExitProcess(1);
		}

		get_tkn();

		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_ON_FUNC_CALL) && IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD) && peek_tkn()->type == T_OPEN_CURLY)
		{
			ReportMessage(lang_stat, cur_tkn, "It seems like you're trying to declare a function but you forgot to type this part '::fn(..)");
		}
		if (n && n->type == N_BINOP && n->t->type == T_COMMA)
		{
			n->flags |= NODE_FLAGS_COMMA_INSIDE_PARENTHESES;
			int a = 0;
		}


	}break;
	case tkn_type2::T_OPEN_CURLY:
	{
		int last_scope_size = cur_scope_count;

		auto ttt = peek_tkn();

		int last_scopes_opened = lang_stat->scopes_opened;
		lang_stat->scopes_opened++;

		CheckParenthesesLevel(lang_stat, ttt);

		int scope_start = peek_tkn()->line;
		int scope_end = peek_tkn()->line;
		cur_scope_count = 0;
		if (peek_tkn()->type != tkn_type2::T_CLOSE_CURLY)
			n = parse_(PREC_CLOSE_CURLY, EQUAL);
		else
			n->type = N_EMPTY;


		cur_scope_count = last_scope_size;

		if (IS_FLAG_OFF(n->flags, NODE_FLAGS_IS_SCOPE))
		{
			n->flags |= NODE_FLAGS_IS_SCOPE;
			int scope_end = peek_tkn()->line;
			SetNodeScopeIdx(lang_stat, &n, cur_scope_count++, scope_start, scope_end);

		}
		else
		{
			node* new_n = new_node(lang_stat, cur_tkn);
			CREATE_STMNT(new_n, n, nullptr);
			new_n->flags = NODE_FLAGS_IS_SCOPE;
			int scope_end = peek_tkn()->line;
			n = new_n;
			SetNodeScopeIdx(lang_stat, &n, cur_scope_count++, scope_start, scope_end);
			//cur_scope_count++;
		}

		lang_stat->scopes_opened--;
		ASSERT(last_scopes_opened == lang_stat->scopes_opened);
		//int ;

		lang_stat->flags |= PSR_FLAGS_IMPLICIT_SEMI_COLON;
		ExpectTkn(T_CLOSE_CURLY);
		get_tkn();
	}break;
	// unary right assoc
	case tkn_type2::T_DOLLAR:
	{
		ASSERT(peek_tkn()->type == tkn_type2::T_OPEN_PARENTHESES)

			n->t = cur_tkn;
		n->type = node_type::N_UNOP;
		n->r = parse_expr();
	}break;
	case tkn_type2::T_EXCLAMATION:
	{
		int a = 0;
	}
	case tkn_type2::T_POINT:
	case tkn_type2::T_MINUS:
	case tkn_type2::T_AMPERSAND:
	case tkn_type2::T_TILDE:
	case tkn_type2::T_MUL:
	{
		n->type = node_type::N_UNOP;
		CheckTwoBinaryOparatorsTogether(n);
		n->r = parse_(PREC_MUL, parser_cond::LESSER);
	}break;
	// array type
	case tkn_type2::T_OPEN_BRACKETS:
	{
		n->flags |= NODE_FLAGS_INDEX_IS_TYPE;
		n->type = node_type::N_INDEX;
		if (peek_tkn()->type != tkn_type2::T_CLOSE_BRACKETS)
			n->l = parse_(PREC_CLOSE_BRACKETS, EQUAL);

		get_tkn();

		n->r = parse_(PREC_COMMA, parser_cond::LESSER_EQUAL);
		if (n->r->type == N_STRUCT_CONSTRUCTION)
		{
			node* strct_constr = n->r;
			//node* strct_constr = n->r;

			n->r = n->r->l;
			
			strct_constr->l = n;

			n = strct_constr;
			n->type = N_ARRAY_CONSTRUCTION;

		}
	}break;
	}

	if (peek_tkn()->type == tkn_type2::T_OPEN_CURLY)
	{
		auto peek = peek_tkn();
		if (peek->type == T_OPEN_CURLY && IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_ON_STRUCT_DECL | PSR_FLAGS_ON_FUNC_DECL | PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD | PSR_FLAGS_IMPLICIT_SEMI_COLON))
		{
			int a = 0;
			n->l = new_node(lang_stat, n);
			n->type = N_STRUCT_CONSTRUCTION;
			n->r = parse_expr();
			n->scp = nullptr;

			lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;

		}
	};


	return n;
}
#define PARSER_CHECK \
if (is_operator(peek_tkn(), &cur_prec) || peek_tkn()->type == T_CLOSE_BRACKETS)\
{\
	bool final_cond;\
	switch (pcond)\
	{\
	case parser_cond::GREATER_EQUAL: { final_cond = cur_prec >= prec; }break;\
	case parser_cond::LESSER_EQUAL: { final_cond = cur_prec <= prec; }break;\
	case parser_cond::GREATER: { final_cond = cur_prec > prec; }break;\
	case parser_cond::LESSER: { final_cond = cur_prec < prec; }break;\
	case parser_cond::EQUAL: { final_cond = cur_prec == prec; }break;\
	}\
	if (final_cond || (peek_tkn()->type == tkn_type2::T_EOF) || peek_tkn()->type == T_CLOSE_BRACKETS)\
	{\
		if (cur_node->r == nullptr)\
			return cur_node->l;\
		else\
			return cur_node;\
	}\
}

void node_iter::CheckTwoBinaryOparatorsTogether(node* cur_node)
{
	auto tkn = peek_tkn();
	int dummy_prec = 0;
	bool is_unary = tkn->type == tkn_type2::T_DOLLAR || tkn->type == tkn_type2::T_AMPERSAND || tkn->type == tkn_type2::T_EXCLAMATION || tkn->type == tkn_type2::T_MUL || tkn->type == tkn_type2::T_MINUS || tkn->type == T_APOSTROPHE || tkn->type == T_TILDE;

	bool two_binary_operators_together = (is_operator(peek_tkn(), &dummy_prec) && is_operator(cur_node->t, &dummy_prec) && !is_unary)
		&& tkn->type != tkn_type2::T_CLOSE_CURLY && tkn->type != tkn_type2::T_EOF
		&& tkn->type != tkn_type2::T_OPEN_CURLY;
	if (two_binary_operators_together)
		ReportMessageOne(lang_stat, tkn, "unexpected token '%s'", (char*)tkn->ToString().c_str());

}
//$parse_
node* node_iter::parse_(int prec, parser_cond pcond)
{
	char msg_hdr[256];
	auto cur_node = new_node(lang_stat, peek_tkn());
	auto ret = cur_node;

	int cur_prec = -888;

	bool is_file = lang_stat->cur_file && lang_stat->cur_file->name == "entity.lng";


	auto begin_tkn = peek_tkn();
	if (begin_tkn->type == T_CLOSE_PARENTHESES || begin_tkn->type == T_CLOSE_CURLY)
	{
		ReportMessageOne(lang_stat, begin_tkn, "Unexpected token", nullptr);
	}
	while (1)
	{
		auto tkn = peek_tkn();
		auto tt = peek_tkn();
		auto peek = peek_tkn();
		int cur_prec = 0;

		bool break_when_node_head = false;
		bool scope_without_curly = false;
		bool cur_is_ident_or_int = false;
		bool return_without_semicolon_on_scope_end = false;

		auto cur = peek_tkn();
		bool next_is_curly = cur->type == T_CLOSE_CURLY;
		bool next_is_else = cur->type == T_CLOSE_CURLY;
		begin_tkn = peek_tkn();
		if (cur_node->l == nullptr)
			cur_node->l = parse_expr();
		else if (peek_tkn()->type == T_OPEN_CURLY && cur_node->l->type == N_STMNT)
		{
			auto last_tp = (peek_tkn() - 1)->type;
			if (last_tp != T_SEMI_COLON && last_tp != T_CLOSE_CURLY)
			{
				ReportMessage(lang_stat, peek_tkn(), "unexpected token");
				ExitProcess(1);
			}
			lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;
			cur_node->type = N_STMNT;
			cur_node->r = parse_expr();
			goto double_colon_scope_label;
		}

		cur_prec = 0;

		break_when_node_head = IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD);
		scope_without_curly = IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_SCOPE_WHITHOUT_CURLY);
		cur_is_ident_or_int = peek_tkn()->type == tkn_type2::T_WORD || peek_tkn()->type == tkn_type2::T_INT || peek_tkn()->type == tkn_type2::T_FLOAT;
		return_without_semicolon_on_scope_end = false;

		cur = peek_tkn();
		next_is_curly = cur->type == T_CLOSE_CURLY;
		next_is_else = cur->type == T_CLOSE_CURLY;
		if (scope_without_curly)
			next_is_else = cur->type == T_WORD && cur->str == "else";

		return_without_semicolon_on_scope_end = cur_is_ident_or_int && (next_is_else || next_is_curly);
		if (return_without_semicolon_on_scope_end)
		{
			cur_node->l->flags = NODE_FLAGS_STMNT_WITHOUT_SEMICOLON;
		}
		// breaking when inside if, while for it found a word in head of the node
		if ((cur_is_ident_or_int && break_when_node_head) || return_without_semicolon_on_scope_end)
		{
			return cur_node->l;
		}



		PARSER_CHECK

			 peek = peek_tkn();
			switch (peek->type)
			{
			case tkn_type2::T_MINUS_MINUS:
			case tkn_type2::T_PLUS_PLUS:
			{
				get_tkn();
				cur_node = NewBinOpNode(lang_stat, nullptr, peek->type, cur_node->l);
				cur_node->type = N_UNOP;
				
				node* nd = new_node(lang_stat, cur_node->t);
				nd->l = cur_node;
				cur_node = nd;

				//get_tkn();
				//continue;
				// return cur_node;
			}break;
			case tkn_type2::T_COLON:
			{
				// double colon
				if ((peek_tkn() + 1)->type == tkn_type2::T_COLON)
				{

					cur_node->type = node_type::N_BINOP;
					cur_node->t = get_tkn();

					auto new_n = new_node(lang_stat, cur_node->t);
					cur_node->r = new_n;
					new_n->type = node_type::N_BINOP;
					new_n->t = get_tkn();
					new_n->r = parse_expr();

					new_n->r->t = begin_tkn;

					// we dont wanna return if prec target is EOF
					if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_IMPLICIT_SEMI_COLON) && prec != 32)
						return cur_node;
					else
						goto double_colon_scope_label;

				}
				// infer type
				else if ((peek_tkn() + 1)->type == tkn_type2::T_EQUAL)
				{

					//equal will at the top of the tree
					auto new_n = new_node(lang_stat, cur_node->l);
					new_n->t = get_tkn();
					new_n->l = cur_node->l;
					new_n->type = N_BINOP;
					cur_node->l = new_n;

					cur_prec = PREC_EQUAL;
				}

			}break;
			// array indexing
			case tkn_type2::T_OPEN_BRACKETS:
			{
				if (prec >= PREC_OPEN_BRACKETS)
					return cur_node->l;

				cur_node->type = node_type::N_INDEX;
				cur_node->t = peek_tkn();
				get_tkn();

				cur_node->type = node_type::N_INDEX;
				if (peek_tkn()->type != tkn_type2::T_CLOSE_BRACKETS)
				{
					cur_node->r = parse_(PREC_CLOSE_BRACKETS, EQUAL);
					get_tkn();
					goto  double_colon_scope_label;
				}

				get_tkn();

			}break;
			case tkn_type2::T_TWO_POINTS:
			{
				// checking if the last operator was a point, '.'
				if (prec == PREC_POINT)
					return cur_node->l;

				// checking if two points is unary
				if ((peek_tkn() + 1)->type == tkn_type2::T_CLOSE_BRACKETS)
				{
					cur_node->type = N_BINOP;
					cur_node->t = get_tkn();

					return cur_node;
				}
				else
				{
					cur_prec = 1;
				}
			}break;
			case tkn_type2::T_OPEN_PARENTHESES:
			{
				// checking if the last operator was a point, '.'
				if (prec == PREC_POINT)
					return cur_node->l;
				cur_node->t = peek_tkn();


				int last_flags = lang_stat->flags;
				lang_stat->flags |= PSR_FLAGS_ON_FUNC_CALL;

				get_tkn();
				cur_node->type = node_type::N_CALL;
				if(peek_tkn()->type != T_CLOSE_PARENTHESES)
					cur_node->r = parse_(PREC_CLOSE_PARENTHESES, parser_cond::EQUAL);

				ExpectTkn(T_CLOSE_PARENTHESES);
				get_tkn();



				lang_stat->flags = last_flags;


				if (peek_tkn()->type == tkn_type2::T_OPEN_PARENTHESES)
				{
					cur_node->call_templates = parse_expr();
				}

				if (peek_tkn()->type == tkn_type2::T_OPEN_CURLY && IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_BREAK_WHEN_NODE_HEAD_IS_WORD))
				{
					ReportMessageOne(lang_stat, cur_node->t, "Unexpected token. Maybe you are trying to declare a function", nullptr);
					ExitProcess(1);


				}

				goto  double_colon_scope_label;
			}break;
			default:
			{

			}break;

			}


		tkn = peek_tkn();
		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_SCOPE_WHITHOUT_CURLY) && tkn->type == T_SEMI_COLON)
			return cur_node->l;

		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_IMPLICIT_SEMI_COLON))
			lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;

		else if (!IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_IMPLICIT_SEMI_COLON))
		{
			// we wanna treat some words as binary operators, so in this case
			// we dont wanna make crash because it's at the head of a node
			if (!IsTknWordStr(tkn, "as"))
			{
				bool val = tkn->type != tkn_type2::T_WORD &&
					tkn->type != tkn_type2::T_FLOAT &&
					tkn->type != tkn_type2::T_STR_LIT &&
					tkn->type != tkn_type2::T_STR_LIT &&
					tkn->type != tkn_type2::T_INT;

				if (!val)
				{
					REPORT_ERROR(tkn->line, tkn->line_offset,
						VAR_ARGS("unexpected token '%s'\n", tkn->ToString().c_str()))
						ExitProcess(1);
				}
			}

			cur_node->t = get_tkn();
			is_operator(cur_node->t, &cur_prec);

			CheckTwoBinaryOparatorsTogether(cur_node);

		}

		if (cur_prec == PREC_SEMI_COLON)
		{
			CheckParenthesesLevel(lang_stat, begin_tkn);

			if (IS_PRS_FLAG_ON(PSR_FLAGS_ON_FUNC_CALL))
			{
				REPORT_ERROR(begin_tkn->line, begin_tkn->line_offset,
					VAR_ARGS("statements aren't allowed inside func calls'\n"))
					ExitProcess(1);
			}
			cur_node->type = node_type::N_STMNT;
		}
		else
		{
			int dummy_prec = 0;
			cur_node->type = node_type::N_BINOP;
			begin_tkn = peek_tkn();
			if (begin_tkn->type == T_CLOSE_CURLY && is_operator(cur_node->l->t, &dummy_prec) && IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_ON_STRUCT_DECL))
			{
				ReportMessageOne(lang_stat, cur_node->l->t, "unexpected token '%s'", (char*)begin_tkn->ToString().c_str());
			}
		}

		tt = peek_tkn();
		// checking the the first tkn is'nt an unary one
		// because we dont want to get its precedence
		if (peek_tkn()->type != tkn_type2::T_MUL && peek_tkn()->type != tkn_type2::T_MINUS)
		{
			PARSER_CHECK
		}

		ASSERT(cur_node->r == nullptr);


		cur_node->r = parse_(cur_prec, parser_cond::LESSER_EQUAL);


	double_colon_scope_label:
		auto nnode = new_node(lang_stat, cur_node->t);
		nnode->l = cur_node;
		cur_node = nnode;

	}
	return ret;
}
/*
void DecNodeScopeIdx(int *flags)
{
	char bit = 24;
	unsigned char cur_idx = ((*flags) >> bit) & 0xff;
	cur_idx--;
	int mask = ~(0xff << bit)
	*flags |= (*flags & mask) | (cur_idx << bit);
}
void IncNodeScopeIdx(int *flags)
{
	char bit = 24;
	unsigned char cur_idx = ((*flags) >> bit) & 0xff;
	cur_idx++;
	int mask = ~(0xff << bit)
	*flags |= (*flags & mask) | (cur_idx << bit);
}
*/

/*
node *node_iter::parse_sub_expression()
{
	ASSERT(false)
	return nullptr;
}
node *FindNode(node *start, bool(*func)(node **))

	node *cur_node = start;

	while(cur_node != nullptr)
	{
		if(func(&cur_node))
			return cur_node;
	}
	return nullptr;
}
node *node_iter::parse_expression(int cur_op)
{
	ASSERT(false)
	return nullptr;
}
*/
bool CheckFuncExtra(lang_state *lang_stat, func_decl* fdecl, scope* scp)
{
	char msg_hdr[256];
	auto fnode = fdecl->func_node;
	if (fnode->extra == nullptr)
		return true;

	FOR_VEC(it, (*fnode->extra))
	{
		auto nd = *it;
		switch (nd->extra_type)
		{
		case N_PLUGIN:
		{
			if (!DescendNameFinding(lang_stat, nd, scp))
				return false;
			ASSERT(nd->type == N_IDENTIFIER || nd->type == N_BINOP)
				own_std::vector<comma_ret> plugin_funcs;
			DescendComma(lang_stat, nd, scp, plugin_funcs);

			FOR_VEC(f, plugin_funcs)
			{
				type2 tp;
				auto d = FindIdentifier(f->decl.name, scp, &tp);
				ASSERT(d && d->type.type == TYPE_FUNC);

				fdecl->plugins.emplace_back(d->type.fdecl);
			}
		}break;

		default:
			ASSERT(false);
		}
	}
	return true;
}
bool CheckFuncRetType(lang_state *lang_stat, func_decl* fdecl, scope* scp)
{
	char msg_hdr[256];
	auto fnode = fdecl->func_node;
	//return type
	if (fnode->l->r != nullptr)
	{
		if (!NameFindingGetType(lang_stat, fnode->l->r, scp, fdecl->ret_type))
			return false;
		fdecl->ret_type.type = FromTypeToVarType(fdecl->ret_type.type);
	}
	else
		fdecl->ret_type.type = enum_type2::TYPE_VOID;


	fdecl->strct_val_ret_offset = fdecl->strct_vals_offset;
	CheckStructValToFunc(fdecl, &fdecl->ret_type);

	// checking if func has return and, if not, report an error

	// asserting we're returning somethins at the end of the scope, but excluding function that have templates
	//getting the correct last stmnt
	if (fdecl->ret_type.type != enum_type2::TYPE_VOID && fnode->r->r->r && IS_FLAG_OFF(fdecl->flags, FUNC_DECL_TEMPLATED))
	{
		bool can_report = false;
		node* check_ret_nd = fnode->r->r;

		while (check_ret_nd->type == N_STMNT)
		{
			check_ret_nd = check_ret_nd->r;
			// function probably has only an if in the entire scope
			if (!check_ret_nd)
				break;
		}
		if (check_ret_nd)
		{
			bool is_key_word = CMP_NTYPE(check_ret_nd, N_KEYWORD);

			if (!is_key_word || (is_key_word && !(check_ret_nd->kw == KW_RETURN)))
				can_report = true;
		}
		else
			can_report = true;


		if (can_report)
		{
			REPORT_ERROR(fdecl->func_node->t->line, fdecl->func_node->t->line_offset, VAR_ARGS("function %s must return a value\n", fdecl->name.c_str()))
				ExitProcess(1);
		}
	}
	return true;
}


struct descend_strct
{
	node* n;
	scope* scp;
};
struct thread_work
{
	std::queue<message*> buffers;
	int buffer_size;
	int front_idx;
	int read_idx_buffer;
	int read_idx;
	bool tree_was_modified;

	void AddMessage(message* msg)
	{
		auto cur_buffer = (buffers.back());
		memcpy(cur_buffer + front_idx, msg, sizeof(message));

		front_idx++;

		if (front_idx >= buffer_size)
		{
			buffers.emplace((message*)malloc(buffer_size * sizeof(message)));
			read_idx_buffer++;
			front_idx = 0;
		}
	}
	message* GetMessage()
	{
		if (read_idx >= front_idx && buffers.size() == 1)
		{
			return nullptr;
		}
		message* cur_buffer = (buffers.front() + read_idx_buffer);


		if (read_idx >= buffer_size)
		{
			read_idx = 0;
			buffers.pop();
			read_idx_buffer--;
		}

		message* ret = cur_buffer + read_idx;
		read_idx++;

		return ret;
	}
}works;
void ModifyTree(node* to_modify, node* val)
{
	memcpy(to_modify, val, sizeof(node));
	works.tree_was_modified = true;
}
void DescendScopeForFuncs(lang_state *lang_stat, scope* s)
{
	FOR_VEC(child, s->children)
	{
		if (*child == nullptr)
			continue;
		DescendScopeForFuncs(lang_stat, *child);
	}
	FOR_VEC(f, s->vars)
	{

		if ((*f)->type.type == enum_type2::TYPE_FUNC)
		{
			func_decl* fdecl = (*f)->type.fdecl;
			// fn defs don't have scope
			if (fdecl->func_node->r == nullptr)
				continue;
			DescendNode(lang_stat, fdecl->func_node->r, fdecl->scp);
		}
	}
}

/*
DWORD WINAPI ThreadProcLang(LPVOID lpParameter)
{
	auto strct = (descend_strct*)lpParameter;

	works.tree_was_modified = true;
	while (works.tree_was_modified)
	{
		DescendNode(lang_stat, strct->n, strct->scp);
		//DescendScopeForFuncs(strct->scp);
		works.tree_was_modified = false;
	}

	message msg;
	msg.type = msg_type::MSG_DONE;
	works.AddMessage(&msg);
	return 1;
}
void TypeCheckTree(node* n, scope* scp, void(*callback)())
{

	works.front_idx = 0;
	works.read_idx = 0;
	works.buffer_size = 64;

	auto buffer = (message*)malloc(works.buffer_size * sizeof(message));

	works.buffers.emplace(buffer);
	descend_strct s;
	s.n = n;
	s.scp = scp;

	CreateThread(NULL, 0, ThreadProcLang, &s, 0, nullptr);

	callback();

}
*/
node* ParseString(lang_state *lang_stat, own_std::string str, int start_line)
{
	auto tkns = (own_std::vector<token2> *)AllocMiscData(lang_stat, sizeof(own_std::vector<token2>));
	memset(tkns, 0, sizeof(own_std::vector<token2>));
	lang_stat->allocated_vectors.emplace_back(tkns);
	Tokenize2((char*)str.data(), str.size(), tkns);

	if (start_line != 0)
	{
		FOR_VEC(t, *tkns)
		{
			t->line = start_line;
		}
	}

	node_iter niter = node_iter(tkns, lang_stat);
	auto ret = niter.parse_all();
	lang_stat->flags &= ~PSR_FLAGS_IMPLICIT_SEMI_COLON;
	return ret;
}
#define IS_COMMA(n) ((n)->type == node_type::N_BINOP && (n)->t->type == tkn_type2::T_COMMA)
#define IS_DECL(n) ((n)->type == node_type::N_DECL)
#define IS_COLON(n) ((n)->type == node_type::N_BINOP && (n)->t->type == tkn_type2::T_COLON)
#define IS_EQUAL(n) ((n)->type == node_type::N_BINOP && (n)->t->type == tkn_type2::T_EQUAL)
#define IS_IDENT(n) ((n)->type == node_type::N_IDENTIFIER)

type2 DescendNode(lang_state *, node* n, scope* given_scp);


#define COMMA_VAR_ARGS 4
#define COMMA_RET_TYPE 3
#define COMMA_RET_IDENT 2
#define COMMA_RET_COLON 1
#define COMMA_RET_EXPR  0
void DescendComma(lang_state *lang_stat, node* n, scope* scp, own_std::vector<comma_ret>& ret)
{
	if (IS_COMMA(n) && IS_FLAG_OFF(n->flags, NODE_FLAGS_COMMA_INSIDE_PARENTHESES))
	{
		DescendComma(lang_stat, n->l, scp, ret);

		DescendComma(lang_stat, n->r, scp, ret);
	}
	else
	{
		comma_ret cret = {};
		memset(&cret, 0, sizeof(comma_ret));
		cret.n = n;

		if (IS_COLON(n))
		{
			cret.type = COMMA_RET_COLON;

			NameFindingGetType(lang_stat, n->r, scp, cret.decl.type);
			cret.decl.name = n->l->t->str.substr();

			ret.emplace_back(cret);
		}

		else if (n->type == node_type::N_VAR_ARGS)
		{
			cret.type = COMMA_VAR_ARGS;
			cret.n = n;

			ret.emplace_back(cret);
		}
		else if (n->type == node_type::N_TYPE)
		{
			cret.type = COMMA_RET_TYPE;
			cret.decl.type = n->decl_type;

			ret.emplace_back(cret);
		}

		else if (IS_IDENT(n))
		{

			cret.type = COMMA_RET_IDENT;

			cret.decl.type.type = enum_type2::TYPE_AUTO;
			cret.decl.name = n->t->str.substr();

			ret.emplace_back(cret);

		}
		// this is an ugly hack, an empty scope has an open curly child node, so in the case of 
		// a declaration of enum, if is scope is empty, the DescedComma will return an array with the 
		// open curly, which will cause a problem in the parsing of the values of the enum
		else if (n->t && n->t->type != T_OPEN_CURLY || n->t == nullptr)
		{
			cret.type = COMMA_RET_EXPR;
			cret.decl.type = DescendNode(lang_stat, n, scp);
			ret.emplace_back(cret);
		}
		lang_stat->call_regs_used++;
	}
}
void DescendTemplates(node* n, scope* scp, own_std::vector<template_expr>*);

own_std::vector<template_expr> DescendNodeTemplates(node* n, scope* scp)
{
	own_std::vector<template_expr> ret;
	ret.reserve(8);



	return ret;
}
void DescendTemplates(node* n, scope* scp, own_std::vector<template_expr>* ret)
{

	if (IS_IDENT(n))
	{
		ASSERT(n->type == node_type::N_IDENTIFIER)

			template_expr tret = {};
		memset(&tret, 0, sizeof(tret));
		tret.name = n->t->str.substr();
		tret.scp = scp->parent;
		ret->emplace_back(tret);
	}
	else if (IS_EQUAL(n))
	{
		template_expr tret = {};
		memset(&tret, 0, sizeof(comma_ret));
		tret.type = 1;
		tret.name = n->l->t->str.substr();
		tret.expr = n->r;
		tret.scp = scp->parent;
		ret->emplace_back(tret);
	}
	else if (IS_COMMA(n))
	{
		DescendTemplates(n->l, scp, ret);
		DescendTemplates(n->r, scp, ret);
	}
	else
	{
		ASSERT(n->type == node_type::N_IDENTIFIER)
			template_expr tret = {};
		memset(&tret, 0, sizeof(comma_ret));
		tret.name = n->t->str.substr();
		tret.scp = scp->parent;
		ret->emplace_back(tret);
	}

}
scope* NewScope(lang_state *lang_stat, scope* scp)
{
	auto ret = (scope*)AllocMiscData(lang_stat, sizeof(scope));
	if(scp)
		scp->children.emplace_back(ret);
	memset(ret, 0, sizeof(scope));
	ret->parent = scp;
	return ret;
}
decl2* NewDecl(lang_state *lang_stat, own_std::string name, type2 tp)
{
	auto ret = (decl2*)AllocMiscData(lang_stat, sizeof(decl2));
	memset(ret, 0, sizeof(decl2));
	new(&ret->name)own_std::string(name.substr());
	ret->type = tp;

	return ret;
}

bool TemplTypeFromStruct(type2* in, type2* out, type2* arg, own_std::string target_templ_name)
{
	if (arg->strct->original_strct && in->strct->name == arg->strct->original_strct->name)
	{
		int arg_strct_templs_count = in->strct->templates.size();
		auto strct_vars = &arg->strct->scp->vars;
		for (int i = 0; i < arg_strct_templs_count; i++)
		{
			auto cur_templ = (*strct_vars)[i];
			if (cur_templ->type.type == enum_type2::TYPE_STRUCT)
			{
				if (TemplTypeFromStruct(in, out, &cur_templ->type, target_templ_name))
					return true;
			}
			else if (cur_templ->name == target_templ_name)
			{
				*out = *cur_templ->type.tp;
				return true;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

// $TemplatesType
own_std::vector<type2> TemplatesTypeToLangArray(lang_state *lang_stat, own_std::vector<template_expr>* templates, own_std::vector<decl2*>* original, own_std::vector<comma_ret>* args)
{
	own_std::vector<type2> ret;
	int i = 0;
	//own_std::string ret;
	FOR_VEC(t, *templates)
	{
		decl2* og_a = (*original)[i];
		auto given_arg = &(*args)[i];

		if (given_arg->decl.type.type == TYPE_STATIC_ARRAY)
			ReportMessage(lang_stat, given_arg->n->t, "Cannot create template from static array types. Try dereferencing it.");

		// if the og func arg is a plain template, which is not inside another type
		if (og_a->type.type == enum_type2::TYPE_TEMPLATE && og_a->type.templ_name == t->name)
		{
			given_arg->decl.type.type = FromVarTypeToType(given_arg->decl.type.type);
			ret.emplace_back(given_arg->decl.type);
		}
		else if (og_a->type.type == enum_type2::TYPE_STRUCT)
		{
			type2 tp;
			if (TemplTypeFromStruct(&og_a->type, &tp, &given_arg->decl.type, t->name))
			{
				ret.emplace_back(tp);
			}

		}
		i++;
	}
	return ret;
}
// $TemplatesType
own_std::string TemplatesTypeToString(own_std::vector<template_expr>* templates, func_decl* original, own_std::vector<comma_ret>* args)
{
	int i = 0;
	own_std::string ret;
	ret += "{";
	FOR_VEC(t, *templates)
	{
		decl2* og_a = original->args[i];
		auto given_arg = &(*args)[i];
		// if the og func arg is a plain template, which is not inside another type
		if (og_a->type.type == enum_type2::TYPE_TEMPLATE && og_a->type.templ_name == t->name)
		{
			ret += TypeToString((*args)[0].decl.type);
			ret += ", ";
		}
		else if (og_a->type.type == enum_type2::TYPE_STRUCT)
		{
			type2 tp;
			if (TemplTypeFromStruct(&og_a->type, &tp, &given_arg->decl.type, t->name))
			{
				ret += TypeToString(tp);
				ret += ", ";

			}

		}
		i++;
	}
	ret.pop_back();
	ret.pop_back();
	ret += "}";
	return ret;
}
own_std::vector<decl2*> GetTemplateTypes(lang_state *lang_stat, own_std::vector<template_expr>* templates, own_std::vector<comma_ret>* args, scope* scp, func_decl* fdecl)
{
	own_std::vector<decl2*> ret;
	scope* aux_scope = NewScope(lang_stat, scp);
	int i = 0;
	// checking single type templates var : T
	FOR_VEC(a, *args)
	{
		auto cur_t = &(*templates)[i];
		// but the template must not be a expr
		if (cur_t->expr == nullptr)
		{
			cur_t->final_type = (type2*)AllocMiscData(lang_stat, sizeof(type2));
			*(cur_t->final_type) = a->decl.type;

			decl2 decl;
			decl.name = cur_t->name.substr();

			decl.type.type = enum_type2::TYPE_TEMPLATE;
			decl.type.tp = cur_t->final_type;

			auto new_decl = NewDecl(lang_stat, decl.name, decl.type);
			aux_scope->vars.emplace_back(new_decl);
			ret.emplace_back(new_decl);

			// assigning templates to the function declaration
			if (fdecl != nullptr)
			{
				int cur_arg = 0;
				FOR_VEC(fparam, fdecl->args)
				{
					(*fparam)->AssignTemplate(lang_stat, cur_t->name, cur_t->final_type, &(*args)[cur_arg]);
					aux_scope->vars.emplace_back(*fparam);
					cur_arg += 1;

				}
			}
		}
		i++;
		/*
		else
		{
			ASSERT(cur_t->final_type->type == a->decl.type.type)
		}
		*/
	}

	// checking reamining templates
	FOR_VEC(t, *templates)
	{
		if (t->final_type == nullptr)
		{
			t->final_type = (type2*)AllocMiscData(lang_stat, sizeof(type2));
			*t->final_type = DescendNode(lang_stat, t->expr, aux_scope);
			decl2 decl;
			decl.name = t->name.substr();
			decl.type.tp = t->final_type;
			decl.type.type = enum_type2::TYPE_TEMPLATE;

			auto new_decl = NewDecl(lang_stat, decl.name, decl.type);
			ret.emplace_back(new_decl);
		}
	}
	//free(aux_scope);
	return ret;
}
own_std::vector<decl2*> DescendTemplatesToDecl(lang_state *lang_stat, node* n, scope* scp, own_std::vector<template_expr>* in_vec)
{
	own_std::vector<decl2*> ret;

	DescendTemplates(n, scp, in_vec);

	FOR_VEC(t, *in_vec)
	{
		decl2 decl;
		decl.name = t->name.substr();
		decl.type.type = enum_type2::TYPE_TEMPLATE;
		decl.type.tp = t->final_type;
		decl.type.templ_name = t->name.substr();

		auto new_decl = NewDecl(lang_stat, decl.name, decl.type);

		ret.emplace_back(new_decl);
	}
	return ret;
}
#define FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE 1
decl2* FindIdentifier(own_std::string name, scope* scp, type2* ret_type, int flags)
{

	// checking self ref
	scope* cur_scope = scp;
	while (cur_scope != nullptr)
	{
		// self ref
		if (IS_FLAG_ON(cur_scope->flags, SCOPE_INSIDE_STRUCT) && cur_scope->tstrct->name == name)
		{
			ret_type->type = enum_type2::TYPE_STRUCT_TYPE;
			ret_type->flags |= TYPE_SELF_REF;
			ret_type->strct = cur_scope->tstrct;
			return ret_type->strct->this_decl;
		}
		cur_scope = cur_scope->parent;
	}


	auto decl = scp->FindVariable(name);
	if (decl == nullptr)
		return nullptr;

	// self ref
	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_STRUCT) && scp->tstrct->name == name)
	{
		ret_type->flags |= TYPE_SELF_REF;
		return decl;
	}

	bool is_recursive_call = IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION) && scp->fdecl->name == name;

	//ASSERT(decl != nullptr)
	if (IS_FLAG_ON(decl->flags, DECL_NOT_DONE)
		&& IS_FLAG_OFF(flags, FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE)
		&& !is_recursive_call)
		return nullptr;



	if (decl->type.type == enum_type2::TYPE_STRUCT_TYPE 
		&& IS_FLAG_ON(decl->type.strct->flags, TP_STRCT_STRUCT_NOT_NODE) 
		&& IS_FLAG_OFF(flags, FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE))
		return nullptr;

	*ret_type = decl->type;

	if (decl->type.type == enum_type2::TYPE_TEMPLATE && decl->type.tp != nullptr)
		*ret_type = *decl->type.tp;
	else if (decl->type.type == TYPE_TYPEDEF)
		*ret_type = decl->type.type_def_decl->type;
	else
		*ret_type = decl->type;
	return decl;
}
int max2(int a, int b)
{
	return a > b ? a : b;
}
void DescendStmnt(lang_state *lang_stat, node* n, scope* scp)
{
	own_std::vector<node*> node_stack;
	node* cur_node = n;
	own_std::vector<scope*> scp_stack;
	while (cur_node->l->type == node_type::N_STMNT)
	{
		node_stack.emplace_back(cur_node);
		/*
		if (IS_FLAG_ON(cur_node->flags, NODE_FLAGS_IS_SCOPE))
		{
			scp_stack.emplace_back(scp);
			scp = GetScopeFromParent(cur_node, scp);
		}
		*/
		cur_node = cur_node->l;
	}

	int size = node_stack.size();
	int cur_scp_idx = scp_stack.size() - 1;

	while (true)
	{
		lang_stat->cur_stmnt = cur_node;

		if (cur_node->l != nullptr && IS_FLAG_OFF(cur_node->l->flags, NODE_FLAGS_IS_PROCESSED3))
		{
			DescendNode(lang_stat, cur_node->l, scp);
			CheckDeclNodeAndMaybeAddEqualZero(lang_stat, cur_node->l, scp);

			cur_node->l->flags |= NODE_FLAGS_IS_PROCESSED3;
			int a = 0;
		}

		if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
		{
			scp->fdecl->per_stmnt_strct_val_sz = max2(scp->fdecl->per_stmnt_strct_val_sz, lang_stat->cur_per_stmnt_strct_val_sz);
		}

		lang_stat->cur_per_stmnt_strct_val_sz = 0;
		lang_stat->lhs_saved = 0;

		if (cur_node->r != nullptr && IS_FLAG_OFF(cur_node->r->flags, NODE_FLAGS_IS_PROCESSED3))
		{
			DescendNode(lang_stat, cur_node->r, scp);

			CheckDeclNodeAndMaybeAddEqualZero(lang_stat, cur_node->r, scp);

			cur_node->r->flags |= NODE_FLAGS_IS_PROCESSED3;
		}

		size--;

		cur_node->flags |= NODE_FLAGS_IS_PROCESSED3;

		if (size < 0)
			break;

		//if (IS_FLAG_ON(cur_node->flags, NODE_FLAGS_IS_SCOPE))
			//scp = scp_stack[cur_scp_idx--];

		cur_node = *(node_stack.end() - 1);
		node_stack.pop_back();

	}
}
int CheckBiggestType(type2* t, int cur_biggest)
{
	int cmp = 0;
	if (t->ptr > 0)
	{
		cmp = 8;
	}
	else
	{
		switch (t->type)
		{
		case enum_type2::TYPE_STRUCT:
			cmp = t->strct->biggest_type;
			break;
		case enum_type2::TYPE_STATIC_ARRAY:
		case enum_type2::TYPE_ARRAY:
			cmp = GetTypeSize(t->tp);
			break;
		default:
			cmp = GetTypeSize(t);
		}
	}

	if (cur_biggest > cmp)
		return cur_biggest;

	return cmp;
}

int get_even_address_with(int mod, int addr)
{
	int val = mod - (addr % mod);
	if (val == mod)
		return addr;

	return addr + val;
}
int SetVariablesAddress(own_std::vector<decl2*>* ar, int start, int* in_out_biggest_type, int start_addr = 0)
{
	int addr = start_addr;
	for (auto var = ar->begin() + start; var < ar->end(); var++)
	{
		auto t = *var;
		if (IS_FLAG_ON(t->flags, DECL_FROM_USING) || t->type.type == TYPE_STRUCT_TYPE)
			continue;
		// asserting that the struct vars are with colon declared
		//ASSERT(t->type == 1)


		*in_out_biggest_type = CheckBiggestType(&t->type, *in_out_biggest_type);

		/*
		if(t->type.ptr > 0)
		{
			other_biggest_type = 8;
		}
		else
		{
			switch(t->type.type)
			{
			case enum_type2::TYPE_STRUCT:
				other_biggest_type = t->type.strct->biggest_type;
			break;
			case enum_type2::TYPE_STATIC_ARRAY:
			case enum_type2::TYPE_ARRAY:
				other_biggest_type = 8;
				break;
			default:
				other_biggest_type = GetTypeSize(&t->type);
			}
		}
		*/

		int t_sz = GetTypeSize(&t->type);
		if(t->type.type == TYPE_STATIC_ARRAY)
			t_sz = GetTypeSize(t->type.tp);

		addr = get_even_address_with(t_sz, addr);
		//*in_out_biggest_type = other_biggest_type > cur_size ? other_biggest_type : cur_size;

		t->offset = addr;

		addr += GetTypeSize(&t->type);
	}
	if (*in_out_biggest_type != 0)
	{
		int has_to_pad = addr % *in_out_biggest_type;
		addr += has_to_pad == 0 ? 0 : *in_out_biggest_type - has_to_pad;
	}
	return addr;
}

enum_type2 FromVarTypeToType(enum_type2 tp)
{
	enum_type2 ret_type;

	switch (tp)
	{

	case enum_type2::TYPE_STATIC_ARRAY:
		ret_type = enum_type2::TYPE_STATIC_ARRAY_TYPE;
		break;

	case enum_type2::TYPE_FUNC_EXTERN:
		ret_type = enum_type2::TYPE_FUNC_PTR;
		break;
	case enum_type2::TYPE_FUNC_PTR:
		ret_type = enum_type2::TYPE_FUNC_DEF;
		break;

	case enum_type2::TYPE_STRUCT_DECL:
		ret_type = enum_type2::TYPE_STRUCT_TYPE;
		break;
	case  enum_type2::TYPE_STRUCT_TYPE:
		ret_type = enum_type2::TYPE_STRUCT;
		break;

	case enum_type2::TYPE_ENUM_DECL:
		ret_type = enum_type2::TYPE_ENUM_TYPE;
		break;
	case  enum_type2::TYPE_ENUM_TYPE:
		ret_type = enum_type2::TYPE_ENUM;
		break;


	case enum_type2::TYPE_FUNC_DECL:
		ret_type = enum_type2::TYPE_FUNC_TYPE;
		break;
	case enum_type2::TYPE_FUNC:
		ret_type = enum_type2::TYPE_FUNC_TYPE;
		break;

	case  enum_type2::TYPE_BOOL:
		ret_type = enum_type2::TYPE_BOOL_TYPE;
		break;
	case  enum_type2::TYPE_S64:
		ret_type = enum_type2::TYPE_S64_TYPE;
		break;
	case  enum_type2::TYPE_S32:
		ret_type = enum_type2::TYPE_S32_TYPE;
		break;
	case  enum_type2::TYPE_S16:
		ret_type = enum_type2::TYPE_S16_TYPE;
		break;
	case  enum_type2::TYPE_S8:
		ret_type = enum_type2::TYPE_S8_TYPE;
		break;

	case  enum_type2::TYPE_F64:
		ret_type = enum_type2::TYPE_F64_TYPE;
		break;
	case  enum_type2::TYPE_F32_RAW:
	case  enum_type2::TYPE_F32:
		ret_type = enum_type2::TYPE_F32_TYPE;
		break;

	case  enum_type2::TYPE_VOID:
		ret_type = enum_type2::TYPE_VOID_TYPE;
		break;

	case  enum_type2::TYPE_ENUM:
		ret_type = enum_type2::TYPE_ENUM;
		break;

	case  enum_type2::TYPE_CHAR:
		ret_type = enum_type2::TYPE_CHAR_TYPE;
		break;

	case  enum_type2::TYPE_U64:
		ret_type = enum_type2::TYPE_U64_TYPE;
		break;
	case  enum_type2::TYPE_U32:
		ret_type = enum_type2::TYPE_U32_TYPE;
		break;
	case  enum_type2::TYPE_U16:
		ret_type = enum_type2::TYPE_U16_TYPE;
		break;
	case  enum_type2::TYPE_U8:
		ret_type = enum_type2::TYPE_U8_TYPE;
		break;

	case  enum_type2::TYPE_STRUCT:
		ret_type = enum_type2::TYPE_STRUCT_TYPE;
		break;

	case  enum_type2::TYPE_TEMPLATE:
		ret_type = enum_type2::TYPE_TEMPLATE;
		break;
	case  enum_type2::TYPE_INT:
		ret_type = enum_type2::TYPE_INT;
		break;
	case  enum_type2::TYPE_AUTO:
		ret_type = enum_type2::TYPE_AUTO;
		break;
	case  enum_type2::TYPE_STR_LIT:
		ret_type = enum_type2::TYPE_STR_LIT;
		break;
	default:
		ASSERT(false)
	}
	return ret_type;
}

enum_type2 FromTypeToVarType(enum_type2 tp)
{
	enum_type2 ret_type;

	switch (tp)
	{
	case enum_type2::TYPE_REL_TYPE:
		ret_type = enum_type2::TYPE_REL;
		break;

	case enum_type2::TYPE_ARRAY_TYPE:
		ret_type = enum_type2::TYPE_ARRAY;
		break;
	case enum_type2::TYPE_STATIC_ARRAY_TYPE:
		ret_type = enum_type2::TYPE_STATIC_ARRAY;
		break;

	case enum_type2::TYPE_FUNC_PTR:
		ret_type = enum_type2::TYPE_FUNC_EXTERN;
		break;
	case enum_type2::TYPE_FUNC_DEF:
		ret_type = enum_type2::TYPE_FUNC_PTR;
		break;

	case enum_type2::TYPE_STRUCT_DECL:
		ret_type = enum_type2::TYPE_STRUCT_TYPE;
		break;
	case  enum_type2::TYPE_STRUCT_TYPE:
		ret_type = enum_type2::TYPE_STRUCT;
		break;

	case enum_type2::TYPE_ENUM_DECL:
		ret_type = enum_type2::TYPE_ENUM_TYPE;
		break;
	case  enum_type2::TYPE_ENUM_TYPE:
		ret_type = enum_type2::TYPE_ENUM;
		break;


	case enum_type2::TYPE_FUNC_DECL:
		ret_type = enum_type2::TYPE_FUNC_TYPE;
		break;
	case enum_type2::TYPE_FUNC_TYPE:
		ret_type = enum_type2::TYPE_FUNC;
		break;

	case  enum_type2::TYPE_BOOL_TYPE:
		ret_type = enum_type2::TYPE_BOOL;
		break;
	case  enum_type2::TYPE_S64_TYPE:
		ret_type = enum_type2::TYPE_S64;
		break;
	case  enum_type2::TYPE_S32_TYPE:
		ret_type = enum_type2::TYPE_S32;
		break;
	case  enum_type2::TYPE_S16_TYPE:
		ret_type = enum_type2::TYPE_S16;
		break;
	case  enum_type2::TYPE_S8_TYPE:
		ret_type = enum_type2::TYPE_S8;
		break;

	case  enum_type2::TYPE_F64_TYPE:
		ret_type = enum_type2::TYPE_F64;
		break;
	case  enum_type2::TYPE_F32_TYPE:
		ret_type = enum_type2::TYPE_F32;
		break;

	case  enum_type2::TYPE_VOID_TYPE:
		ret_type = enum_type2::TYPE_VOID;
		break;

	case  enum_type2::TYPE_U64_TYPE:
		ret_type = enum_type2::TYPE_U64;
		break;
	case  enum_type2::TYPE_U32_TYPE:
		ret_type = enum_type2::TYPE_U32;
		break;
	case  enum_type2::TYPE_U16_TYPE:
		ret_type = enum_type2::TYPE_U16;
		break;
	case  enum_type2::TYPE_U8_TYPE:
		ret_type = enum_type2::TYPE_U8;
		break;

	case  enum_type2::TYPE_CHAR_TYPE:
		ret_type = enum_type2::TYPE_CHAR;
		break;

	case  enum_type2::TYPE_TEMPLATE:
		ret_type = enum_type2::TYPE_TEMPLATE;
		break;
	case  enum_type2::TYPE_INT:
		ret_type = enum_type2::TYPE_INT;
		break;
	case  enum_type2::TYPE_ENUM:
		ret_type = enum_type2::TYPE_ENUM;
		break;
	case  enum_type2::TYPE_AUTO:
		ret_type = enum_type2::TYPE_AUTO;
		break;
	case  enum_type2::TYPE_MACRO_EXPR:
		ret_type = enum_type2::TYPE_MACRO_EXPR;
		break;
	case  enum_type2::TYPE_TYPEDEF:
		ret_type = enum_type2::TYPE_TYPEDEF;
		break;
	case  enum_type2::TYPE_STR_LIT:
		ret_type = enum_type2::TYPE_STR_LIT;
		break;
	default:
		ASSERT(false)
	}
	return ret_type;
}
scope* GetScopeFromParent(lang_state *lang_stat, node* n, scope* given_scp)
{
	scope* scp = nullptr;
	/*
	unsigned char scope_idx = (n->flags >> 24) & 0xff;

	ASSERT(IS_FLAG_ON(n->flags, NODE_FLAGS_IS_SCOPE))

	if(scope_idx >= given_scp->children.size())
	{
		given_scp->children.resize((scope_idx + 1) * 2);
	}

	// creating a new scope
	if(given_scp->children[scope_idx] == nullptr)
	{
		scp = NewScope(given_scp);
		given_scp->children[scope_idx] = scp;
	}
	else
	{
		scp = given_scp->children[scope_idx];
	}
	*/
	ASSERT(n->type == N_SCOPE);
	if (!n->scp)
	{
		scp = (scope*)AllocMiscData(lang_stat, sizeof(scope));
		memset(scp, 0, sizeof(scope));
		given_scp->children.emplace_back(scp);
		scp->parent = given_scp;
		scp->line_start = n->scope_line_start;
		scp->line_end = n->scope_line_end;
		n->scp = scp;
	}
	scp = n->scp;
	ASSERT(scp != nullptr);

	// setting the outer struct or function to the scope
	if (IS_FLAG_ON(given_scp->flags, SCOPE_INSIDE_FUNCTION))
	{
		scp->flags |= SCOPE_INSIDE_FUNCTION;
		scp->fdecl = scp->parent->fdecl;
	}
	if (IS_FLAG_ON(given_scp->flags, SCOPE_INSIDE_STRUCT))
	{
		scp->flags |= SCOPE_INSIDE_STRUCT;
		scp->tstrct = scp->parent->tstrct;
	}
	return scp;
}
template <typename T>
T GetExpressionValT(tkn_type2 tp, T a, T b)
{
	switch (tp)
	{
	case tkn_type2::T_COND_EQ:	return a == b;
	case tkn_type2::T_COND_NE:	return a != b;
	case tkn_type2::T_MINUS:	return a - b;
	case tkn_type2::T_PLUS:	return a + b;
	case tkn_type2::T_MUL:	return a * b;
	case tkn_type2::T_DIV:	return a / b;
	case tkn_type2::T_PIPE:	return (int)a | (int)b;
	default:ASSERT(false)
	}


}
int GetExpressionVal(node* n, scope* scp)
{
	switch (n->type)
	{
	case node_type::N_INT:	return n->t->i;
	case node_type::N_IDENTIFIER:
	{
		type2 ret_type;
		auto ident = FindIdentifier(n->t->str, scp, &ret_type);
		ASSERT(ident->type.type == TYPE_INT);
		return ident->type.i;
	}break;
	case node_type::N_BINOP:
	{
		int lhs = GetExpressionVal(n->l, scp);
		int rhs = GetExpressionVal(n->r, scp);
		switch (n->t->type)
		{
		case tkn_type2::T_COND_AND:	 return lhs & rhs;
		case tkn_type2::T_MINUS: return lhs - rhs;
		case tkn_type2::T_PLUS:	 return lhs + rhs;
		case tkn_type2::T_MUL:	 return lhs * rhs;
		case tkn_type2::T_COND_EQ:	 return lhs == rhs;
		default: ASSERT(false)
			//case tkn_type2::T_DIV:	return lhs / rhs;
		}
	}break;
	case node_type::N_UNOP:
	{
		int rhs = GetExpressionVal(n->r, scp);
		switch (n->t->type)
		{
		case tkn_type2::T_MINUS: return -rhs;
		case tkn_type2::T_TILDE: return ~rhs;
		default: ASSERT(false)
		}
	}break;
	default:
		ASSERT(false)
	}
}
bool TryInstantiateStruct(lang_state *lang_stat, type_struct2* original, own_std::string templ_name, scope* scp, type_struct2** ret, own_std::vector<comma_ret>& args, type2* from_type = nullptr)
{
	auto strct = original;
	own_std::vector<template_expr> in_vec;

	type2 ret_type;
	memset(&ret_type, 0, sizeof(ret_type));
	auto instantions_exist = FindIdentifier(templ_name, scp, &ret_type, FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE);

	type_struct2* new_strct;
	scope* new_scope;

	int templates_idx_end = args.size();

	if (from_type)
	{
		ASSERT(args.size() == 0);
		comma_ret cret = {};
		memset(&cret, 0, sizeof(comma_ret));
		cret.type = COMMA_RET_IDENT;

		cret.decl.type = *from_type;
		//cret.decl.name = n->t->str.substr();

		args.emplace_back(cret);
		templates_idx_end = 1;

	}

	if (!instantions_exist)
	{
		auto tdecls = DescendTemplatesToDecl(lang_stat, strct->strct_node->l, strct->scp, &in_vec);

		ASSERT(in_vec.size() == args.size())

			scope* target_scope = strct->scp->parent;

		auto templates_types = GetTemplateTypes(lang_stat, &in_vec, &args, target_scope, nullptr);
		// @test new_scope = NewScope(target_scope);
		new_scope = NewScope(lang_stat, scp->parent);
		//

		new_scope->vars.assign(templates_types.begin(), templates_types.end());

		// later right below, we dont wanna add the templates as the struct's variables, so we're keeping track


		// creating new struct from the template
		new_strct = (type_struct2*)AllocMiscData(lang_stat, sizeof(type_struct2));
		memset(new_strct, 0, sizeof(type_struct2));
		new_scope->flags |= SCOPE_INSIDE_STRUCT;
		new_scope->tstrct = new_strct;

		new_strct->original_strct = strct;
		new_strct->scp = new_scope;

		type2 tp;
		tp.strct = new_strct;
		tp.type = enum_type2::TYPE_STRUCT_TYPE;
		instantions_exist = NewDecl(lang_stat, templ_name, tp);

		// @test original->scp->parent->vars.emplace_back(instantions_exist);
		scp->parent->vars.emplace_back(instantions_exist);
		//

		new_strct->flags = TP_STRCT_STRUCT_NOT_NODE;
		new_strct->this_decl = instantions_exist;
	}
	else
	{
		new_strct = instantions_exist->type.strct;
		new_scope = new_strct->scp;
		// returning if the struct is done and it 
		if (IS_FLAG_OFF(new_strct->flags, TP_STRCT_STRUCT_NOT_NODE))
		{
			ret_type.type = enum_type2::TYPE_STRUCT_TYPE;
			ret_type.strct = new_strct;
			*ret = new_strct;
			return true;
		}
	}
	int last_flags = lang_stat->flags;
	lang_stat->flags &= ~PSR_FLAGS_DONT_DECLARE_VARIABLES;
	lang_stat->flags |= PSR_FLAGS_DONT_CHANGE_TEMPL_STRCT_ND_NAME;
	while (true)
	{
		lang_stat->something_was_declared = false;
		if (!DescendNameFinding(lang_stat, original->strct_node->r->r, new_scope))
		{
			lang_stat->flags = last_flags;
			return false;
		}

		if (!lang_stat->something_was_declared)
			break;
	}
	lang_stat->flags = last_flags;

	*ret = new_strct;

	//ASSERT(ncall->l->t->type == tkn_type2::T_WORD)

	new_strct->name = templ_name.substr();


	new_strct->vars.assign(new_scope->vars.begin() + templates_idx_end, new_scope->vars.end());

	new_strct->size = SetVariablesAddress(&new_strct->vars, 0, &new_strct->biggest_type);

	/*
	FOR_VEC(f, new_strct->op_overloads)
	{
		DescendNode((*f)->func_node->r, (*f)->scp);
	}
	*/

	ret_type.type = enum_type2::TYPE_STRUCT_TYPE;
	ret_type.strct = new_strct;

	//ncall->l->t->str = templ_name;

	lang_stat->something_was_declared = true;
	new_strct->flags &= ~TP_STRCT_STRUCT_NOT_NODE;
	return true;
}
bool InstantiateArFromType(lang_state *lang_stat, type2& ar_type, scope* scp, type2* ret_type, type_struct2** ret_struct, node* n)
{
	type2 dummy_tp;
	auto ar_strct = FindIdentifier("array", scp, &dummy_tp);

	if (!ar_strct)
	{
		ReportMessage(lang_stat, n->t, "'array' not found. Try imporintg std.lng to your scope");
		ExitProcess(1);
	}

	// returning if the the ar_type is template with no type asssocieted with
	if (ar_type.type == enum_type2::TYPE_TEMPLATE && !ar_type.tp)
	{
		ret_type->type = enum_type2::TYPE_STRUCT_TYPE;
		ret_type->strct = ar_strct->type.strct;
		return true;
	}


	own_std::vector<type2> types;
	types.emplace_back(ar_type);
	own_std::string sname = FuncNameWithTempls(lang_stat, "array", &types);

	own_std::vector<comma_ret> dummy_ar;

	if (!TryInstantiateStruct(lang_stat, ar_strct->type.strct, sname, scp, ret_struct, dummy_ar, &ar_type))
	{
		return false;
	}
	return true;
}
type2* NewType(lang_state *lang_stat, type2* from)
{
	auto ret = (type2*)AllocMiscData(lang_stat, sizeof(type2));
	memcpy(ret, from, sizeof(type2));
	return ret;
}
bool FromStaticArToAr(lang_state *lang_stat, type2* ret_type, scope* scp, node* n)
{
	if (n->ar_lit_tp)
	{
		*ret_type = *n->ar_lit_tp;
		return true;
	}
	type_struct2* ret_struct;


	auto last_tp = ret_type->tp->type;
	ret_type->tp->type = FromVarTypeToType(last_tp);

	if (!InstantiateArFromType(lang_stat, *ret_type->tp, scp, ret_type, &ret_struct, n))
	{
		ret_type->tp->type = last_tp;
		return false;
	}
	ret_type->tp->type = last_tp;


	ret_type->type = enum_type2::TYPE_STRUCT;
	ret_type->strct = ret_struct;
	ret_type->ptr++;

	n->ar_lit_tp = NewType(lang_stat, ret_type);
	return true;
}
node* GetLastStmntNode(lang_state* lang_stat, node* n)
{
	if (n->type != N_STMNT)
	{
		return n;
	}
	else if (n->r)
	{
		return GetLastStmntNode(lang_stat, n->r);
	}
	else if (n->l)
	{
		return GetLastStmntNode(lang_stat, n->l);
	}

	ASSERT(0);
	return nullptr;


}
node *GetLastStmntType(lang_state *lang_stat, node* n, scope* scp, type2& ret_type)
{
	node* last_stmnt = GetLastStmntNode(lang_stat, n);
	if(last_stmnt->type != N_EMPTY)
		NameFindingGetType(lang_stat, last_stmnt, scp, ret_type);

	return last_stmnt;
}
void GetIfExprType(lang_state *lang_stat, node* n, scope* scp, type2& ret_type, int only_type)
{
	scp = n->l->r->scp;
	auto last_st = GetLastStmntType(lang_stat, n->l->r->r, scp, ret_type);
	node* cur = n->r;
	type2 aux_type;
	if (only_type || !cur || IS_FLAG_OFF(last_st->flags, NODE_FLAGS_STMNT_WITHOUT_SEMICOLON))
		return;
	
	while (cur->type == N_ELSE || cur->type == N_ELSE_IF)
	{
		if (cur->type == N_ELSE)
		{
			GetLastStmntType(lang_stat, cur->r->r, scp, aux_type);
			break;
		}
		else
			GetLastStmntType(lang_stat, cur->l->r->r, scp, aux_type);

		if (!CompareTypes(&ret_type, &aux_type))
			ASSERT(0);
		cur = cur->r;
	}
}

// $NameFindingGetType $NameType
#define NM_FND_TP_RETURN_EVEN_IDENT_NOT_DONE 1
bool NameFindingGetType(lang_state *lang_stat, node* n, scope* scp, type2& ret_type, int flags)
{
	char msg_hdr[256];
	switch (n->type)
	{
	case node_type::N_WHILE:
	{
		ret_type.type = TYPE_VOID;
	}break;
	case node_type::N_FLOAT:
	{
		ret_type.type = TYPE_F32_RAW;
	}break;
	case node_type::N_KEYWORD:
	{
		switch (n->kw)
		{
		case KW_FALSE:
		case KW_TRUE: ret_type.type = TYPE_BOOL; break;
		case KW_REL:
		{
			node* rnode = n;
			if (rnode->l)
			{
				if (!GetTypeFromTkns(rnode->l->t, ret_type))
					ASSERT(false)

					ret_type.rel_lhs = ret_type.type;
			}
			// default type
			else
			{
				ret_type.rel_lhs = enum_type2::TYPE_S64;
			}
			type2* tp = nullptr;
			if (rnode->r->ptr_tp == nullptr)
			{
				tp = (type2*)AllocMiscData(lang_stat, sizeof(type2));
				memset(tp, 0, sizeof(type2));
			}
			else
				tp = rnode->r->ptr_tp;


			if (!NameFindingGetType(lang_stat, rnode->r, scp, *tp))
				return false;

			ret_type.rel_rhs = tp;
			ret_type.flags |= TYPE_IS_REL;
			ret_type.type = enum_type2::TYPE_REL_TYPE;
		}break;
		}
	}break;
	case node_type::N_STR_LIT:
	{
		ret_type.type = enum_type2::TYPE_STR_LIT;
		ret_type.str_len = n->t->str.size();
		/*
		ret_type.type = enum_type2::TYPE_ARRAY;
		ret_type.tp   = new type2();
		ret_type.ptr  = 1;
		//ret_type.str_lit  = n->t->str;
		//
		*/
	}break;
	case node_type::N_INDEX:
	{

		auto rhs = DescendNameFinding(lang_stat, n->r, scp);;

		if (!rhs)
			return false;
		// declaring an array
		if (IS_FLAG_ON(n->flags, NODE_FLAGS_INDEX_IS_TYPE))
		{
			// return the already gotten type

			if (n->ar_lit_tp)
			{
				ret_type = *n->ar_lit_tp;
				return true;
			}
			else
			{
				type2 ar_type;
				type2* ar_type_ptr = nullptr;
				if (!NameFindingGetType(lang_stat, n->r, scp, ar_type))
					return false;

				if (ar_type.type == enum_type2::TYPE_TEMPLATE)
				{
					ret_type.type = enum_type2::TYPE_TEMPLATE;
					return true;
				}

				// array is static if it has a number
				if (n->l && !n->ar_lit_tp)
				{
					ar_type.type = FromTypeToVarType(ar_type.type);

					int ar_size = GetExpressionVal(n->l, scp);

					ar_type_ptr = (type2*)AllocMiscData(lang_stat, sizeof(type2));
					memset(ar_type_ptr, 0, sizeof(type2));


					ar_type_ptr->type = enum_type2::TYPE_STATIC_ARRAY_TYPE;
					ar_type_ptr->ar_size = ar_size;
					ar_type_ptr->tp = (type2*)AllocMiscData(lang_stat, sizeof(type2));
					memset(ar_type_ptr->tp, 0, sizeof(type2));
					*ar_type_ptr->tp = ar_type;

				}
				else
				{
					type_struct2* ret_struct;

					if (!InstantiateArFromType(lang_stat, ar_type, scp, &ret_type, &ret_struct, n))
						return false;

					ar_type_ptr = (type2*)AllocMiscData(lang_stat, sizeof(type2));
					memset(ar_type_ptr, 0, sizeof(type2));

					ar_type_ptr->type = enum_type2::TYPE_STRUCT_TYPE;
					ar_type_ptr->strct = ret_struct;
				}
				ASSERT(ar_type_ptr)

					n->ar_lit_tp = ar_type_ptr;
				ret_type = *ar_type_ptr;
			}
		}
		// getting the dereferenced array type
		else
		{
			type2 lhs_type;
			if (!NameFindingGetType(lang_stat, n->l, scp, lhs_type))
				return false;

			if (!DescendNameFinding(lang_stat, n->r, scp))
				return false;

			switch (lhs_type.type)
			{
			case enum_type2::TYPE_STATIC_ARRAY:
			{
				ret_type = *lhs_type.tp;
				ret_type.ptr++;
			}break;
			case enum_type2::TYPE_STRUCT:
			{
				auto index_op = lhs_type.strct->FindOpOverload(lang_stat, overload_op::INDEX_OP);
				if (!index_op && IS_PRS_FLAG_ON(PSR_FLAGS_REPORT_UNDECLARED_IDENTS))
				{
					REPORT_ERROR(n->t->line, n->t->line_offset,
						VAR_ARGS("struct '%s' don't operator '[]' overload",
							lhs_type.strct->name.c_str()
						)
					);
					ExitProcess(1);
				}
				if (!index_op)
					return false;

				DescendNode(lang_stat, n, scp);

				ret_type = index_op->ret_type;
				//ret_type.ptr++;
			}break;
			default:
				ASSERT(false)
			}
			/*
			ASSERT(lhs->type.tp != nullptr)
			ret_type = *lhs->type.tp;
			ret_type.ptr++;
			*/
			return true;
		}
	}break;
	case node_type::N_CONST:
	{
		ASSERT(NameFindingGetType(lang_stat, n->r, scp, ret_type))
			ret_type.type = TYPE_INT;
		ret_type.is_const = true;
	}break;
	case node_type::N_APOSTROPHE:
	{
		ret_type.type = enum_type2::TYPE_CHAR;
	}break;
	case node_type::N_UNOP:
	{
		switch (n->t->type)
		{
		case tkn_type2::T_DOLLAR:
		{
			ASSERT(IS_FLAG_ON(n->r->flags, NODE_FLAGS_IS_PROCESSED2))

				ret_type = *n->ar_lit_tp;

		}break;
		case tkn_type2::T_AMPERSAND:
		{
			if (!DescendNameFinding(lang_stat, n->r, scp))
				return false;

			if (!NameFindingGetType(lang_stat, n->r, scp, ret_type))
				return false;


			if (IsNodeUnop(n->r, T_MUL))
				memcpy(n, n->r->r, sizeof(node));
			// index already returns a pointer so we removing the "taking address of"
			else if (CMP_NTYPE(n->r, N_INDEX))
				memcpy(n, n->r, sizeof(node));
			else
			{
				switch (ret_type.type)
				{
					/*
				{
					//ReportMessage(lang_stat, n->t, "Cannot take address of static arrays. Try just indexing it, use the plain variable.");

				}break;
				*/
				case enum_type2::TYPE_FUNC:
				{
					ret_type.type = TYPE_FUNC_PTR;
					return true;
				}break;
				case enum_type2::TYPE_S64:
				case enum_type2::TYPE_S32:
				case enum_type2::TYPE_S16:
				case enum_type2::TYPE_S8:
				case enum_type2::TYPE_U64:
				case enum_type2::TYPE_U32:
				case enum_type2::TYPE_U16:
				case enum_type2::TYPE_U8:
				case enum_type2::TYPE_BOOL:
				case enum_type2::TYPE_F32:
				case enum_type2::TYPE_F64:
				case enum_type2::TYPE_STRUCT:
				case enum_type2::TYPE_CHAR:
				case enum_type2::TYPE_ENUM:
				case enum_type2::TYPE_ARRAY:
				case enum_type2::TYPE_STATIC_ARRAY:
					break;
				case enum_type2::TYPE_STRUCT_TYPE:
				{
					ReportMessage(lang_stat, n->t, "Can't take address of struct types");
				};
				default:
				{
					ASSERT(false)
				}break;
				}
			}

			ret_type.ptr++;

		}break;
		case tkn_type2::T_POINT:
		{
			ASSERT(n->r->type == node_type::N_IDENTIFIER);

			type2 ret_type;
			auto ident = FindIdentifier(n->r->t->str, scp, &ret_type);

			if (!ident)
			{
				if (IS_PRS_FLAG_ON(PSR_FLAGS_REPORT_UNDECLARED_IDENTS))
					ReportUndeclaredIdentifier(lang_stat, n->t);
				return false;
			}
			if (!ident) return false;
			ASSERT(ident->type.type == TYPE_ENUM_IDX_32)

				return true;
		}break;
		case tkn_type2::T_MUL:
		{
			if (!NameFindingGetType(lang_stat, n->r, scp, ret_type, flags))
				return false;


			bool should_deref = false;

			switch (ret_type.type)
			{
			case enum_type2::TYPE_INT:
			{
				REPORT_ERROR(n->t->line, n->t->line_offset, VAR_ARGS("Can't make type out a ptr number, try u32, s32 or something like that"));
			}break;
			case enum_type2::TYPE_FUNC_DECL:
			case enum_type2::TYPE_STRUCT_DECL:
				ASSERT(false)
					break;

			case enum_type2::TYPE_S64_TYPE:
			case enum_type2::TYPE_S32_TYPE:
			case enum_type2::TYPE_S16_TYPE:
			case enum_type2::TYPE_S8_TYPE:
			case enum_type2::TYPE_U64_TYPE:
			case enum_type2::TYPE_U32_TYPE:
			case enum_type2::TYPE_U16_TYPE:
			case enum_type2::TYPE_U8_TYPE:
			case enum_type2::TYPE_BOOL_TYPE:
			case enum_type2::TYPE_STRUCT_TYPE:
			case enum_type2::TYPE_FUNC_TYPE:
			case enum_type2::TYPE_ENUM_TYPE:
			case enum_type2::TYPE_F32_TYPE:
			case enum_type2::TYPE_F64_TYPE:
			case enum_type2::TYPE_VOID_TYPE:
			case enum_type2::TYPE_CHAR_TYPE:
				ret_type.ptr++;
				break;
			case enum_type2::TYPE_S64:
			case enum_type2::TYPE_S32:
			case enum_type2::TYPE_S16:
			case enum_type2::TYPE_S8:
			case enum_type2::TYPE_U64:
			case enum_type2::TYPE_U32:
			case enum_type2::TYPE_U16:
			case enum_type2::TYPE_U8:
			case enum_type2::TYPE_BOOL:
			case enum_type2::TYPE_STRUCT:
			case enum_type2::TYPE_F32:
			case enum_type2::TYPE_F64:
			case enum_type2::TYPE_VOID:
			case enum_type2::TYPE_CHAR:
				ret_type.ptr--;
				break;
			case enum_type2::TYPE_TEMPLATE:
				break;
			default:
				ASSERT(false)
			}
		}break;
		default:
		{

			if (!NameFindingGetType(lang_stat, n->r, scp, ret_type))
				return false;

		}break;
		}
	}break;
	case node_type::N_FUNC_DECL:
	{
		auto a = 0;

		if (!FunctionIsDone(lang_stat, n, scp, &ret_type, DONT_DESCEND_SCOPE))
			return false;
		ret_type.type = TYPE_FUNC_PTR;

	}break;
	case node_type::N_TYPE:
	{

		ret_type = n->decl_type;
	}break;
	case node_type::N_CALL:
	{
		if (!CallNode(lang_stat, n, scp, &ret_type))
			return false;
	}break;
	case node_type::N_IDENTIFIER:
	{
		int f = (flags & NM_FND_TP_RETURN_EVEN_IDENT_NOT_DONE) * FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE;
		auto ident = FindIdentifier(n->t->str, scp, &ret_type, f);
		if (ident == nullptr)
		{
			if (IS_PRS_FLAG_ON(PSR_FLAGS_REPORT_UNDECLARED_IDENTS))
				ReportUndeclaredIdentifier(lang_stat, n->t);
			return false;
		}

		ret_type.e_decl = ident;
	}break;
	case node_type::N_INT:
	{
		ret_type.type = enum_type2::TYPE_S32;
		return true;
	}break;
	case node_type::N_ARRAY_CONSTRUCTION:
	{
		if (!NameFindingGetType(lang_stat, n->l, scp, ret_type))
			return false;
	}break;
	case node_type::N_STMNT:
	{
		if (n->r && NameFindingGetType(lang_stat, n->r, scp, ret_type))
			return true;;
		if (n->l && NameFindingGetType(lang_stat, n->l, scp, ret_type))
			return true;;
		return false;
	}break;
	case node_type::N_SCOPE:
	{
		if (!NameFindingGetType(lang_stat, n->r, scp, ret_type))
			return false;
	}break;
	case node_type::N_STRUCT_CONSTRUCTION:
	{
		if (!NameFindingGetType(lang_stat, n->l, scp, ret_type))
			return false;
		if (ret_type.type == TYPE_STRUCT_TYPE)
			ret_type.type = TYPE_STRUCT;
	}break;
	case node_type::N_CAST:
	{

		if (!NameFindingGetType(lang_stat, n->l, scp, ret_type))
			return false;
		if (!DescendNameFinding(lang_stat, n->l, scp))
			return false;

		ret_type.type = FromTypeToVarType(ret_type.type);
	}break;
	case node_type::N_FUNC_DEF:
	{
		auto a = 0;

		if (!FunctionIsDone(lang_stat, n, scp, &ret_type, DONT_DESCEND_SCOPE))
			return false;
		ret_type.type = TYPE_FUNC_DEF;

	}break;
	case node_type::N_BINOP:
	{
		switch (n->t->type)
		{
		case tkn_type2::T_POINT:
		{
			if (!PointLogic(lang_stat, n, scp, &ret_type))
				return false;
		}break;
		case T_COND_NE:
		case T_COND_EQ:
		case T_GREATER_EQ:
		case T_GREATER_THAN:
		case T_LESSER_EQ:
		case T_LESSER_THAN:
		{
			ret_type.type = TYPE_BOOL;
		}break;
		default:
		{
			if (!NameFindingGetType(lang_stat, n->l, scp, ret_type))
				return false;
		}break;
		}
	}break;
	case N_IF:
	{
		GetIfExprType(lang_stat, n, scp, ret_type, false);
	}break;
	default:
		ASSERT(false)
	}
	return true;
}

bool DescendArgsOfModifiedFunc(lang_state *lang_stat, func_decl *f)
{
	// getting the func decl args
	int cur_scp_vars_size = f->scp->vars.size();
	if (!DescendNameFinding(lang_stat, f->func_node->l->l->r, f->scp))
		return false;

	for (int i = cur_scp_vars_size; i < f->scp->vars.size(); i++)
	{
		decl2* d = f->scp->vars[i];
		d->flags |= DECL_IS_ARG;
	}

	return true;
}
// this adds a function to the parent scope of the found function scope
bool AddNewTemplFuncFromLangArrayTemplTypesToScope(lang_state *lang_stat, own_std::string original_name, scope* scp, own_std::vector<type2>* final_types, func_decl** fdecl_out = nullptr)
{
	type2 ret_type;
	// instantiating a new it_next func
	auto it_next = FindIdentifier(original_name, scp, &ret_type);
	if (!it_next)
		return false;

	own_std::string fname = FuncNameWithTempls(lang_stat, original_name, final_types);
	auto found_decl = FindIdentifier(fname, scp, &ret_type, FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE);
	func_decl* fdecl;
	if (!found_decl)
	{
		fdecl = it_next->type.fdecl->new_func();
		fdecl->func_node = it_next->type.fdecl->func_node->NewTree(lang_stat);

		// @test fdecl->scp = NewScope(it_next->type.fdecl->scp->parent);
		fdecl->scp = NewScope(lang_stat, scp);
		//

		fdecl->scp->flags = SCOPE_INSIDE_FUNCTION;
		fdecl->scp->fdecl = fdecl;
		type2 tp;
		tp.type = enum_type2::TYPE_FUNC;
		tp.fdecl = fdecl;
		decl2* new_decl = NewDecl(lang_stat, fname, tp);
		new_decl->flags = DECL_NOT_DONE;

		// @test it_next->type.fdecl->scp->parent->vars.emplace_back(new_decl);
		lang_stat->root->vars.emplace_back(new_decl);
		lang_stat->funcs_scp->vars.emplace_back(new_decl);
		//


		found_decl = new_decl;
		fdecl->name = fname.substr();

	}
	else
	{
		if (IS_FLAG_OFF(found_decl->flags, DECL_NOT_DONE))
		{
			*fdecl_out = found_decl->type.fdecl;
			return true;
		}
		fdecl = found_decl->type.fdecl;
	}


	ASSERT(final_types->size() == fdecl->templates.size())

		own_std::vector<decl2*> templates_types;

	int i = 0;

	if (IS_FLAG_OFF(fdecl->flags, FUNC_DECL_TEMPLATES_DECLARED_TO_SCOPE))
	{
		FOR_VEC(t, fdecl->templates)
		{
			templates_types.emplace_back(NewDecl(lang_stat, t->name, (*final_types)[i]));
			i++;
		}
		fdecl->scp->vars.assign(templates_types.begin(), templates_types.end());
		fdecl->flags |= FUNC_DECL_TEMPLATES_DECLARED_TO_SCOPE;
	}
	// getting the func ret_type
	if (!CheckFuncRetType(lang_stat, fdecl, fdecl->scp))
		return false;


	if (!DescendArgsOfModifiedFunc(lang_stat, fdecl))
		return false;

	// adding scope vars to args
	if (fdecl->args.size() == 0)
		fdecl->args.assign(fdecl->scp->vars.begin() + fdecl->templates.size(), fdecl->scp->vars.end());

	// getting the scope vars
	//DescendNode(fdecl->func_node->r, fdecl->scp);
	if (fdecl->func_node->r && !DescendNameFinding(lang_stat, fdecl->func_node->r->r, fdecl->scp))
		return false;

	found_decl->flags &= ~DECL_NOT_DONE;
	if (fdecl->func_node->r)
		DescendNode(lang_stat, fdecl->func_node->r->r, fdecl->scp);


	if (fdecl_out)
		*fdecl_out = fdecl;
	fdecl->flags |= FUNC_DECL_IS_DONE;
	type2 tp;
	tp.type = TYPE_FUNC;
	tp.fdecl = fdecl;
	fdecl->templates.clear();
	fdecl->this_decl = NewDecl(lang_stat, fdecl->name, tp);
	//ASSERT(!IsThereAFunction((char *)fdecl->name.c_str()));
	lang_stat->cur_file->global->vars.emplace_back(fdecl->this_decl);
	return true;
}
bool InstantiateTemplateFunction(lang_state *lang_stat, func_decl* fdecl, own_std::vector<comma_ret>* args)
{
	scope* target_scope = fdecl->templates[0].scp;
	auto templates_types = GetTemplateTypes(lang_stat, &fdecl->templates, args, target_scope, fdecl);
	if (IS_FLAG_OFF(fdecl->flags, FUNC_DECL_TEMPLATES_DECLARED_TO_SCOPE))
	{
		fdecl->scp->vars.assign(templates_types.begin(), templates_types.end());
		fdecl->flags |= FUNC_DECL_TEMPLATES_DECLARED_TO_SCOPE;
	}

	// getting the func ret_type
	auto ret_type = DescendNameFinding(lang_stat, fdecl->func_node->l->r, fdecl->scp);
	if (!ret_type)
		return false;

	// getting the func decl args
	//DescendNode(fdecl->func_node->l->l->r, fdecl->scp);
	if(!DescendArgsOfModifiedFunc(lang_stat, fdecl))
	//if (!DescendNameFinding(lang_stat, fdecl->func_node->l->l->r, fdecl->scp))
		return false;


	// getting the scope vars
	//DescendNode(fdecl->func_node->r, fdecl->scp);
	if (!DescendNameFinding(lang_stat, fdecl->func_node->r, fdecl->scp))
		return false;

	DescendNode(lang_stat, fdecl->func_node->r, fdecl->scp);
	return true;
}
void ModifyCommaRetTypes(own_std::vector<comma_ret>* ar, scope* scp)
{
	// getting type of args
	FOR_VEC(t, *ar)
	{
		switch (t->type)
		{
		case COMMA_RET_IDENT:
		{
			ASSERT(FindIdentifier(t->decl.name, scp, &t->decl.type))
		}break;
		//case COMMA_RET_TYPE:
		case COMMA_RET_EXPR:
			//t->decl.type = t->tp;
			break;
		}
	}
}
void ReportMessageOne(lang_state *lang_stat, token2* tkn, char* msg, void* data)
{
	char msg_hdr[256];
	char final_msg[256];
	snprintf(final_msg, 256, msg, data);

	REPORT_ERROR(tkn->line, tkn->line_offset, VAR_ARGS(final_msg));
	ExitProcess(1);
}
void ReportMessage(lang_state *lang_stat, token2* tkn, char* msg)
{
	char msg_hdr[256];
	REPORT_ERROR(tkn->line, tkn->line_offset, VAR_ARGS(msg));
	ExitProcess(1);
}
void ReportTypeMismatch(lang_state *lang_stat, token2* tkn, type2* lhs, type2* rhs)
{
	char msg_hdr[256];
	REPORT_ERROR(tkn->line, tkn->line_offset, VAR_ARGS("type mismatch. Expected %s, received %s\n\n",
		TypeToString(*lhs).c_str(), TypeToString(*rhs).c_str()))
		ExitProcess(1);
}
void BuildMacroTree(lang_state *lang_stat, scope* scp, node* n, unsigned int start_line)
{
	if (n->t)
		n->t->line = start_line;
	switch (n->type)
	{
	case N_IDENTIFIER:
	{
		if (n->t->str == "__CALL_SITE_SRC_FILE")
		{
			n->t->str.assign(lang_stat->cur_file->name);
			n->type = node_type::N_STR_LIT;
		}
		else if (n->t->str == "__LINE__")
		{
			n->t->i = start_line;
			n->type = node_type::N_INT;
		}
		else if (n->t->str == "__CALL_SITE_LN_NUM")
		{
			n->t->i = start_line - 1;
			n->type = node_type::N_INT;
		}
		else if (n->t->str == "__CALL_SITE_SRC_LINE")
		{
			char* line = lang_stat->cur_file->lines[start_line - 1];
			auto len = strlen(line);
			n->t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
			n->t->str = line;
			//n->t->str  = line;
			n->type = node_type::N_STR_LIT;
		}
		else
		{
			type2 ret_tp;
			auto decl = FindIdentifier(n->t->str, scp, &ret_tp);
			if (decl)
			{
				switch (decl->type.type)
				{
				case TYPE_MACRO_EXPR:
				{
					memcpy(n, decl->type.nd, sizeof(node));
				}break;
				}
			}
		}

	}break;
	default:
		if (n->l)
			BuildMacroTree(lang_stat, scp, n->l, start_line);
		if (n->r)
			BuildMacroTree(lang_stat, scp, n->r, start_line);
	}
}
node* CreateNewVarArgCall(lang_state *lang_stat, node* val, node* expr, int ptr, node* ncall)
{
	own_std::vector<node*> var_args_params;
	// call to new_var_arg
	// arg 1
	// casting to u64
	auto u64_nd = CreateNodeFromType(lang_stat, &lang_stat->u64_decl->type, ncall->l->t);
	auto casted = NewTypeNode(lang_stat, u64_nd, N_CAST, new_node(lang_stat, val), ncall->t);
	casted->t = ncall->t;
	var_args_params.emplace_back(casted);

	// arg 2
	var_args_params.emplace_back(NewIntNode(lang_stat, ptr, val->t));

	// arg 3
	auto tp_data_call = NewTypeNode(lang_stat, NewIdentNode(lang_stat, "get_type_data", ncall->t),
		N_CALL,
		new_node(lang_stat, expr), ncall->t);
	tp_data_call->t = ncall->t;

	var_args_params.emplace_back(tp_data_call);

	// creating the final func call
	return MakeFuncCallArgs(lang_stat, "new_var_arg", nullptr, var_args_params, ncall->l->t);
}

decl2 *CreateStructTuple(lang_state* lang_stat, node* n, scope* scp)
{

	type2 dummy;
	own_std::vector<comma_ret> commas;

	auto prev_flags = n->flags;
	n->flags &= ~NODE_FLAGS_COMMA_INSIDE_PARENTHESES;
	DescendComma(lang_stat, n, scp, commas);
	n->flags = prev_flags;

	own_std::string tuple_name = "tuple_";

	bool all_builtin = true;
	FOR_VEC(c, commas)
	{
		type2 tp;
		NameFindingGetType(lang_stat, c->n, scp, tp);
		tuple_name += TypeToString(tp) + "_";
		if (c->decl.type.type == TYPE_STRUCT_TYPE)
			all_builtin = false;
	}
	tuple_name.pop_back();

	decl2* tuple_exist = nullptr;
	if (all_builtin)
		tuple_exist = FindIdentifier(tuple_name, lang_stat->root, &dummy);
	else
		tuple_exist = FindIdentifier(tuple_name, scp, &dummy);


	decl2* ret = nullptr;
	if (!tuple_exist)
	{
		//memset(new_decl, 0, sizeof(decl2));
		type_struct2* tstrct = (type_struct2*)AllocMiscData(lang_stat, sizeof(type_struct2));
		memset(tstrct, 0, sizeof(type_struct2));

		n->type = N_SCOPE;
		scope* child_scp = GetScopeFromParent(lang_stat, n, scp);
		n->type = N_BINOP;
		n->scp = nullptr;
		tstrct->name = own_std::string(tuple_name);

		child_scp->flags = SCOPE_INSIDE_STRUCT;
		int i = 0;
		FOR_VEC(c, commas)
		{
			decl2* d = DeclareDeclToScopeAndMaybeToFunc(lang_stat, "", &c->decl.type, child_scp, n);
			if (c->type == COMMA_RET_COLON)
			{
			}
			else
			{
				d->name = own_std::string("t") + std::to_string(i).c_str();
			}

			i++;
		}
		child_scp->tstrct = tstrct;
		//ret_type.strct = tstrct;

		scope* dst_scp = scp;
		if (all_builtin)
			dst_scp = lang_stat->root;
		decl2* tuple_decl = DeclareDeclToScopeAndMaybeToFunc(lang_stat, tuple_name, &dummy, dst_scp, n);

		//ret_type.type = enum_type2::TYPE_STRUCT_TYPE;

		//ret_type.strct->this_decl = tuple_decl;
		tstrct->strct_node = n;
		tstrct->scp = child_scp;
		tstrct->flags |= TP_STRCT_TUPLE;
		tstrct->this_decl = tuple_decl;
		tuple_decl->type.strct = tstrct;
		tuple_decl->type.type = TYPE_STRUCT_TYPE;
		ret = tuple_decl;

		tstrct->size = SetVariablesAddress(&tstrct->scp->vars, 0, &tstrct->biggest_type);
	}
	else
		ret = tuple_exist;
	return ret;
}
//$CallNode
bool CallNode(lang_state *lang_stat, node* ncall, scope* scp, type2* ret_type, decl2* decl_func)
{
	char msg_hdr[256];
	decl2* lhs;

	if (ncall->r && !DescendNameFinding(lang_stat, ncall->r, scp))
		return false;

	//	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
	//		int last_ar_lit_sz = scp->fdecl->array_literal_sz;

	if (!decl_func)
	{
		if (ncall->l->type == N_FUNC_DECL)
		{
			if (!DescendNameFinding(lang_stat, ncall->l, scp))
				return false;
			ModifyFuncDeclToName(lang_stat, ncall->l->fdecl, ncall->l, scp);
		}
		lhs = DescendNameFinding(lang_stat, ncall->l, scp);
		if (!lhs)
			return false;

		if (ncall->r && IS_FLAG_OFF(lhs->type.fdecl->flags, FUNC_DECL_MACRO) && !DescendNameFinding(lang_stat, ncall->r, scp))
		{
			//	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
			//		int last_ar_lit_sz = scp->fdecl->array_literal_sz;
			return false;
		}
	}
	else
		lhs = decl_func;

	//if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
//		int last_ar_lit_sz = scp->fdecl->array_literal_sz;

	own_std::vector<comma_ret> args;
	if (ncall->r)
		DescendComma(lang_stat, ncall->r, scp, args);

	// struct instantiation
	switch (lhs->type.type)
	{
	case enum_type2::TYPE_OVERLOADED_FUNCS:
	case enum_type2::TYPE_FUNC_EXTERN:
	case enum_type2::TYPE_FUNC_PTR:
	case enum_type2::TYPE_FUNC:
	{
		func_decl* fdecl = lhs->type.fdecl;

		if (IS_FLAG_ON(ncall->flags, NODE_FLAGS_IS_PROCESSED2))
		{
			*ret_type = fdecl->ret_type;

			return true;
		}

		bool is_var_args = IS_FLAG_ON(lhs->type.fdecl->flags, FUNC_DECL_VAR_ARGS);
		if (!is_var_args)
		{
			int required_args_count = lhs->type.fdecl->args.size();
			if (required_args_count != args.size())
			{
				auto fdecl = lhs->type.fdecl;
				int func_ln = fdecl->func_node->t->line - 1;
				auto from_fl = fdecl->from_file;

				ASSERT(from_fl)

					REPORT_ERROR(ncall->t->line, ncall->t->line_offset,
						VAR_ARGS("function call's  number of arguments dont match. Expected %d, got %d\n\nHere's the func definition: \n\n%s\n\nHere's the call:\n",
							required_args_count, (int)args.size(), GetFileLn(lang_stat, func_ln, from_fl)
						)
					)
					ExitProcess(1);
			}
		}

		if (lhs->name == "get_type_data")
		{
			lhs->type.fdecl->flags |= FUNC_DECL_INTERNAL;
			*ret_type = fdecl->ret_type;
		}
		else if (lhs->name == "typeof")
		{
			lhs->type.fdecl->flags |= FUNC_DECL_INTERNAL;
			NameFindingGetType(lang_stat, ncall->r, scp, *ret_type);
			auto new_call = CreateNewVarArgCall(lang_stat, NewIntNode(lang_stat, 0, ncall->l->t), ncall->r, ret_type->ptr, ncall);

			auto t = ncall->t;

			memcpy(ncall, new_call, sizeof(node));

			ncall->t = t;

			if (!DescendNameFinding(lang_stat, ncall, scp))
				return false;

			*ret_type = DescendNode(lang_stat, ncall, scp);
		}
		else if (lhs->name == "enum_count")
		{
			int a = 0;
			ret_type->type = enum_type2::TYPE_INT;
			lhs->type.fdecl->flags |= FUNC_DECL_INTERNAL;
			//lhs->fe

			// getting type of args
			FOR_VEC(t, args)
			{
				if (t->type == COMMA_RET_IDENT)
				{
					ASSERT(FindIdentifier(t->decl.name, scp, &t->decl.type))
				}
				else if (t->type == COMMA_RET_EXPR)
					ASSERT(false);
			}

			if (!(args[0].decl.type.type != TYPE_ENUM))
			{
				ReportMessage(lang_stat, ncall->r->t, "enum_count only accepts enum as args");
				ExitProcess(1);
			}

		}
		else if (lhs->name == "sizeof")
		{
			int a = 0;
			ret_type->type = enum_type2::TYPE_INT;
			lhs->type.fdecl->flags |= FUNC_DECL_INTERNAL;
		}
		else if (IS_FLAG_ON(lhs->type.fdecl->flags, FUNC_DECL_MACRO))
		{
			int cur_arg = 0;
			FOR_VEC(a, args)
			{
				fdecl->args[cur_arg]->type.nd = a->n;
				cur_arg++;
			}

			auto new_tree = fdecl->func_node->r->r->NewTree(lang_stat);

			auto last_tkn = ncall->t;
			BuildMacroTree(lang_stat, fdecl->scp, new_tree, ncall->t->line);
			memcpy(ncall, new_tree, sizeof(node));

			ncall->flags = 0;
			if (!DescendNameFinding(lang_stat, ncall, scp))
				return false;
			return true;
		}
		else
		{
			// getting type of args
			FOR_VEC(t, args)
			{
				if (t->type == COMMA_RET_IDENT)
				{
					ASSERT(FindIdentifier(t->decl.name, scp, &t->decl.type))
				}
				// a tuple
				else if (t->type == COMMA_RET_EXPR && IS_COMMA(t->n))
				{
					decl2 *tuple_decl = CreateStructTuple(lang_stat, t->n, scp);
					node* struct_constr = new_node(lang_stat, t->n->t);

					struct_constr->type = N_STRUCT_CONSTRUCTION;
					struct_constr->l = NewIdentNode(lang_stat, tuple_decl->name, t->n->t);
					struct_constr->r = new_node(lang_stat, t->n);
					struct_constr->r->flags = 0;
					DescendNameFinding(lang_stat, struct_constr, scp);

					node *bin = NewTypeNode(lang_stat, nullptr, N_UNOP, struct_constr, ncall->t);
					bin->t->type = T_AMPERSAND;
					memcpy(t->n, bin, sizeof(node));

					t->decl.type = DescendNode(lang_stat, t->n, scp);
					//t->decl.type.type = FromTypeToVarType(t->decl.type.type);

				}
				/*
				else if (t->type == COMMA_RET_EXPR)
					t->decl.type = t->;
				*/
			}
			// choosing overload funcs
			if (lhs->type.type == enum_type2::TYPE_OVERLOADED_FUNCS)
			{
				own_std::vector<type2> args_types;
				FOR_VEC(t, args)
				{
					args_types.emplace_back(t->decl.type);
				}

				auto gotten_func = lhs->type.ChooseFuncOverload(lang_stat, &args_types);

				if (!gotten_func)
					return false;

				fdecl = gotten_func;
				ncall->r->t->str = fdecl->name.substr();
			}

			// func instantiation
			if (fdecl->templates.size() > 0)
			{
				scope* target_scope = fdecl->templates[0].scp;
				own_std::vector<type2> templ_types;

				if (ncall->call_templates)
				{
					own_std::vector<comma_ret> templ_ar;
					DescendComma(lang_stat, ncall->call_templates, scp, templ_ar);
					ModifyCommaRetTypes(&templ_ar, scp);
					FOR_VEC(t, templ_ar)
					{
						templ_types.emplace_back(t->decl.type);
					}

				}
				// func call has templates specified
				else
				{
					templ_types = TemplatesTypeToLangArray(lang_stat, &fdecl->templates, &lhs->type.fdecl->args, &args);
					// transforming the var types into types
					FOR_VEC(t, templ_types)
					{
						//t->type = FromVarTypeToType(t->type);
					}
				}

				func_decl* new_func;



				auto last_fl = lang_stat->cur_file;
				//lang_stat->cur_file = fdecl->from_file;

				if (!AddNewTemplFuncFromLangArrayTemplTypesToScope(lang_stat, fdecl->name, scp, &templ_types, &new_func))
				{
					//lang_stat->cur_file = last_fl;
					return false;
				}
				//lang_stat->cur_file = last_fl;

				ret_type->type = enum_type2::TYPE_FUNC;
				ret_type->fdecl = new_func;
				fdecl = new_func;
				ncall->l->t->str = new_func->name;
				new_func->templates.clear();
			}
			else
			{
				*ret_type = fdecl->ret_type;
			}

			/*
			// transfering type to tp
			FOR_VEC(t, args)
			{
				if (t->type == COMMA_RET_EXPR)
					t->tp = t->decl.type;
			}
			*/

			int fdecl_arg_idx = 0;

			// getting the size of all args that are strct vals
			if (fdecl->strct_vals_sz > scp->fdecl->call_strcts_val_sz)
				scp->fdecl->call_strcts_val_sz = fdecl->strct_vals_sz;

			// comparing just until we get to var args
			for (int i = 0; i < fdecl->args.size(); i++)
			{
				auto t = &args[i];

				auto f_arg = fdecl->args[fdecl_arg_idx];
				bool comp_val = CompareTypes(&f_arg->type, &t->decl.type, false);

				// create implicit cast
				if (comp_val && f_arg->type.type != TYPE_STRUCT &&
					f_arg->type.type != TYPE_ENUM && f_arg->type.type != t->decl.type.type)
				{
					auto cast_tp_nd = CreateNodeFromType(lang_stat, &f_arg->type, ncall->t);
					auto casted = NewTypeNode(lang_stat, cast_tp_nd, N_CAST, new_node(lang_stat, t->n), ncall->t);
					memcpy(t->n, casted, sizeof(node));
				}

				// checking for constructors sinces types don't match
				if (!comp_val && f_arg->type.type == enum_type2::TYPE_STRUCT && f_arg->type.ptr == 0)
				{
					own_std::vector<type2> tp_ar;
					tp_ar.emplace_back(t->decl.type);

					if (f_arg->type.strct->constructors.size() == 0)
					{
						if(IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_REPORT_UNDECLARED_IDENTS))
							return false;
						else
						{
							REPORT_ERROR(ncall->t->line, ncall->t->line_offset,
								VAR_ARGS("struct '%s' doesn't have any constructors for '%s'", f_arg->type.strct->name.c_str(), TypeToString(t->decl.type).c_str())
							);
							ExitProcess(1);
						}
					}

					auto constr = f_arg->type.strct->FindExistingOverload(lang_stat, &f_arg->type.strct->constructors, (void*)&f_arg->type, &tp_ar, true);
					if (!constr)
						return false;

					auto new_call = NewBinOpNode(lang_stat, 
						NewIdentNode(lang_stat, constr->name, ncall->l->t),
						tkn_type2::T_MUL,
						t->n->NewTree(lang_stat));

					new_call->t = ncall->l->t;
					new_call->type = node_type::N_CALL;

					scp->fdecl->per_stmnt_strct_val_sz += constr->ret_type.strct->size;

					memcpy(t->n, new_call, sizeof(node));

				}
				else
				{
					if (!comp_val)
					{
						REPORT_ERROR(t->n->t->line, t->n->t->line_offset, VAR_ARGS("On call to '%s', argument %d type mismatch. Expected %s, received %s\n\n", fdecl->name.c_str(),
							i + 1, TypeToString(f_arg->type).c_str(), TypeToString(t->decl.type).c_str()))
							ExitProcess(1);
					}
					// ASSERT(comp_val)
				}

				fdecl_arg_idx++;
			}

			auto var_arg_strct = FindIdentifier("var_arg", scp, ret_type);

			if (fdecl_arg_idx < args.size())
				ASSERT(var_arg_strct);

			int var_args_arg_start = fdecl_arg_idx;

			// in var_args
			for (int i = fdecl->args.size(); i < args.size(); i++)
			{
				auto a = &args[i];
				// inserting something like
				// new_var_arg(a, ptr, get_type_data(a))
				char code[128];
				/*

				own_std::vector<node*> var_args_params;
				node* new_nd = new_node(lang_stat, a->n);
				// call to new_var_arg
				// arg 1
				// casting to u64
				auto u64_nd = CreateNodeFromType(lang_stat, &lang_stat->u64_decl->type);
				auto casted = NewTypeNode(lang_stat, u64_nd, N_CAST, new_nd);
				casted->t = ncall->t;
				var_args_params.emplace_back(casted);

				// arg 2
				var_args_params.emplace_back(NewIntNode(lang_stat, a->tp.ptr));

				// arg 3
				auto tp_data_call = NewTypeNode(lang_stat, NewIdentNode(lang_stat, "get_type_data"),
												N_CALL,
												new_nd);

				var_args_params.emplace_back(tp_data_call);

				// creating the final func call
				auto var_args_call_nd = MakeFuncCallArgs(lang_stat, "new_var_arg", nullptr, var_args_params);

				*/
				auto var_args_call_nd = CreateNewVarArgCall(lang_stat, a->n, a->n, a->decl.type.ptr, ncall);

				memcpy(a->n, var_args_call_nd, sizeof(node));

				if (!DescendNameFinding(lang_stat, a->n, scp))
				{
					ASSERT(false);
				}


				//scp->fdecl->call_strcts_val_sz += var_arg_strct->type.strct->size;
				fdecl_arg_idx++;
			}

			if (fdecl->ret_type.type == TYPE_STRUCT && fdecl->ret_type.ptr == 0)
				scp->fdecl->per_stmnt_strct_val_sz += fdecl->ret_type.strct->size;

			// getting the amount of var_args and increasing the size of strct vals
			if (fdecl_arg_idx > 0)
			{
				int total_var_args = fdecl_arg_idx - var_args_arg_start;
				if (scp->fdecl->total_of_var_args < total_var_args)
				{
					int diff = total_var_args - scp->fdecl->total_of_var_args;
					scp->fdecl->call_strcts_val_sz += (diff)*var_arg_strct->type.strct->size;
					scp->fdecl->total_of_var_args = total_var_args;
				}
			}
		}

		ncall->flags |= NODE_FLAGS_IS_PROCESSED2;
		ncall->flags |= NODE_FLAGS_CALL_RET_ANON;
		return true;
	}break;
	case enum_type2::TYPE_STRUCT_TYPE:
	{

		// getting type of args
		FOR_VEC(t, args)
		{
			if (t->type == COMMA_RET_IDENT)
			{
				ASSERT(FindIdentifier(t->decl.name, scp, &t->decl.type));
			}
			/*
			else if (t->type == COMMA_RET_EXPR)
				t->decl.type = t->tp;
				*/


		}

		auto type = DescendNameFinding(lang_stat, ncall->r, scp);

		if (!type)
			return false;

		if (lhs->type.strct->templates.size() != args.size())
		{
			auto strct = lhs->type.strct;
			REPORT_ERROR(ncall->t->line, ncall->t->line_offset,
				VAR_ARGS("struct declaration's number of templates dont match. Expected %d, got %d\n\nHere's the struct declaration definition: \n\n%s\n\nHere's the call:\n",
					(int)strct->templates.size(), (int)args.size(),
					GetFileLn(lang_stat, strct->strct_node->t->line - 1, strct->from_file)
				)
			);
			ExitProcess(1);
		}

		// Breaking if struct template instantiation's FUNCTION PARAMETER
		// is a plain template without any type whitin it
		// Or if it's a builting type (in this case, the value will be simply 1)
		// Because we can't instantiate a tempalte without type
		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT) || (long long)type != 1 && type->type.type == enum_type2::TYPE_TEMPLATE)
		{
			ret_type->type = enum_type2::TYPE_STRUCT_TYPE;
			ret_type->strct = lhs->type.strct;
			ret_type->flags |= TYPE_NOT_INSTANTIATED_YET;
			return true;
		}


		own_std::string templ_name;
		templ_name += ncall->l->t->str;
		templ_name += "_";
		// getting type of args
		FOR_VEC(t, args)
		{
			type2 aux = t->decl.type;
			//enum_type2 tp = 
			aux.type = t->decl.type.type;
			aux.type = FromTypeToVarType(aux.type);
			templ_name += TypeToString(aux);
			templ_name += "_";
		}
		templ_name.pop_back();


		type_struct2* ret_strct;
		if (!TryInstantiateStruct(lang_stat, lhs->type.strct, templ_name, scp, &ret_strct, args))
			return false;

		ASSERT(IS_FLAG_OFF(ret_strct->flags, TP_STRCT_TEMPLATED))

			ret_type->type = enum_type2::TYPE_STRUCT_TYPE;
		ret_type->strct = ret_strct;

		if (IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_DONT_CHANGE_TEMPL_STRCT_ND_NAME))
		{
			/*
			ncall->l->FreeTree();
			ncall->r->FreeTree();
			ncall->l = nullptr;
			ncall->r = nullptr;
			*/

			ncall->type = node_type::N_IDENTIFIER;
			if (!ncall->t)
			{
				ncall->t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
				memset(ncall->t, 0, sizeof(token2));
			}


			ncall->t->str = templ_name.substr();
		}

	}break;
	default:
		ReportMessage(lang_stat, ncall->t, "Cannot call a function definition");
		// not handled call type
		ASSERT(false)
	}
	return true;
}

void CheckStructValToFunc(func_decl* fdecl, type2* type)
{
	// making struct values to be ptrs
	if (type->type == enum_type2::TYPE_STRUCT && type->ptr == 0)
	{
		//type->ptr = 1;
		type->flags |= TYPE_STRCT_WAS_FROM_VAL;
		int strct_sz = GetTypeSize(type);
		fdecl->strct_vals_sz += strct_sz;
		fdecl->strct_vals_sz = get_even_address_with(16, fdecl->strct_vals_sz);
	}
}
void NewVarArgToScope(lang_state *lang_stat, scope* scp, type2* tp, func_decl* fdecl)
{
	type2 aux_type;
	auto ar_decl = FindIdentifier("array", scp, &aux_type);

	ASSERT(ar_decl)

		own_std::vector<comma_ret> dummy_arg;

	own_std::string sname = own_std::string("array_") + TypeToString(aux_type);

	type_struct2* ret_strct;
	if (!TryInstantiateStruct(lang_stat, ar_decl->type.strct, sname, scp, &ret_strct, dummy_arg, tp))
	{
		ASSERT(false)
	}

	aux_type.type = enum_type2::TYPE_STRUCT;
	aux_type.strct = ret_strct;

	auto new_decl = NewDecl(lang_stat, "ar_args", aux_type);
	new_decl->flags |= TYPE_VAR_ARGS_STRCT;
	fdecl->args.emplace_back(new_decl);
	scp->vars.emplace_back(new_decl);
}


bool FunctionIsDone(lang_state *lang_stat, node* n, scope* scp, type2* ret_type, int flags)
{
	char msg_hdr[256];
	node* fnode = n;


	bool is_outsider = IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_LINK_NAME) || IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_OUTSIDER) || fnode->type == node_type::N_FUNC_DEF;

	scope* child_scp;

	//if (is_outsider)
	//	child_scp = scp;
	//else
	child_scp = GetScopeFromParent(lang_stat, fnode->r, scp);
	child_scp->type = SCP_TYPE_FUNC;

	child_scp->line_start = n->t->line;
	child_scp->line_end = n->r->t->line;

	if (fnode->fdecl == nullptr)
	{
		fnode->fdecl = (func_decl*)AllocMiscData(lang_stat, sizeof(func_decl));
		memset(fnode->fdecl, 0, sizeof(func_decl));
		fnode->fdecl->from_file = lang_stat->cur_file;
	}
	func_decl* fdecl = fnode->fdecl;
	fdecl->func_node = fnode;
	fdecl->scp = child_scp;
	child_scp->flags |= SCOPE_INSIDE_FUNCTION;
	child_scp->fdecl = fdecl;
	ret_type->fdecl = fdecl;

	int template_end_idx = fdecl->templates.size();
	// templates
	if (fnode->l->l->l != nullptr && fdecl->templates.size() == 0)
	{
		ret_type->type = enum_type2::TYPE_FUNC_TYPE;
		ret_type->fdecl = fdecl;
		fdecl->flags |= FUNC_DECL_TEMPLATED;
		auto tdecls = DescendTemplatesToDecl(lang_stat, fnode->l->l->l->r, child_scp, &fdecl->templates);
		child_scp->vars.assign(tdecls.begin(), tdecls.end());
		template_end_idx = child_scp->vars.size();


	}

	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_OUTSIDER) ? FUNC_DECL_IS_OUTSIDER : 0;

	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_CONSTRUCTOR) ? FUNC_DECL_CONSTRUCTOR : 0;

	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_INTERNAL) ? FUNC_DECL_INTERNAL : 0;
	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_MACRO) ? FUNC_DECL_MACRO : 0;
	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_THIS) ? FUNC_DECL_THIS : 0;
	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_COMP) ? FUNC_DECL_COMP : 0;
	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_TEST) ? FUNC_DECL_TEST : 0;
	fdecl->flags |= IS_FLAG_ON(fnode->flags, NODE_FLAGS_ALIGN_STACK_WHEN_CALL) ? FUNC_DECL_ALIGN_STACK_WHEN_CALL : 0;

	fdecl->from_file = lang_stat->cur_file;

	bool is_link_name = IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_LINK_NAME);
	if (is_link_name)
	{
		fdecl->flags |= FUNC_DECL_LINK_NAME;
		fdecl->link_name = fnode->str->substr();
	}

	// args
	if (IS_FLAG_OFF(flags, DONT_DESCEND_ARGS) && fnode->l->l->r != nullptr && fdecl->args.size() == 0
		&& IS_FLAG_OFF(fdecl->flags, FUNC_DECL_ARGS_GOTTEN))
	{
		if (template_end_idx == 0 && !is_outsider)
			ASSERT(child_scp->vars.size() == 0);

		int last_flags = lang_stat->flags;

		// making sure we will not instantiate structs args
		if (template_end_idx > 0)
			lang_stat->flags |= PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT;


		lang_stat->flags |= PSR_FLAGS_DONT_DECLARE_VARIABLES;
		lang_stat->flags |= PSR_FLAGS_RET_NIL_EVEN_WHEN_PTR_TO_STRUCT_NOT_DONE;
		lang_stat->flags &= ~PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS;
		if (!DescendNameFinding(lang_stat, fnode->l->l->r, child_scp))
		{
			lang_stat->flags = last_flags;
			//lang_stat->flags &= ~PSR_FLAGS_DONT_DECLARE_VARIABLES;
			//lang_stat->flags &= ~PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT;
			return false;
		}

		lang_stat->flags &= ~PSR_FLAGS_DONT_DECLARE_VARIABLES;

		own_std::vector<comma_ret> args;
		DescendComma(lang_stat, fnode->l->l->r, child_scp, args);

		//lang_stat->flags &= ~PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT;

		lang_stat->flags = last_flags;

		bool is_var_args = false;
		int i = 0;

		decl2* normal_func_first_arg_decl_with_using = nullptr;

		// getting type of args
		FOR_VEC(t, args)
		{
			// ERROR: var_args should be at the end
			ASSERT(!is_var_args);



            // parameters with name and type
            if (t->type == COMMA_VAR_ARGS)
            {
                fdecl->flags |= FUNC_DECL_VAR_ARGS;
                // getting where the var_args start
                // we're subtracting one because the var counts as an argument
                fdecl->var_args_start_offset = (args.size() - 1) * 8 + 8;
                is_var_args = true;
                type2 aux_type;
                continue;
                //child_scp->vars.emplace_back(NewDecl(lang_stat, t->decl.name, t->decl.type));
            }

			//decl2* new_decl = new decl2();
			type2 dummy_type;
			memset(&dummy_type, 0, sizeof(type2));
			auto new_decl = DeclareDeclToScopeAndMaybeToFunc(lang_stat, "", &dummy_type, child_scp, t->n);

			new_decl->flags |= DECL_IS_ARG;


			decl2* enum_decl = nullptr;
			// prably struct names
			if (t->type == COMMA_RET_IDENT)
			{
				if (t->decl.type.type == enum_type2::TYPE_AUTO)
				{
					enum_decl = FindIdentifier(t->decl.name, child_scp, &t->decl.type);
					ASSERT(enum_decl);
					t->decl.type.type = FromTypeToVarType(t->decl.type.type);
				}
				else
				{
					t->decl.type.type = FromTypeToVarType(t->decl.type.type);
				}

				new_decl->type = t->decl.type;
				new_decl->name = "unamed";

				if (new_decl->type.type == TYPE_ENUM)
					new_decl->type.e_decl = enum_decl;
				//child_scp->vars.emplace_back(NewDecl(lang_stat, "unamed", t->tp));
			}

			// parameters with name and type
			else if (t->type == COMMA_RET_COLON || t->type == COMMA_RET_TYPE)
			{
				enum_type2 tp = t->decl.type.type;
				t->decl.type.type = FromTypeToVarType(tp);
				new_decl->type = t->decl.type;
				new_decl->name = t->decl.name;

				enum_decl = t->decl.type.e_decl;

				if (new_decl->type.type == TYPE_ENUM)
					new_decl->type.from_enum = enum_decl;

				//child_scp->vars.emplace_back(NewDecl(lang_stat, t->decl.name, t->decl.type));
			}
			// probably ptr type *u8, *u16 etc
			else if (t->type == COMMA_RET_EXPR)
			{
				t->decl.type.type = FromTypeToVarType(t->decl.type.type);
				new_decl->type = t->decl.type;
				new_decl->name = "unamed_arg";
				//child_scp->vars.emplace_back(NewDecl(lang_stat, "unamed", t->tp));
			}
			if (t->decl.type.type == TYPE_STRUCT && IS_FLAG_ON(t->decl.type.strct->flags, TP_STRCT_TUPLE))
			{
				new_decl->type.ptr = 1;
			}
			// making struct values to be ptrs
			CheckStructValToFunc(fdecl, &new_decl->type);

			// assiging parameters directly to args if it's an outisder func
			if (is_outsider)
				fdecl->args.emplace_back(new_decl);
			//else
				//child_scp->vars.emplace_back(new_decl);

			// we can only have unamed parametrs if 
			// 1) its an outsider or internal function
			// 2) if it is the firs argument of a normal function
			if (t->type != COMMA_RET_COLON)
			{
				int flags = NODE_FLAGS_FUNC_INTERNAL;
				bool function_allows_unamed = IS_FLAG_ON(fdecl->flags, flags) || n->type == N_FUNC_DEF;
				bool normal_function_unamed = (i == 0 && !function_allows_unamed);
				
				ASSERT(function_allows_unamed || normal_function_unamed);

				if (normal_function_unamed)
				{
					new_decl->name += std::to_string(rand() % 0xffffff).c_str();
					normal_func_first_arg_decl_with_using = new_decl;
				}
			}

			i++;

		}
		// scope vars to args, excluding templates
		if (!is_outsider)
			fdecl->args.assign(child_scp->vars.begin() + template_end_idx, child_scp->vars.end());

		if(normal_func_first_arg_decl_with_using)
			AddStructMembersToScopeWithUsing(lang_stat, normal_func_first_arg_decl_with_using, fdecl->scp, NewIdentNode(lang_stat, normal_func_first_arg_decl_with_using->name, n->t));
		//fdecl->vars.assign(child_scp->vars.begin() + template_end_idx, child_scp->vars.end());
	}
	fdecl->flags |= FUNC_DECL_ARGS_GOTTEN;
	// if the function has templates, we don't wanna check the rest of it
	if (template_end_idx > 0)
		return true;

	//return type
	if (!CheckFuncRetType(lang_stat, fdecl, child_scp))
		return false;

	if (!CheckFuncExtra(lang_stat, fdecl, child_scp))
		return false;

	if (IS_FLAG_ON(fdecl->flags, FUNC_DECL_IS_OUTSIDER))
	{
		lang_stat->outsider_funcs.emplace_back(fdecl);
	}

	/*
		fdecl->strct_val_ret_offset = fdecl->strct_vals_offset;
		CheckStructValToFunc(fdecl, &fdecl->ret_type);
		*/

	int args_end_idx = fdecl->args.size();

	fdecl->call_strcts_val_offset = fdecl->strct_vals_sz;
	fnode->r->scp = child_scp;

	if (IS_FLAG_OFF(flags, DONT_DESCEND_SCOPE) && IS_FLAG_OFF(fdecl->flags, FUNC_DECL_MACRO))
	{
		// scope
		if (fnode->r != nullptr && !DescendNameFinding(lang_stat, fnode->r, child_scp))
			return false;

		//fdecl->vars.insert(fdecl->vars.end(), child_scp->vars.begin() + args_end_idx, child_scp->vars.end());
	}
	if (n->type == N_OP_OVERLOAD && fdecl->templates.size() == 0)
	{
		CheckOverloadFunction(lang_stat, fdecl);
	}
	fdecl->flags |= FUNC_DECL_IS_DONE;

	// checking if test func has a return type of s32
	if (IS_FLAG_ON(fnode->flags, NODE_FLAGS_FUNC_TEST))
	{
		if (fdecl->ret_type.type != enum_type2::TYPE_S32)
		{
			REPORT_ERROR(fnode->t->line, fnode->t->line_offset, VAR_ARGS("test function must return s32:\n")
			);
		}
	}

	if (IS_FLAG_ON(fdecl->flags, FUNC_DECL_COMP) && lang_stat->comp_time_funcs.find(fdecl) == lang_stat->comp_time_funcs.end())
	{
		lang_stat->comp_time_funcs[fdecl] = fdecl;
	}

	return true;
}

import_strct* NewImport(lang_state *lang_stat, import_type type, own_std::string alias, unit_file* fl)
{
	auto ret = (import_strct*)AllocMiscData(lang_stat, sizeof(import_strct));
	memset(ret, 0, sizeof(import_strct));

	ret->type = type;
	ret->alias = alias;
	ret->fl = fl;
	return ret;
}
node* NewUnopNode(lang_state *lang_stat, node* lhs, tkn_type2 t, node* rhs)
{
	auto new_t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
	memset(new_t, 0, sizeof(token2));
	new_t->type = t;

	auto un = new_node(lang_stat, rhs->t);

	un->type = node_type::N_UNOP;
	un->t = new_t;
	if (lhs)
		un->l = lhs;
	else
		un->r = rhs;

	return un;
}
node* NewBinOpNode(lang_state *lang_stat, node* lhs, tkn_type2 t, node* rhs)
{
	auto new_t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
	memset(new_t, 0, sizeof(token2));

	new_t->type = t;

	token2* tkn = lhs ? lhs->t : rhs->t;
	new_t->line = tkn->line;
	new_t->line_offset = tkn->line_offset;
	new_t->line_str = tkn->line_str;

	auto bin = new_node(lang_stat, tkn);

	bin->type = node_type::N_BINOP;
	bin->t = new_t;
	bin->l = lhs;
	bin->r = rhs;

	return bin;
}
node* NewTypeNode(lang_state *lang_stat, node* lhs, node_type nd, node* rhs, token2 *t)
{
	auto bin = new_node(lang_stat, t);
	bin->t = t;
	bin->type = nd;
	bin->l = lhs;
	bin->r = rhs;

	return bin;
}

node* NewIntNode(lang_state *lang_stat, long long i, token2 *t)
{
	auto new_tkn = (token2*)AllocMiscData(lang_stat, sizeof(token2));
	memset(new_tkn, 0, sizeof(token2));
	new_tkn->type = tkn_type2::T_INT;
	new_tkn->i = (int)i;
	new_tkn->line = t->line;
	new_tkn->line_offset = t->line_offset;
	new_tkn->line_str = t->line_str;

	auto ident = new_node(lang_stat, t);
	ident->type = node_type::N_INT;
	
	ident->t = new_tkn;

	return ident;
}
node* NewIdentNode(lang_state *lang_stat, own_std::string name, token2 *t)
{
	auto new_tkn = (token2*)AllocMiscData(lang_stat, sizeof(token2));
	memset(new_tkn, 0, sizeof(token2));
	new_tkn->type = tkn_type2::T_WORD;
	new_tkn->str = name.substr();
	new_tkn->line = t->line;
	new_tkn->line_offset = t->line_offset;
	new_tkn->line_str = t->line_str;

	auto ident = new_node(lang_stat, t);
	ident->type = node_type::N_IDENTIFIER;
	ident->t = new_tkn;

	return ident;
}
void BottomOfTheTreeInsertLhs(lang_state *lang_stat, node* n, node* insert)
{
	node* cur_n = n;
	// going to the bottom of the tree
	while (cur_n->l != nullptr)
	{
		cur_n = cur_n->l;
	}
	auto new_n = NewBinOpNode(lang_stat,
		insert,
		tkn_type2::T_POINT,
		cur_n->NewTree(lang_stat));
	memcpy(cur_n, new_n, sizeof(node));
}
decl2* PointLogic(lang_state *lang_stat, node* n, scope* scp, type2* ret_tp)
{
	char msg_hdr[256];
	auto lhs_decl = DescendNameFinding(lang_stat, n->l, scp);

	if (!lhs_decl)
		return nullptr;

	type2 lhs_tp;

	NameFindingGetType(lang_stat, n->l, scp, lhs_tp);

	// when lhs is 1, it means that a builtin type was returned, if the lhs_tp isn't a struct
	// lhs_tp gets a struct value if the n->l is a function call for example,
	if ((long long)lhs_decl == 1 && (lhs_tp.type != enum_type2::TYPE_STRUCT_TYPE && lhs_tp.type != enum_type2::TYPE_STRUCT))
		ASSERT(false)

		// we don't wanna get templated struct members at this phase, in case
		// lhs is a strct

		decl2 dummy_decl;
	dummy_decl.type = lhs_tp;
	decl2* lhs = &dummy_decl;

	if (lhs->type.type == enum_type2::TYPE_AUTO)
		return (decl2*)1;

	// getting how many lhs should be saved when we encounter another pointer that also need to be dereferenced
	if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_ASSIGN_SAVED_REGS))
	{
		if (lhs->type.ptr > 0)
		{

			lang_stat->lhs_saved++;
			/*
			if(scp->fdecl->biggest_saved_lhs < lang_stat->lhs_saved)
				//scp->fdecl->biggest_saved_lhs = lang_stat->lhs_saved;
				*/
		}
	}
	switch (lhs->type.type)
	{
	case enum_type2::TYPE_ENUM_TYPE:
	{
		ret_tp->type = enum_type2::TYPE_ENUM_IDX_32;
		ret_tp->from_enum = lhs_decl;

		auto e_decl = lhs_decl->type.GetEnumDecl(n->r->t->str);

		if (IS_PRS_FLAG_ON(PSR_FLAGS_REPORT_UNDECLARED_IDENTS) && !e_decl)
		{
			REPORT_ERROR(n->r->t->line, n->r->t->line_offset,
				VAR_ARGS("name '%s' is not part of enum '%s'",
					n->r->t->str.c_str(), lhs_decl->name.c_str()
				)
			);
			ExitProcess(1);
		}
		if (!e_decl)
			return nullptr;

		ret_tp->e_idx = e_decl->type.e_idx;
		return e_decl;

	}break;
	case enum_type2::TYPE_STATIC_ARRAY:
	{
		if (n->r->t->str == "len")
		{
			// transforming the node into a int node
			//free(n->l);
			//free(n->r);

			memcpy(n, NewIntNode(lang_stat, lhs->type.ar_size, n->t), sizeof(node));

			ret_tp->type = enum_type2::TYPE_INT;
		}
		else
		{
			ReportMessageOne(lang_stat, n->r->t, "'%s' is not part of array literals:", (void*)n->r->t->str.c_str());
			ExitProcess(1);

			ASSERT(false)
		}

		return (decl2*)1;
	}break;

	case enum_type2::TYPE_STRUCT_TYPE:
	{
		return FindIdentifier(n->r->t->str, lhs->type.strct->scp, ret_tp);

		//ReportMessageOne(lang_stat, n->r->t, "struct type '%s' is not a variable that can be indexed:", (void*)n->l->t->str.c_str());
	}break;
	case enum_type2::TYPE_STRUCT:
	{
		// maybe modifying the tree for members that are being used with "using" keyword
		auto ret = DescendNameFinding(lang_stat, n->r, lhs->type.strct->scp);
		// correcting the tree that was modified
		if (n->r->type == node_type::N_BINOP && n->r->t->type == tkn_type2::T_POINT)
		{
			type2 ret_type;
			//ret = PointLogic(ret->using_node, ret->type.strct->scp, &ret_type);
			BottomOfTheTreeInsertLhs(lang_stat, n->r, n->l);

			memcpy(n, n->r, sizeof(node));

		}

		auto strct = lhs->type.strct;
		if (strct->original_strct && strct->original_strct->name == "rel_ptr")
		{
			// getting the second template type, which is the the rel_ptr type
			auto sec_templ = strct->scp->vars[1]->type.tp;

			ASSERT(sec_templ->type == enum_type2::TYPE_STRUCT_TYPE)
				ret = sec_templ->strct->FindDecl(n->r->t->str);
			// decl not found
			ASSERT(ret);

			auto op_func = lhs->type.strct->FindOpOverload(lang_stat, overload_op::DEREF_OP);
			ASSERT(op_func)
				* ret_tp = ret->type;

			MakeRelPtrDerefFuncCall(lang_stat, op_func, n->l);
		}
		else
		{
			if (n->r->type == N_INT)
			{
				ASSERT(IS_FLAG_ON(lhs->type.strct->flags, TP_STRCT_TUPLE));
				ret = lhs->type.strct->scp->vars[n->r->t->i];

			}
			else
			{
				type2 dummy;
				ret = FindIdentifier(n->r->t->str, lhs->type.strct->scp, &dummy);


				if (!ret)
				{
					REPORT_ERROR(n->r->t->line, n->r->t->line_offset,
						VAR_ARGS("'%s' is not part of struct '%s'\n",
							n->r->t->str.c_str(), lhs->type.strct->name.c_str()
						)
					)
						ExitProcess(1);
				}
			}
			ASSERT(ret);
			*ret_tp = ret->type;
			return ret;
		}


		return ret;
	}break;
	case enum_type2::TYPE_IMPORT:
	{
		own_std::string name;

		bool was_call = false;

		if (n->r->type == node_type::N_IDENTIFIER)
			name = n->r->t->str.substr();
		else if (n->r->type == node_type::N_CALL)
		{
			name = n->r->l->t->str.substr();
			// checking name finding of arguments of the call

			was_call = true;
		}
		auto ret_decl = lhs->type.imp->FindDecl(name);

		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_REPORT_UNDECLARED_IDENTS) && !ret_decl)
			ReportUndeclaredIdentifier(lang_stat, n->t);

		if (!ret_decl)
			return nullptr;

		switch (ret_decl->type.type)
		{
		case enum_type2::TYPE_FUNC_EXTERN:
		case enum_type2::TYPE_FUNC:
			//if (!CallNode(lang_stat, n->r, scp, ret_tp, ret_decl))
			//	return nullptr;
			*ret_tp = ret_decl->type;

			return ret_decl;
			break;
		default:
			*ret_tp = ret_decl->type;
		}
		return ret_decl;
	}break;
	default:
		// not handled point type
		REPORT_ERROR(n->r->t->line, n->r->t->line_offset,
			VAR_ARGS("identifier '%s' is not part of type '%s'",
				n->r->t->str.c_str(), TypeToString(lhs_tp).c_str()
			)
		);
		ExitProcess(1);

	}
	return nullptr;
}

void ReportUndeclaredIdentifier(lang_state *lang_stat, token2* t)
{
	char msg_hdr[256];
	REPORT_ERROR(t->line, t->line_offset,
		VAR_ARGS("undeclared identifier: %s\n", t->str.c_str())
	);
	//ExitProcess(1);
	//ASSERT(false)
	lang_stat->flags |= PSR_FLAGS_ERRO_REPORTED;
	printf("-------------**-----------\n");
}
own_std::string FuncNameWithTempls(lang_state *lang_stat, own_std::string fname, own_std::vector<type2>* types)
{
	own_std::string ret;
	ret += fname;
	ret += "_";

	FOR_VEC(t, *types)
	{
		ret += TypeToString(*t);
		ret += "_";
	}
	ret.pop_back();

	return ret;
}
decl2* DeclareDeclToScopeAndMaybeToFunc(lang_state *lang_stat, own_std::string name, type2* tp, scope* scp, node* nd)
{
	/*
	if (name == "entry")
		auto a = 0;
		*/

		// we only want to declare functions and structs. We're not interested in its contents
	auto new_decl = (decl2*)AllocMiscData(lang_stat, sizeof(decl2));
	memset(new_decl, 0, sizeof(decl2));
	new_decl->name = name.substr();
	new_decl->type = *tp;
	new_decl->decl_nd = nd;
	new_decl->from_file = lang_stat->cur_file;
	new_decl->type.type = FromTypeToVarType(tp->type);

	bool is_func_extern = new_decl->type.type == TYPE_FUNC_EXTERN;
	bool is_func = (new_decl->type.type == TYPE_FUNC);

	// setting struct "this_decl" to the new declared decl and name
	if (new_decl->type.type == TYPE_STRUCT_TYPE)
	{
		new_decl->type.strct->this_decl = new_decl;
		new_decl->type.strct->name = new_decl->name;

	}
	// @test
	/*
	// naming the fdecl with this func name
	else if (is_func)
	{
		new_decl->type.fdecl->name = new_decl->name.substr();
	}
	*/
	// adding flags for the extern func
	else if (new_decl->type.type == TYPE_FUNC_EXTERN)
	{
		new_decl->type.fdecl->flags |= FUNC_DECL_EXTERN;
	}
	// 
	else if (new_decl->type.type == TYPE_ENUM)
		new_decl->type.from_enum = tp->e_decl;

	// @test to delete this doesn't work
	// naming the fdecl with this func name
	if (is_func || is_func_extern)
	{
		new_decl->type.fdecl->name = new_decl->name.substr();

		if (IS_FLAG_OFF(new_decl->type.fdecl->flags, FUNC_DECL_TEMPLATED))
			lang_stat->funcs_scp->vars.emplace_back(new_decl);

		scp->vars.emplace_back(new_decl);
	}
	else
		scp->vars.emplace_back(new_decl);

	lang_stat->something_was_declared = true;
	// declaring variables to function vars
	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
		scp->fdecl->vars.emplace_back(new_decl);

	// adding the data sect size
	if (IS_FLAG_ON(scp->flags, SCOPE_IS_GLOBAL))
	{
		if (CanAddToDataSect(new_decl->type.type))
		{
			new_decl->flags |= DECL_IS_GLOBAL;
			new_decl->offset = AddDataSectSizeRetPrevSize(lang_stat, GetTypeSize(&new_decl->type));

		}
	}

	return new_decl;
}
own_std::string MangleFuncNameWithArgs(lang_state *lang_stat, func_decl* fdecl, own_std::string original_name, int start_arg)
{
	own_std::string name;
	own_std::vector<type2> args_types;
	for (auto a = fdecl->args.begin() + start_arg; a < fdecl->args.end(); a++)
	{
		args_types.emplace_back((*a)->type);
	}
	name = FuncNameWithTempls(lang_stat, original_name, &args_types);
	return name;
}

#ifdef DEBUG_NAME
struct dbg_name_
{
	bool* success;
	bool set_not_found;
	node* nd;
	node* to_set;
	~dbg_name_()
	{
		/*
		if (*success)
			lang_stat->not_found_nd = nd;
		else
		{
			if (nd->t && nd->t->line == 217)
			{
				auto a = 0;
			}
		}
		if (set_not_found)
			to_set->not_found_nd = lang_stat->not_found_nd;
			*/
	}
};
#endif

void ReportDeclaredTwice(lang_state *lang_stat, node* twice, decl2* decl)
{
	char msg_hdr[256];
	int decl_exist_ln = decl->decl_nd->t->line;
	REPORT_ERROR(twice->t->line, twice->t->line_offset,
		VAR_ARGS("variable '%s' declred here:\n\n%d|%s\n\nHas the same name as this one\n",
			decl->name.c_str(), decl_exist_ln, GetFileLn(lang_stat, decl_exist_ln - 1, decl->from_file)
		)
	)
		ExitProcess(1);
}
node* CreateNodeFromType(lang_state *lang_stat, type2* tp, token2 *t)
{
	node* nd;
    decl2 *decl;
	if (tp->IsStrct(&decl))
	{
		nd = NewIdentNode(lang_stat, decl->name, t);
	}
	// if the type of the array is a builting, we have the create a new node with the type
	else
	{
		nd = new_node(lang_stat, t);
		nd->type = N_TYPE;
		nd->decl_type = *tp;
		nd->decl_type.type = FromVarTypeToType(nd->decl_type.type);
	}
	return nd;
}

bool IsNodeUnop(node* nd, tkn_type2 tkn)
{
	if (CMP_NTYPE(nd, N_UNOP) && nd->t->type == tkn)
	{
		return true;
	}
	return false;
}
bool IsNodeOperator(node* nd, tkn_type2 tkn)
{
	if (CMP_NTYPE(nd, N_BINOP) && nd->t->type == tkn)
	{
		return true;
	}
	return false;
}
void TransformSingleFuncToOvrlStrct(lang_state *lang_stat, decl2* decl_exist)
{
	func_decl* f = decl_exist->type.fdecl;
	decl_exist->type.overload_funcs = (func_overload_strct*)AllocMiscData(lang_stat, sizeof(func_overload_strct));
	memset(decl_exist->type.overload_funcs, 0, sizeof(func_overload_strct));

	decl_exist->type.overload_funcs->name = decl_exist->name.substr();

	if (IS_FLAG_OFF(f->flags, FUNC_DECL_TEMPLATED) /* && f->op_overload == COND_EQ_OP*/)
	{
		auto op_str = OvrldOpToStr(f->op_overload);
		auto op_str_len = op_str.size();

		// popping the operator string from the end of the name
		for (int i = 0; i < op_str_len; i++)
		{
			f->name.pop_back();
		}
		f->name = MangleFuncNameWithArgs(lang_stat, f, f->name, 1) + op_str;
	}


	type2 tp = {};
	tp.type = TYPE_FUNC;
	tp.fdecl = f;
	lang_stat->cur_file->global->vars.emplace_back(NewDecl(lang_stat, f->name, tp));

	decl_exist->type.overload_funcs->fdecls.emplace_back(f);
	decl_exist->type.type = enum_type2::TYPE_OVERLOADED_FUNCS;
}

void AddNewDeclToFileGlobalScope(lang_state *lang_stat, decl2* d)
{
	lang_stat->cur_file->global->vars.emplace_back(d);
}
node* CreateDeclNode(lang_state *lang_stat, own_std::string name, type2* tp, token2 *t)
{
	auto new_decl = NewBinOpNode(lang_stat, NewIdentNode(lang_stat, name, t),
		T_COLON,
		CreateNodeFromType(lang_stat, tp, t));

	return new_decl;
}
node* CreateDeclNode(lang_state *lang_stat, own_std::string name, enum_type2 type, token2 *t)
{
	type2 tp_;
	memset(&tp_, 0, sizeof(type2));
	tp_.type = type;
	return CreateDeclNode(lang_stat, name, &tp_, t);

}
node* CreateBinOpRhsBool(lang_state *lang_stat, node* lhs, keyword kw, tkn_type2 bin)
{
	auto kw_val = new_node(lang_stat, lhs->t);
	kw_val->type = N_KEYWORD;
	kw_val->kw = kw;

	auto ret = NewBinOpNode(lang_stat, lhs, bin, kw_val);

	return ret;
}
node* CreateBoolExpression(lang_state *lang_stat, node* var, node* expr, scope* scp)
{
	/*
	auto kw_false = new_node(lang_stat);
	kw_false->type = N_KEYWORD;
	kw_false->kw = KW_FALSE;

	auto kw_true = new_node(lang_stat);
	kw_true->type = N_KEYWORD;
	kw_true->kw = KW_TRUE;

	auto eq_true  = NewBinOpNode(lang_stat, var, T_EQUAL, kw_true);
	auto eq_false = NewBinOpNode(lang_stat, var, T_EQUAL, kw_false);
	*/

	auto eq_true = CreateBinOpRhsBool(lang_stat, var, KW_TRUE, T_EQUAL);
	auto eq_false = CreateBinOpRhsBool(lang_stat, var, KW_FALSE, T_EQUAL);

	eq_true->flags = NODE_FLAGS_IS_PROCESSED2;
	eq_false->flags = NODE_FLAGS_IS_PROCESSED2;


	auto scope_nd = new_node(lang_stat, expr->t);
	scope_nd->type = N_SCOPE;
	auto stmnt_nd = new_node(lang_stat, expr->t);
	stmnt_nd->type = N_STMNT;
	stmnt_nd->l = eq_true;
	scope_nd->r = stmnt_nd;

	auto if_logic = NewTypeNode(lang_stat, expr, N_BINOP, scope_nd, expr->t);

	scope_nd = new_node(lang_stat, expr->t);
	scope_nd->type = N_SCOPE;
	stmnt_nd = new_node(lang_stat, expr->t);
	stmnt_nd->type = N_STMNT;
	stmnt_nd->l = eq_false;
	scope_nd->r = stmnt_nd;
	auto else_logic = NewTypeNode(lang_stat, nullptr, N_BINOP, scope_nd, expr->t);
	else_logic->type = N_ELSE;

	auto new_if = NewTypeNode(lang_stat, if_logic, N_IF, else_logic, expr->t);
	new_if->t = expr->t;

	//memcpy(equal_stmnt, new_if, sizeof(node));

	if (!DescendNameFinding(lang_stat, new_if, scp))
		ASSERT(false);
	return new_if;
}

#define PROCESS_ND_IS_ND 0
#define PROCESS_ND_IS_INT 1
#define PROCESS_ND_FLAGS_TAKE_ADDRESS_OF 0x100
node* ProcessArgNd(lang_state *lang_stat, void* arg, int flags)
{
	int tp = flags & 0xff;

	node* arg_nd;
	switch (tp)
	{
	case PROCESS_ND_IS_INT:
	{
		//arg_nd = NewIntNode(lang_stat, (long long)arg, );
	}break;
	case PROCESS_ND_IS_ND:
	{
		arg_nd = (node*)arg;
	}break;
	}

	if (IS_FLAG_ON(flags, PROCESS_ND_FLAGS_TAKE_ADDRESS_OF))
	{
		ASSERT(tp != PROCESS_ND_IS_INT);

		auto new_arg = NewUnopNode(lang_stat, nullptr, T_AMPERSAND, arg_nd);
		arg_nd = new_arg;
	}
	return arg_nd;

}
node* NewThreeArgNd(lang_state *lang_stat, own_std::string name, node* arg1, node* arg2, node* arg3)
{
	own_std::vector<node*> args;
	args.emplace_back(arg1);
	args.emplace_back(arg2);
	args.emplace_back(arg3);

	auto call_nd = MakeFuncCallArgs(lang_stat, name, nullptr, args, arg1->t);
	//call_nd->t = 
	return call_nd;
}
/*
node* NewThreeArgNd2(own_std::string name, void* arg1, int arg1_type, void* arg2, int arg2_type, void* arg3, int arg3_type)
{
	node* arg1_nd = ProcessArgNd(lang_stat, arg1, arg1_type);
	node* arg2_nd = ProcessArgNd(lang_stat, arg2, arg2_type);
	node* arg3_nd = ProcessArgNd(lang_stat, arg3, arg3_type);


	return NewThreeArgNd(name, arg1_nd, arg2_nd, arg3_nd);

}
*/

#define NEW_THREE_ARG_NDFLAGS_I_I(name, arg1, flags1, arg2, arg3)\
			NewThreeArgNd2(name, (void*)arg1, PROCESS_ND_IS_ND | (flags1), (void *)arg2, PROCESS_ND_IS_INT, (void *)arg3, PROCESS_ND_IS_INT);

#define NEW_BIN_OP_ND_I(nd, tkn, i)\
	NewBinOpNode(lang_stat, nd, tkn, NewIntNode(lang_stat, i, nd->t));

void CheckDeclNodeAndMaybeAddEqualZero(lang_state *lang_stat, node* n, scope* scp)
{
	if (IsNodeOperator(n, T_COLON) && IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION) && n->r->type != N_STRUCT_DECL
		//&& IS_FLAG_ON(n->flags, NODE_FLAGS_IS_PROCESSED2)
		)
	{
		type2 dummy_tp;
		decl2* decl = FindIdentifier(n->l->t->str, scp, &dummy_tp);

		if (decl->type.type == TYPE_STATIC_ARRAY || decl->type.ptr > 0)
			return;

		ASSERT(decl)

			node* final_nd;
		if (decl->type.type == TYPE_STRUCT)
		{
			auto ref_nd = NewUnopNode(lang_stat, nullptr, T_AMPERSAND, n->l);
			ref_nd->t->line = n->t->line;
			lang_stat->void_decl->type.ptr++;
			auto void_nd = CreateNodeFromType(lang_stat, &lang_stat->void_decl->type, n->t);
			auto casted1 = NewTypeNode(lang_stat, void_nd, N_CAST, ref_nd, n->t);
			lang_stat->void_decl->type.ptr--;

			final_nd = NewThreeArgNd(lang_stat, "_own_memset", casted1, NewIntNode(lang_stat, 0, n->t), NewIntNode(lang_stat, decl->type.strct->size, n->t));

		}
		else
		{
			if (decl->type.type == TYPE_F32 || decl->type.type == TYPE_F64)
			{
				auto f_nd = NewTypeNode(lang_stat, nullptr, N_FLOAT, nullptr, n->t);
				f_nd->t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
				f_nd->t->f = 0;
				final_nd = NewBinOpNode(lang_stat, n->l, T_EQUAL, f_nd);
			}
			else
			{
				final_nd = NEW_BIN_OP_ND_I(n->l, T_EQUAL, 0);
			}
		}

		auto stmnt_nd = NewTypeNode(lang_stat, new_node(lang_stat, n), node_type::N_STMNT, final_nd, n->t);
		memcpy(n, stmnt_nd, sizeof(node));
		n->flags |= NODE_FLAGS_STMNT_ZERO_INITIALIZED;

		decl->decl_nd = n->l;
		//	n->l->flags |= NODE_FLAGS_IS_PROCESSED2;
	}
}
bool IsSameStrct(type2* lhs, type_struct2* strct)
{
	if (lhs->type == TYPE_STRUCT && lhs->ptr == 0 && lhs->strct == strct)
		return true;
	return false;
}

void EnumToTypeSect(lang_state *lang_stat, own_std::string enum_name, scope* scp)
{
	auto type_sect = &lang_stat->type_sect;
	auto& vars = scp->vars;
	own_std::vector<char> buffer;
	own_std::vector<char> strct_str_tbl;


	buffer.insert(buffer.end(), sizeof(type_data), 0);

	type_data* strct_ptr = (type_data*)buffer.data();

	int offset_to_str_tbl = sizeof(type_data) * (vars.size() + 1);
	offset_to_str_tbl -= offsetof(type_data, name);

	strct_ptr->name = offset_to_str_tbl;
	strct_ptr->name_len = enum_name.length();
	strct_ptr->tp = enum_type2::TYPE_ENUM;
	strct_ptr->entries = vars.size();

	InsertIntoCharVector(&strct_str_tbl, (void*)enum_name.data(), enum_name.size());

	int* var_ptr = nullptr;

	int i = 0;

	FOR_VEC(v, vars)
	{
		if (IS_FLAG_ON((*v)->flags, DECL_FROM_USING))
			continue;

		int last_size = buffer.size();
		buffer.insert(buffer.end(), sizeof(int), 0);

		var_ptr = (int*)&buffer[last_size];

		offset_to_str_tbl = strct_str_tbl.size();

		ASSERT((*v)->name.size() < 65000 && offset_to_str_tbl < 65000);

		*var_ptr = (int)((short)offset_to_str_tbl | (short)(*v)->name.size());

		InsertIntoCharVector(&strct_str_tbl, (void*)(*v)->name.data(), (*v)->name.size());

		i++;
	}

	type_sect->insert(type_sect->begin(), strct_str_tbl.begin(), strct_str_tbl.end());
	type_sect->insert(type_sect->begin(), buffer.begin(), buffer.end());
}

void AddStructMembersToScopeWithUsing(lang_state *lang_stat, decl2 *decl, scope *scp, node *by_name_nd)
{
	ASSERT(decl->type.type = TYPE_STRUCT);

	// adding every var from struct to scope, and rearranging 
	// to tree, so that later this tree will be used to get the correct var
	FOR_VEC(v, decl->type.strct->scp->vars)
	{
		auto new_decl = (decl2*)AllocMiscData(lang_stat, sizeof(decl2));
		memcpy(new_decl, *v, sizeof(decl2));
		new_decl->flags |= DECL_FROM_USING;

		// assigning the using_n to the bottom of the tree
		if ((*v)->using_node == nullptr)
		{
			new_decl->name = (*v)->name;

			new_decl->using_node = NewBinOpNode(lang_stat,
				new_node(lang_stat, by_name_nd),
				tkn_type2::T_POINT,
				NewIdentNode(lang_stat, (*v)->name, by_name_nd->t));

		}
		// assigning the using_n to the top of the tree
		else
		{
			new_decl->using_node = (*v)->using_node->NewTree(lang_stat);
			BottomOfTheTreeInsertLhs(lang_stat, new_decl->using_node, new_node(lang_stat, by_name_nd));
		}
		//memcpy(n, decl->using_n, sizeof(node));

		scp->vars.emplace_back(new_decl);
	}
}
node* MakeMemCpyCall(lang_state *lang_stat, node *lhs, node *rhs, int size)
{
	auto arg1 = NewUnopNode(lang_stat, nullptr, T_AMPERSAND, lhs);
	auto arg2 = NewUnopNode(lang_stat, nullptr, T_AMPERSAND, rhs);

	lang_stat->void_decl->type.ptr++;
	auto void_nd = CreateNodeFromType(lang_stat, &lang_stat->void_decl->type, lhs->t);

	auto casted1 = NewTypeNode(lang_stat, void_nd, N_CAST, arg1, lhs->t);
	auto casted2 = NewTypeNode(lang_stat, void_nd, N_CAST, arg2, lhs->t);

	lang_stat->void_decl->type.ptr--;

	auto call_nd = NewThreeArgNd(lang_stat, "memcpy", casted1, casted2, NewIntNode(lang_stat, size, lhs->t));
	return call_nd;
}

void GetFilesInDirectory(own_std::string dir_str, own_std::vector<char *>* contents, own_std::vector<char *>* file_names)
{

#ifdef LINUX
	const char *path = dir_str.c_str();  // Directory path, "." for current directory
	struct dirent *entry;    // Struct to hold information about each directory entry
	DIR *dir = opendir(path);  // Open the directory

	if (dir == NULL) {
		perror("Unable to open directory");
		TCHAR buffer[MAX_PATH] = { 0 };
		PlatformExeFullparh(buffer, MAX_PATH);
		printf("dir is: %s, exe path %s\n", path, buffer);
		return 1;
	}

	// Loop over directory entries
	while ((entry = readdir(dir)) != NULL) {
		// Ignore "." and ".." entries
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
			file_names->emplace_back(entry->d_name);

			printf("%s\n", entry->d_name);  // Print the name of each file or directory
		}
	}

	closedir(dir);  // Close the directory

#else

	WIN32_FIND_DATA ffd;
	char buffer[128];

	HANDLE hFind = FindFirstFile((dir + "\\*").c_str(), &ffd);

	if(hFind == INVALID_HANDLE_VALUE)
	{
		printf("directory \"%s\" not found", dir.c_str());
		ASSERT(0);
		return;
	}
	BOOL found_file = 1;
	FindNextFile(hFind, &ffd);
	while (true)
	{
		found_file = FindNextFile(hFind, &ffd);
		if (!found_file)
			break;
		int read = 0;
		//char *data = ReadEntireFileLang(ffd.cFileName, &read);
		//contents->emplace_back(own_std::string(data, read));
		int len = strlen(ffd.cFileName) + 1;
		char* name = heap_alloc((mem_alloc *)__lang_globals.data, len);
		memcpy(name, ffd.cFileName, len);
		file_names->emplace_back(name);
	}

#endif
}
void AddFolderToScope(lang_state * lang_stat, scope *scp, own_std::string folder, import_type imp_type, own_std::string imp_name)
{
	own_std::vector<own_std::string> file_contents;
	own_std::vector<char *> file_names;

	GetFilesInDirectory(lang_stat->work_dir+"/"+ folder, nullptr, &file_names);

	own_std::vector<unit_file*> files_added;
	own_std::string prev_work_dir = lang_stat->work_dir;
	lang_stat->work_dir = lang_stat->work_dir+ "/"+folder;
	FOR_VEC(str, file_names)
	{
		files_added.emplace_back(AddNewFile(lang_stat, *str));
	}
	lang_stat->work_dir = prev_work_dir;

	FOR_VEC(f, files_added)
	{
		type2 tp;
		tp.type = enum_type2::TYPE_IMPORT;
		tp.imp = NewImport(lang_stat, imp_type, imp_name, *f);

		scp->imports.emplace_back(NewDecl(lang_stat, "__import", tp));
	}

	//printf("files added");
	FOR_VEC(f1, files_added)
	{
		type2 tp;
		tp.type = enum_type2::TYPE_IMPORT;
		tp.imp = NewImport(lang_stat, import_type::IMP_IMPLICIT_NAME, "", *f1);
		FOR_VEC(f2, files_added)
		{
			if (*f1 == *f2)
				continue;


			(*f2)->global->imports.emplace_back(NewDecl(lang_stat, "__import", tp));
		}
	}
}

bool IsKeyword(node* n, keyword kw)
{
	return n->type == N_KEYWORD && n->kw == kw;
}
// $DescendNameFinding
decl2* DescendNameFinding(lang_state *lang_stat, node* n, scope* given_scp)
{
	char msg_hdr[256];
	bool success = false;
#ifdef DEBUG_NAME
	lang_stat->not_found_nd = n;
	dbg_name_ dbg_name;
	memset(&dbg_name, 0, sizeof(dbg_name_));
	dbg_name.success = &success;
	dbg_name.nd = n;
#endif
	scope* scp = given_scp;
	type2 ret_type = {};
	message msg;

	memset(&msg, 0, sizeof(msg));
	memset(&ret_type, 0, sizeof(ret_type));

	msg.scp = scp;
	msg.n = n;
	msg.stmnt = lang_stat->cur_stmnt;



	if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_SCOPE))
	{
		//scp = GetScopeFromParent(n, given_scp);
	}


	if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
	{
		scp->fdecl->reached_nd = n;
	}
	switch (n->type)
	{
	case node_type::N_HASHTAG:
	{
		switch (n->r->type)
		{
		case node_type::N_IF:
		{
			node* cur = n->r;
			do 
			{
				if (!DescendNameFinding(lang_stat, cur, scp))
					return (decl2*)0;

				if (cur->type == N_ELSE)
					memcpy(n, cur->r, sizeof(node));
				else if (GetExpressionVal(cur->l->l, scp) == 1)
				{
					memcpy(n, cur->l->r, sizeof(node));
					break;
				}
				cur = cur->r;
			} while (cur->type == N_ELSE || cur->type == N_ELSE_IF);
		}break;
		}
	}break;
	case node_type::N_KEYWORD:
	{
		switch (n->kw)
		{
		case KW_DBG_BREAK:
		{
		}break;
		case KW_USING:
		{
			decl2* decl = DescendNameFinding(lang_stat, n->r, scp);

			if (!decl)
				return nullptr;

			if (IS_FLAG_ON(n->r->flags, NODE_FLAGS_IS_PROCESSED))
				return (decl2*)1;

			node* by_name_nd = nullptr;;
			own_std::string by_name_str;
			decl2* decl_to_get_from;
			switch (n->r->type)
			{
			case node_type::N_BINOP:
			{
				switch (n->r->t->type)
				{
				case tkn_type2::T_COLON:
				{
					ASSERT(n->r->l->type == node_type::N_IDENTIFIER)
						by_name_nd = n->r->l;
					by_name_str = n->r->l->t->str;


				}break;
				case tkn_type2::T_POINT:
				{
					by_name_nd = n->r;
				}break;
				default:
					// this kind of expression not allowd with using
					ASSERT(false)
				}
			}break;
			case node_type::N_IDENTIFIER:
			{
				by_name_nd = n->r;
			}break;
			default:
				ASSERT(false)
			}
			if (!decl)
				return nullptr;

			AddStructMembersToScopeWithUsing(lang_stat, decl, scp, by_name_nd);

			n->r->flags |= NODE_FLAGS_IS_PROCESSED;
		}break;
		default:
		{
			//scp = GetScopeFromParent(n->l, given_scp);
			if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp))
				return nullptr;
			if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp))
				return nullptr;
		}break;
		}
	}break;
	case node_type::N_DESUGARED:
	{
		//scp = GetScopeFromParent(n->l, given_scp);
		if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp))
			return nullptr;
		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp))
			return nullptr;
	}break;
	case node_type::N_APOSTROPHE:
	{

	}break;
	case node_type::N_UNOP:
	{
		switch (n->t->type)
		{
		case tkn_type2::T_AMPERSAND:
		{
			type2 ret_type;

			if (!NameFindingGetType(lang_stat, n, scp, ret_type))
				return nullptr;



		}break;
		case tkn_type2::T_POINT:
		{
			ASSERT(n->r->type == node_type::N_IDENTIFIER);

			type2 ret_type;
			auto ident = FindIdentifier(n->r->t->str, scp, &ret_type);

			if (!ident) return nullptr;

			ASSERT(ident->type.type == TYPE_ENUM_IDX_32)

				return ident;
		}break;
		case tkn_type2::T_DOLLAR:
		{
			if (IS_FLAG_ON(n->r->flags, NODE_FLAGS_IS_PROCESSED2))
				return (decl2*)1;

			n->flags |= NODE_FLAGS_AR_LIT_ANON;

			if (!DescendNameFinding(lang_stat, n->r, scp))
				return nullptr;

			own_std::vector<comma_ret> args;

			//int last_ar_lit_sz = lang_stat->cur_ar_lit_sz;

			if (n->r)
				DescendComma(lang_stat, n->r, scp, args);

			// getting type of args
			FOR_VEC(t, args)
			{
				if (t->type == COMMA_RET_IDENT)
					ASSERT(FindIdentifier(t->decl.name, scp, &t->decl.type))
					/*
				else if (t->type == COMMA_RET_EXPR)
					t->decl.type = t->decl;
					*/
			}

			//ASSERT(IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))

			auto ref_type = args[0].decl.type;

			if (ref_type.type == enum_type2::TYPE_INT)
				ref_type.type = enum_type2::TYPE_S32;

			// comparing types
			FOR_VEC(t, args)
			{
				ASSERT(CompareTypes(&ref_type, &t->decl.type))
			}

			auto new_tp = (type2*)AllocMiscData(lang_stat, sizeof(type2));
			memcpy(new_tp, &ref_type, sizeof(type2));

			auto ar_tp = (type2*)AllocMiscData(lang_stat, sizeof(type2));
			ar_tp->type = enum_type2::TYPE_STATIC_ARRAY;
			ar_tp->tp = new_tp;
			ar_tp->ar_size = args.size();


			ret_type = *ar_tp;

			n->ar_byte_sz = GetTypeSize(&ref_type) * args.size();

			scp->fdecl->array_literal_sz += n->ar_byte_sz;

			n->ar_lit_tp = ar_tp;
			//lang_stat->cur_ar_lit_sz = last_ar_lit_sz;
			n->r->flags |= NODE_FLAGS_IS_PROCESSED2;
		}break;

		default:
		{

			if (!DescendNameFinding(lang_stat, n->r, scp))
				return nullptr;
		}break;
		}
	}break;
	case node_type::N_STR_LIT:
	{
		return (decl2*)1;
	}break;
	case node_type::N_IMPORT_LIB:
	{
		ASSERT(n->r->type == node_type::N_STR_LIT)
			if (IS_FLAG_OFF(n->r->flags, NODE_FLAGS_IS_PROCESSED))
			{
				lang_stat->linker_options += n->r->t->str + " ";
				n->r->flags |= NODE_FLAGS_IS_PROCESSED;
			}
	}break;
	case node_type::N_TYPEDEF:
	{
		ASSERT(n->r->type == node_type::N_BINOP && n->r->t->str == "as");

		if (!DescendNameFinding(lang_stat, n->r->l, scp))
			return nullptr;

		type2 other_type;

		own_std::string name = n->r->r->t->str;
		auto decl_exist = FindIdentifier(name, scp, &other_type);

		if (!decl_exist)
		{
			other_type = DescendNode(lang_stat, n->r->l, scp);

			decl2* type_decl = FromTypeToDecl(lang_stat, &other_type);
			other_type.type = TYPE_TYPEDEF;
			other_type.type_def_decl = type_decl;
			DeclareDeclToScopeAndMaybeToFunc(lang_stat, n->r->r->t->str, &other_type, scp, n);
		}
	}break;
	case node_type::N_IMPORT:
	{
		if (n->r->type == node_type::N_BINOP && n->r->t->str == "as")
		{
			bool was_added = false;
			own_std::string alias = n->r->r->t->str.substr();
			own_std::string imp_name = n->r->l->t->str.substr();
			FOR_VEC(dcl, scp->imports)
			{
				if ((*dcl)->type.imp->alias == alias)
					was_added = true;
			}
			if (was_added == false)
			{
				AddFolderToScope(lang_stat, scp, imp_name, import_type::IMP_BY_ALIAS, alias);
				/*
				auto ret_fl = AddNewFile(lang_stat, imp_name);
				type2 tp;
				tp.type = enum_type2::TYPE_IMPORT;
				tp.imp = NewImport(lang_stat, import_type::IMP_BY_ALIAS, alias, ret_fl);
				tp.imp->alias = alias.substr();

				own_std::string name = own_std::string("imp_") + alias;

				scp->imports.emplace_back(NewDecl(lang_stat, name, tp));
				*/

			}
		}
		else
		{
			bool was_added = false;
			own_std::string file_name = n->r->t->str.substr();
			FOR_VEC(dcl, scp->imports)
			{
				if ((*dcl)->type.imp->fl->name == file_name)
					was_added = true;
			}
			if (was_added == false)
			{
				AddFolderToScope(lang_stat, scp, n->r->t->str, import_type::IMP_IMPLICIT_NAME, "");
			}
		}
		n->type = N_EMPTY;
	}break;
	case node_type::N_FUNC_DEF:
	case node_type::N_FUNC_DECL:
	{
		if (IS_FLAG_ON(n->flags, NODE_FLAGS_FUNC_CONSTRUCTOR))
		{
			if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_PROCESSED2))
				return (decl2*)1;

			if (!FunctionIsDone(lang_stat, n, given_scp, &ret_type, 0))
				return nullptr;

			// asserting that the return type is a struct
			ASSERT(n->fdecl->ret_type.type == enum_type2::TYPE_STRUCT)

				ret_type.type = enum_type2::TYPE_FUNC_TYPE;
			ret_type.fdecl = n->fdecl;

			// getting args types, because their type names will be the name of the construct
			own_std::vector<type2> tps;
			FOR_VEC(a, n->fdecl->args)
			{
				tps.emplace_back((*a)->type);
			}

			own_std::string fname = FuncNameWithTempls(lang_stat, own_std::string("constr_") + TypeToString(n->fdecl->ret_type), &tps);

			n->fdecl->name = fname;
			auto array_ctor = &n->fdecl->ret_type.strct->constructors;

			// just adding a new func_overload_strct if it doesn't exist
			if (array_ctor->size() == 0)
			{
				func_overload_strct s;
				memset(&s, 0, sizeof(func_overload_strct));
				array_ctor->emplace_back(s);
				//array_ctor->reserve(1);
			}

			(*array_ctor)[0].fdecls.emplace_back(n->fdecl);
			DeclareDeclToScopeAndMaybeToFunc(lang_stat, n->fdecl->name, &ret_type, scp, n);
			n->flags |= NODE_FLAGS_IS_PROCESSED2;
		}
		else
		{
			if (!FunctionIsDone(lang_stat, n, given_scp, &ret_type, 0))
				return nullptr;
			/*
			// args
			if(!DescendNameFinding(lang_stat, n->l->l->r, scp))
				return nullptr;

			// scope
			if(!DescendNameFinding(lang_stat, n->r, scp))
				return nullptr;
				*/
		}

	}break;
	case node_type::N_STRUCT_DECL:
	{
		// scope
		if (!DescendNameFinding(lang_stat, n->r->r, scp))
			return nullptr;
	}break;
	case node_type::N_INDEX:
	{
		// indexing
		if (IS_FLAG_OFF(n->flags, NODE_FLAGS_INDEX_IS_TYPE))
		{
			auto lhs = DescendNameFinding(lang_stat, n->l, scp);
			if (!lhs)
				return nullptr;
			auto rhs = DescendNameFinding(lang_stat, n->r, scp);

			if (!rhs)
				return nullptr;

			// slicing array
			if (n->r->type == N_BINOP && n->r->t->type == T_TWO_POINTS)
			{
				enum two_points_enum
				{
					START_AND_END,
					ONLY_END,
					ONLY_START,
					NOTHING,
				};
				own_std::vector<node*> args;

				node* ar_len_nd;

				// creating a func call with the arrays data and len
				// slice_array(ar.data, ar.len, start, end)
				//
				// array is from struct
				node* data_nd = nullptr;

				two_points_enum two_points_type;

				node* lhs_two = nullptr;
				node* rhs_two = nullptr;

				// has start and end idx
				if (n->r->l && n->r->r)
				{
					lhs_two = n->r->l;
					rhs_two = n->r->r;
				}
				// has only end idx
				else if (!n->r->l && n->r->r)
				{
					rhs_two = n->r->r;
				}
				// has only start idx
				else if (n->r->l && !n->r->r)
				{
					lhs_two = n->r->l;
				}
				// has none
				else if (!n->r->l && !n->r->r)
				{
					two_points_type = NOTHING;
				}

				type2* ar_type;
				node* new_n = n->l->NewTree(lang_stat);
				if (lhs->type.type == enum_type2::TYPE_STRUCT)
				{

					data_nd = NewBinOpNode(lang_stat,
						new_n,
						tkn_type2::T_POINT,
						NewIdentNode(lang_stat, "data", n->t));

					auto len_nd = NewBinOpNode(lang_stat,
						new_n,
						tkn_type2::T_POINT,
						NewIdentNode(lang_stat, "len", n->t));

					ar_len_nd = len_nd;

					ar_type = &lhs->type.strct->vars[0]->type;
				}
				// array is static
				else if (lhs->type.type == enum_type2::TYPE_STATIC_ARRAY)
				{
					data_nd = new_n;

					ar_len_nd = NewIntNode(lang_stat, lhs->type.ar_size, n->t);
					ar_type = lhs->type.tp;
				}
				else if (lhs->type.type == enum_type2::TYPE_STR_LIT)
				{
					data_nd = new_n;

					ar_len_nd = NewIntNode(lang_stat, lhs->type.str_len, n->t);
					ar_type = &lang_stat->u8_decl->type;
				}


				data_nd->t->line = n->t->line;

				args.emplace_back(data_nd);
				args.emplace_back(ar_len_nd);

				// has start and end idx
				if (n->r->l && n->r->r)
				{
					args.emplace_back(n->r->l);
					args.emplace_back(n->r->r);
				}
				// has only end idx
				else if (!n->r->l && n->r->r)
				{
					args.emplace_back(NewIntNode(lang_stat, 0, n->t));
					args.emplace_back(n->r->r);
				}
				// has only start idx
				else if (n->r->l && !n->r->r)
				{
					args.emplace_back(n->r->l);
					args.emplace_back(ar_len_nd);
				}
				// has none
				else if (!n->r->l && !n->r->r)
				{
					args.emplace_back(NewIntNode(lang_stat, 0, n->t));
					args.emplace_back(ar_len_nd);
				}


				node* call_nd = MakeFuncCallArgs(lang_stat, "slice_array", nullptr, args, n->t);
				call_nd->t = n->t;
				memcpy(n, call_nd, sizeof(node));


				// creating specified template types for the call
				//
				// if the type of the array is a struct we simply add a ident nd as templ type
				n->call_templates = CreateNodeFromType(lang_stat, ar_type, n->t);

				DescendNameFinding(lang_stat, n, scp);

				auto a = 0;
				//DescendNode(n, scp);
			}
			else
			{
				type2 lhs_type;
				NameFindingGetType(lang_stat, n->l, scp, lhs_type);

				if (!lhs_type.IsStrct(nullptr) && lhs_type.type != TYPE_STATIC_ARRAY)
				{
					if (lhs_type.ptr > 0)
					{
						REPORT_ERROR(n->t->line, n->t->line,
							VAR_ARGS("variable '%s' is pointer, and pointers cannot be indexed\n", lhs->name.c_str())
						);
						ExitProcess(1);
					}
					else if (lhs_type.ptr == 0)
					{
						REPORT_ERROR(n->t->line, n->t->line,
							VAR_ARGS("variable '%s' cannot be indexed\n", lhs->name.c_str())
						);
						ExitProcess(1);
					}
				}

				//creating an implicit deref for arrays that are ptrs
				if (lhs_type.ptr > 0)
				{
					int ptr = lhs_type.ptr;

					node* top_nd = n->l;
					while (ptr > 0)
					{
						top_nd = NewUnopNode(lang_stat, nullptr, T_MUL, top_nd);
						ptr--;
					}
					n->l = top_nd;
				}


				switch (lhs_type.type)
				{
				case enum_type2::TYPE_STATIC_ARRAY:
				case enum_type2::TYPE_ARRAY:
				case enum_type2::TYPE_ARRAY_DYN:
				{
					if (lhs_type.tp->type == enum_type2::TYPE_STRUCT_TYPE
						|| lhs_type.tp->type == enum_type2::TYPE_STRUCT)
					{
						ASSERT(lhs_type.tp->strct->this_decl)
							return lhs_type.tp->strct->this_decl;
					}

					return FromBuiltinTypeToDecl(lang_stat, lhs_type.tp->type);
				}break;
				case enum_type2::TYPE_STRUCT:
				{
					auto op_func = lhs_type.strct->FindOpOverload(lang_stat, overload_op::INDEX_OP);
					if (!op_func)
						return nullptr;
				}break;
				default:
					// cannot index this type
					ASSERT(false)
				}
			}

		}
		// type
		else
		{
		}
	}break;
	case node_type::N_INT:
	case node_type::N_FLOAT:
		return (decl2*)1;
	case node_type::N_TYPE:
	{
		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS))
		{
			type2 tp;
			tp.type = FromTypeToVarType(n->decl_type.type);
			scp->vars.emplace_back(NewDecl(lang_stat, "unamed param", tp));
		}
	}break;
	case node_type::N_IDENTIFIER:
	{
		auto ident = FindIdentifier(n->t->str, scp, &ret_type);
		if (ident && IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS))
		{
			scp->vars.emplace_back(NewDecl(lang_stat, "unamed param", ident->type));
		}
		/*
		if (lhs->name == "WriteFile")
			auto a = 0;
			*/

		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_REPORT_UNDECLARED_IDENTS) && !ident)
			ReportUndeclaredIdentifier(lang_stat, n->t);

		if (ident)
		{
			if (IS_FLAG_ON(ident->flags, DECL_FROM_USING))
			{
				auto new_n = ident->using_node->NewTree(lang_stat);
				new_n->flags |= NODE_FLAGS_POINT_FROM_USING;
				memcpy(n, new_n, sizeof(node));
			}
		}

		return ident;
	}break;

	case node_type::N_OP_OVERLOAD:
	{
		ret_type.type = enum_type2::TYPE_FUNC_DECL;
		node* fnode = n;

		if (n->fdecl != nullptr && IS_FLAG_ON(n->fdecl->flags, FUNC_DECL_IS_DONE))
			return (decl2*)1;

		//lang_stat->flags |= PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT;

		// checking if function is templated
		int _flags = n->l->l->l ? DONT_DESCEND_SCOPE : 0;

		if (!FunctionIsDone(lang_stat, n, given_scp, &ret_type, _flags))
		{
			lang_stat->flags &= ~PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT;
			return nullptr;
		}
		lang_stat->flags &= ~PSR_FLAGS_NO_INSTANTIATION_BUT_RET_STRCT;

		n->fdecl->flags |= FUNC_DECL_IS_DONE;

		auto self = n->fdecl->args[0];
		ASSERT(self->type.type == enum_type2::TYPE_STRUCT && self->type.ptr == 1)
			switch (n->ovrld_op)
			{
			case overload_op::INDEX_OP:
			{
				ASSERT(n->fdecl->args.size() == 2)
					auto arg2 = &n->fdecl->args[1]->type;
				ASSERT(arg2->type == enum_type2::TYPE_U64 && arg2->ptr == 0)
			}break;
			case overload_op::DEREF_OP:
			case overload_op::FOR_OP:
				ASSERT(n->fdecl->args.size() == 1)
					break;
			case overload_op::PLUS_OP:
			case overload_op::MINUS_OP:
			case overload_op::MUL_OP:
			case overload_op::ASSIGN_OP:
			case overload_op::COND_EQ_OP:
				ASSERT(n->fdecl->args.size() == 2)
					break;
			default:
				ASSERT(false)
					break;
			}

		n->fdecl->op_overload = n->ovrld_op;
		// assigning this overload function to the first arg, which should be
		// a strct ptr to struct, in case of templated strct, a ptr to the original one
		// so that structs that were devired from that one can query its "mother" struct
		// for ovoerloaded funcs
		self->type.strct->AddOpOverload(lang_stat, n->fdecl, n->ovrld_op);

		//self->type.strct->op_overloads.emplace_back(n->fdecl);

		//self->flags = DECL_NOT_DONE;
		if (n->fdecl->templates.size() == 0)
		{
			//self->flags = 0;
			CheckOverloadFunction(lang_stat, n->fdecl);
		}
		//lang_stat->cur_file->global->vars.emplace_back(n->fdecl);
	}break;
	case node_type::N_CALL:
	{
		type2 r;
		if (!CallNode(lang_stat, n, scp, &r))
			return nullptr;

	}break;
	case node_type::N_BINOP:
	{
		switch (n->t->type)
		{
		case tkn_type2::T_MINUS_EQUAL:
		case tkn_type2::T_PLUS_EQUAL:
			// %EQUAL
		case tkn_type2::T_EQUAL:
		{
			decl2* lhs = DescendNameFinding(lang_stat, n->l, scp);
			decl2* rhs = DescendNameFinding(lang_stat, n->r, scp);
			if (n->l != nullptr && !lhs)
				return nullptr;

			if (n->r != nullptr && !rhs)
				return nullptr;
			

			if ((long long)lhs != 1)
			{
				if (lhs->type.is_const)
				{
					// assigin
					if (n->l->type == node_type::N_BINOP)
					{
						if (IS_FLAG_OFF(lhs->type.flags, TYPE_CONST_WAS_DECLARED))
						{
							lhs->type.flags = TYPE_CONST_WAS_DECLARED;
							lhs->type.s64 = DescendNode(lang_stat, n->r, scp).i;
						}
					}
					else
					{
						ASSERT(IS_FLAG_OFF(lhs->type.flags, TYPE_CONST_WAS_DECLARED))
					}
				}
				else
				{
					bool is_str_lit = lhs->type.type == enum_type2::TYPE_STR_LIT && lhs->type.tp == nullptr;
					// type inference 
					if (lhs->type.type == enum_type2::TYPE_AUTO)
					{
						bool was_const = lhs->type.is_const;
						if (NameFindingGetType(lang_stat, n->r, scp, lhs->type))
						{
							if (lhs->type.type == enum_type2::TYPE_STRUCT_TYPE)
								lhs->type.type = enum_type2::TYPE_STRUCT;
							else if (lhs->type.type == enum_type2::TYPE_STATIC_ARRAY_TYPE)
								lhs->type.type = enum_type2::TYPE_STATIC_ARRAY;
							else if (lhs->type.type == enum_type2::TYPE_F32_RAW)
								lhs->type.type = enum_type2::TYPE_F32;
							else if (lhs->type.type == enum_type2::TYPE_INT)
								lhs->type.type = enum_type2::TYPE_S32;
							else if (lhs->type.type == enum_type2::TYPE_FUNC_DEF)
								ReportMessage(lang_stat, n->t, "function definition is not a value, so it cant be stored in a variable");

							n->l->r = CreateNodeFromType(lang_stat, &lhs->type, n->t);

							lhs->type.is_const = was_const;

							if (lhs->type.type == TYPE_VOID && lhs->type.ptr == 0)
							{
								ReportMessage(lang_stat, n->t, "cannot assign plain void to variables\n");
							}
							n->l->r = CreateNodeFromType(lang_stat, &lhs->type, n->t);

							lhs->type.is_const = was_const;
						}
						else
							return nullptr;

						// str lit to array
						if (is_str_lit)
						{

							// we're spliting the a := b into a : type; a = b;
							type2 u8_tp;
							memset(&u8_tp, 0, sizeof(type2));
							u8_tp.type = enum_type2::TYPE_U8;
							type_struct2* ret_struct;

							bool val = InstantiateArFromType(lang_stat, u8_tp, scp, &lhs->type, &ret_struct, n);
							ASSERT(val)

								lhs->type.type = enum_type2::TYPE_STRUCT;
							lhs->type.strct = ret_struct;

						}
						else
						{

						}
					}

					// this decl will be used later at the bytecode generation
					// to tell at what offset the array is
					if (IsNodeUnop(n->r, T_DOLLAR))
					{
						n->r->flags &= ~NODE_FLAGS_AR_LIT_ANON;
						n->r->ar_lit_decl = lhs;
					}

					node* equal_stmnt = n;

					// if we have a var decl before the assignment, and the decl is a struct, we have to split the 
					// assignment and the decl into two different stmnts
					if (IsNodeOperator(n->l, tkn_type2::T_COLON))
					{
						// lhs = rhs;	
						auto new_equal = NewBinOpNode(lang_stat,
							n->l->l,
							tkn_type2::T_EQUAL,
							new_node(lang_stat, n->r));

						new_equal->t->line = n->t->line;

						// transforming the nd into a stmnt
						n->type = node_type::N_STMNT;
						memcpy(n->r, new_equal, sizeof(node));
						equal_stmnt = n->r;
					}
					bool is_bool = lhs->type.type == TYPE_BOOL;
					bool is_struct_val = lhs->type.type == TYPE_STRUCT && lhs->type.ptr == 0
						&& IsNodeUnop(equal_stmnt->r, T_MUL);

					if (is_bool && IS_FLAG_OFF(n->flags, NODE_FLAGS_IS_PROCESSED2) && equal_stmnt->r->type == N_BINOP && equal_stmnt->r->t->type != T_POINT)
					{
						node* bool_expr = CreateBoolExpression(lang_stat, equal_stmnt->l, equal_stmnt->r, scp);
						memcpy(equal_stmnt, bool_expr, sizeof(node));
					}
					else if (is_struct_val)
					{
						decl2* memcpy_func = FindIdentifier("memcpy", scp, &ret_type);
						if (!memcpy_func)
							return nullptr;
						auto arg1 = NewUnopNode(lang_stat, nullptr, T_AMPERSAND, equal_stmnt->l);
						auto arg2 = NewUnopNode(lang_stat, nullptr, T_AMPERSAND, equal_stmnt->r);

						lang_stat->void_decl->type.ptr++;
						auto void_nd = CreateNodeFromType(lang_stat, &lang_stat->void_decl->type, n->t);

						auto casted1 = NewTypeNode(lang_stat, void_nd, N_CAST, arg1, n->t);
						auto casted2 = NewTypeNode(lang_stat, void_nd, N_CAST, arg2, n->t);

						lang_stat->void_decl->type.ptr--;

						auto call_nd = NewThreeArgNd(lang_stat, "memcpy", casted1, casted2, NewIntNode(lang_stat, lhs->type.strct->size, n->t));

						memcpy(equal_stmnt, call_nd, sizeof(node));

						DescendNameFinding(lang_stat, equal_stmnt, scp);

						DescendNode(lang_stat, equal_stmnt, scp);
					}
				}
			}
		}break;
		case tkn_type2::T_POINT:
		{
			type2 tp;
			auto ret = PointLogic(lang_stat, n, scp, &ret_type);
			if (!ret)
				return nullptr;
			return ret;
		}break;
		//%COLON
		case tkn_type2::T_COLON:
		{
			own_std::string decl_name = n->l != nullptr ? n->l->t->str.substr() : std::to_string((long long)n).c_str();


#ifdef DEBUG_NAME
			dbg_name_ dbg_name;
			memset(&dbg_name, 0, sizeof(dbg_name_));
			dbg_name.success = &success;
			dbg_name.set_not_found = true;
			dbg_name.to_set = n;
			dbg_name.nd = n;
#endif
			// creating a new node for an implied name
			if (n->l == nullptr)
			{
				n->l = new_node(lang_stat, n->t);
				n->l->type = node_type::N_IDENTIFIER;
				n->l->t = (token2*)AllocMiscData(lang_stat, sizeof(token2));
				memset(n->l->t, 0, sizeof(token2));
				n->l->t->str = decl_name.substr();
			}

			auto decl_exist = FindIdentifier(decl_name, scp, &ret_type, FIND_IDENT_FLAGS_RET_IDENT_EVEN_NOT_DONE);

			/*
			if(IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_REPORT_UNDECLARED_IDENTS) && !decl_exist)
				//REPORT ERROR: undeclared_identifier
				ASSERT(false)
				*/

				// another func with the same name found
			if (decl_exist && decl_exist->type.type == enum_type2::TYPE_FUNC && decl_exist->decl_nd != n)
			{
				ASSERT(decl_exist->decl_nd);
				if (decl_exist->decl_nd->type == N_OP_OVERLOAD)
				{
					ReportMessage(lang_stat, n->t, "At the moment we only allow overloading of operators");
					ExitProcess(1);
				}

				TransformSingleFuncToOvrlStrct(lang_stat, decl_exist);
			}
			func_overload_strct* overload_strct = nullptr;

			// assigning to overloaded funcs
			if (decl_exist && decl_exist->type.type == enum_type2::TYPE_OVERLOADED_FUNCS)
			{

				overload_strct = decl_exist->type.overload_funcs;
			}

			if (decl_exist != nullptr && IS_FLAG_OFF(decl_exist->flags, DECL_NOT_DONE))
			{
				// variable was already declared
				if (decl_exist->decl_nd != n)
				{
					ReportDeclaredTwice(lang_stat, n, decl_exist);
					/*
					int decl_exist_ln = decl_exist->decl_nd->t->line;
					REPORT_ERROR(n->l->t->line, n->l->t->line_offset,
						VAR_ARGS("variable '%s' declred here:\n\n%d|%s\n\nHas the same name as this one\n",
							decl_exist->name.c_str(), decl_exist_ln, GetFileLn(decl_exist_ln - 1)
						)
					)
					*/
					ExitProcess(1);
				}
				return decl_exist;
			}

			bool can_declare = false;

			type2 ret_type;
			memset(&ret_type, 0, sizeof(ret_type));

			// if type was not ommited
			if (n->r)
			{
				switch (n->r->type)
				{
				case node_type::N_KEYWORD:
				{
					switch (n->r->kw)
					{
					case KW_REL:
					{
						if (!NameFindingGetType(lang_stat, n->r, scp, ret_type))
							return nullptr;

					}break;
					default:
						ASSERT(false)
					}
				}break;
				case node_type::N_CALL:
				{
					node* ncall = n->r;
					if (!CallNode(lang_stat, n->r, scp, &ret_type))
						return nullptr;

				}break;;
				case node_type::N_IDENTIFIER:
				case node_type::N_STR_LIT:
				case node_type::N_TYPE:
				case node_type::N_CONST:
				case node_type::N_UNOP:
				{
					bool was_call = CMP_NTYPE(n->r, N_CALL);
					bool was_ptr = CMP_NTYPE(n->r, N_UNOP) && n->r->t->type == T_MUL;

					bool is_ret_nil = IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_DONT_DECLARE_VARIABLES);
					int flags_to_get_type = was_ptr * NM_FND_TP_RETURN_EVEN_IDENT_NOT_DONE * !is_ret_nil;
					//DescendNameNM_FND_TP_RETURN_EVEN_IDENT_NOT_DONE
					if (!NameFindingGetType(lang_stat, n->r, scp, ret_type, flags_to_get_type))
						return nullptr;

					if (ret_type.type == TYPE_STRUCT_TYPE && ret_type.strct->templates.size() > 0 && IS_FLAG_OFF(ret_type.flags, TYPE_NOT_INSTANTIATED_YET))
					{
						auto strct = ret_type.strct;
						REPORT_ERROR(n->r->t->line, n->r->t->line_offset,
							VAR_ARGS("This struct declaration must have templates! Check out its header:\n\n %s\n",
								GetFileLn(lang_stat, strct->strct_node->t->line - 1, strct->from_file)
							)
						);
						ExitProcess(1);
					}
					// declaring an iterator from array
				}break;
				case node_type::N_FUNC_DEF:
				{
					ret_type.type = enum_type2::TYPE_FUNC_DEF;
					lang_stat->flags |= PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS;
					if (!FunctionIsDone(lang_stat, n->r, scp, &ret_type, DONT_DESCEND_SCOPE))
					{
						lang_stat->flags &= ~PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS;
						return nullptr;
					}
					lang_stat->flags &= ~PSR_FLAGS_DECLARE_ONLY_TYPE_PARAMTS;
				}break;
				case node_type::N_FUNC_DECL:
				{
					if (!FunctionIsDone(lang_stat, n->r, scp, &ret_type, 0))
						return nullptr;
					ret_type.type = enum_type2::TYPE_FUNC_DECL;

				}break;
				case node_type::N_ENUM_DECL:
				{
					ret_type.type = enum_type2::TYPE_ENUM_DECL;
					node* enode = n->r;

					if (!decl_exist)
					{

						decl_exist = DeclareDeclToScopeAndMaybeToFunc(lang_stat, decl_name.substr(), &ret_type, scp, n);
						ret_type.type = enum_type2::TYPE_ENUM_TYPE;
						decl_exist->flags |= DECL_NOT_DONE;
						decl_exist->type = ret_type;
					}

					scope* child_scp = GetScopeFromParent(lang_stat, enode->r, given_scp);

					decl_exist->type.scp = child_scp;

					lang_stat->flags |= PSR_FLAGS_ON_ENUM_DECL;
					own_std::vector<comma_ret> names;
					DescendComma(lang_stat, enode->r->r, child_scp, names);
					lang_stat->flags &= PSR_FLAGS_ON_ENUM_DECL;

					int cur_idx = 0;

					FOR_VEC(n, names)
					{
						auto new_decl = (decl2*)AllocMiscData(lang_stat, sizeof(decl2));

						memset(new_decl, 0, sizeof(decl2));

						switch (n->type)
						{
						case COMMA_RET_EXPR:
						{
							if (!IsNodeOperator(n->n, tkn_type2::T_EQUAL))
								ReportMessage(lang_stat, n->n->t, "Only assignment expression is allowed for enum");

							// rhs must be know at compile time
							auto rhs_tp = DescendNode(lang_stat, n->n->r, scp);

							if (rhs_tp.type != enum_type2::TYPE_INT)
								ReportMessage(lang_stat, n->n->r->t, "Value must be know at compile time");

							cur_idx = GetExpressionVal(n->n->r, scp);
							new_decl->name = n->n->l->t->str.substr();
						}break;
						case COMMA_RET_IDENT:
						{
							new_decl->name = n->decl.name.substr();
						}break;
						default:
							ASSERT(false)
						}

						new_decl->type.type = enum_type2::TYPE_ENUM_IDX_32;

						new_decl->type.from_enum = decl_exist;
						new_decl->type.e_idx = cur_idx;

						child_scp->vars.emplace_back(new_decl);
						cur_idx++;
					}
					ret_type.scp = child_scp;
					decl_exist->type.e_decl = decl_exist;
					EnumToTypeSect(lang_stat, decl_exist->name, child_scp);
					own_std::string final_name = own_std::string("$$enum ") + decl_exist->name;
					// inserting an symbol for the type
					//
					lang_stat->type_sect_syms.emplace_back(
						machine_sym(lang_stat, SYM_TYPE_DATA, (unsigned int)lang_stat->type_sect.size(), (char*)std_str_to_heap(lang_stat, &final_name))
					);
				}break;
				case node_type::N_UNION_DECL:
				case node_type::N_STRUCT_DECL:
				{
					ret_type.type = enum_type2::TYPE_STRUCT_DECL;
					node* snode = n->r;

					//snode->tstrct = snode->tstrct == nullptr ? new type_struct2() : snode->tstrct;
					if (snode->tstrct == nullptr)
					{
						snode->tstrct = (type_struct2*)AllocMiscData(lang_stat, sizeof(type_struct2));
						memset(snode->tstrct, 0, sizeof(type_struct2));
					}

					//memset(new_decl, 0, sizeof(decl2));

					snode->tstrct->name = decl_name;


					scope* child_scp = GetScopeFromParent(lang_stat, snode->r, given_scp);
					child_scp->flags |= SCOPE_INSIDE_STRUCT;
					child_scp->tstrct = snode->tstrct;

					if (!decl_exist)
					{

						ret_type.strct = snode->tstrct;

						decl_exist = DeclareDeclToScopeAndMaybeToFunc(lang_stat, decl_name.substr(), &ret_type, scp, n);
						ret_type.type = enum_type2::TYPE_STRUCT_TYPE;
						decl_exist->flags |= DECL_NOT_DONE;
						decl_exist->type = ret_type;
						ret_type.strct->this_decl = decl_exist;
					}


					auto tstrct = snode->tstrct;
					tstrct->strct_node = snode;
					tstrct->scp = child_scp;
					ret_type.strct = tstrct;

					if (snode->l == nullptr)
					{
						// scope
						if (!DescendNameFinding(lang_stat, snode->r->r, child_scp))
							return nullptr;

						INSERT_VEC((tstrct->vars), child_scp->vars);

						// check self ref
						FOR_VEC(v, tstrct->vars)
						{
							auto vtp = &(*v)->type;
							if (IsSameStrct(vtp, tstrct)
								|| (vtp->type == TYPE_STATIC_ARRAY && IsSameStrct(vtp->tp, tstrct) && vtp->ptr == 0)
								)
							{
								ReportMessage(lang_stat, (*v)->decl_nd->t, "Cannot self declare the struct");
								ExitProcess(1);
							}
						}

						if (n->r->type == node_type::N_STRUCT_DECL)
							tstrct->size = SetVariablesAddress(&tstrct->vars, 0, &tstrct->biggest_type);

						// union
						else
						{
							int biggest_var = 0;
							tstrct->biggest_type = 0;
							FOR_VEC(v, child_scp->vars)
							{
								int cur_vsize = GetTypeSize(&(*v)->type);
								if (cur_vsize > biggest_var)
									biggest_var = cur_vsize;
								tstrct->biggest_type = CheckBiggestType(&(*v)->type, tstrct->biggest_type);
							}
							tstrct->size = biggest_var;
						}

					}
					// if the struct has templates we wanto to just declare it
					// and later we get the types
					else
					{
						if (tstrct->templates.size() == 0)
							DescendTemplates(snode->l, scp, &tstrct->templates);

						tstrct->flags |= TP_STRCT_TEMPLATED;
					}

					tstrct->from_file = lang_stat->cur_file;

					if (decl_exist->type.type == TYPE_STRUCT_TYPE && decl_exist->type.strct->templates.size() == 0)
					{
						decl_exist->type.strct->ToTypeSect(&lang_stat->type_sect_str_table2, &lang_stat->type_sect, &lang_stat->type_sect_str_tbl_sz);
						own_std::string final_name = own_std::string("$$") + decl_exist->type.strct->name;
						// inserting an symbol for the type
						//
						lang_stat->type_sect_syms.emplace_back(
							machine_sym(lang_stat, SYM_TYPE_DATA, (unsigned int)lang_stat->type_sect.size(), (char*)std_str_to_heap(lang_stat, &final_name))
						);
					}
					//decl_exist->flags &= ~DECL_NOT_DONE;
					//tstrct->flags |= TP_STRCT_STRUCT_NODE;
					can_declare = true;

				}break;
				case node_type::N_INDEX:
				{
					if (!NameFindingGetType(lang_stat, n->r, scp, ret_type))
						return nullptr;
				}break;
				case node_type::N_BINOP:
				{
					switch (n->r->t->type)
					{
					case tkn_type2::T_COLON:
					{
						node* cnode = n->r->r;
						switch (cnode->type)
						{
						case node_type::N_STRUCT_DECL:
						{
							//cnode->tstrct = cnode->tstrct == nullptr ? new type_struct2() : cnode->tstrct;
							if (cnode->tstrct == nullptr)
							{
								cnode->tstrct = (type_struct2*)AllocMiscData(lang_stat, sizeof(type_struct2));
								memset(cnode->tstrct, 0, sizeof(type_struct2));
							}

						}break;
						case node_type::N_FUNC_DECL:
						{
							// "forward declaring" the function in case of recursion
							if (cnode->fdecl == nullptr && cnode->flags == 0 && !cnode->l->l->l)
							{
								cnode->fdecl = (func_decl*)AllocMiscData(lang_stat, sizeof(func_decl));
								memset(cnode->fdecl, 0, sizeof(func_decl));
								cnode->fdecl->from_file = lang_stat->cur_file;

								ret_type.type = TYPE_FUNC_TYPE;
								ret_type.fdecl = cnode->fdecl;
								decl_exist = DeclareDeclToScopeAndMaybeToFunc(lang_stat, decl_name.substr(), &ret_type, scp, n);
								decl_exist->flags = DECL_NOT_DONE;
							}
						}break;
						}
						auto colon = DescendNameFinding(lang_stat, n->r, scp);

						if (colon == nullptr)
						{
#ifdef DEBUG_NAME
							if (decl_name == "main")
							{
								//lang_stat->not_found_nd = n;
								//lang_stat->not_founds.emplace_back(plang_stat->not_found_nd);
								auto st = FindIdentifier("string", scp, &ret_type);
							}
#endif
							return nullptr;
						}
						colon->decl_nd = n;
						ret_type = colon->type;
					}break;
					// TUPLE
					case T_COMMA:
					{
						if (!DescendNameFinding(lang_stat, n->r, scp))
							return nullptr;
						//snode->tstrct = snode->tstrct == nullptr ? new type_struct2() : snode->tstrct;

						decl2 *tuple = CreateStructTuple(lang_stat, n->r, scp);

						n->r->type = N_IDENTIFIER;
						n->r->t->str = own_std::string(tuple->name);

						type_struct2 *tstrct = tuple->type.strct;

						ret_type.strct = tstrct;
						ret_type.type = TYPE_STRUCT_TYPE;
					}break;
					default:
					{
						ASSERT(false)
					}break;
					}
				}break;
				default:
				{
					ASSERT(false)
				}break;
				}
			}

			if (decl_exist)
				decl_exist->flags &= ~DECL_NOT_DONE;

			if (IS_FLAG_OFF(lang_stat->flags, PSR_FLAGS_DONT_DECLARE_VARIABLES) && decl_exist == nullptr)
			{
				// adding the new func to the overloaded array
				if (overload_strct)
				{
					decl_name = MangleFuncNameWithArgs(lang_stat, ret_type.fdecl, decl_name, 0);
					ret_type.fdecl->name = decl_name;
					overload_strct->fdecls.emplace_back(ret_type.fdecl);
				}
				// we only want to declare functions and structs. We're not interested in its contents
				return DeclareDeclToScopeAndMaybeToFunc(lang_stat, decl_name.substr(), &ret_type, scp, n);
			}
			else
			{
				if (decl_exist)
					return decl_exist;
				else
					return (decl2*)1;
			}
		}break;

		default:
		{
			node* cur = n->l;
			while (cur->l && cur->l->type == N_BINOP)
			{
				cur = cur->l;
			}
			if (cur != nullptr)
				DescendNameFinding(lang_stat, cur, scp);

			if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp))
				return nullptr;

			if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp))
				return nullptr;

		}break;
		}
	}break;
	case node_type::N_FOR:
	{
		if (!n->r->scp)
			n->r->scp = GetScopeFromParent(lang_stat, n->r, scp);
		decl2* var_decl = nullptr;
		node* for_nd = n;

		scope* for_scope = n->r->scp;


		if (IsKeyword(n->l, KW_REV))
			n = n->l->r;
		else
			n = n->l;

		if (IsNodeOperator(n, T_IN) && !IsNodeOperator(n->l, T_COLON))
		{
			ASSERT(n->l->type == N_IDENTIFIER);
			var_decl = DescendNameFinding(lang_stat, n->l, for_scope);
			if (!var_decl)
			{
				ret_type = DescendNode(lang_stat, n->r, for_scope);
				if (ret_type.type == TYPE_STATIC_ARRAY)
				{
					ret_type = *ret_type.tp;
					ret_type.type = FromVarTypeToType(ret_type.type);
					ret_type.ptr++;
				}
				if (ret_type.type == TYPE_INT)
					ret_type.type = TYPE_S32_TYPE;

				var_decl = DeclareDeclToScopeAndMaybeToFunc(lang_stat, n->l->t->str, &ret_type, for_scope);
			}
		}

		if(!DescendNameFinding(lang_stat, n->l, for_scope))
			return nullptr;



		//DescendNameFinding(lang_stat, n->l, for_scope);
		if (!DescendNameFinding(lang_stat, for_nd->r, for_scope))
			return nullptr;
		/*
		lang_stat->flags |= PSR_FLAGS_INSIDER_FOR_DECL;
		if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp) && scp->parent != nullptr)
		{
			lang_stat->flags &= ~PSR_FLAGS_INSIDER_FOR_DECL;
			return nullptr;
		}
		lang_stat->flags &= ~PSR_FLAGS_INSIDER_FOR_DECL;
		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp) && scp->parent != nullptr)
			return nullptr;
			*/


			//ASSERT(it.type == enum_type2::TYPE_STRUCT && it.strct->original_strct->name == "iterator")

	}break;
	case node_type::N_SCOPE:
	{
		if (!n->scp)
			n->scp = GetScopeFromParent(lang_stat, n, scp);

		scp = n->scp;

		if (!n->r->l && !n->r->r)
			return (decl2*)1;

		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp) && scp->parent != nullptr)
		{
			return nullptr;
		}
	}break;
	case node_type::N_WHILE:
	{
		//scp = GetScopeFromParent(n, given_scp);
		if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp) && scp->parent != nullptr)
			return nullptr;
		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp) && scp->parent != nullptr)
			return nullptr;
	}break;
	case node_type::N_ELSE:
	{
		//scp = GetScopeFromParent(n, given_scp);
		/*
		if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp) && scp->parent != nullptr)
			return nullptr;
			*/
		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp) && scp->parent != nullptr)
			return nullptr;
	}break;
	case node_type::N_ELSE_IF:
	case node_type::N_IF:
	{
		//scp = GetScopeFromParent(n, given_scp);
		if (n->l->l != nullptr && !DescendNameFinding(lang_stat, n->l->l, scp) && scp->parent != nullptr)
			return nullptr;
		if (n->l->r != nullptr && !DescendNameFinding(lang_stat, n->l->r, scp) && scp->parent != nullptr)
			return nullptr;



		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp) && scp->parent != nullptr)
			return nullptr;
	}break;
	case node_type::N_STMNT:
	{

		//	decl2 *ret_decl = nullptr;
		//	DescendStmntMode(n, scp, DMODE_DNAME, (void *)&ret_decl);
		//	return ret_decl;

		own_std::vector<node*> node_stack;
		own_std::vector<scope*> scp_stack;
		node* cur_node = n;
		while (cur_node->l->type == node_type::N_STMNT)
		{
			node_stack.emplace_back(cur_node);
			/*
			if (IS_FLAG_ON(cur_node->flags, NODE_FLAGS_IS_SCOPE))
			{
				scp_stack.emplace_back(scp);
				scp = GetScopeFromParent(cur_node, scp);
			}
			*/
			cur_node = cur_node->l;
		}

		int cur_scp_idx = scp_stack.size() - 1;

		int size = node_stack.size();
		while (true)
		{
			lang_stat->cur_stmnt = cur_node;



			if (cur_node->l != nullptr && IS_FLAG_OFF(cur_node->l->flags, NODE_FLAGS_IS_PROCESSED))
			{
				auto ret_decl = DescendNameFinding(lang_stat, cur_node->l, scp);
				// we only want to return if we're not in global scope
				if (!ret_decl && IS_FLAG_OFF(scp->flags, SCOPE_IS_GLOBAL))
					return nullptr;

#ifdef DEBUG_GLOBAL_NOT_FOUND
				if (!ret_decl && IS_FLAG_ON(scp->flags, SCOPE_IS_GLOBAL))
				{
					lang_stat->flags |= PSR_FLAGS_SOMETHING_IN_GLOBAL_NOT_FOUND;
					lang_stat->global_decl_not_found.emplace_back(cur_node->r);
					//return nullptr;
				}
#endif

				cur_node->l->flags &= ~NODE_FLAGS_IS_PROCESSED;
			}
			if (cur_node->r != nullptr && IS_FLAG_OFF(cur_node->r->flags, NODE_FLAGS_IS_PROCESSED))
			{
				auto ret_decl = DescendNameFinding(lang_stat, cur_node->r, scp);
				if (!ret_decl && IS_FLAG_OFF(scp->flags, SCOPE_IS_GLOBAL))
					return nullptr;

#ifdef DEBUG_GLOBAL_NOT_FOUND
				if (!ret_decl && IS_FLAG_ON(scp->flags, SCOPE_IS_GLOBAL))
				{
					lang_stat->flags |= PSR_FLAGS_SOMETHING_IN_GLOBAL_NOT_FOUND;
					//lang_stat->global_decl_not_found = cur_node->r;
					lang_stat->global_decl_not_found.emplace_back(cur_node->r);
					//return nullptr;
				}
#endif


				cur_node->r->flags &= ~NODE_FLAGS_IS_PROCESSED;
			}

			size--;

			cur_node->flags |= NODE_FLAGS_IS_PROCESSED;

			if (size < 0)
				break;

			//if (IS_FLAG_ON(cur_node->flags, NODE_FLAGS_IS_SCOPE))
				//scp = scp_stack[cur_scp_idx--];



			cur_node = *(node_stack.end() - 1);
			node_stack.pop_back();

		}

	}break;
	case N_ARRAY_CONSTRUCTION:
	{
		if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp))
			return nullptr;

		if (!n->scp)
		{
			n->exprs = (own_std::vector<comma_ret> *)AllocMiscData(lang_stat, sizeof(own_std::vector<comma_ret>));
			n->scp = scp;
			DescendComma(lang_stat, n->r->r, scp, *n->exprs);
		}
		if (!n->l->l)
		{
			n->l->l = NewIntNode(lang_stat, n->exprs->size(), n->t);
		}
		NameFindingGetType(lang_stat, n->l, scp, ret_type);

		

		FOR_VEC(c, *n->exprs)
		{
			if (!DescendNameFinding(lang_stat, c->n, n->scp))
				return 0;
			//if()
		}
		
		int a = 0;

	}break;
	case N_STRUCT_CONSTRUCTION:
	{
		if (n->l == nullptr)
			return nullptr;

		//decl2* strct = DescendNameFinding(lang_stat, n->l, scp);
		decl2* strct = FindIdentifier(n->l->t->str, scp, &ret_type);
		NameFindingGetType(lang_stat, n->l, scp, ret_type);

		if (!n->scp)
		{
			scope* strct_scp = strct->type.strct->scp;
			scope *new_scp = NewScope(lang_stat, scp);
			new_scp->vars = strct_scp->vars;

			n->exprs = (own_std::vector<comma_ret> *)AllocMiscData(lang_stat, sizeof(own_std::vector<comma_ret>));
			n->scp = new_scp;
			node* descend = n->r;
			if (n->r->type == N_SCOPE)
				descend = descend->r;

			DescendComma(lang_stat, descend, new_scp, *n->exprs);
		}
		

		if (IS_FLAG_ON(strct->type.strct->flags, TP_STRCT_TUPLE))
		{
			FOR_VEC(c, *n->exprs)
			{
				if (!DescendNameFinding(lang_stat, c->n, n->scp))
					return 0;
				//if()
			}
		}
		else
		{
			FOR_VEC(c, *n->exprs)
			{
				if (!DescendNameFinding(lang_stat, c->n->l, n->scp))
					return 0;
				//if()
			}
		}
		
		int a = 0;
	}break;

	default:
	{

		if (n->l != nullptr && !DescendNameFinding(lang_stat, n->l, scp) && scp->parent != nullptr)
			return nullptr;
		if (n->r != nullptr && !DescendNameFinding(lang_stat, n->r, scp) && scp->parent != nullptr)
			return nullptr;
	}break;
	}
	success = true;
	return (decl2*)1;
}
node* CreateTreeArgs(lang_state *lang_stat, own_std::vector<node*>& args, node_type n_tp, tkn_type2 t_tp)
{
	node* n_args = nullptr;
	if (args.size() > 1)
	{
		n_args = NewBinOpNode(lang_stat, args[0], t_tp, args[1]);
		n_args->type = n_tp;


		for (auto a = args.begin() + 2; a < args.end(); a++)
		{
			auto new_n = NewBinOpNode(lang_stat, n_args, t_tp, *a);
			new_n->type = n_tp;
			n_args = new_n;
		}
	}
	else if (args.size() == 1)
		n_args = args[0];
	return n_args;
}

node* MakeFuncCallArgs(lang_state *lang_stat, own_std::string fname, node* ref, own_std::vector<node*>& args, token2 *t)
{
	node* n_args = new_node(lang_stat, t);

	if (ref)
	{
		n_args->r = ref->NewTree(lang_stat);
		n_args->t->type = tkn_type2::T_AMPERSAND;
		n_args->type = node_type::N_UNOP;
	}


	if (args.size() > 1)
	{
		n_args = NewBinOpNode(lang_stat, args[0], tkn_type2::T_COMMA, args[1]);

		for (auto a = args.begin() + 2; a < args.end(); a++)
		{
			auto new_n = NewBinOpNode(lang_stat, n_args, tkn_type2::T_COMMA, *a);
			n_args = new_n;
		}
	}
	else if (args.size() == 1)
		n_args = args[0];


	auto new_n = NewTypeNode(lang_stat, 
		NewIdentNode(lang_stat, fname, t),
		node_type::N_CALL,
		n_args, t);

	return new_n;
	//FREE? free(n)
}
void MakeRelPtrDerefFuncCall(lang_state *lang_stat, func_decl* op_func, node* n)
{
	if (n->type != N_CALL)
	{
		n->r = n->NewTree(lang_stat);
		n->t->type = tkn_type2::T_AMPERSAND;
		n->type = node_type::N_UNOP;
	}

	auto new_n = NewTypeNode(lang_stat, 
		NewIdentNode(lang_stat, op_func->name, n->t),
		node_type::N_CALL,
		new_node(lang_stat, n), n->t);

	memcpy(n, new_n, sizeof(node));
	//FREE? free(n)
}

void MaybeCreateCast(lang_state *lang_stat, node* ln, node* rn, type2* lp, type2* rp)
{
	bool static_ar = lp->type == TYPE_STATIC_ARRAY && rp->type == TYPE_STATIC_ARRAY_TYPE;
	// creating a cast for types that are different
	if (lp->type != TYPE_FUNC_PTR && lp->type != TYPE_ENUM && rp->type != TYPE_STR_LIT && lp->type != rp->type && rp->type != TYPE_INT && rp->type != TYPE_F32_RAW && !static_ar)
	{
		auto t_nd = CreateNodeFromType(lang_stat, lp, ln->t);
		auto new_nd = NewTypeNode(lang_stat, t_nd, N_CAST, new_node(lang_stat, rn), ln->t);
		new_nd->t = rn->t;
		//n>r = new_nd;
		memcpy(rn, new_nd, sizeof(node));
	}
}
int Return5()
{
	return 5;
}
own_std::string StringifyNode(node* n);
void ModifyFuncDeclToName(lang_state *lang_stat, func_decl *fdecl, node *n, scope *scp)
{
	lang_stat->func_ptrs_decls.emplace_back(fdecl);

	node* old_fnode = fdecl->func_node;
	node* new_fnode = new_node(lang_stat, fdecl->func_node);
	fdecl->func_node = new_fnode;
	
	char buffer[64];
	snprintf(buffer, 64, "func_ptr_decl%d", rand() % 100000);
	fdecl->name = buffer;
	type2 ftype;
	ftype.type = TYPE_FUNC;
	ftype.fdecl = fdecl;

	auto fptr_decl = NewDecl(lang_stat, buffer, ftype);
	fdecl->this_decl = fptr_decl;
	scp->vars.emplace_back(fptr_decl);
	lang_stat->funcs_scp->vars.emplace_back(fptr_decl);


	memcpy(n, NewIdentNode(lang_stat, buffer, n->t), sizeof(node));
	//lang_stat->roo.emplace_back(fdecl);
}
void ModifyNodeIntOrFloat(type2 &ret_type, node *n)
{
	if (ret_type.type == enum_type2::TYPE_INT)
	{
		n->type = N_INT;
		n->t->i = ret_type.i;
		//ret_type.i *= -1;
	}
	else if (ret_type.type == enum_type2::TYPE_F32_RAW)
	{
		n->type = N_FLOAT;
		//ret_type.f = -1;
		n->t->f = ret_type.f;
	}
}
// $DescendNode
type2 DescendNode(lang_state *lang_stat, node* n, scope* given_scp)
{
	char msg_hdr[256];

	scope* scp = given_scp;
	type2 ret_type = {};
	message msg;

	memset(&msg, 0, sizeof(msg));
	memset(&ret_type, 0, sizeof(ret_type));

	msg.scp = scp;
	msg.n = n;
	msg.stmnt = lang_stat->cur_stmnt;

	bool is_cond_eq = false;

	if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_SCOPE))
	{
		//scp = GetScopeFromParent(n, given_scp);
	}

	switch (n->type)
	{
	case node_type::N_APOSTROPHE:
	{
		ret_type.type = enum_type2::TYPE_CHAR;
		//ret_type.ptr = 1;
	}break;;
	case node_type::N_STR_LIT:
	{
		ret_type.type = enum_type2::TYPE_STR_LIT;
		//ret_type.ptr = 1;
	}break;;
	case node_type::N_ELSE:
	{

		//checking if the scope isn't empty and descending it
		if (n->r && n->r->r != nullptr)
		{
			DescendNode(lang_stat, n->r, scp);
		}
	}break;
	case node_type::N_SCOPE:
	{
		if (!n->scp)
			n->scp = GetScopeFromParent(lang_stat, n, scp);

		scp = n->scp;

		if (!n->r->l && !n->r->r)
			break;
		//return ret_type;

		DescendNode(lang_stat, n->r, scp);
	}break;
	case node_type::N_FOR:
	{
		//scp = GetScopeFromParent(n->r, given_scp);
		if (IsNodeOperator(n->l, T_IN) && n->l->r->type == N_IDENTIFIER)
		{
			type2 rhs = DescendNode(lang_stat, n->l->r, scp);

			if (rhs.type == TYPE_STATIC_ARRAY)
			{
				node* len_nd = NewIntNode(lang_stat, rhs.i, n->t);
				node* zero_nd = NewIntNode(lang_stat, 0, n->t);
				node* index_start_nd = NewTypeNode(lang_stat, new_node(lang_stat, n->l->r), N_INDEX, zero_nd, n->t);
				node* index_end_nd = NewTypeNode(lang_stat, new_node(lang_stat, n->l->r), N_INDEX, len_nd, n->t);

				node* two_points_bin_nd = NewBinOpNode(lang_stat, index_start_nd, T_TWO_POINTS, index_end_nd);
				memcpy(n->l->r, two_points_bin_nd, sizeof(node));
			}
			else
			{
				ASSERT(0)
			}
		
		}
		/*
		else
		{

			own_std::string iterated = n->l->l->t->str;

			type2 ar_type;

			// check if iterated was already declared
			auto decl_exist = FindIdentifier(iterated, scp, &ar_type);
			if (decl_exist)
			{
				ReportDeclaredTwice(lang_stat, n->l->l, decl_exist);
			}

			NameFindingGetType(lang_stat, n->l->r, scp, ar_type);

			ASSERT(n->l->type == node_type::N_BINOP)

				ASSERT(ar_type.ptr == 0)

				own_std::string ar_type_str = StringifyNode(n->l->r);

			own_std::string iterator = iterated + "iterator";
			own_std::string ar_str = n->l->r->t->str;

			char code[256];
			char* it_name = (char*)iterator.c_str();
			char* ar_name = (char*)ar_str.c_str();
			char* itrd = (char*)iterated.c_str();



			snprintf(code, 512, "\
				%s:= it_next(&%s);\n\
				if !%s break;\n\
			", itrd, it_name, itrd);
			node* cond_break = ParseString(lang_stat, code, n->t->line);

			node* while_n = nullptr;
			node* top_n = nullptr;
			if (ar_type.type == enum_type2::TYPE_STRUCT)
			{
				auto op_func = ar_type.strct->FindOpOverload(lang_stat, overload_op::FOR_OP);
				if (!op_func)
				{
					REPORT_ERROR(n->t->line, n->t->line_offset,
						VAR_ARGS("struct '%s' doesn't have a iterator overload:\n", ar_type.strct->name.c_str())
					)

						ExitProcess(1);
				}
				char* func_name = (char*)op_func->name.c_str();
				ASSERT(op_func)

					snprintf(code, 512, "\
				%s := %s(&%s);\
				while 1\n\
				{\n\
				}", it_name, func_name, ar_type_str.c_str(),
						it_name, ar_name);

				node* scope_n = ParseString(lang_stat, code, n->t->line);
				top_n = scope_n;
				while_n = scope_n->r->r;
			}
			else
			{
				snprintf(code, 512, "\
				%s : iterator(&%s);\
				it_init(&%s, &%s);\
				while 1\n\
				{\n\
				}", it_name, ar_type_str.c_str(),
					it_name, ar_name);

				node* scope_n = ParseString(lang_stat, code, n->t->line);
				top_n = scope_n;
				while_n = scope_n->r;
			}

			node* new_stmnt = new_node(lang_stat, n->t);
			CREATE_STMNT(new_stmnt, cond_break, n->r);

			node* new_stmnt2 = new_node(lang_stat, n->t);
			CREATE_STMNT(new_stmnt2, new_stmnt, nullptr);

			while_n->r = new_stmnt2;
			//SetNodeScopeIdx(&scope_n->flags, scp->children.size());
			top_n->type = node_type::N_DESUGARED;
			DescendNameFinding(lang_stat, top_n, scp);

			DescendNode(lang_stat, top_n, scp);
			top_n->flags = n->flags;

			memcpy(n, top_n, sizeof(node));
		}
		*/
		DescendNode(lang_stat, n->r, scp);

	}break;
	case node_type::N_WHILE:
	{
		// cond
		if (n->l != nullptr)
		{
			DescendNode(lang_stat, n->l, scp);
		}
		//checking if the scope isn't zero and descending it
		if (n->r && n->r->r != nullptr)
		{
			DescendNode(lang_stat, n->r, scp);
		}
	}break;
	case node_type::N_DESUGARED:
	{
		//scp = GetScopeFromParent(n->l, given_scp);
		if (n->l)
			DescendNode(lang_stat, n->l, scp);
		if (n->r)
			DescendNode(lang_stat, n->r, scp);
	}break;
	case node_type::N_ELSE_IF:
	case node_type::N_IF:
	{
		//scp = GetScopeFromParent(n, given_scp);
		// cond
		if (n->l->l != nullptr)
		{
			ret_type = DescendNode(lang_stat, n->l->l, scp);
		}
		//checking if the scope isn't zero and descending it
		if (n->l->r && n->l->r->r)
		{
			DescendNode(lang_stat, n->l->r, scp);
		}
		GetIfExprType(lang_stat, n, scp, ret_type, true);

		//node *cur:

		// else blocks
		if (n->r != nullptr)
		{
			DescendNode(lang_stat, n->r, scp);
		}


	}break;
	case node_type::N_TYPE:
	{
		ret_type = n->decl_type;
	}break;
	case node_type::N_FUNC_DEF:
	{
		int a = 0;
		scp = GetScopeFromParent(lang_stat, n->r, scp);

		if (n->l->l->r)
			// args
			DescendNode(lang_stat, n->l->l->r, scp);
		// return type
		if (n->l->r)
			DescendNode(lang_stat, n->l->r, scp);

		ret_type.type = TYPE_FUNC_DEF;
		ret_type.fdecl = n->fdecl;
	}break;
	case node_type::N_CAST:
	{
		auto lhs_type = DescendNode(lang_stat, n->l, scp);
		auto rhs_type = DescendNode(lang_stat, n->r, scp);

		bool can_rhs_be_ptr = rhs_type.ptr > 0 || rhs_type.type == TYPE_STATIC_ARRAY
			|| rhs_type.ptr == 0 && (rhs_type.type == TYPE_INT || (GetTypeSize(&rhs_type) == 8));

		if (lhs_type.ptr > 0 && !can_rhs_be_ptr)
		{

			ReportMessageOne(lang_stat, n->t, "Cannot convert '%s' to a pointer", (char*)TypeToString(rhs_type).c_str());
			ExitProcess(1);
		}
		if (GetTypeSize(&lhs_type) < 8 && rhs_type.ptr > 0)
		{
			ReportMessageOne(lang_stat, n->t, "pointers cannot be converted to smaller size '%s'", (char*)TypeToString(lhs_type).c_str());
		}


		if (lhs_type.IsFloat() && lhs_type.ptr == 0)
		{
			ASSERT(rhs_type.type == TYPE_S32 || rhs_type.type == TYPE_S64
				|| rhs_type.type == TYPE_F32 || rhs_type.type == TYPE_F64);
		}
		if (rhs_type.IsFloat() && rhs_type.ptr == 0)
		{
			ASSERT(lhs_type.type == TYPE_F32_TYPE || lhs_type.type == TYPE_F32_TYPE
				|| lhs_type.type == TYPE_S32_TYPE || lhs_type.type == TYPE_S32_TYPE
			    || lhs_type.type == TYPE_U32_TYPE);
		}

		switch (lhs_type.type)
		{
		case enum_type2::TYPE_S64_TYPE:
		case enum_type2::TYPE_U64_TYPE:
			// can only convert to u64 from a struct, if the struct is a ptr
			if (rhs_type.ptr == 0 && rhs_type.type == enum_type2::TYPE_STRUCT)
			{
				//ReportMessage(lang_stat, n->t,);
				REPORT_ERROR(n->t->line, n->t->line_offset,
					VAR_ARGS("Non ptr structs cannot be converted to integers, in this case \"%s\"\n", StringifyNode(n->r).c_str())
				);
				ExitProcess(1);
			}
				break;
		case enum_type2::TYPE_STR_LIT:
		case enum_type2::TYPE_F64_TYPE:
		case enum_type2::TYPE_CHAR_TYPE:
		case enum_type2::TYPE_F32_TYPE:
		case enum_type2::TYPE_S32_TYPE:
		case enum_type2::TYPE_S16_TYPE:
		case enum_type2::TYPE_S8_TYPE:
		case enum_type2::TYPE_U32_TYPE:
		case enum_type2::TYPE_U16_TYPE:
		case enum_type2::TYPE_U8_TYPE:
		case enum_type2::TYPE_VOID_TYPE:
		case enum_type2::TYPE_BOOL_TYPE:
			break;
		case enum_type2::TYPE_STRUCT_TYPE:
			// can only convert to struct type, if the struct type is ptr and the rhs is 8 bytes

			if (lhs_type.ptr == 0 && (rhs_type.type != enum_type2::TYPE_U64 || rhs_type.type != enum_type2::TYPE_S64))
			{
				ReportMessage(lang_stat, n->l->t, "Cast cannot be a plain struct, try a struct ptr");
				ExitProcess(1);
				//ASSERT(false)
			}
			break;
		default:
			ASSERT(false)
		}

		ret_type = lhs_type;
		ret_type.type = FromTypeToVarType(ret_type.type);
	}break;
	case node_type::N_UNOP:
	{
		switch (n->t->type)
		{
			// getting array instantiation size
		case tkn_type2::T_DOLLAR:
		{
			if (IS_FLAG_ON(n->r->flags, NODE_FLAGS_IS_PROCESSED2))
				return *n->ar_lit_tp;

		}break;
		case tkn_type2::T_EXCLAMATION:
		{
			ret_type = DescendNode(lang_stat, n->r, scp);

			// assigning to a arbitrary value
			tkn_type2 opposite = T_PLUS;

			// changing bool expression operators to its opposite
			if (n->r->type == N_BINOP)
			{
				switch (n->r->t->type)
				{
				case tkn_type2::T_COND_EQ:
					opposite = T_COND_NE;
					break;
				case tkn_type2::T_COND_NE:
					opposite = T_COND_EQ;
					break;
				case tkn_type2::T_LESSER_THAN:
					opposite = T_GREATER_EQ;
					break;
				case tkn_type2::T_LESSER_EQ:
					opposite = T_GREATER_THAN;
					break;
				case tkn_type2::T_GREATER_THAN:
					opposite = T_LESSER_EQ;
					break;
				case tkn_type2::T_GREATER_EQ:
					opposite = T_LESSER_THAN;
					break;
				}

				if (opposite != T_PLUS)
				{
					n->r->t->type = opposite;
					memcpy(n, n->r, sizeof(node));
				}
			}
		}break;
		case tkn_type2::T_MINUS:
		{
			ret_type = DescendNode(lang_stat, n->r, scp);
			ModifyNodeIntOrFloat(ret_type, n);
			if (ret_type.type == TYPE_F32 || ret_type.type == TYPE_F32_RAW)
			{
				ret_type.f *= -1;
				n->t->f *= -1;
			}
			else
			{
				ret_type.i *= -1;
				n->t->i *= -1;
			}
		}break;
		case tkn_type2::T_AMPERSAND:
		{
			node* maybe_lhs_or_modified_nd = n->r;
			if (IsNodeUnop(n->r, T_MUL))
			{
				memcpy(n, n->r->r, sizeof(node));

				maybe_lhs_or_modified_nd = n;
			}
			// index already returns a pointer so we removing the "taking address of"
			else if (CMP_NTYPE(n->r, N_INDEX))
			{
				memcpy(n, n->r, sizeof(node));
				maybe_lhs_or_modified_nd = n;
			}

			NameFindingGetType(lang_stat, n, scp, ret_type);
			auto tp = DescendNode(lang_stat, maybe_lhs_or_modified_nd, scp);

			if (tp.type == TYPE_INT)
			{
				ReportMessage(lang_stat, n->t, "Cannot take address of number literals");
				ExitProcess(1);
			}

			if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_AFTER_TYPE_CHECK))
				DescendNode(lang_stat, maybe_lhs_or_modified_nd, scp);
		}break;
		case tkn_type2::T_MUL:
		{
			ret_type = DescendNode(lang_stat, n->r, scp);

			// overload mul deref when the struct is value
			if (ret_type.type == enum_type2::TYPE_STRUCT && ret_type.ptr == 0)
			{
				auto op_func = ret_type.strct->FindOpOverload(lang_stat, overload_op::DEREF_OP);
				ASSERT(op_func)
					/*
				bool is_correct_ovrld = CompareTypes(&op_func->args[0]->type, &ret_type);
				if (!is_correct_ovrld)
				{
					printf("operator '*' overload. ");
					ReportTypeMismatch(n->t, &op_func->args[0]->type, &ret_type);
				}
				*/


					ret_type = op_func->ret_type;
				own_std::vector<node*> args;

				node* ref_nd = NewBinOpNode(lang_stat, nullptr, tkn_type2::T_AMPERSAND, new_node(lang_stat, n->r));
				ref_nd->type = node_type::N_UNOP;
				args.emplace_back(ref_nd);

				node* ncall = MakeFuncCallArgs(lang_stat, op_func->name, nullptr, args, n->t);
				ncall->t = n->t;

				memcpy(n, ncall, sizeof(node));

				DescendNameFinding(lang_stat, n, scp);
			}
			else
			{

				bool should_deref = false;
				switch (ret_type.type)
				{
				case enum_type2::TYPE_FUNC_DECL:
				case enum_type2::TYPE_STRUCT_DECL:
					ASSERT(false)
						break;
				case enum_type2::TYPE_F64_TYPE:
				case enum_type2::TYPE_F32_TYPE:
				case enum_type2::TYPE_S64_TYPE:
				case enum_type2::TYPE_S32_TYPE:
				case enum_type2::TYPE_S16_TYPE:
				case enum_type2::TYPE_S8_TYPE:
				case enum_type2::TYPE_U64_TYPE:
				case enum_type2::TYPE_U32_TYPE:
				case enum_type2::TYPE_U16_TYPE:
				case enum_type2::TYPE_U8_TYPE:
				case enum_type2::TYPE_STRUCT_TYPE:
				case enum_type2::TYPE_FUNC_TYPE:
				case enum_type2::TYPE_BOOL_TYPE:
				case enum_type2::TYPE_VOID_TYPE:
				case enum_type2::TYPE_ENUM_TYPE:
				case enum_type2::TYPE_CHAR_TYPE:
					should_deref = false;
					break;
				case enum_type2::TYPE_F64:
				case enum_type2::TYPE_F32:
				case enum_type2::TYPE_S64:
				case enum_type2::TYPE_S32:
				case enum_type2::TYPE_S16:
				case enum_type2::TYPE_S8:
				case enum_type2::TYPE_U64:
				case enum_type2::TYPE_U32:
				case enum_type2::TYPE_U16:
				case enum_type2::TYPE_U8:
				case enum_type2::TYPE_BOOL:
				case enum_type2::TYPE_STRUCT:
				case enum_type2::TYPE_VOID:
				case enum_type2::TYPE_CHAR:
					should_deref = true;
					break;
				default:
				{
					ASSERT(false)
				}break;
				}
				if (should_deref)
				{
					if (ret_type.ptr <= 0)
					{
						ReportMessageOne(lang_stat, n->t, "type '%s' cannot be dereferenced",
							(void*)TypeToString(ret_type).c_str()
						);
						ExitProcess(1);
					}

					ret_type.ptr--;
				}
				else
				{
					ret_type.ptr++;
				}
			}

		}break;
		default:
			ret_type = DescendNode(lang_stat, n->r, scp);
		}
	}break;
	case node_type::N_CALL:
	{
		type2 ltp = DescendNode(lang_stat, n->l, scp);


		// temporary struct vals
		type2* fret = &ltp.fdecl->ret_type;
        decl2* decl;
		if (fret->IsStrct(&decl) && fret->ptr == 0)
			lang_stat->cur_per_stmnt_strct_val_sz += decl->type.strct->size;

		int last_call_regs_used = lang_stat->call_regs_used;

		own_std::vector<comma_ret> args;
		if (n->r != nullptr)
			DescendComma(lang_stat, n->r, scp, args);

		int var_args_count = 0;
		// getting type of args
		FOR_VEC(t, args)
		{
			if (t->type == COMMA_RET_IDENT)
			{
				ASSERT(FindIdentifier(t->decl.name, scp, &t->decl.type));
			}
			if (t->type == COMMA_VAR_ARGS)
			{
				//var_args_count++;
			}
		}
		ret_type = ltp.fdecl->ret_type;

		func_decl* fdecl = ltp.fdecl;
		// cant call regular functions inside comp ones, unless it has code, in that case it should be from base.lng file
		if (IS_FLAG_ON(scp->fdecl->flags, FUNC_DECL_COMP) && IS_FLAG_OFF(fdecl->flags, FUNC_DECL_INTERNAL) && fdecl->code != nullptr)
		{
			ReportMessage(lang_stat, n->t, "Regular funtions cant be called inside comp ones");
			ExitProcess(1);
		}

		if (IS_FLAG_ON(fdecl->flags, FUNC_DECL_COMP))
		{
			ASSERT(fdecl->code);

			char* addr = lang_stat->GetCodeAddr(fdecl->code->executable);
			//int ret_comp = ((int(*)())addr)();
		}
		else if (IS_FLAG_ON(fdecl->flags, FUNC_DECL_INTERNAL))
		{
			if (fdecl->name == "sizeof")
			{
				ret_type.type = TYPE_INT;
				ret_type.i = GetTypeSize(&args[0].decl.type);
			}
			else
				ASSERT(0);
		}


		switch (ltp.type)
		{
		case enum_type2::TYPE_STRUCT_TYPE:
		{
			auto found_strct = FindIdentifier(n->r->t->str, scp, &ret_type);
			if (!found_strct)
			{

			}
		}break;
		case enum_type2::TYPE_BUILT_IN_MACRO:
		{

			auto macro_decl = ltp.macro_builtin;

			ASSERT(macro_decl != nullptr)

				type2 macro_ret;
			macro_decl(n, n->r, lang_stat->cur_stmnt, &macro_ret);

		}break;
		case enum_type2::TYPE_FUNC_EXTERN:
		case enum_type2::TYPE_FUNC:
		case enum_type2::TYPE_FUNC_PTR:
		{
			if (IS_FLAG_OFF(ltp.fdecl->flags, FUNC_DECL_VAR_ARGS))
			{
				ASSERT(ltp.fdecl != nullptr && ltp.fdecl->args.size() == args.size())
			}
			else
			{

			}


			if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_ASSIGN_SAVED_REGS))
			{
				// getting how the max of size of how mny registers should be save for the function
				if (last_call_regs_used > scp->fdecl->biggest_saved_regs)
					scp->fdecl->biggest_saved_regs = last_call_regs_used;

				// getting how the max func call args that there is in this func
				if (args.size() > scp->fdecl->biggest_call_args)
				{
					scp->fdecl->biggest_call_args = args.size();
					// accounting the the struct that will be passed during var args
					// and the array that will hold the data
					scp->fdecl->biggest_call_args += 16;
					scp->fdecl->biggest_call_args += sizeof(type_data) * var_args_count;

				}
			}

		}break;
		default:
		{
			ASSERT(false);
		}break;
		}

		lang_stat->call_regs_used = last_call_regs_used;
	}break;
	case node_type::N_STMNT:
	{

		//DescendStmntMode(n, scp, DMODE_DNODE, nullptr);
		DescendStmnt(lang_stat, n, scp);
	}break;
	case node_type::N_FLOAT:
	{
		ret_type.type = enum_type2::TYPE_F32_RAW;
		ret_type.f = n->t->f;
	}break;
	case node_type::N_INT:
	{
		ret_type.type = enum_type2::TYPE_INT;
		ret_type.i = n->t->i;

	}break;
	case node_type::N_IDENTIFIER:
	{
		auto decl = FindIdentifier(n->t->str, scp, &ret_type);
		if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_REPORT_UNDECLARED_IDENTS) && !decl)
			ReportUndeclaredIdentifier(lang_stat, n->t);

		if (decl->type.type == TYPE_INT)
		{
			ASSERT(decl->type.is_const);
			n->type = N_INT;
			n->t->i = decl->type.i;
			ret_type.type = TYPE_INT;
			ret_type.i = decl->type.i;

		}

	}break;
	case node_type::N_LAMBDA:
	{

		ret_type.type = enum_type2::TYPE_FUNC_PTR;
		ret_type.fdecl = (func_decl*)AllocMiscData(lang_stat, sizeof(func_decl));
		memset(ret_type.fdecl, 0, sizeof(func_decl));
		own_std::vector<decl2*> args;

		auto last_fdecl = scp->fdecl;

		scp->fdecl = ret_type.fdecl;
		scp->flags |= SCOPE_INSIDE_FUNCTION;

		ret_type.fdecl->func_node = n;


		// args
		own_std::vector<comma_ret> types;
		DescendComma(lang_stat, n->l->l, scp, types);

		int i = 0;
		for (auto t = types.begin(); t < types.end(); t++)
		{

			auto new_decl = NewDecl(lang_stat, t->decl.name, t->decl.type);
			ret_type.fdecl->args.emplace_back(new_decl);
			scp->vars.emplace_back(new_decl);
			i++;
		}

		// return type
		if (n->l->r != nullptr)
			ret_type.fdecl->ret_type = DescendNode(lang_stat, n->l->r, scp);
		else
			ret_type.fdecl->ret_type.type = enum_type2::TYPE_AUTO;

		// scope
		// DescendNode(n->r, scp);


		//ret_type.fdecl->args = args;

		ret_type.fdecl->scp = NewScope(lang_stat, given_scp);
		memcpy(ret_type.fdecl->scp, scp, sizeof(scope));

		scp->fdecl = last_fdecl;

		msg.type = msg_type::MSG_FUNC_DECL;
		msg.lhs_type = ret_type;
		works.AddMessage(&msg);
	}break;
	case node_type::N_KEYWORD:
	{
		switch (n->kw)
		{
		case KW_RET_TYPE:
		{
			ret_type = DescendNode(lang_stat, n->r, scp);
			ASSERT(ret_type.type == enum_type2::TYPE_FUNC_PTR)

				ret_type = ret_type.fdecl->ret_type;
		}break;
		case KW_NIL:
		{
			ret_type.type = enum_type2::TYPE_INT;
			ret_type.flags = TYPE_FLAGS_AUTO_CAST;
			ret_type.ptr = 1;
		}break;
		case KW_FALSE:
		case KW_TRUE: ret_type.type = TYPE_BOOL; break;
		case KW_RETURN:
		{
			if (n->r != nullptr)
				ret_type = DescendNode(lang_stat, n->r, scp);

			if (IS_FLAG_OFF(scp->flags, SCOPE_INSIDE_FUNCTION))
			{
				REPORT_ERROR(n->t->line, n->t->line_offset, VAR_ARGS("return should only be used inside functions\n"))
					ExitProcess(1);
			}

			// assigning the return type of the function in case it has auto return auto as type
			func_decl* fdecl = scp->fdecl;
			ASSERT(fdecl)
				if (fdecl != nullptr)
				{
					if (fdecl->ret_type.type == enum_type2::TYPE_AUTO)
					{
						fdecl->ret_type = ret_type;
						fdecl->ret_type.type = FromTypeToVarType(fdecl->ret_type.type);
					}
					else if (!CompareTypes(&fdecl->ret_type, &ret_type) && fdecl->ret_type.type != enum_type2::TYPE_TEMPLATE && fdecl->ret_type.type != enum_type2::TYPE_VOID)
					{

						auto fdecl = scp->fdecl;

						if (!CompareTypes(&fdecl->ret_type, &ret_type, false))
						{
							// function didn't expect any return value
							if (fdecl->ret_type.type == enum_type2::TYPE_VOID)
							{
								REPORT_ERROR(n->t->line, n->t->line_offset, VAR_ARGS("function \"%s\" didn't expect any return value\n", fdecl->name.c_str()))
							}
							// return types mismatch
							else
							{
								REPORT_ERROR(n->t->line, n->t->line_offset, VAR_ARGS("return value mismatches with function return type.Expected %s, received %s\n\n%s\n...\n",
									TypeToString(fdecl->ret_type).c_str(), TypeToString(ret_type).c_str(), GetFileLn(lang_stat, fdecl->func_node->t->line - 1)))
							}
						}
						// return types are different
						ASSERT(false)
					}

					//fdecl->ret_type = ret_type;
				}

			if (ret_type.type == TYPE_BOOL && ret_type.ptr == 0 && n->r->type == N_BINOP)
			{
				type2 t_bool;
				t_bool.type = TYPE_BOOL_TYPE;
				own_std::string var_name = "ret_bool32132";
				DeclareDeclToScopeAndMaybeToFunc(lang_stat, var_name, &t_bool, scp);
				// creating a ret_var
				auto decl = CreateDeclNode(lang_stat, var_name, TYPE_BOOL, n->t);

				// if expr
				auto if_expr = CreateBoolExpression(lang_stat, decl->l, n->r, scp);

				// returning ret_bool
				n->r = decl->l;

				own_std::vector<node*> tree_args;
				tree_args.emplace_back(decl);
				tree_args.emplace_back(if_expr);
				tree_args.emplace_back(new_node(lang_stat, n));

				auto new_tree = CreateTreeArgs(lang_stat, tree_args, N_STMNT, (tkn_type2)0);
				memcpy(n, new_tree, sizeof(node));
			}

			msg.type = msg_type::MSG_RETURN;
		}break;
		case KW_DBG_BREAK:
		{
		}break;
		case KW_MATCH:
		{
			msg.type = msg_type::MSG_MATCH;
			if (n->r != nullptr)
				ret_type = DescendNode(lang_stat, n->r, scp);
			if (n->l != nullptr)
				ret_type = DescendNode(lang_stat, n->l, scp);
		}break;
		default:
		{
			if (n->r != nullptr)
				ret_type = DescendNode(lang_stat, n->r, scp);
			if (n->l != nullptr)
				ret_type = DescendNode(lang_stat, n->l, scp);
		}break;
		}

		msg.rhs_type = ret_type;

		//works.AddMessage(&msg);
	}break;
	case node_type::N_INDEX:
	{
		// type
		if (IS_FLAG_ON(n->flags, NODE_FLAGS_INDEX_IS_TYPE))
		{
			ret_type = *n->ar_lit_tp;
			/*
			auto new_type = new type2();

			ret_type = DescendNode(n->r, scp);
			memcpy(new_type, &ret_type, sizeof(type2));
			memset(&ret_type, 0, sizeof(type2));
			if (n->l != nullptr)
			{
				ret_type.type = enum_type2::TYPE_ARRAY;
				new_type->ar_size = n->l->t->i;
			}
			else
			{
				ret_type.type = enum_type2::TYPE_ARRAY_DYN;
			}

			ret_type.tp   = new_type;
			*/

		}
		// indexing
		else
		{
			type2 lhs = DescendNode(lang_stat, n->l, scp);
			type2 rhs = DescendNode(lang_stat, n->r, scp);

			bool rhs_can_be_index = rhs.type == TYPE_U64 && rhs.ptr == 0 || rhs.type == TYPE_INT;

			if (!rhs_can_be_index)
			{
				ReportMessageOne(lang_stat, n->r->t, "type '%s' cannot be used as index, use u64", (char*)TypeToString(rhs).c_str());
			}

			switch (lhs.type)
			{
			case enum_type2::TYPE_U8:
			case enum_type2::TYPE_U16:
			case enum_type2::TYPE_U32:
			case enum_type2::TYPE_U64:
			case enum_type2::TYPE_S8:
			case enum_type2::TYPE_S16:
			case enum_type2::TYPE_S32:
			case enum_type2::TYPE_S64:
				ASSERT(lhs.ptr > 0)
					ret_type = lhs;

				break;
			case enum_type2::TYPE_STATIC_ARRAY:
			case enum_type2::TYPE_ARRAY:
			case enum_type2::TYPE_ARRAY_DYN:
				ret_type = *lhs.tp;
				ret_type.ptr++;
				break;
			case enum_type2::TYPE_STRUCT:
			{
				//operator overload
				auto op_func = lhs.strct->FindOpOverload(lang_stat, overload_op::INDEX_OP);
				if (!op_func)
				{
					REPORT_ERROR(n->l->t->line, n->l->t->line_offset,
						VAR_ARGS("struct '%s' doesn't have an overload of operator index\n",
							lhs.strct->name.c_str()
						)
					)
						ExitProcess(1);
				}
				ASSERT(op_func)

					ret_type = op_func->ret_type;
				//ret_type.type = FromTypeToVarType(ret_type.type);

				own_std::vector<node*> args;


				auto ref_nd = NewUnopNode(lang_stat,
					nullptr,
					tkn_type2::T_AMPERSAND,
					n->l);

				ref_nd->t->line = n->t->line;


				// if we have "&*var" it means we should just get the var, since
				// it's already a ptr
				if (IsNodeUnop(ref_nd->r, tkn_type2::T_MUL))
					ref_nd = ref_nd->r->r;


				args.emplace_back(ref_nd);
				auto u64_nd = CreateNodeFromType(lang_stat, &lang_stat->u64_decl->type, n->t);
				auto casted = NewTypeNode(lang_stat, u64_nd, N_CAST, n->r, n->t);

				args.emplace_back(casted);

				auto call_nd = MakeFuncCallArgs(lang_stat, op_func->name, nullptr, args, n->t);
				call_nd->t = n->t;



				memcpy(n, call_nd, sizeof(node));

				if (!DescendNameFinding(lang_stat, n, scp))
					ASSERT(false)
					//return ret_type;
			}break;
			default:
				ASSERT(false)
					break;
			}
			//ret_type.ptr++;
			//ret_type = *lhs.tp;
		}

	}break;
	case node_type::N_ENUM_DECL:
	{
	}break;
	case node_type::N_OP_OVERLOAD:
	{
	}break;
	case node_type::N_FUNC_DECL:
	{
		// skipping if func has templates
		if (n->l->l->l || IS_FLAG_ON(n->flags, NODE_FLAGS_FUNC_MACRO))
			break;
		//return ret_type;
		int last_flags = scp->flags;
		auto last_f = scp->fdecl;

		scp->fdecl = n->fdecl;

		//scp = GetScopeFromParent(n, scp);

		// args
		//if (n->l->l->r)
		//	DescendNode(n->l->l->r, scp);
		// return type
		//if (n->l->r)
		//	DescendNode(n->l->r, scp);

		if (n->r)
		{
			auto before_flags = lang_stat->flags;
			auto before_plugin = lang_stat->plugins_for_func;

			if (n->fdecl->plugins.size() > 0)
			{
				lang_stat->flags |= PSR_FLAGS_HAS_PLUGINS;
				lang_stat->plugins_for_func = n->fdecl;

			}

			DescendNode(lang_stat, n->r, scp);

			lang_stat->flags = before_flags;
			lang_stat->plugins_for_func = before_plugin;
		}

		if (IS_FLAG_ON(scp->flags, SCOPE_INSIDE_FUNCTION))
		{
			scp->fdecl->per_stmnt_strct_val_sz = max2(scp->fdecl->per_stmnt_strct_val_sz, lang_stat->cur_per_stmnt_strct_val_sz);
		}

		lang_stat->cur_per_stmnt_strct_val_sz = 0;
		ret_type.type = TYPE_FUNC;
		ret_type.fdecl = n->fdecl;

		scp->fdecl = last_f;
		scp->flags = last_flags;

	}break;
	case node_type::N_UNION_DECL:
	case node_type::N_STRUCT_DECL:
	{
		if (n->l)
			break;
		//return ret_type;
	//scp = GetScopeFromParent(n, scp);

		DescendNode(lang_stat, n->r, scp);
	}break;
	case node_type::N_BINOP:
	{
		switch (n->t->type)
		{
		case tkn_type2::T_COND_AND:
		case tkn_type2::T_COND_OR:
		{
			type2 ltp = DescendNode(lang_stat, n->l, scp);
			type2 rtp = DescendNode(lang_stat, n->r, scp);

			if (!CompareTypes(&ltp, &rtp))
				ReportTypeMismatch(lang_stat, n->t, &ltp, &rtp);
			ret_type = ltp;

			// creating an implict "... == true"

			if (n->r->type != N_BINOP && (n->r->type != N_UNOP && n->r->t->type != T_EXCLAMATION))
			{
				auto rhs = CreateBinOpRhsBool(lang_stat, new_node(lang_stat, n->r), KW_TRUE, T_COND_EQ);
				memcpy(n->r, rhs, sizeof(node));
			}
			if (n->l->type != N_BINOP && (n->l->type != N_UNOP && n->l->t->type != T_EXCLAMATION))
			{
				auto lhs = CreateBinOpRhsBool(lang_stat, new_node(lang_stat, n->l), KW_TRUE, T_COND_EQ);
				memcpy(n->l, lhs, sizeof(node));
			}

		}break;
		case tkn_type2::T_COND_EQ:
		case tkn_type2::T_COND_NE:
			is_cond_eq = true;
		case tkn_type2::T_LESSER_THAN:
		case tkn_type2::T_LESSER_EQ:
		case tkn_type2::T_GREATER_THAN:
		case tkn_type2::T_GREATER_EQ:
		{
			type2 ltp = DescendNode(lang_stat, n->l, scp);

			type2 rtp = DescendNode(lang_stat, n->r, scp);

			if (ltp.type == TYPE_INT)
			{

			}

			if (ltp.type == TYPE_STRUCT && rtp.ptr == 0 || ltp.type == TYPE_STRUCT && ltp.ptr == 0)
			{
				own_std::vector<type2> tp_ar;

				if (ltp.ptr > 0)
				{
					ReportMessage(lang_stat, n->r->t, "lhs must be a value, but it is a ptr");
					ExitProcess(1);
				}

				if (rtp.type == TYPE_STRUCT)
				{
					if (rtp.ptr == 0)
						rtp.ptr++;
					else
					{
						ReportMessage(lang_stat, n->r->t, "The struct parameters for a operator overloading will already be implicit ptr");
						ExitProcess(1);
					}
				}
				tp_ar.emplace_back(rtp);

				auto op_func = ltp.strct->FindExistingOverload(lang_stat, nullptr, (void*)COND_EQ_OP, &tp_ar, false);

				if (!op_func)
				{
					ReportMessageOne(lang_stat, n->r->t, "struct '%s' doesn't have an operator '==' overloading", (void*)ltp.strct->name.c_str());
				}

				// struct_lhs==(&lhs, &rhs)
				own_std::vector<node*> arg_ar;

				node* ref_nd_lhs = NewUnopNode(lang_stat, nullptr, tkn_type2::T_AMPERSAND, n->l);
				arg_ar.emplace_back(ref_nd_lhs);

				node* rhs = n->r;
				if (rtp.type == TYPE_STRUCT)
					rhs = NewUnopNode(lang_stat, nullptr, tkn_type2::T_AMPERSAND, n->r);
				arg_ar.emplace_back(rhs);

				auto call_nd = MakeFuncCallArgs(lang_stat, op_func->name, nullptr, arg_ar, n->t);
				call_nd->t = n->t;
				memcpy(n, call_nd, sizeof(node));

				if (!DescendNameFinding(lang_stat, n, scp))
					ASSERT(false)

					ret_type = op_func->ret_type;
			}
			else
			{
				if (!CompareTypes(&ltp, &rtp))
					ReportTypeMismatch(lang_stat, n->t, &ltp, &rtp);


				ret_type.type = enum_type2::TYPE_BOOL;
				bool is_unsigned = ltp.ptr > 0 || IsUnsigned(ltp.type);
				n->is_unsigned = is_unsigned;

				MaybeCreateCast(lang_stat, n->l, n->r, &ltp, &rtp);
				/*
				if (rtp.type != ltp.type && rtp.ptr == 0 )
				{
					auto lhs_tp = CreateNodeFromType(lang_stat, &ltp);
					auto casted = NewTypeNode(lang_stat, lhs_tp, N_CAST, new_node(lang_stat, n->r));

					memcpy(n->r, casted, sizeof(node));
				}
				*/
			}

		}break;
		case tkn_type2::T_COMMA:
		{
			if (n->l != nullptr)
				DescendNode(lang_stat, n->l, scp);
			if (n->r != nullptr)
				DescendNode(lang_stat, n->r, scp);
		}break;
		case tkn_type2::T_MUL:
		case tkn_type2::T_MINUS:
		case tkn_type2::T_PERCENT:
		case tkn_type2::T_PLUS:
		{
			type2 ltp = DescendNode(lang_stat, n->l, scp);
			type2 rtp = DescendNode(lang_stat, n->r, scp);

			ModifyNodeIntOrFloat(ltp, n->l);
			ModifyNodeIntOrFloat(rtp, n->r);

			ret_type = ltp;
			if (ltp.type == TYPE_INT && rtp.type == TYPE_INT)
			{
				n->type = N_INT;
				n->t->i = GetExpressionValT<int>(n->t->type, ltp.i, rtp.i);
				ret_type.i = n->t->i;
				//GetExpressionVal(n);
			}

			// cannot perform this op on ptr
			if (ltp.ptr > 0 || rtp.ptr > 0)
			{
				type2* ptr_tp = ltp.ptr > 0 ? &ltp : &rtp;
				REPORT_ERROR(n->t->line, n->t->line_offset,
					VAR_ARGS("Operation '%s' cannot be performed on pointers", n->t->ToString().c_str()
					)
				);
				ExitProcess(1);
			}

			if (ret_type.type == enum_type2::TYPE_STRUCT && ret_type.ptr == 0)
			{
				overload_op op;
				switch (n->t->type)
				{
				case T_MINUS:
				{
					op = overload_op::MINUS_OP;
				}break;
				case T_MUL:
				{
					op = overload_op::MUL_OP;
				}break;
				case T_PLUS:
				{
					op = overload_op::PLUS_OP;
				}break;
				default:
					ASSERT(0)
				}
				auto op_func = ret_type.strct->FindOpOverload(lang_stat, op);
				ASSERT(op_func)
					/*
				bool is_correct_ovrld = CompareTypes(&op_func->args[0]->type, &ret_type);
				if (!is_correct_ovrld)
				{
					printf("operator '*' overload. ");
					ReportTypeMismatch(n->t, &op_func->args[0]->type, &ret_type);
				}
				*/


				ret_type = op_func->ret_type;
				own_std::vector<node*> args;

				node* ref_nd = NewBinOpNode(lang_stat, nullptr, tkn_type2::T_AMPERSAND, new_node(lang_stat, n->l));
				ref_nd->type = node_type::N_UNOP;
				args.emplace_back(ref_nd);

				node *sec_arg = new_node(lang_stat, n->r);
				if (op_func->args[1]->type.ptr > 0)
				{
					ref_nd = NewBinOpNode(lang_stat, nullptr, tkn_type2::T_AMPERSAND, sec_arg);
					ref_nd->type = node_type::N_UNOP;
					sec_arg = ref_nd;
				}
				args.emplace_back(sec_arg);

				node* ncall = MakeFuncCallArgs(lang_stat, op_func->name, nullptr, args, n->t);
				ncall->t = n->t;

				memcpy(n, ncall, sizeof(node));

				DescendNameFinding(lang_stat, n, scp);
			}
			else if (!CompareTypes(&ltp, &rtp))
			{
				ReportTypeMismatch(lang_stat, n->t, &ltp, &rtp);
			}


			/*
			auto ModifyNd = [](node** nd, type2& ret_type, enum_type2 tp) {
				node* n = *nd;
				if (tp == enum_type2::TYPE_F32)
				{
					ret_type.f = GetExpressionValT<float>(n->t->type, n->l->t->f, n->r->t->f);
					n->type = node_type::N_FLOAT;
					n->t->f = ret_type.f;
				}
				else if (tp == enum_type2::TYPE_INT)
				{
					ret_type.i = GetExpressionValT<int>(n->t->type, n->l->t->i, n->r->t->i);
					n->type = node_type::N_INT;
					n->t->i = ret_type.i;
				}
				free(n->l);
				free(n->r);
			};
			tkn_type2 ltp_nd = n->l->t->type;
			tkn_type2 rtp_nd = n->r->t->type;

			if ((ltp_nd == tkn_type2::T_INT && ltp_nd == tkn_type2::T_INT) ||
				(ltp.type == enum_type2::TYPE_F32 && rtp.type == enum_type2::TYPE_F32))
				ModifyNd(&n, ret_type, ltp.type);
				*/




		}break;
		case tkn_type2::T_MINUS_EQUAL:
		case tkn_type2::T_PLUS_EQUAL:
		case tkn_type2::T_EQUAL:
		{
			if (IS_PRS_FLAG_ON(PSR_FLAGS_ON_ENUM_DECL))
				break;
			//return ret_type;

			type2 ltp;
			if (n->l != nullptr)
				ltp = DescendNode(lang_stat, n->l, scp);
			// don't want assign again a const val
			if (ltp.is_const)
				break;
			//return ret_type;

			type2 rtp = DescendNode(lang_stat, n->r, scp);

			if (ltp.type == enum_type2::TYPE_AUTO)
			{
				// rtp must be a known type
				ASSERT(n->l->t->type == tkn_type2::T_COLON && rtp.type != enum_type2::TYPE_AUTO)
					if (rtp.type == enum_type2::TYPE_INT)
						rtp.type = enum_type2::TYPE_S32;
				// tp holds a ptr to the lhs decl.type
				*ltp.tp = rtp;
				ltp = *ltp.tp;
			}
			else
			{
				ret_type = ltp;
				if (ltp.type == enum_type2::TYPE_STRUCT && ltp.ptr == 0)
				{
					auto op_func = ltp.strct->FindOpOverload(lang_stat, overload_op::ASSIGN_OP);
					bool is_same_strct = rtp.type == enum_type2::TYPE_STRUCT && rtp.strct->name == ltp.strct->name;

					//ASSERT(op_func)

					// if the struct doesnt have an equal operator overload, we'll just do
					// a memcpy later and if rhs and lhs are different structs
					// so we dont want to enter this cond rtp and ltp are the same type
					/*
					if (!is_same_strct && !op_func)
					{
						int a = 0;
					}
					*/
					if (!is_same_strct)
					//else
					{
						if (!op_func)
						{
							node * call = MakeMemCpyCall(lang_stat, n->l, n->r, ltp.strct->size);
							memcpy(n, call, sizeof(node));
							/*
							//auto arg1 
							REPORT_ERROR(n->l->t->line, n->l->t->line_offset,
								VAR_ARGS("operator '=' not found whithin struct '%s'", ltp.strct->name.c_str())
							);
							ExitProcess(1);
							*/
						}
						else
						{
							bool is_correct_ovrld = CompareTypes(&op_func->args[1]->type, &rtp);
							if (!is_correct_ovrld)
							{
								printf("operator '=' overload. ");
								ReportTypeMismatch(lang_stat, n->t, &op_func->args[1]->type, &rtp);
							}

							//ASSERT(CompareTypes(&op_func->args[1]->type, &rtp))

							auto rel_ptr = n->l->NewTree(lang_stat);
							n->l->type = node_type::N_UNOP;
							n->l->t->type = tkn_type2::T_AMPERSAND;
							n->l->r = rel_ptr;

							auto new_args = NewBinOpNode(lang_stat,
								n->l,
								tkn_type2::T_COMMA,
								n->r);

							auto new_n = NewTypeNode(lang_stat,
								NewIdentNode(lang_stat, op_func->name, n->t),
								node_type::N_CALL,
								new_args, n->t);
							new_n->t = n->t;
							//FREE? free(n)
							memcpy(n, new_n, sizeof(node));

							DescendNode(lang_stat, n, scp);

							n->flags &= ~NODE_FLAGS_CALL_RET_ANON;
						}
					}
					else
					{
						if (!CompareTypes(&ltp, &rtp))
							ReportTypeMismatch(lang_stat, n->t, &ltp, &rtp);
						node * call = MakeMemCpyCall(lang_stat, n->l, n->r, ltp.strct->size);
						memcpy(n, call, sizeof(node));
					}
				}
				else
				{
					if (ltp.type == TYPE_INT)
						ReportMessage(lang_stat, n->t, "lhs must be a memory value");

					if (!CompareTypes(&ltp, &rtp))
						ReportTypeMismatch(lang_stat, n->t, &ltp, &rtp);

					// excluding the colon node
					if (IsNodeUnop(n->r, T_DOLLAR))
					{
						//ASSERT(IsNodeOperator(n->l, T_COLON));

						memcpy(n, n->r, sizeof(node));
					}

					if (rtp.type == TYPE_FUNC && n->r->type == N_FUNC_DECL)
					{
						ModifyFuncDeclToName(lang_stat, rtp.fdecl, n->r, scp);
					}
					else
						MaybeCreateCast(lang_stat, n->l, n->r, &ltp, &rtp);
					if (ltp.type == enum_type2::TYPE_STATIC_ARRAY && ltp.ptr == 0)
					{
						node * call = MakeMemCpyCall(lang_stat, n->l, n->r, GetTypeSize(&ltp));
						memcpy(n, call, sizeof(node));

					}
					/*
					// creating a cast for types that are different
					if (ltp.type != TYPE_ENUM && rtp.type != TYPE_STR_LIT && ltp.type != rtp.type)
					{
						auto t_nd = CreateNodeFromType(lang_stat, &ltp);
						auto new_nd = NewTypeNode(lang_stat, t_nd, N_CAST, n->r);
						new_nd->t = n->t;
						n->r = new_nd;
						//memcpy(n->r, new_nd, sizeof(node));
					}
					*/
				}
			}
		}break;
		case tkn_type2::T_COLON:
		{
			// if type was not ommited
			type2 rtp;
			if (n->r)
			{
				// for the moment, we dont want plugin functions to modify the type of a declaration
				auto before_flags = lang_stat->flags;
				lang_stat->flags &= ~PSR_FLAGS_HAS_PLUGINS;

				rtp = DescendNode(lang_stat, n->r, scp);

				lang_stat->flags = before_flags;
			}

			auto decl = FindIdentifier(n->l->t->str, scp, &ret_type);
			ASSERT(decl)

				if (decl->type.type == enum_type2::TYPE_AUTO)
					ret_type.tp = &decl->type;
				else
					ret_type = decl->type;
		}break;
		case tkn_type2::T_POINT:
		{

			PointLogic(lang_stat, n, scp, &ret_type);
			if (ret_type.type == TYPE_ENUM_IDX_32)
			{
				//decl2 *d = ret_type.from_enum->type.GetEnumDecl(n->r->t->str);
				//n->type = N_INT;
				//n->t->i = d->type.e_idx;
			}

		}break;
		case tkn_type2::T_WORD:
		{

		}break;
		default:
		{
			if (n->l && n->r)
			{
				type2 ltp = DescendNode(lang_stat, n->l, scp);

				type2 rtp = DescendNode(lang_stat, n->r, scp);
				if (!CompareTypes(&ltp, &rtp))
				{
					ReportTypeMismatch(lang_stat, n->t, &ltp, &rtp);
				}

				ret_type = ltp;
			}
		}break;
		}
		message msg;
	}break;
	case N_ARRAY_CONSTRUCTION:
	{
		NameFindingGetType(lang_stat, n->l, scp, ret_type);
		//FindIdentifier(n->l->t->str, scp, &ret_type);
	}break;
	case N_STRUCT_CONSTRUCTION:
	{
		NameFindingGetType(lang_stat, n->l, scp, ret_type);
		//FindIdentifier(n->l->t->str, scp, &ret_type);
	}break;
	default:
	{
		if (n->l != nullptr)
			DescendNode(lang_stat, n->l, scp);
		if (n->r != nullptr)
			DescendNode(lang_stat, n->r, scp);
	}break;
	}

	if (IS_FLAG_ON(lang_stat->flags, PSR_FLAGS_HAS_PLUGINS))
	{
		ASSERT(lang_stat->plugins_for_func);
		auto fdecl = lang_stat->plugins_for_func;

		FOR_VEC(it, fdecl->plugins)
		{
			auto f = *it;
			ASSERT(f->code);

			comp_time_type_info type_info = {};



			if (n->type == N_IDENTIFIER)
			{
			}

			char* addr = lang_stat->GetCodeAddr(f->code->executable);
			((void (*)(void *, void*, void*))addr)(lang_stat, n, &type_info);

			if (IS_FLAG_ON(n->flags, NODE_FLAGS_WAS_MODIFIED))

			{
				n->flags &= ~NODE_FLAGS_WAS_MODIFIED;
				ASSERT(DescendNameFinding(lang_stat, n, scp) != nullptr);
				ret_type = DescendNode(lang_stat, n, scp);
				n->flags &= ~NODE_FLAGS_WAS_MODIFIED;
			}
			int a = 0;
		}
	}

	return ret_type;
}
own_std::string StringifyNode(node* n)
{
	own_std::string ret;
	if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_SCOPE))
	{
		ret += "{";
	}
	if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_PARENTHESES))
	{
		ret += "(";
	}

	switch (n->type)
	{
	case node_type::N_KEYWORD:
	{
		switch (n->kw)
		{
		case keyword::KW_RETURN:
		{
			ret += "return ";
		}break;
		}
	}break;
	case node_type::N_FUNC_DECL:
	{
		func_decl* fdecl = n->decl_type.fdecl;

		//poping open_curly braces
		ret.pop_back();

		ret += TypeToString(fdecl->ret_type) + " ";
		ret += fdecl->name + "(";

		for (auto s : fdecl->args)
		{
			ret += TypeToString(s->type) + " ";
			ret += s->name + ", ";
		}
		ret.pop_back();
		ret.pop_back();
		ret += ")";
		ret += "{";
		ret += StringifyNode(n->r);
		ret += "}";
		n->flags = 0;
	}break;
	case node_type::N_TYPE:
		ret += TypeToString(n->decl_type);
		break;
	case node_type::N_IF:
	{
		ret += "if ";

		if (n->l->l != nullptr)
			ret += StringifyNode(n->l->l);

		if (n->l->r != nullptr)
			ret += StringifyNode(n->l->r);

		if (n->r != nullptr)
			ret += StringifyNode(n->r);

	}break;
	case node_type::N_ELSE:
	{
		ret += "else ";

		if (n->l->l != nullptr)
			ret += StringifyNode(n->l->l);

		if (n->l->r != nullptr)
			ret += StringifyNode(n->l->r);

		if (n->r != nullptr)
			ret += StringifyNode(n->r);
	}break;
	case node_type::N_ELSE_IF:
	{
		ret += "else if";

		if (n->l->l != nullptr)
			ret += StringifyNode(n->l->l);

		if (n->l->r != nullptr)
			ret += StringifyNode(n->l->r);

		if (n->r != nullptr)
			ret += StringifyNode(n->r);
	}break;
	case node_type::N_CALL:
	{
		ret += StringifyNode(n->l);
		ret += "(";

		if (n->r != nullptr)
			ret += StringifyNode(n->r);
		ret += ")";
	}break;
	case node_type::N_FLOAT:
	case node_type::N_INT:
	case node_type::N_IDENTIFIER:
	{
		ret += n->t->ToString();
	}break;
	case node_type::N_INDEX:
	{
		ret += StringifyNode(n->l);
		ret += "[";
		ret += StringifyNode(n->r);
		ret += "]";
	}break;
	case node_type::N_DECL:
	{
		ret += TypeToString(n->l->decl_type);
		ret += StringifyNode(n->r);
	}break;
	case node_type::N_UNOP:
	{
		ret += n->t->ToString();
		ret += StringifyNode(n->r);
	}break;
	case node_type::N_BINOP:
	{
		ret += StringifyNode(n->l);
		ret += n->t->ToString();
		ret += StringifyNode(n->r);
	}break;
	case node_type::N_STMNT:
	{
		if (n->l != nullptr)
		{
			ret += StringifyNode(n->l);
			if (n->l->type != node_type::N_STMNT)
				ret += ";";
		}
		if (n->r != nullptr)
		{
			ret += StringifyNode(n->r);
			if (n->r->type != node_type::N_STMNT)
				ret += ";";
		}
	}break;
	default:
		ASSERT(false)
	}

	if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_SCOPE))
	{
		ret += "}";
	}
	if (IS_FLAG_ON(n->flags, NODE_FLAGS_IS_PARENTHESES))
	{
		ret += ")";
	}
	return ret;
}
node* node_iter::parse(tkn_type2 target)
{
	ASSERT(false)
		return nullptr;
}
bool CheckOverloadFunction(lang_state* lang_stat, func_decl* f)
{
	struct defer_strct
	{
		unit_file* last;
		lang_state* _l;
		defer_strct(lang_state* l, func_decl* f)
		{
			_l = l;
			last = _l->cur_file;
			_l->cur_file = f->from_file;
		}
		~defer_strct()
		{
			_l->cur_file = last;
		}
	};

	;
	if (IS_FLAG_ON(f->flags, FUNC_DECL_IS_DONE))
		return true;

	defer_strct dfr(lang_stat, f);

	if (f->func_node->l->r && !NameFindingGetType(lang_stat, f->func_node->l->r, f->scp, f->ret_type))
		return false;

	// getting the func decl args
	int cur_scp_vars_size = f->scp->vars.size();
	if (!DescendArgsOfModifiedFunc(lang_stat, f))
		return false;

	if (f->args.size() == 0)
		f->args.insert(f->args.end(), f->scp->vars.begin() + f->templates.size(), f->scp->vars.end());

	f->call_strcts_val_offset = f->strct_vals_sz;

	f->func_node->r->scp = f->scp;
	// getting the scope vars
	if (!DescendNameFinding(lang_stat, f->func_node->r, f->scp))
		return false;




	//f->name = f->name + OvrldOpToStr(f->op_overload);

	//return type
	if (!CheckFuncRetType(lang_stat, f, f->scp))
		return false;

	/*
	f->strct_val_ret_offset = f->strct_vals_offset;

	CheckStructValToFunc(f, &f->ret_type);
	*/
	f->flags |= FUNC_DECL_IS_DONE;

	DescendNode(lang_stat, f->func_node->r, f->scp);

	type2 tp;
	tp.type = enum_type2::TYPE_FUNC;
	tp.fdecl = f;
	// @test f->scp->parent->vars.emplace_back(NewDecl(lang_stat, f->name, tp));

	auto decl = NewDecl(lang_stat, f->name, tp);
	f->this_decl = decl;
	lang_stat->cur_file->global->vars.emplace_back(decl);
	//lang_stat->root->vars.emplace_back(decl);
	//lang_stat->funcs_scp->vars.emplace_back(decl);
	// 
	/*
	if(!IsThereAFunction((char *)f->name.c_str()))
		NewFuncToCompile(f);
		*/
	f->templates.clear();


	return true;
}
own_std::string OvrldOpToStr(overload_op op)
{
	switch (op)
	{
	case overload_op::COND_EQ_OP:
	{
		return "==";
	}break;
	case overload_op::INDEX_OP:
	{
		return "[]";
	}break;
	case overload_op::MINUS_OP:
	{
		return "-";
	}break;
	case overload_op::MUL_OP:
	{
		return "*";
	}break;
	case overload_op::PLUS_OP:
	{
		return "+";
	}break;
	case overload_op::FOR_OP:
	{
		return "for";
	}break;
	case overload_op::DEREF_OP:
	{
		return "deref";
	}break;
	case overload_op::ASSIGN_OP:
	{
		return "=";
	}break;
	default:
		ASSERT(false)
	}

}

func_decl* type_struct2::FindExistingOverload(lang_state *lang_stat, own_std::vector<func_overload_strct>* funcs, void* data, own_std::vector<type2>* tps, bool search_constructors)
{
	if (search_constructors && funcs->size() > 0)
	{
		type2* ret_type = (type2*)data;
		auto name = FuncNameWithTempls(lang_stat, own_std::string("constr_") + TypeToString(*ret_type), tps);
		FOR_VEC(f, (*funcs)[0].fdecls)
		{
			if ((*f)->name == name)
				return *f;
		}
	}
	else
	{
		own_std::string fname;
		overload_op op = (overload_op)(long long)data;

		type2 dummy_tp;


		func_decl* ret = nullptr;
		decl2* found_f = nullptr;
		FOR_VEC(f, op_overloads_funcs)
		{
			if ((*f)->type.type == TYPE_FUNC)
			{
				fname = this->name + OvrldOpToStr(op);
				found_f = FindIdentifier(fname, lang_stat->root, &dummy_tp);

				if (found_f)
					return found_f->type.fdecl;

				auto fdecl = (*f)->type.fdecl;
				if (fdecl->name == fname)
				{
					ret = fdecl;
					break;
				}
			}
			// TYPE_OVERLOAD_FUNCS
			else
			{
				// getting the overloaded name first


				own_std::string fname = this->name + "_";
				switch (op)
				{
				case COND_EQ_OP:
				{
					fname += TypeToString((*tps)[0]);
				}break;
				default:
					ASSERT(false)
				}
				fname += OvrldOpToStr(op);

				found_f = FindIdentifier(fname, lang_stat->root, &dummy_tp);

				if (found_f)
					return found_f->type.fdecl;

				FOR_VEC(ovrld, (*f)->type.overload_funcs->fdecls)
				{
					if ((*ovrld)->name == fname)
					{
						ret = (*ovrld);
						break;
					}
				}
			}
		}
		if (ret)
		{
			if (!CheckOverloadFunction(lang_stat, ret))
				return nullptr;

			return ret;
		}
		else
		{
			// since we dindt find the func, maybe it's a templated one, so we searching in the original struct
			if (original_strct)
			{
				FOR_VEC(f, original_strct->op_overloads_funcs)
				{
					if ((*f)->type.type == TYPE_FUNC)
					{
						auto fdecl = (*f)->type.fdecl;
						if (fdecl->op_overload == op)
						{
							ret = CreateNewOpOverload(lang_stat, fdecl, op);
							AddOpOverload(lang_stat, ret, op);
						}
					}
					// TYPE_OVERLOAD_FUNCS
					else if ((*f)->type.overload_funcs->ovrld_op == op)
					{
						FOR_VEC(ovrld, (*f)->type.overload_funcs->fdecls)
						{
							ret = CreateNewOpOverload(lang_stat, (*ovrld), op);
							AddOpOverload(lang_stat, ret, op);
						}
					}
				}
			}
		}
		if (ret)
		{
			if (!CheckOverloadFunction(lang_stat, ret))
				return nullptr;

			return ret;
		}

	}
	return nullptr;
}
func_decl* type_struct2::FindOpOverload(lang_state *lang_stat, overload_op tp, own_std::vector<type2>* tp_ptr)
{
	/*
	FOR_VEC(f, op_overloads_funcs)
	{
		if((*f)->type == TYPE_FUNC)
		{

		}
		//TYPE_OVERLOAD_FUNCS
		else
		{
			FOR_VEC(ovlrd, (*f)->type.overload_funcs->fdecls)
			{
				auto func = (*ovrld)->type.fdecl;
				if(IS_FLAG_OFF(func->type.fdecl->flags, FUNC_DECL_IS_DONE))
				{
					auto new_func = func;
					if(!CheckOverloadFunction(new_func))
						auto a = 0;
				}
			}
		}
	}
	*/
	FOR_VEC(f, op_overloads)
	{
		if ((*f)->op_overload == tp)
		{
			if (IS_FLAG_OFF((*f)->flags, FUNC_DECL_IS_DONE))
			{
				auto new_func = (*f);
				if (!CheckOverloadFunction(lang_stat, new_func))
					return nullptr;
			}
			return (*f);
		}
	}
	// trying to find the overload in the original struct that this 
	// one was originated from, if we've gotten here it means that 
	// the op overload was not instantiated yet for this struct
	if (original_strct)
	{
		auto found_in_og = original_strct->FindOpOverload(lang_stat, tp);
		if (found_in_og)
		{
			own_std::string f_name = this->name.substr() + OvrldOpToStr(tp);

			type2 dummy_tp;
			auto found_f = FindIdentifier(f_name, lang_stat->root, &dummy_tp);

			if (found_f)
				return found_f->type.fdecl;

			auto new_func = CreateNewOpOverload(lang_stat, found_in_og, tp);
			/*
			// creating overloaded function for this struct
			auto new_func = found_in_og->new_func();
			new_func->func_node = found_in_og->func_node->NewTree();
			new_func->flags = FUNC_DECL_IS_OP_OVERLOAD;

			new_func->op_overload = tp;

			new_func->scp = NewScope(found_in_og->scp->parent);
			new_func->scp->flags = SCOPE_INSIDE_FUNCTION;
			new_func->scp->fdecl = new_func;
			//this->op_overloads.emplace_back(new_func);

			ASSERT(this->scp)

			int cur_templ = 0;

			// checking this scope templates and assiging it to overload templates
			FOR_VEC(templ, new_func->templates)
			{
				auto cur_strct_var = this->scp->vars[cur_templ];
				if (templ->name == cur_strct_var->name)
					new_func->scp->vars.emplace_back(NewDecl(lang_stat, templ->name, *cur_strct_var->type.tp));

				cur_templ++;
			}

			new_func->name = this->name.substr();
			new_func->op_overload = tp;
			*/

			this->AddOpOverload(lang_stat, new_func, tp);
			if (!CheckOverloadFunction(lang_stat, new_func))
				return nullptr;

			return new_func;
		}
	}
	return nullptr;
}
void CompileFile(lang_state *lang_stat, unit_file* fl)
{
	
	Tokenize2(fl->contents, fl->contents_sz, &fl->tkns, &fl->lines);
	node_iter niter(&fl->tkns, lang_stat);
	fl->s = niter.parse_all();
}

unit_file* AddNewFile(lang_state *lang_stat, own_std::string name)
{
	FOR_VEC(f, lang_stat->files)
	{
		if ((*f)->name == name)
			return *f;
	}
	own_std::string dir = lang_stat->work_dir + "/" + name;

	unsigned int read=0;
	char* file_cstr = (char*)dir.c_str();
	char* file = ReadEntireFileLang(file_cstr, &read);

	if (read == 0)
	{
		printf("file \"%s\" is empty, will not be compiled\n", file_cstr);
		return nullptr;
	}
	ASSERT(file != 0);

	auto new_f = (unit_file*)AllocMiscData(lang_stat, sizeof(unit_file));
	memset(new_f, 0, sizeof(unit_file));
	int bar_idx = name.find_last_of("\\/");
	if (bar_idx != -1)
	{

		new(&new_f->name)own_std::string(name.substr(bar_idx + 1));
	}
	else
		new(&new_f->name)own_std::string(name.substr());

	bar_idx = dir.find_last_of("\\/");
	new(&new_f->path)own_std::string(dir.substr(0, bar_idx));

	new_f->contents = file;
	new_f->contents_sz = read;
	new_f->global = NewScope(lang_stat, lang_stat->root);
	new_f->global->flags |= SCOPE_IS_GLOBAL;
	new_f->global->type = SCP_TYPE_FILE;
	new_f->global->file = new_f;
	lang_stat->files.emplace_back(new_f);


	if (lang_stat->base_lang)
		new_f->global->imports.emplace_back(lang_stat->base_lang);
	//free(file);


	int last_flags = lang_stat->flags;
	lang_stat->flags = 0;
	auto last_cur_file = lang_stat->cur_file;
	lang_stat->cur_file = new_f;

	CompileFile(lang_stat, new_f);

	lang_stat->cur_file = last_cur_file;
	lang_stat->flags = last_flags;

	return new_f;
}
func_decl* type_struct2::CreateNewOpOverload(lang_state *lang_stat, func_decl* original, overload_op tp)
{
	// creating overloaded function for this struct
	auto new_func = original->new_func();
	new_func->func_node = original->func_node->NewTree(lang_stat);
	new_func->flags = FUNC_DECL_IS_OP_OVERLOAD;

	new_func->op_overload = tp;

	new_func->scp = NewScope(lang_stat, original->scp->parent);
	new_func->scp->flags = SCOPE_INSIDE_FUNCTION;
	new_func->scp->fdecl = new_func;
	//this->op_overloads.emplace_back(new_func);
	//this->AddOpOverload(lang_stat, new_func, tp);

	ASSERT(this->scp)

		int cur_templ = 0;

	// checking this scope templates and assiging it to overload templates
	FOR_VEC(templ, new_func->templates)
	{
		auto cur_strct_var = this->scp->vars[cur_templ];
		if (templ->name == cur_strct_var->name)
			new_func->scp->vars.emplace_back(NewDecl(lang_stat, templ->name, *cur_strct_var->type.tp));

		cur_templ++;
	}

	new_func->name = this->name.substr();
	new_func->op_overload = tp;


	return new_func;
}

decl2* scope::FindVariable(own_std::string name)
{
	scope* cur_scope = this;

	while (cur_scope != nullptr)
	{
		// imports scope
		FOR_VEC(d, cur_scope->imports)
		{
			auto f = (*d)->type.imp;
			// we can only get names from imps that are implicit
			// but if the given name is the alias then we return the imp decl
			if (f->type == import_type::IMP_BY_ALIAS)
			{
				if (f->alias == name)
					return (*d);
				else
					continue;
			}


			auto ret = f->FindDecl(name);
			if (ret)
				return ret;
		}
		for (int i = 0; i < cur_scope->vars.size(); i++)
		{
			decl2* var = cur_scope->vars[i];
			if (var->name == name)
			{
				// getting a variable that is in the same func scope
				if (IS_FLAG_ON(flags, SCOPE_INSIDE_FUNCTION))
				{
					if (IS_FLAG_ON(cur_scope->flags, SCOPE_INSIDE_FUNCTION) && cur_scope->fdecl == fdecl
						|| IS_FLAG_ON(cur_scope->flags, SCOPE_IS_GLOBAL)
						|| (cur_scope->parent == nullptr)
						|| var->type.type == TYPE_STRUCT_TYPE
						|| var->type.type == TYPE_FUNC)
						return var;

					else
						continue;
				}
				else
				{
					return var;
				}
			}
		}
		cur_scope = cur_scope->parent;
	}
	return nullptr;
}
