#include "IR.h"

bool IsAstSimple(lang_state *lang_stat, ast_rep *ast);
decl2* PointLogic(lang_state* lang_stat, node* n, scope* scp, type2* ret_tp);
bool NameFindingGetType(lang_state* lang_stat, node* n, scope* scp, type2& ret_type, int);
void GenStackThenIR(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out, ir_val* dst_val, ir_val *i=nullptr);
void CreateOppositeRegAssigmentAfterCondChecking(lang_state* lang_stat, own_std::vector<ir_rep>* out, int sub_if_idx, int if_idx, int reg);
bool IsNodeOperator(node* nd, tkn_type2 tkn);


#define STACK_PTR_REG 8
#define BASE_STACK_PTR_REG 9
#define RET_1_REG 10
#define RET_2_REG 11
#define FILTER_PTR 12

#define MAKE_DST_IR_VAL(ir_tp, ptr) (((short)ir_tp) | (((int)ptr)<<16))
#define MAKE_DST_IR_VAL(ir_tp, ptr) (((short)ir_tp) | (((int)ptr)<<16))

ast_rep *NewAst()
{
	auto ret = (ast_rep*)__lang_globals.alloc(__lang_globals.data, sizeof(ast_rep));
    memset(ret, 0, sizeof(ast_rep));
    return ret;

}


bool CheckStmntWithoutSemicolon(lang_state* lang_stat, own_std::vector<ast_rep *>* ar)
{
	FOR_VEC(stmnt, (*ar))
	{
		ast_rep* st = *stmnt;
		bool is_stmnt_without_semicolon = st->stmnt_without_semicolon;
		if (is_stmnt_without_semicolon)
		{
			// stmnt without semicolon should be at the end of the scope, because they are like a return
			ASSERT((stmnt + 1) == ar->end());
			return true;
		}

	}
	return false;
}

ast_rep* CreateAstBin(lang_state* lang_stat, tkn_type2 op, ast_rep *lhs, ast_rep *rhs, node *lhs_n, scope *scp)
{
	ast_rep* ret = NewAst();
	ret->type = AST_BINOP;
	ret->op = op;
	ret->lhs_tp = DescendNode(lang_stat, lhs_n, scp);
	ret->expr.emplace_back(lhs);
	ret->expr.emplace_back(rhs);

	return ret;
}
ast_rep *AstFromNode(lang_state *lang_stat, node *n, scope *scp)
{
    ast_rep *ret = NewAst();
	ret->line_number = n->t->line;
	type2 dummy_type;
    switch(n->type)
    {
	case N_TYPE:
	{
		ret->type = AST_TYPE;
		ret->tp = n->decl_type;
	}break;
	case node_type::N_UNOP:
	{
		switch (n->t->type)
		{
		case T_MINUS_MINUS:
		{
			ret->type = AST_MINUS_MINUS;
			ret->unop_assign.tp = DescendNode(lang_stat, n->r, scp);
			ret->unop_assign.ast = AstFromNode(lang_stat, n->r, scp);
		}break;
		case T_PLUS_PLUS:
		{
			ret->type = AST_PLUS_PLUS;
			ret->unop_assign.tp = DescendNode(lang_stat, n->r, scp);
			ret->unop_assign.ast = AstFromNode(lang_stat, n->r, scp);
		}break;
		case T_EXCLAMATION:
		{
			ret->type = AST_OPPOSITE;
			ret->ast = AstFromNode(lang_stat, n->r, scp);
		}break;
		case  T_TILDE:
		{
			ret->type = AST_NEGATE;
			ret->ast = AstFromNode(lang_stat, n->r, scp);
		}break;
		case  T_MINUS:
		{
			ret->type = AST_NEGATIVE;
			ret->ast = AstFromNode(lang_stat, n->r, scp);
		}break;
		case  T_AMPERSAND:
		{
			ret->type = AST_ADDRESS_OF;
			ret->ast = AstFromNode(lang_stat, n->r, scp);
		}break;
		case T_MUL:
		{
			ret->type = AST_DEREF;
			node* cur = n;
			while (cur->type == N_UNOP && n->t->type == T_MUL)
			{
				ret->deref.times++;
				cur = cur->r;
			}
			ret->deref.exp = AstFromNode(lang_stat, cur, scp);
			ret->deref.type = DescendNode(lang_stat, n, scp);
		}break;
		default:
			ASSERT(0)
		}
	}break;
	case node_type::N_CAST:
    {
        ret->type = AST_CAST;
		NameFindingGetType(lang_stat, n->l, scp, ret->cast.type);
		ret->cast.type.type = FromTypeToVarType(ret->cast.type.type);
        ret->cast.casted = AstFromNode(lang_stat, n->r, scp);
        //ret->cast.type = AstFromNode(lang_stat, n->l, scp);
    }break;
	case node_type::N_INDEX:
    {
        ret->type = AST_INDEX;
		ret->index.type = AST_INDEX_TP_NORMAL;
		ret->index.lhs_type = DescendNode(lang_stat, n->l, scp);
		ret->index.lhs = AstFromNode(lang_stat, n->l, scp);
		ret->index.rhs = AstFromNode(lang_stat, n->r, scp);
    }break;
	case node_type::N_ELSE:
    {
        ret->type = AST_ELSE;
        ret->cond.scope = AstFromNode(lang_stat, n->r, scp);
    }break;
	case node_type::N_ELSE_IF:
    {
        ret->type = AST_ELSE_IF;
        ret->cond.cond = AstFromNode(lang_stat, n->l->l, scp);
        ret->cond.scope = AstFromNode(lang_stat, n->l->r, scp);
    }break;
	case node_type::N_WHILE:
    {
        ret->type = AST_WHILE;
        ret->loop.cond = AstFromNode(lang_stat, n->l, scp);
        ret->loop.scope = AstFromNode(lang_stat, n->r, scp);
    }break;
	case node_type::N_IF:
    {
        ret->type = AST_IF;

        ret->cond.cond = AstFromNode(lang_stat, n->l->l, scp);

        ret->cond.scope = AstFromNode(lang_stat, n->l->r, scp);
        node *cur = n->r;
		while (cur && (cur->type == N_ELSE_IF || cur->type == N_ELSE))
		{
			ret->cond.elses.emplace_back(AstFromNode(lang_stat, cur, scp));
			cur = cur->r;
		}


		if (CheckStmntWithoutSemicolon(lang_stat, &ret->cond.scope->stats))
			GetLastStmntType(lang_stat, n->l->r->r, scp, ret->cond.expr_type);

		FOR_VEC(cond, ret->cond.elses)
		{
			ast_rep* c = *cond;
			CheckStmntWithoutSemicolon(lang_stat, &c->cond.scope->stats);
		}


    }break;
	case node_type::N_BINOP:
    {
		//ret->t = n->l->t;
        if(n->t->type == T_COLON)
            return AstFromNode(lang_stat, n->l, scp);

        ret->type = AST_BINOP;
        ret->op = n->t->type;

		own_std::vector<node*> node_stack;
		node* cur_node = n;
        node_stack.emplace_back(n);

        // putting all equal operators in a single array
		while (cur_node->l && cur_node->l->type == node_type::N_BINOP && cur_node->l->t->type == n->t->type) 
		{
			node_stack.emplace_back(cur_node->l);
			cur_node = cur_node->l;
		}
		int last_idx = node_stack.size() - 1;
		node *cur = *(node_stack.begin() + last_idx);

		ret->lhs_tp = DescendNode(lang_stat, cur->l, scp);

		ast_rep *lhs = AstFromNode(lang_stat, cur->l, scp);
		ast_rep *rhs = AstFromNode(lang_stat, cur->r, scp);

		ret->expr.emplace_back(lhs);
		ret->expr.emplace_back(rhs);

        for(int i = node_stack.size() - 2; i >= 0; i--)
        {
            cur = *(node_stack.begin() + i);
            rhs = AstFromNode(lang_stat, cur->r, scp);
            ret->expr.emplace_back(rhs);
        }


        switch(n->t->type)
        {
		case T_POINT:
		{
			ret->point_get_last_val = true;

			node *first_node = *(node_stack.begin() + last_idx);

			dummy_type = DescendNode(lang_stat, first_node->l, scp);
			
			if (dummy_type.type == TYPE_ENUM_TYPE)
			{
				decl2 *d = dummy_type.e_decl->type.GetEnumDecl(n->r->t->str);
				ret->type = AST_INT;
				ret->num = d->type.e_idx;;
			}
			else
			{

				auto new_ar = (own_std::vector<ast_point> *) AllocMiscData(lang_stat, sizeof(own_std::vector<ast_point>));
				memset(new_ar, 0, sizeof(*new_ar));

				ast_point aux;
				aux.decl_strct = dummy_type.strct->this_decl;
				aux.exp = ret->expr[0];
				new_ar->emplace_back(aux);

				decl2* strct = aux.decl_strct;
				for (int i = node_stack.size() - 1; i >= 0; i--)
				{
					first_node = *(node_stack.begin() + i);

					decl2* is_struct = nullptr;
					bool is_tuple = IS_FLAG_ON(strct->type.strct->flags, TP_STRCT_TUPLE);
					if (is_tuple)
					{
						is_struct = strct->type.strct->scp->vars[first_node->t->i];
					}
					else
						is_struct = FindIdentifier(first_node->r->t->str, strct->type.strct->scp, &dummy_type);
					ASSERT(is_struct);
					if (is_struct)
					{
						aux.decl_strct = is_struct;
						aux.exp = AstFromNode(lang_stat, first_node->r, strct->type.strct->scp);
						if (is_tuple)
						{
							aux.exp->type = AST_IDENT;
							aux.exp->decl = is_struct;
						}
				
						strct = aux.decl_strct;
						rhs->decl = is_struct;
					}
					new_ar->emplace_back(aux);
				}
				// THIS CAUSES A LEAK
				memcpy(&ret->points, new_ar, sizeof(*new_ar));
			}
			//ret->points = *new_ar;

		}break;
		case T_COND_OR:
		case T_PIPE:
		case T_COND_AND:
		case T_COMMA:
		case T_MUL:
		case T_COND_NE:
		case T_LESSER_EQ:
		case T_PLUS_EQUAL:
		case T_MINUS_EQUAL:
		case T_COND_EQ:
		case T_AMPERSAND:
		case T_LESSER_THAN:
		case T_GREATER_EQ:
		case T_GREATER_THAN:
		case T_MINUS:
		case T_DIV:
		case T_PERCENT:
		{
		}break;
        case T_EQUAL:
        {
            
        }break;
        case T_PLUS:
        {
            
        }break;
        default:
            ASSERT(0);
        }
    }break;
	case node_type::N_INT:
    {
        ret->type = AST_INT;
        ret->num = n->t->i;
    }break;
	case node_type::N_IDENTIFIER:
    {
        ret->type = AST_IDENT;
        type2 ret_type;
		ret->decl = FindIdentifier(n->t->str, scp, &ret_type);
		if (!ret->decl)
		{
			ret->str = n->t->str.substr();
		}
    }break;
	case node_type::N_KEYWORD:
    {
        switch(n->kw)
        {
        case KW_DBG_BREAK:
        {
            ret->type = AST_DBG_BREAK;
        }break;
        case KW_BREAK:
        {
            ret->type = AST_BREAK;
        }break;
        case KW_USING:
        {
            ret->type = AST_EMPTY;
        }break;
        case KW_TRUE:
        {
            ret->type = AST_INT;
			ret->num = 1;
        }break;
        case KW_NIL:
        case KW_FALSE:
        {
            ret->type = AST_INT;
			ret->num = 0;
        }break;
        case KW_RETURN:
        {
            ret->type = AST_RET;
			if (n->r)
			{
				ret->ret.ast = AstFromNode(lang_stat, n->r, scp);
				ret->ret.tp = DescendNode(lang_stat, n->r, scp);
			}
        }break;
		default:
			ASSERT(0);
        }
    }break;
	case node_type::N_STR_LIT:
	{
		ret->type = AST_STR_LIT;
		ret->str = own_std::string(n->t->str);
	}break;
	case node_type::N_SCOPE:
    {
		ret->type = AST_STATS;
		ret->line_number = n->r->t->line;
		ast_rep* rhs = AstFromNode(lang_stat, n->r, n->scp);
		if (rhs->type == AST_STATS)
			INSERT_VEC(ret->stats, rhs->stats);
		else
			ret->stats.emplace_back(rhs);
		/*
		if (n->r && n->r->type != N_STMNT && n->r->type != N_IF)
		{
		}
		else
			ret = AstFromNode(lang_stat, n->r, n->scp);
			*/
    }break;
	case node_type::N_ARRAY_CONSTRUCTION:
	{
		auto last = lang_stat->cur_func;
        ret->type = AST_ARRAY_COSTRUCTION;
		type2 tp_ar;
		NameFindingGetType(lang_stat, n->l, scp, tp_ar);

		ret->ar_constr.type = *tp_ar.tp;

		int tp_size = GetTypeSize(&tp_ar);
		lang_stat->cur_strct_constrct_size_per_statement += tp_size;

		ret->ar_constr.at_offset = lang_stat->cur_strct_constrct_size_per_statement;


		int cur_sz = lang_stat->cur_func->strct_constrct_size_per_statement;
		lang_stat->cur_func->strct_constrct_size_per_statement = max(cur_sz, lang_stat->cur_strct_constrct_size_per_statement);

		//type_struct2* strct =  ret->strct_constr.strct;
		FOR_VEC(c, *n->exprs)
		{
			//ast_struct_construct_info info;
			//info.var = strct->FindDecl(c->n->l->t->str);
			ast_rep *a = AstFromNode(lang_stat, c->n, scp);
			ret->ar_constr.commas.emplace_back(a);
		}
		lang_stat->cur_strct_constrct_size_per_statement -= tp_size;
	}break;
	case node_type::N_STRUCT_CONSTRUCTION:
	{
		auto last = lang_stat->cur_func;
        ret->type = AST_STRUCT_COSTRUCTION;
		ret->strct_constr.strct = FindIdentifier(n->l->t->str, scp, &dummy_type)->type.strct;

		int tp_size = ret->strct_constr.strct->size;
		lang_stat->cur_strct_constrct_size_per_statement += tp_size;

		ret->strct_constr.at_offset = lang_stat->cur_strct_constrct_size_per_statement;


		int cur_sz = lang_stat->cur_func->strct_constrct_size_per_statement;
		lang_stat->cur_func->strct_constrct_size_per_statement = max(cur_sz, lang_stat->cur_strct_constrct_size_per_statement);

		type_struct2* strct =  ret->strct_constr.strct;
		int i = 0;
		FOR_VEC(c, *n->exprs)
		{
			ast_struct_construct_info info;
			if (IS_FLAG_ON(strct->flags, TP_STRCT_TUPLE))
			{
				info.var = strct->scp->vars[i];
				info.exp = AstFromNode(lang_stat, c->n, scp);
			}
			else
			{
				info.var = strct->FindDecl(c->n->l->t->str);
				info.exp = AstFromNode(lang_stat, c->n->r, scp);
			}
			ret->strct_constr.commas.emplace_back(info);
			i++;
		}
		lang_stat->cur_strct_constrct_size_per_statement -= tp_size;
	}break;
	case node_type::N_OP_OVERLOAD:
	case node_type::N_FUNC_DECL:
	{
        ret->type = AST_FUNC;
		ASSERT(n->fdecl);
        ret->func.fdecl = n->fdecl;

		auto last = lang_stat->cur_func;
		lang_stat->cur_func = n->fdecl;
        
        ret->func.fdecl = n->fdecl;

        ret->func.stats = AstFromNode(lang_stat, n->r->r, n->fdecl->scp);
		if (ret->func.stats->type != AST_STATS)
		{
			ast_rep* new_ast = NewAst();
			new_ast->type = AST_STATS;
			new_ast->stats.emplace_back(ret->func.stats);
			ret->func.stats = new_ast;
		}

		lang_stat->cur_func = last;

	}break;
	case node_type::N_CALL:
	{
		ret->type = AST_CALL;
		decl2* decl = FindIdentifier(n->l->t->str, scp, &dummy_type);
		ret->call.fdecl = decl->type.fdecl;
		ret->call.in_func = scp->fdecl;

		ret->call.indirect = false;
		if (decl->type.type == TYPE_FUNC_PTR)
		{
			ret->call.indirect = true;
			ret->call.func_ptr_var = decl;
		}

		func_decl* f = ret->call.fdecl;

		if (IS_FLAG_ON(f->flags, FUNC_DECL_INTERNAL))
		{
			if (f->name == "sizeof")
			{
				dummy_type = DescendNode(lang_stat, n->r, scp);
				ret->type = AST_INT;
				ret->num = GetTypeSize(&dummy_type);
				
			}
			else
				ASSERT(0);
		}
		else if(n->r)
		{

			if (n->r->type == N_BINOP && n->r->t->type == T_COMMA)
			{
				ast_rep* args = AstFromNode(lang_stat, n->r, scp);
				ret->call.args = args->expr;
			}
			else
			{
				ret->call.args.emplace_back(AstFromNode(lang_stat, n->r, scp));
			}
		}
	}break;
	case node_type::N_FLOAT:
	{
		ret->type = AST_FLOAT;
		ret->f32 = n->t->f;

	}break;
	case node_type::N_STMNT:
	{
		ret->type = AST_STATS;
		ret->line_number = n->t->line;
		own_std::vector<node*> node_stack;
		node* cur_node = n;
		node_stack.emplace_back(cur_node);
		while (cur_node->l->type == node_type::N_STMNT)
		{
			node_stack.emplace_back(cur_node->l);
			cur_node = cur_node->l;
		}
		int size = node_stack.size();

		ast_rep* lhs = AstFromNode(lang_stat, cur_node->l, scp);
		if (IS_FLAG_ON(cur_node->l->flags, NODE_FLAGS_STMNT_WITHOUT_SEMICOLON))
			lhs->stmnt_without_semicolon = true;

		if (lhs->type == AST_STATS)
			INSERT_VEC(ret->stats, lhs->stats);
		else
			ret->stats.emplace_back(lhs);

        for(int i = node_stack.size() - 1; i >= 0; i--)
        {
			node* s = *(node_stack.begin() + i);
			if (!s->r)
				continue;
			ast_rep* rhs = AstFromNode(lang_stat, s->r, scp);

			if (IS_FLAG_ON(s->r->flags, NODE_FLAGS_STMNT_WITHOUT_SEMICOLON))
				rhs->stmnt_without_semicolon = true;

			if (rhs->type == AST_STATS)
				INSERT_VEC(ret->stats, rhs->stats);
			else
				ret->stats.emplace_back(rhs);

			/*
			if (s->l->type != N_STMNT)
				ret->stats.emplace_back(AstFromNode(lang_stat, s->l, scp));
			if(s->r->type != N_STMNT)
				ret->stats.emplace_back(AstFromNode(lang_stat, s->r, scp));
			*/

        }
	}break;
	case N_FOR:
	{
		ret->type = AST_FOR;
		node* for_nd = n;
		scp = n->r->scp;
		bool is_rev = false;
		if (n->l->type == N_KEYWORD && n->l->kw == KW_REV)
		{
			is_rev = true;
			n = n->l->r;
		}
		else
			n = n->l;
		if (IsNodeOperator(n, T_IN))
		{
			node* var_name_nd = n->l;
			ast_rep* var_name = AstFromNode(lang_stat, var_name_nd, scp);
			ast_rep* var_name_og = var_name;
			if (IsNodeOperator(n->r, T_TWO_POINTS))
			{
				type2 var_type = DescendNode(lang_stat, var_name_nd, scp);

				node* start_val_nd = n->r->l;
				node* end_val_nd = n->r->r;
				ast_rep* start_val = AstFromNode(lang_stat, start_val_nd, scp);
				ast_rep* end_val = AstFromNode(lang_stat, end_val_nd, scp);

				tkn_type2 cmp_op;

				if (var_type.ptr > 0)
				{
					ast_rep* address = NewAst();
					address->type = AST_ADDRESS_OF;
					address->ast = var_name;
					address->line_number = n->t->line;

					ast_rep* cast_to_ptr_u64 = NewAst();
					cast_to_ptr_u64->type = AST_CAST;
					cast_to_ptr_u64->cast.casted = address;
					cast_to_ptr_u64->cast.type.type = TYPE_U64;
					cast_to_ptr_u64->cast.type.ptr = 1;
					cast_to_ptr_u64->line_number = n->t->line;
					var_name = cast_to_ptr_u64;

					cast_to_ptr_u64 = NewAst();
					cast_to_ptr_u64->type = AST_CAST;
					cast_to_ptr_u64->cast.casted = start_val;
					cast_to_ptr_u64->cast.type.type = TYPE_U64;
					cast_to_ptr_u64->cast.type.ptr = 1;
					cast_to_ptr_u64->line_number = n->t->line;
					start_val = cast_to_ptr_u64;


					cast_to_ptr_u64 = NewAst();
					cast_to_ptr_u64->type = AST_CAST;
					cast_to_ptr_u64->cast.casted = end_val;
					cast_to_ptr_u64->cast.type.type = TYPE_U64;
					cast_to_ptr_u64->cast.type.ptr = 1;
					cast_to_ptr_u64->line_number = n->t->line;
					end_val = cast_to_ptr_u64;

					// cast(*u64)&var_name = cast(*64)(cast(u64)var_name + sizeof(type))
					ast_rep *cast_to_u64 = NewAst();
					cast_to_u64->type = AST_CAST;
					cast_to_u64->cast.casted = var_name_og;
					cast_to_u64->cast.type.type = TYPE_U64;
					cast_to_u64->cast.type.ptr = 0;

					ast_rep *type_size = NewAst();
					type_size->type = AST_INT;
					var_type.ptr--;
					type_size->num = GetTypeSize(&var_type);
					var_type.ptr++;

					ast_rep *bin_plus = NewAst();
					bin_plus->type = AST_BINOP;
					bin_plus->op = T_PLUS;
					bin_plus->lhs_tp = var_type;
					bin_plus->expr.emplace_back(cast_to_u64);
					bin_plus->expr.emplace_back(type_size);

					if (is_rev)
						bin_plus->op = T_MINUS;

					ast_rep *equal = NewAst();
					equal->type = AST_BINOP;
					equal->op = T_EQUAL;
					equal->lhs_tp = var_type;
					equal->expr.emplace_back(var_name);
					equal->expr.emplace_back(bin_plus);
					ret->for_info.at_loop_end_stat = equal;
					equal->line_number = n->t->line;

				}
				else
				{
					ast_rep* inc_val = NewAst();

					if (is_rev)
					{
						inc_val->type = AST_MINUS_MINUS;
					}
					else
					{
						inc_val->type = AST_PLUS_PLUS;
					}
					inc_val->unop_assign.ast = var_name;
					inc_val->unop_assign.tp = var_type;
					ret->for_info.at_loop_end_stat = inc_val;
				}
				
				if (is_rev)
				{
					cmp_op = T_GREATER_THAN;
				}
				else
				{
					cmp_op = T_LESSER_THAN;
				}
				ret->for_info.start_stat = CreateAstBin(lang_stat, T_EQUAL, var_name, start_val, start_val_nd, scp);

				ret->for_info.cond_stat = CreateAstBin(lang_stat, cmp_op, var_name, end_val, end_val_nd, scp);

			}
			else
			{
				ASSERT(0)
			}

		}
		else
		{
			ASSERT(0)
		}
		ret->for_info.scope = AstFromNode(lang_stat, for_nd->r, scp);
	}break;
	case N_APOSTROPHE:
	{
		ret->type = AST_CHAR;
		ret->num = n->t->i;
	}break;
	case N_EMPTY:
		break;
    default:
        ASSERT(0);
    }
    return ret;
}

void GetIRVal(lang_state *lang_stat, ast_rep *ast, ir_val *val)
{
    switch(ast->type)
    {
    case AST_STRUCT_COSTRUCTION:
    {
        val->type = IR_TYPE_ON_STACK;
        val->i = ast->strct_constr.at_offset;
		//val->is_unsigned = IsUnsigned(ast->decl->type.type);
    }break;
    case AST_STR_LIT:
	{
		//InsertIntoDataSect(lang_stat, (void *))
		val->type = IR_TYPE_STR_LIT;
		val->str = std_str_to_heap(lang_stat, &ast->str);
	}break;
    case AST_IDENT:
    {
        val->type = IR_TYPE_DECL;
        val->decl = ast->decl;
		val->ptr = ast->decl->type.ptr;
		val->reg_sz = GetTypeSize(&ast->decl->type);
		val->reg_sz = min(val->reg_sz, 8);
		val->is_unsigned = false;
		if (ast->decl->type.type == TYPE_STATIC_ARRAY)
			val->is_unsigned = IsUnsigned(ast->decl->type.tp->type);
		else if(ast->decl->type.type != TYPE_STRUCT && ast->decl->type.ptr == 0)
			val->is_unsigned = IsUnsigned(ast->decl->type.type);
		if (ast->decl->type.ptr > 0)
			val->is_unsigned = true;
		val->is_float = ast->decl->type.type == TYPE_F32;
    }break;
    case AST_FLOAT:
    {
        val->type = IR_TYPE_F32;
        val->f32 = ast->f32;
		val->reg_sz = 8;
		val->is_unsigned = false;
		val->is_float = true;
    }break;
    case AST_CHAR:
    case AST_INT:
    {
        val->type = IR_TYPE_INT;
        val->i = ast->num;
		val->is_unsigned = ast->num < 0 ? false : true;
		val->reg_sz = 8;
    }break;
    default:
        ASSERT(0)
    }

}

void GenIRBinOp(lang_state *lang_stat, ast_rep *ast, ir_val *lhs, ir_val *rhs, own_std::vector<ir_rep> *out)
{
    switch(ast->op)
    {
    case T_PLUS:
    {
        
    }break;
    default:
        ASSERT(0)

    }
}
bool IsAstSimple(lang_state *lang_stat, ast_rep *ast)
{
    if(ast->type == AST_INT || ast->type == AST_IDENT)
        return true;
    return false;
}

void IRCreateEndBlock(lang_state *lang_stat, int begin_sub_if_idx, own_std::vector<ir_rep> *out, ir_type type)
{
    int end_idx = out->size();

    ir_rep ir;
    ir.type = type;
    ir.block.other_idx = begin_sub_if_idx;

    ir_rep *begin = &(*out)[begin_sub_if_idx];

    out->emplace_back(ir);


	begin->block.other_idx = out->size();
	switch (type)
	{
	case IR_END_STMNT:
	{
		ASSERT(begin->type == IR_BEGIN_STMNT);
	}break;
	}
}
int IRCreateBeginBlock(lang_state *lang_stat, own_std::vector<ir_rep> *out, ir_type type, void *data = nullptr)
{
    ir_rep ir;
    ir.type = type;
    int ret = out->size();

    out->emplace_back(ir);
	if (type == IR_BEGIN_STMNT)
	{
		out->back().block.stmnt.line = (int)(long long)data;
		//ir.type = IR_NOP;
		//out->emplace_back(ir);
	}

    return ret;
}

ir_type FromTokenOpToIRType(tkn_type2 op)
{
	switch (op)
	{
	case T_COND_EQ:
		return IR_CMP_EQ;
	case T_GREATER_EQ:
		return IR_CMP_GE;
	case T_GREATER_THAN:
		return IR_CMP_GT;
	case T_LESSER_THAN:
		return IR_CMP_LT;
	case T_LESSER_EQ:
		return IR_CMP_LE;
	case T_COND_NE:
		return IR_CMP_NE;
	default:
		ASSERT(0)
	}
}
#define REG_FREE_FLAG 0x100
#define REG_SPILLED 0x200
void FreeReg(lang_state* lang_stat, char reg_idx)
{
	lang_stat->regs[reg_idx] &= ~REG_FREE_FLAG;
}
void FreeAllRegs(lang_state* lang_stat)
{
	memset(lang_stat->regs, 0, sizeof(lang_stat->regs));
	//lang_stat->regs[reg_idx] &= ~REG_FREE_FLAG;
}
void AllocSpecificReg(lang_state* lang_stat, char idx)
{
	ASSERT(IS_FLAG_OFF(lang_stat->regs[idx], REG_FREE_FLAG));
	lang_stat->regs[idx] |= REG_FREE_FLAG;
}
char AllocReg(lang_state* lang_stat)
{
	for (int i = 0; i < 7; i++)
	{
		if (IS_FLAG_OFF(lang_stat->regs[i], REG_FREE_FLAG))
		{
			lang_stat->regs[i] |= REG_FREE_FLAG;
			return i;
		}
	}
	ASSERT(0);
}
void GetIRBin(lang_state *lang_stat, ast_rep *ast_bin, own_std::vector<ir_rep> *out, ir_type type, void *data = nullptr)
{
	ir_rep ir;
	memset(&ir, 0, sizeof(ir_rep));
	//ir.assign.to_assign.type = IR_TYPE_REG;
	//ir.assign.to_assign.reg = AllocReg(lang_stat);


	ir.bin.lhs.type = IR_TYPE_REG;
	ir.bin.lhs.reg_sz = 8;
	ir.bin.lhs.reg = AllocReg(lang_stat);

	ir.bin.rhs.type = IR_TYPE_REG;
	ir.bin.rhs.reg_sz = 8;
	ir.bin.rhs.reg = AllocReg(lang_stat);
	GenStackThenIR(lang_stat, ast_bin->expr[0], out, &ir.bin.lhs, &ir.bin.lhs);
	GenStackThenIR(lang_stat, ast_bin->expr[1], out, &ir.bin.rhs, &ir.bin.rhs);

	ir.bin.rhs.is_unsigned = ir.bin.lhs.is_unsigned;

	ir.type = type;
	ir.bin.op = ast_bin->op;
	switch (type)
	{
	case IR_CMP_NE:
	case IR_CMP_LE:
	case IR_CMP_EQ:
	case IR_CMP_LT:
	case IR_CMP_GE:
	case IR_CMP_GT:
	{
		ir.bin.it_is_jmp_if_true = (bool)data;
	}break;
	default:
		ASSERT(0);
	}

	bool lhs_reg_ptr_1 = ir.bin.lhs.type == IR_TYPE_REG && ir.bin.lhs.ptr == 1;
	bool rhs_reg_ptr_1 = ir.bin.rhs.type == IR_TYPE_REG && ir.bin.rhs.ptr == 1;
	// sometimes the ptr can be -1, meaning it wasm the rhs of a point
	// -1 would tell to not defer the var yet
	//if(ir.bin.lhs.type == IR_TYPE_DECL)
	ir.bin.lhs.deref += 1;

	int diff = ir.bin.lhs.ptr - ir.bin.lhs.deref;
	if (diff < 0)
		diff = 0;
	//ir.bin.lhs.deref += diff;;
	//if (ir.bin.lhs.ptr > 0)
		//ir.bin.lhs.deref = 1;

	//if(ir.bin.rhs.type == IR_TYPE_DECL)
	ir.bin.rhs.deref += 1;
	diff = ir.bin.rhs.ptr - ir.bin.rhs.deref;
	if (diff < 0)
		diff = 0;
	//ir.bin.rhs.deref += diff;
	//if (ir.bin.rhs.ptr > 0)
		//ir.bin.rhs.deref++;

	out->emplace_back(ir);



		//ir.type = IR_CMP;


}

void GetIRFromAst(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out);
bool IsCondAndOr(tkn_type2 t)
{
	return t == T_COND_AND || t == T_COND_OR;
}
void GetIRCond(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out)
{
	ir_rep ir = {};
	if (ast->type == AST_BINOP && ast->op != T_POINT)
	{
		if (!IsCondAndOr(ast->op))
		{
			tkn_type2 opposite = OppositeCondCmp(ast->op);
			ast->op = opposite;
			GetIRBin(lang_stat, ast, out, FromTokenOpToIRType(opposite), (void*)false);
		}
		else
		{
			GetIRFromAst(lang_stat, ast, out);
		}
	}
	else if (ast->type == AST_OPPOSITE && ast->opposite.exp->type == AST_BINOP)
	{
		if (IsCondAndOr(ast->opposite.exp->op))
		{
			int if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_BLOCK);
			int sub_if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_SUB_IF_BLOCK);
			int cond_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_COND_BLOCK);

			GetIRFromAst(lang_stat, ast->opposite.exp, out);

			IRCreateEndBlock(lang_stat, cond_idx, out, IR_END_COND_BLOCK);

			char reg = AllocReg(lang_stat);

			CreateOppositeRegAssigmentAfterCondChecking(lang_stat, out, sub_if_idx, if_idx, reg);

			ir.type = IR_CMP_EQ;
			ir.bin.op = T_COND_EQ;
			ir.bin.lhs.type = IR_TYPE_REG;
			ir.bin.lhs.reg = reg;
			ir.bin.lhs.reg_sz = 8;
			ir.bin.lhs.deref = 1;
			ir.bin.rhs.type = IR_TYPE_INT;
			ir.bin.rhs.i = 0;
			out->emplace_back(ir);
		}
		else
		{
			char reg = AllocReg(lang_stat);
			ir.bin.lhs.type = IR_TYPE_REG;
			ir.bin.lhs.reg = reg;
			ir.bin.lhs.reg_sz = 8;

			GenStackThenIR(lang_stat, ast->opposite.exp, out, &ir.bin.lhs);
			ir.type = IR_CMP_NE;
			ir.bin.op = T_COND_NE;
			ir.bin.lhs.type = IR_TYPE_REG;
			ir.bin.lhs.reg = reg;
			ir.bin.lhs.reg_sz  = 8;
			ir.bin.lhs.deref  = 1;
			ir.bin.rhs.type = IR_TYPE_INT;
			ir.bin.rhs.i = 0;
			out->emplace_back(ir);
		}
	}
	else
	{
		ir.type = IR_CMP_EQ;
		ir.bin.op = T_COND_EQ;
		ir.bin.lhs.type = IR_TYPE_REG;
		ir.bin.lhs.reg  = AllocReg(lang_stat);
		ir.bin.lhs.reg_sz  = 8;
		ir.bin.rhs.type = IR_TYPE_INT;
		ir.bin.rhs.i = 0;
		GenStackThenIR(lang_stat, ast, out, &ir.bin.lhs);

		if (ast->type == AST_IDENT || ast->type == AST_OPPOSITE || ast->type == AST_CALL)
		{
			ir.bin.lhs.deref = 1;
		}
		else if (ast->type == AST_DEREF || ast->type == AST_BINOP)
		{
			ir.bin.lhs.deref += 1;
		}
		else
		{
		}
		ir.bin.rhs.is_unsigned = ir.bin.lhs.is_unsigned;;
		ir.bin.lhs.ptr = 1;
		out->emplace_back(ir);


	}
}

void GenIRSubBin(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out, ir_val_type to, void* data)
{
	ast_rep* lhs_exp = ast->expr[0];
	ast_rep* rhs_exp = ast->expr[1];

	bool lhs_complex = IsAstSimple(lang_stat, lhs_exp);
	bool rhs_complex = IsAstSimple(lang_stat, rhs_exp);

	//switch(lhs_complex)

}

void PushAstsInOrder(lang_state* lang_stat, ast_rep* ast, own_std::vector<ast_rep*>* out);
void PushArrayOfAsts(lang_state* lang_stat, own_std::vector<ast_rep *> *ar, ast_rep *op, own_std::vector<ast_rep*>* out)
{
	PushAstsInOrder(lang_stat, *ar->begin(), out);
	for (int i = 1; i < ar->size(); i++)
	{
		ast_rep* cur = (*ar)[i];
		PushAstsInOrder(lang_stat, cur, out);
		out->emplace_back(op);
		//lhs /
	}

}
void PushAstsInOrder(lang_state* lang_stat, ast_rep* ast, own_std::vector<ast_rep*>* out)
{
	switch (ast->type)
	{
	case AST_CALL:
	{

		FOR_VEC(arg, ast->call.args)
		{
			PushAstsInOrder(lang_stat, (*arg), out);
		}
		out->emplace_back(ast);
	}break;
	case AST_INT:
	case AST_FLOAT:
	case AST_STR_LIT:
	case AST_CHAR:
	case AST_IDENT:
	{
		out->emplace_back(ast);
	}break;
	case AST_NEGATE:
	case AST_NEGATIVE:
	case AST_OPPOSITE:
	{
		PushAstsInOrder(lang_stat, ast->ast, out);
		out->emplace_back(ast);
	}break;
	case AST_CAST:
	{
		PushAstsInOrder(lang_stat, ast->cast.casted, out);
		out->emplace_back(ast);
	}break;
	case AST_ARRAY_COSTRUCTION:
	{
		PushAstsInOrder(lang_stat, ast->ar_constr.commas[0], out);
		for (int i = 1; i < ast->ar_constr.commas.size(); i++)
		{
			ast_rep* cur = ast->ar_constr.commas[i];
			PushAstsInOrder(lang_stat, cur, out);
		}
		out->emplace_back(ast);
	}break;
	case AST_STRUCT_COSTRUCTION:
	{
		PushAstsInOrder(lang_stat, ast->strct_constr.commas[0].exp, out);
		for (int i = 1; i < ast->strct_constr.commas.size(); i++)
		{
			ast_rep* cur = ast->strct_constr.commas[i].exp;
			PushAstsInOrder(lang_stat, cur, out);
		}
		out->emplace_back(ast);
	}break;
	case AST_ADDRESS_OF:
	{
		PushAstsInOrder(lang_stat, ast->ast, out);
		out->emplace_back(ast);
	}break;
	case AST_INDEX:
	{
		PushAstsInOrder(lang_stat, ast->index.lhs, out);
		PushAstsInOrder(lang_stat, ast->index.rhs, out);
		out->emplace_back(ast);
	}break;
	case AST_DEREF:
	{
		PushAstsInOrder(lang_stat, ast->deref.exp, out);
		out->emplace_back(ast);
	}break;
	case AST_BINOP:
	{
		own_std::vector<ast_rep*>* ar = nullptr;
		if (ast->op == T_POINT)
		{
			PushAstsInOrder(lang_stat, ast->points[0].exp, out);
			for (int i = 1; i < ast->points.size(); i++)
			{
				ast_rep* cur = ast->points[i].exp;

				PushAstsInOrder(lang_stat, cur, out);
				//lhs /
			}
			out->emplace_back(ast);
		}
		else
		{
			PushArrayOfAsts(lang_stat, &ast->expr, ast, out);
		}
	}break;
	default:
		ASSERT(0);
	}
}



void GenIRComplex(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out, ir_val *dst_val)
{
	ASSERT(0);
	short reg = dst_val->reg;
	//char ptr = dst_
	ir_rep ir = {};

	int begin_complex_idx = out->size();
	ir.type = IR_BEGIN_COMPLEX;

	ir.complx.dst = *dst_val;
	out->emplace_back(ir);
	
	own_std::vector<ast_rep*> exps;
	PushAstsInOrder(lang_stat, ast, &exps);

	int state = 0;


}
bool IsIrValFloat(ir_val* val)
{
	type2* decl_type = &val->decl->type;

	bool is_decl = val->type == IR_TYPE_DECL;
	bool is_ar_float = false;
	if(is_decl)
		is_ar_float = decl_type->type == TYPE_STATIC_ARRAY && decl_type->tp->type == TYPE_F32;

	return is_decl && (val->decl->type.type == TYPE_F32  || is_ar_float) || val->is_float || val->type == IR_TYPE_F32;
}

void CreateOppositeRegAssigmentAfterCondChecking(lang_state *lang_stat, own_std::vector<ir_rep> *out, int sub_if_idx, int if_idx, int reg)
{
	ir_rep ir = {};
	ir.type = IR_ASSIGNMENT;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg = reg;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.only_lhs = true;

	ir.assign.lhs.type = IR_TYPE_INT;
	ir.assign.lhs.i = 0;
	out->emplace_back(ir);


	ir.type = IR_BREAK_OUT_IF_BLOCK;
	out->emplace_back(ir);

	IRCreateEndBlock(lang_stat, sub_if_idx, out, IR_END_SUB_IF_BLOCK);

	sub_if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_SUB_IF_BLOCK);
	ir = {};
	ir.type = IR_ASSIGNMENT;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg = reg;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.only_lhs = true;

	ir.assign.lhs.type = IR_TYPE_INT;
	ir.assign.lhs.i = 1;
	out->emplace_back(ir);
	IRCreateEndBlock(lang_stat, sub_if_idx, out, IR_END_SUB_IF_BLOCK);
	IRCreateEndBlock(lang_stat, if_idx, out, IR_END_IF_BLOCK);

}
void GinIRMemCpy(lang_state* lang_stat, own_std::vector<ir_rep>* out)
{
	ir_rep ir;
	
	ir.type = IR_ASSIGNMENT;
	ir.assign.only_lhs = true;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.is_float = false;
	ir.assign.to_assign.reg = 0;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.lhs.type = IR_TYPE_INT;
	ir.assign.lhs.i = 0;
	ir.assign.lhs.is_float = false;
	ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
	out->emplace_back(ir);
	

	int block_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_BLOCK);
	int loop_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_LOOP_BLOCK);
	//int stmnt_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_STMNT, (void *)(long long)ast->line_number);
	//int cond_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_BLOCK);
	//GetIRCond(lang_stat, ast->loop.cond, out);
	//IRCreateEndBlock(lang_stat, stmnt_idx, out, IR_END_STMNT);

	// while reg0 < arg_reg2
	ir = {};
	ir.type = IR_CMP_GE;
	ir.bin.op = T_GREATER_EQ;
	ir.bin.only_lhs = false;
	ir.bin.it_is_jmp_if_true = false;
	ir.bin.lhs.type = IR_TYPE_REG;
	ir.bin.lhs.reg = 0;
	ir.bin.lhs.reg_sz = 4;
	ir.bin.lhs.deref = 1;
	ir.bin.rhs.type = IR_TYPE_ARG_REG;
	ir.bin.rhs.reg = 2;
	ir.bin.rhs.reg_sz = 4;
	ir.bin.rhs.deref = 1;
	//ir.bin.to_assign.reg_sz = 4;

	out->emplace_back(ir);

	// reg1 = arg_reg0 + reg0
	ir.type = IR_ASSIGNMENT;
	ir.assign.only_lhs = false;
	ir.assign.to_assign.is_float = false;
	ir.assign.lhs.is_float = false;
	ir.assign.rhs.is_float = false;
	//ir.assign.only_lhs = true;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg = 1;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.lhs.type = IR_TYPE_ARG_REG;
	ir.assign.lhs.reg = 0;
	ir.assign.lhs.reg_sz = 8;
	ir.assign.lhs.deref = 1;
	ir.assign.op = T_PLUS;
	ir.assign.rhs.type = IR_TYPE_REG;
	ir.assign.rhs.reg = 0;
	ir.assign.rhs.reg_sz = 8;
	ir.assign.rhs.deref = 1;
	ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
	out->emplace_back(ir);

	// reg2 = arg_reg1 + reg0
	ir.type = IR_ASSIGNMENT;
	//ir.assign.only_lhs = true;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg = 2;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.lhs.type = IR_TYPE_ARG_REG;
	ir.assign.lhs.reg = 1;
	ir.assign.lhs.reg_sz = 8;
	ir.assign.lhs.deref = 1;
	ir.assign.op = T_PLUS;
	ir.assign.rhs.type = IR_TYPE_REG;
	ir.assign.rhs.reg = 0;
	ir.assign.rhs.reg_sz = 8;
	ir.assign.rhs.deref = 1;
	ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
	out->emplace_back(ir);

	// *reg1 = *reg2
	ir.type = IR_ASSIGNMENT;
	ir.assign.only_lhs = true;
	//ir.assign.only_lhs = true;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg = 1;
	ir.assign.to_assign.reg_sz = 1;
	ir.assign.to_assign.deref = 1;
	ir.assign.lhs.type = IR_TYPE_REG;
	ir.assign.lhs.reg = 2;
	ir.assign.lhs.reg_sz = 1;
	ir.assign.lhs.deref = 2;
	ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
	out->emplace_back(ir);

	// reg0 = reg0 + 1
	ir.type = IR_ASSIGNMENT;
	ir.assign.only_lhs = false;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg = 0;
	ir.assign.to_assign.deref = 0;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.lhs.type = IR_TYPE_REG;
	ir.assign.lhs.reg = 0;
	ir.assign.lhs.reg_sz = 8;
	ir.assign.lhs.deref = 1;
	ir.assign.op = T_PLUS;
	ir.assign.rhs.type = IR_TYPE_INT;
	ir.assign.rhs.i = 1;
	ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
	out->emplace_back(ir);
	//IRCreateEndBlock(lang_stat, cond_idx, out, IR_END_IF_BLOCK);
	IRCreateEndBlock(lang_stat, loop_idx, out, IR_END_LOOP_BLOCK);
	IRCreateEndBlock(lang_stat, block_idx, out, IR_END_BLOCK);

}
#define IR_VAL_FROM_POINT 0x100
void GinIRFromStack(lang_state* lang_stat, own_std::vector<ast_rep *> &exps, own_std::vector<ir_rep> *out, ir_val *top_info)
{
	own_std::vector<ir_val> stack;
	stack.reserve(4);
	ir_rep ir;
	for(int j = 0; j < exps.size(); j++)
	{
		ast_rep* e = exps[j];
		
		ir_val val;
		memset(&val, 0, sizeof(ir_val));
		memset(&ir, 0, sizeof(ir_rep));
		ir.type = IR_NONE;

		switch (e->type)
		{
		case AST_CALL:
		{

			int cur_biggest = e->call.in_func->biggest_call_args;
			e->call.in_func->biggest_call_args = max(cur_biggest, e->call.fdecl->args.size());

			
			int more_stack_vals = stack.size() - e->call.args.size();
			if (more_stack_vals > 0)
			{
				// spilling
				ir_val* to_spill = stack.begin();

				cur_biggest = e->call.in_func->to_spill_size;

				e->call.in_func->to_spill_size = max(cur_biggest, more_stack_vals);

				int cur_spill_offset = 8;

				while (more_stack_vals > 0)
				{
					if (to_spill->type == IR_TYPE_REG)
					{
						ir = {};
						ir.type = IR_ASSIGNMENT;
						ir.assign.to_assign.type = IR_TYPE_ON_STACK;
						ir.assign.to_assign.reg_sz = 8;
						ir.assign.to_assign.on_stack_type = ON_STACK_SPILL;
						ir.assign.to_assign.i = cur_spill_offset;
						ir.assign.only_lhs = true;
						ir.assign.lhs = *to_spill;
						ir.assign.lhs.deref = 1;
						out->emplace_back(ir);


						cur_spill_offset += 8;
					}
					to_spill++;
					more_stack_vals--;
				}
			}

			if (e->call.args.size() > 0)
			{
				for (int i = e->call.args.size() - 1; i >= 0; i--)
				{
					ir_val* top = &stack[stack.size() - 1];
					stack.pop_back();
					ir = {};
					ir.type = IR_ASSIGNMENT;
					ir.assign.to_assign.type = IR_TYPE_ARG_REG;
					ir.assign.to_assign.reg = i;
					ir.assign.to_assign.reg_sz = 8;
					ir.assign.to_assign.is_float = top->is_float;
					ir.assign.only_lhs = true;

					ir.assign.lhs = *top;
					ir.assign.lhs.deref = 1;
					ir.assign.lhs.reg_sz = 8;
					ir.assign.lhs.deref += top->deref;
					out->emplace_back(ir);

					if (top->type == IR_TYPE_REG)
					{
						FreeReg(lang_stat, top->reg);
					}
				}
			}
			
			ir.type = IR_CALL;

			ir.call.is_outsider = false;
			if (IS_FLAG_ON(e->call.fdecl->flags, FUNC_DECL_IS_OUTSIDER))
				ir.call.is_outsider = true;

			ir.call.fdecl = e->call.fdecl;

			if (e->call.indirect)
			{
				ir.type = IR_INDIRECT_CALL;
				ir.bin.lhs.type = IR_TYPE_DECL;
				ir.bin.lhs.decl = e->call.func_ptr_var;
				ir.bin.lhs.deref = 1;
				ir.bin.lhs.is_float = false;
			}
			if (e->call.fdecl->name == "memcpy")
			{
				GinIRMemCpy(lang_stat, out);
			}
			else
				out->emplace_back(ir);



			// if is not last, we will move to some reg
			if ((j + 1) < exps.size())
			{
				ir = {};
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_REG;
				ir.assign.to_assign.reg = AllocReg(lang_stat);
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.only_lhs = true;

				ir.assign.lhs.type = IR_TYPE_RET_REG;
				ir.assign.lhs.reg = 0;
				ir.assign.lhs.reg_sz = 8;
				ir.assign.lhs.deref = 1;
				out->emplace_back(ir);
				val = ir.assign.to_assign;
			}
			else
			{
				val.type = IR_TYPE_RET_REG;
				val.reg = 0;
			}
			// copying ret structs to cur_stack
			func_decl* call = e->call.fdecl;
			if (call->ret_type.type == TYPE_STRUCT && call->ret_type.ptr == 0)
			{
				int strct_sz = GetTypeSize(&call->ret_type);
				lang_stat->cur_strct_ret_size_per_statement += strct_sz;
				int cur_offset = lang_stat->cur_strct_ret_size_per_statement;

				// arg_reg0 = &struct_ret
				ir = {};
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_ARG_REG;
				ir.assign.to_assign.reg = 0;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.to_assign.is_float = false;
				ir.assign.only_lhs = true;

				ir.assign.lhs.type = IR_TYPE_ON_STACK;
				ir.assign.lhs.on_stack_type = ON_STACK_STRUCT_RET;
				ir.assign.lhs.deref = 0;
				ir.assign.lhs.i = cur_offset;
				out->emplace_back(ir);

				// arg_reg1 = *ret_reg0
				ir = {};
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_ARG_REG;
				ir.assign.to_assign.reg = 1;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.to_assign.is_float = false;
				ir.assign.only_lhs = true;

				ir.assign.lhs.type = IR_TYPE_RET_REG;
				ir.assign.lhs.reg = 0;
				ir.assign.lhs.reg_sz = 8;
				ir.assign.lhs.deref = 1;
				out->emplace_back(ir);

				// arg_reg2 = sizeof(struct)
				ir = {};
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_ARG_REG;
				ir.assign.to_assign.reg = 2;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.to_assign.is_float = false;
				ir.assign.only_lhs = true;

				ir.assign.lhs.type = IR_TYPE_INT;
				ir.assign.lhs.i = strct_sz;
				out->emplace_back(ir);

				GinIRMemCpy(lang_stat, out);

				val.type = IR_TYPE_ON_STACK;
				val.on_stack_type = ON_STACK_STRUCT_RET;
				val.i = lang_stat->cur_strct_ret_size_per_statement;

			}
			else
			{
				val.reg_sz = GetTypeSize(&call->ret_type);
				val.is_float = call->ret_type.type == TYPE_F32;
			}
			//unspilling
			if (stack.size() > 0)
			{
				ir_val* begin = stack.begin();

				int cur_spill_offset = 8;
				while (begin < stack.end())
				{
					if (begin->type == IR_TYPE_REG)
					{
						ir = {};
						ir.type = IR_ASSIGNMENT;
						ir.assign.to_assign = *begin;
						ir.assign.to_assign.deref = 0;
						ir.assign.to_assign.reg_sz = 8;
						ir.assign.only_lhs = true;
						ir.assign.lhs.type = IR_TYPE_ON_STACK;
						ir.assign.lhs.on_stack_type = ON_STACK_SPILL;
						ir.assign.lhs.deref = 1;
						ir.assign.lhs.i = cur_spill_offset;
						ir.assign.lhs.reg_sz = 8;
						out->emplace_back(ir);

						cur_spill_offset += 8;
					}
					begin++;
				}
				
			}

			stack.emplace_back(val);;
		}break;
		case AST_ADDRESS_OF:
		{
			ir_val* top = &stack[stack.size() - 1];


			//ASSERT(top->type == IR_TYPE_DECL);
			//ASSERT(top->ptr > 0);
			//if (e->deref.times == 0) break;
			//top->ptr = -1;

			if (top->type != IR_TYPE_REG)
			{
				ir.type = IR_ASSIGNMENT;

				ir.assign.to_assign.type = IR_TYPE_REG;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.to_assign.reg = AllocReg(lang_stat);
				ir.assign.only_lhs = true;
				ir.assign.lhs = *top;
				ir.assign.lhs.ptr = 0;
				*top = ir.assign.to_assign;
				//ir.assign.lhs.ptr = e->deref.times;
				out->emplace_back(ir);
			}
			top->ptr = -1;

			if (top->type == IR_TYPE_REG)
			{
				char saved = top->reg;
				top->reg_ex &= ~0x100;
				top->reg = saved;
				top->deref = 0;
			}

		}break;
		case AST_ARRAY_COSTRUCTION:
		{
			ir_val* top = &stack.back();
			int offset = e->ar_constr.at_offset;
			int tp_sz = GetTypeSize(&e->ar_constr.type);
			//top->i = offset;

			//ir.assign.to_assign.i = offset;
			for (int i = e->ar_constr.commas.size() - 1; i >= 0; i--)
			{
				//ast_struct_construct_info* cinfo = &e->strct_constr.commas[i];
				top = &stack.back();
				stack.pop_back();

				ir.type = IR_ASSIGNMENT;
				ir.assign.only_lhs = true;
				ir.assign.to_assign.is_unsigned = top->is_unsigned;
				ir.assign.to_assign.type = IR_TYPE_ON_STACK;
				ir.assign.to_assign.on_stack_type = ON_STACK_STRUCT_CONSTR;
				ir.assign.to_assign.i = offset - tp_sz * i;
				ir.assign.to_assign.reg_sz = tp_sz;
				ir.assign.lhs = *top;
				out->emplace_back(ir);
			}
			val.type = IR_TYPE_ON_STACK;
			val.i = offset;
			stack.emplace_back(val);
		}break;
		case AST_STRUCT_COSTRUCTION:
		{
			ir_val* top = &stack.back();
			int offset = e->strct_constr.at_offset;
			//top->i = offset;

			//ir.assign.to_assign.i = offset;
			for (int i = e->strct_constr.commas.size() - 1; i >= 0; i--)
			{
				ast_struct_construct_info* cinfo = &e->strct_constr.commas[i];
				top = &stack.back();
				stack.pop_back();

				ir.type = IR_ASSIGNMENT;
				ir.assign.only_lhs = true;
				ir.assign.to_assign.is_unsigned = top->is_unsigned;
				ir.assign.to_assign.type = IR_TYPE_ON_STACK;
				ir.assign.to_assign.on_stack_type = ON_STACK_STRUCT_CONSTR;
				ir.assign.to_assign.i = offset - cinfo->var->offset;
				ir.assign.to_assign.reg_sz = GetTypeSize(&cinfo->var->type);
				ir.assign.lhs = *top;
				out->emplace_back(ir);
			}
			val.type = IR_TYPE_ON_STACK;
			val.i = offset;
			stack.emplace_back(val);
		}break;
		case AST_DEREF:
		{
			ir_val* top = &stack[stack.size() - 1];

			//top->ptr++;
			//break;
			//top->ptr--;
			
			bool was_reg = false;
			if (top->type == IR_TYPE_REG)
			{
				was_reg = true;
				//ASSERT(top->ptr <= 0);
			}
			top->deref++;
			top->reg_sz = GetTypeSize(&e->deref.type);


			int val_to_modify = top->type == IR_TYPE_REG ? -1 : 1;
			
			/*
			if ((top->ptr + -val_to_modify) != 0)
			{
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_REG;
				if (top->type == IR_TYPE_REG)
				{
					ir.assign.to_assign = *top;
				}
				else
					ir.assign.to_assign.reg = AllocReg(lang_stat);

				ir.assign.only_lhs = true;
				ir.assign.lhs = *top;
				ir.assign.to_assign.reg_sz = top->reg_sz;
				ir.assign.to_assign.ptr = 0;
				top->ptr = top->ptr + -(e->deref.times * val_to_modify);
				ir.assign.lhs.ptr = top->ptr;
				out->emplace_back(ir);
				//break;
			}
				*/



			//top->type = IR_TYPE_REG;
			//top->reg = ir.assign.to_assign.reg;
			//top->reg_sz = ir.assign.to_assign.reg_sz;

		}break;
		case AST_OPPOSITE:
		{
			ir_val* top = &stack[stack.size() - 1];

            int if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_BLOCK);
            int sub_if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_SUB_IF_BLOCK);
            int cond_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_COND_BLOCK);

			ir = {};
			ir.type = IR_CMP_NE;
			ir.bin.op = T_COND_NE;
			ir.bin.lhs = *top;
			ir.bin.lhs.deref += 1;
			ir.bin.rhs.type = IR_TYPE_INT;
			ir.bin.rhs.i = 1;
			ir.bin.rhs.is_unsigned = top->is_unsigned;
			out->emplace_back(ir);

            IRCreateEndBlock(lang_stat,cond_idx, out, IR_END_COND_BLOCK);

			int reg = 0;
			if (top->type == IR_TYPE_REG)
				reg = top->reg;
			else
				reg = AllocReg(lang_stat);


			CreateOppositeRegAssigmentAfterCondChecking(lang_stat, out, sub_if_idx, if_idx, reg);
			top->type = IR_TYPE_REG;
			top->reg_sz = 8;
			top->reg = reg;

		}break;
		case AST_NEGATE:
		{
			ir_val* top = &stack[stack.size() - 1];
			ir.type = IR_ASSIGNMENT;
			ir.assign.op = T_MINUS;

			if (top->type == IR_TYPE_REG)
				ir.assign.to_assign = *top;
			else
			{
				ir.assign.to_assign.type = IR_TYPE_REG;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.to_assign.reg = AllocReg(lang_stat);
			}
			ir.assign.rhs = *top;
			ir.assign.lhs.type = IR_TYPE_INT;
			ir.assign.lhs.i = -1;
			ir.assign.lhs.is_unsigned = top->is_unsigned;
			ir.assign.only_lhs = false;
			out->emplace_back(ir);

			*top = ir.assign.to_assign;
		}break;
		case AST_NEGATIVE:
		{
			ir_val* top = &stack[stack.size() - 1];
			ir.type = IR_ASSIGNMENT;
			ir.assign.op = T_MUL;
			ir.assign.to_assign = *top;
			ir.assign.to_assign.deref--;
			ir.assign.lhs = *top;
			ir.assign.lhs.deref++;
			ir.assign.rhs.type = IR_TYPE_INT;
			ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
			if (top->is_float)
			{
				ir.assign.rhs.type = IR_TYPE_F32;
				ir.assign.rhs.f32 = -1.0;
				ir.assign.rhs.is_float = true;
				top->deref = 0;
			}
			else
			{
				ir.assign.rhs.is_float = false;
				ir.assign.rhs.i = -1;
			}
			out->emplace_back(ir);
			//top->reg_sz = e->cast.type;
		}break;
		case AST_CAST:
		{
			ir_val* top = &stack[stack.size() - 1];

			char add = 0;
			bool is_int = e->cast.type.type == TYPE_U32 || e->cast.type.type == TYPE_S32 || e->cast.type.type == TYPE_S64 || e->cast.type.type == TYPE_U64;
			if (e->cast.type.ptr >= 1 && is_int && top->ptr == -1)
			{
				top->deref = 1;
				//ASSERT()
			}
			if (is_int && top->is_float == true)
			{
				ir = {};
				ir.type = IR_CAST_F32_TO_INT;
				ir.bin.lhs.type = IR_TYPE_REG;
				ir.bin.lhs.reg = AllocReg(lang_stat);
				ir.bin.lhs.is_float = false;
				ir.bin.rhs = *top;
				if (top->type == IR_TYPE_DECL)
					ir.bin.rhs.deref = 1;
				else
					ir.bin.rhs.deref++;

				out->emplace_back(ir);

				*top = ir.bin.lhs;
			}
			else if (e->cast.type.type == TYPE_F32 && top->is_float == false)
			{
				ir = {};
				ir.type = IR_CAST_INT_TO_F32;
				ir.bin.lhs.type = IR_TYPE_REG;
				ir.bin.lhs.reg = AllocReg(lang_stat);
				ir.bin.lhs.is_float = true;
				ir.bin.rhs = *top;
				if (top->type == IR_TYPE_DECL)
					ir.bin.rhs.deref = 1;
				else
					ir.bin.rhs.deref++;

				out->emplace_back(ir);

				*top = ir.bin.lhs;
			}
			//if (top->ptr > e->cast.type.ptr && e->cast.type.ptr > 0)
				//top->deref++;
			top->ptr = e->cast.type.ptr;
			/*
			if (e->cast.type.ptr == 0 && top->type != IR_TYPE_INT)
			{
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_REG;
				ir.assign.to_assign.reg = AllocReg(lang_stat);
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.only_lhs = true;
				ir.assign.lhs = *top;
				ir.assign.lhs.deref = 1;
				*top = ir.assign.to_assign;
				out->emplace_back(ir);
			}
			*/

			top->reg_sz = GetTypeSize(&e->cast.type);

		}break;
		case AST_INT:
		case AST_CHAR:
		case AST_STR_LIT:
		case AST_FLOAT:
		case AST_IDENT:
		{
			GetIRVal(lang_stat, e, &val);
			//val.ptr = 0;
			stack.emplace_back(val);

		}break;
		case AST_INDEX:
		{
			ir_val* top = &stack[stack.size() - 1];
			ir_val* one_minus_top = &stack[stack.size() - 2];

			stack.pop_back();

			char offset_reg = AllocReg(lang_stat);
			ir.type = IR_ASSIGNMENT;
			ir.assign.op = T_MUL;
			ir.assign.to_assign.type = IR_TYPE_REG;
			ir.assign.to_assign.reg = offset_reg;
			ir.assign.to_assign.reg_sz = 8;
			ir.assign.lhs = *top;
			ir.assign.lhs.is_float = false;
			ir.assign.rhs.type = IR_TYPE_INT;
			ir.assign.rhs.i = GetTypeSize(e->index.lhs_type.tp);
			ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
			out->emplace_back(ir);

			ir.type = IR_ASSIGNMENT;
			ir.assign.to_assign.type = IR_TYPE_REG;
			ir.assign.to_assign.reg_sz = 8;
			ir.assign.op = T_PLUS;

			if (top->type != IR_TYPE_REG)
				ir.assign.to_assign.reg = AllocReg(lang_stat);
			else
				ir.assign.to_assign.reg = top->reg;

			ir.assign.rhs.type = IR_TYPE_REG;
			ir.assign.rhs.reg = offset_reg;
			ir.assign.rhs.reg_sz = 8;
			ir.assign.rhs.deref = 1;
			ir.assign.rhs.is_float = false;
			ir.assign.lhs = *one_minus_top;
			ir.assign.lhs.is_unsigned = ir.assign.rhs.is_unsigned;
			ir.assign.lhs.is_float = false;
			out->emplace_back(ir);

			top = &stack.back();
			bool was_float = IsIrValFloat(one_minus_top);;
			*top = ir.assign.to_assign;
			top->is_float = was_float;

		}break;
		case AST_BINOP:
		{
			ASSERT(stack.size() >= 2);
			if (e->op == T_POINT)
			{
				ast_rep* next = exps[min(j + 1, exps.size() - 1)];


				ir_val* first = (stack.end()) - e->points.size();
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_REG;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.to_assign.reg = AllocReg(lang_stat);
				ir.assign.op = e->op;

				if (first->ptr > 0 && first->type == IR_TYPE_REG)
				{
				//	first->ptr--;
				}
				ir.assign.only_lhs = false;
				ir.assign.lhs = *first;
				ir.assign.lhs.deref = first->ptr;

				stack.pop_back();

				char prev_ptr = first->ptr;
				for (int i = 1; i < e->points.size(); i++)
				{
					auto cur = first + i;
					ir.assign.rhs = *cur;
					ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
					ir.assign.rhs.is_float = IsIrValFloat(cur);
					ir.assign.lhs.deref = prev_ptr;

					prev_ptr = cur->ptr;
					if (i > 1)
						ir.assign.lhs.deref += 1;

					//if(i == (e->points.size() - 1))
					//ir.assign.rhs.ptr = 1;

					out->emplace_back(ir);
					ir.assign.lhs = ir.assign.to_assign;

					stack.pop_back();
				}
				val = ir.assign.to_assign;
				val.ptr = ir.assign.rhs.ptr + 1;
				val.deref = 1;
				val.is_float = ir.assign.rhs.is_float;
				val.is_unsigned = ir.assign.rhs.is_unsigned;
				//val.deref = .ptr + 1;
				val.reg_ex |= IR_VAL_FROM_POINT;
				stack.emplace_back(val);
			}
			else
			{
				ir_val* top = &stack[stack.size() - 1];
				ir_val* one_minus_top = &stack[stack.size() - 2];

				//ir.assign.to_assign.reg = AllocReg(lang_stat);
				ir.type = IR_ASSIGNMENT;
				ir.assign.to_assign.type = IR_TYPE_REG;
				ir.assign.to_assign.reg_sz = 8;
				ir.assign.op = e->op;
				ir.assign.only_lhs = false;
				ir.assign.lhs = stack[stack.size() - 2];
				ir.assign.rhs = stack[stack.size() - 1];
				ir.assign.rhs.is_unsigned = ir.assign.lhs.is_unsigned;
				ir.assign.rhs.is_float = ir.assign.lhs.is_float;
				ir.assign.to_assign.is_float = ir.assign.lhs.is_float;

				ASSERT(e->op != T_COND_AND && e->op != T_COND_OR);

				if (IsIrValFloat(top))
					ir.assign.to_assign.is_float = true;

				if (top->type == IR_TYPE_REG)
					FreeReg(lang_stat, top->reg);


				//if (ir.assign.lhs.type == IR_TYPE_DECL)
					ir.assign.lhs.deref += 1;
				//if (ir.assign.rhs.type == IR_TYPE_DECL)
					ir.assign.rhs.deref += 1;

				// reusing the reg
				if (one_minus_top->type == IR_TYPE_REG)
				{
					ir.assign.to_assign.type = IR_TYPE_REG;
					ir.assign.to_assign.reg = one_minus_top->reg;
				}
				else
					ir.assign.to_assign.reg = AllocReg(lang_stat);


				stack.pop_back();

				top = &stack[stack.size() - 1];
				top->type = IR_TYPE_REG;
				top->reg_ex = 0;
				top->reg = ir.assign.to_assign.reg;
				top->is_float = ir.assign.to_assign.is_float;
				top->ptr = 0;
				top->deref = 0;
				out->emplace_back(ir);
			}

			

		}break;
		default:
			ASSERT(0);
		}
		*top_info = stack.back();
	}
}

void GenStackThenIR(lang_state *lang_stat, ast_rep *ast, own_std::vector<ir_rep> *out, ir_val *dst_val, ir_val *top)
{
	own_std::vector<ast_rep*> exps;

	switch (ast->type)
	{
	case AST_CALL:
	{
		FOR_VEC(arg, ast->call.args)
		{
			ast_rep* a = *arg;
			PushAstsInOrder(lang_stat, a, &exps);
		}
		exps.emplace_back(ast);
	}break;
	case AST_CAST:
	case AST_ADDRESS_OF:
	case AST_ARRAY_COSTRUCTION:
	case AST_INDEX:
	case AST_OPPOSITE:
	case AST_NEGATIVE:
	case AST_DEREF:
	{
		PushAstsInOrder(lang_stat, ast, &exps);
	}break;
	case AST_BINOP:
	{
		PushAstsInOrder(lang_stat, ast, &exps);
	}break;
	case AST_FLOAT:
	case AST_INT:
	case AST_CHAR:
	case AST_IDENT:
	{
		GetIRVal(lang_stat, ast, dst_val);
		dst_val->ptr = 1;
		return;
	}break;
	default:
		ASSERT(0);
	}

	ir_rep ir;
	int begin_complex_idx = out->size();
	ir_val top_info;
	GinIRFromStack(lang_stat, exps, out, &top_info);

	if (!dst_val)
		return;;

	bool diff_type = top_info.type != dst_val->type && dst_val->type != IR_NONE;
	if (!diff_type && top_info.reg != dst_val->reg || diff_type)
	{
		ir.type = IR_ASSIGNMENT;
		ir.assign.to_assign = *dst_val;
		ir.assign.to_assign.is_float = top_info.is_float;
		ir.assign.to_assign.reg = dst_val->reg;

		if(top_info.ptr < 0)
			ir.assign.to_assign.ptr = top_info.ptr;
		else
			ir.assign.to_assign.ptr = 0;
		ir.assign.only_lhs = true;
		ir.assign.lhs = top_info;
		ir.assign.lhs = top_info;
		ir.assign.lhs.deref += 1 + dst_val->deref;
		dst_val->is_float = top_info.is_float;
		dst_val->ptr = top_info.ptr;
		out->emplace_back(ir);
	}
	if (top)
		*top = top_info;
	//dst_val->reg_ex = top_info.reg_ex;
	//ir.type = IR_END_COMPLEX;
	//out->emplace_back(ir);

	/*
	if (ast->expr.size() == 2)
	{
		

	}
	else
	{

	}
	ir_rep ir;

	ASSERT(ast->type == AST_BINOP);
	ast_rep* lhs_exp = ast->expr[0];
	ast_rep* rhs_exp = ast->expr[1];

	int reg = dst_val->reg;

	ir.type = IR_ASSIGNMENT;
	ir.assign.to_assign = *dst_val;
	ir.assign.op = ast->op;


	ir.assign.only_lhs = false;
	bool lhs_complex = IsAstSimple(lang_stat, lhs_exp);
	bool rhs_complex = IsAstSimple(lang_stat, rhs_exp);

	char type = (char)lhs_complex | (((char)rhs_complex) << 1);
	switch (type)
	{
	// no simple
	case 0:
	{
		GenIRComplex(lang_stat, ast, out, dst_val);
		
	}break;		
	// only lhs simple
	case 1:
	{
		ASSERT(0);
	}break;		
	// only rhs simple
	case 2:
	{
		ASSERT(0);
	}break;		
	// both simple
	case 3:
	{
		GetIRVal(lang_stat, lhs_exp, &ir.assign.lhs);
		GetIRVal(lang_stat, rhs_exp, &ir.assign.rhs);
	}break;		
	}

	out->emplace_back(ir);

	for(int i = 2; i < ast->expr.size(); i++)
	{
		ast_rep* cur = ast->expr[i];

		if (IsAstSimple(lang_stat, cur))
		{
			ir.assign.lhs.type = IR_TYPE_REG;
			ir.assign.lhs.reg = reg;

			GetIRVal(lang_stat, cur, &ir.assign.rhs);
		}
		else
			ASSERT(0);
		out->emplace_back(ir);
		 
	}
	*/
}
void DoStructConstruction(lang_state* lang_stat, ast_rep* ast, ir_val_type dts_type, void* data, own_std::vector<ir_rep> *out)
{
	ir_rep ir;
	ir.type = IR_ASSIGNMENT;
	ir.assign.to_assign.type = IR_TYPE_ON_STACK;
	ir.assign.to_assign.reg_sz = 8;
	//ir.assign.to_assign.i = ast->strct_constr.at_offset;
	FOR_VEC(info, ast->strct_constr.commas)
	{
		ir.assign.to_assign.i = ast->strct_constr.at_offset + info->var->offset;

		if (IsAstSimple(lang_stat, info->exp))
		{
			ir.assign.only_lhs = true;
			GetIRVal(lang_stat, info->exp, &ir.assign.lhs);
		}
		else
			ASSERT(0);

		out->emplace_back(ir);
	}
}

void GenIRMaybeComplex(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out, ir_val *out_val, ir_val *complex_info, bool lhs_equal = false)
{
	ASSERT(0)
	if (IsAstSimple(lang_stat, ast))
	{
		GetIRVal(lang_stat, ast, out_val);
	}
	else
	{
		// making sure we have some ptr to assign to if we're trying to modify it, instead of just its value
		if (lhs_equal && ast->type == AST_DEREF)
			// decrement and also making sure its not lesser than 0
			ast->deref.times = max(0, ast->deref.times - 1);

		if (complex_info->reg == -1)
			complex_info->reg = AllocReg(lang_stat);
		else
			AllocSpecificReg(lang_stat, complex_info->reg);
		GenIRComplex(lang_stat, ast, out, complex_info);
		*out_val = *complex_info;
	}
}

int GetAstTypeSize(lang_state* lang_stat, ast_rep* ast)
{
	switch (ast->type)
	{
	case AST_ARRAY_COSTRUCTION:
	case AST_ADDRESS_OF:
	{
		return 8;
	}break;
	case AST_FLOAT:
	case AST_INT:
	{
		return 4;
	}break;
	case AST_CALL:
	{
		return GetTypeSize(&ast->call.fdecl->ret_type);
	}break;
	case AST_BINOP:
	{
		if (ast->op == T_POINT)
		{
			return GetAstTypeSize(lang_stat, ast->points.back().exp);
		}
		else
			return GetAstTypeSize(lang_stat, ast->expr[0]);
	}break;
	case AST_CAST:
	{
		int prev = ast->cast.type.ptr;

		//ast->cast.type.ptr = max(0, prev - 1);

		int sz = GetTypeSize(&ast->cast.type);
		ast->cast.type.ptr = prev;
		return  sz;
	}break;
	case AST_NEGATIVE:
	{
		return GetAstTypeSize(lang_stat, ast->ast);
	}break;
	case AST_DEREF:
	{
		return GetAstTypeSize(lang_stat, ast->deref.exp);
	}break;
	case AST_INDEX:
	{
		return GetTypeSize(&ast->index.lhs_type);
	}break;
	case AST_IDENT:
	{
		return GetTypeSize(&ast->decl->type);
	}break;
	default:
		ASSERT(0);
	}
	return 0;
}

void GenIfExpr(lang_state* lang_stat, ast_rep* ast, own_std::vector<ir_rep>* out)
{
	ir_rep ir = {};

	ir.type = IR_ASSIGNMENT;
	ir.assign.to_assign.type = IR_TYPE_RET_REG;
	ir.assign.to_assign.reg_sz = 8;
	ir.assign.to_assign.reg = 0;
	ir.assign.only_lhs = true;

	GenStackThenIR(lang_stat, ast, out, &ir.assign.lhs);
	ir.assign.lhs.deref++;
	out->emplace_back(ir);

}
void GenLhsEqual(lang_state* lang_stat, ast_rep* lhs_ast, type2 *lhs_tp, own_std::vector<ir_rep>* out, ir_val *assign)
{
	ir_rep ir = {};
	ir.type = IR_ASSIGNMENT; 
	ir.assign.to_assign.reg = AllocReg(lang_stat);
	ir.assign.to_assign.ptr = 1;
	ir.assign.to_assign.type = IR_TYPE_REG;
	ir.assign.to_assign.reg_sz = min(ir.assign.to_assign.reg_sz, 8);

	// 
	ir.assign.to_assign.deref = -1;

	ir.assign.to_assign.reg_sz = min(8, GetTypeSize(lhs_tp));
	ASSERT(ir.assign.to_assign.reg_sz != 0);

	GenStackThenIR(lang_stat, lhs_ast, out, &ir.assign.to_assign, &ir.assign.to_assign);
	if (ir.assign.to_assign.deref == -1)
		ir.assign.to_assign.deref = 0;


	ir.assign.to_assign.reg_sz = min(8, GetTypeSize(lhs_tp));
	ASSERT(ir.assign.to_assign.reg_sz != 0);

	char final_deref = -1;
	if (ir.assign.to_assign.type == IR_TYPE_DECL)
		final_deref++;
	//ir.assign.to_assign.deref = 0;
	else if (ir.assign.to_assign.type == IR_TYPE_REG)
	{
		final_deref++;

	}
		//ir.assign.to_assign.deref = 1;
	else
		ASSERT(0);


	final_deref += ir.assign.to_assign.deref;

	ir.assign.to_assign.deref = final_deref;

	//ir.assign.to_assign.deref = ir.assign.to_assign.ptr + (ir.assign.to_assign.deref) - 1;
	ASSERT(ir.assign.to_assign.deref >= 0);
	*assign = ir.assign.to_assign;
}
void GetIRFromAst(lang_state *lang_stat, ast_rep *ast, own_std::vector<ir_rep> *out)
{
	ir_rep ir = {};
    switch(ast->type)
    {
	case AST_FLOAT:
	case AST_INT:
	{

	}break;
	case AST_UNOP:
	{
		ASSERT(0);
	}break;
	case AST_FUNC:
	{
		func_decl* last_func = lang_stat->cur_func;
		lang_stat->cur_func = ast->func.fdecl;
		FOR_VEC(arg, ast->func.fdecl->vars)
		{
			decl2* a = *arg;
			if (a->type.type == TYPE_TEMPLATE)
				continue;
            ir.fdecl = ast->func.fdecl;
			if(IS_FLAG_ON(a->flags, DECL_IS_ARG))
				ir.type = IR_DECLARE_ARG;
			else 
				ir.type = IR_DECLARE_LOCAL;
			ir.decl = a;
			out->emplace_back(ir);
		}
        ir.type = IR_STACK_BEGIN;
		ir.fdecl = ast->func.fdecl;
        //ir.num  = ast->func.fdecl->stack_size;
		ir.fdecl->biggest_call_args = 0;
        out->emplace_back(ir);

		GetIRFromAst(lang_stat, ast->func.stats, out);

        ir.type = IR_STACK_END;
        //ir.num  = ast->func.fdecl->stack_size;
        out->emplace_back(ir);
		lang_stat->cur_func = last_func;
	}break;
    case AST_CALL:
	{
		GenStackThenIR(lang_stat, ast, out, nullptr);
		return;
	}break;
    case AST_RET:
	{
		ir.type = IR_RET;
		ast_rep *rhs_ast = ast->ret.ast;
		ir.ret.no_ret_val = false;
		if (!rhs_ast)
		{
			ir.ret.no_ret_val = true;
			out->emplace_back(ir);
			return;
		}
		ir.ret.assign.to_assign.type = IR_TYPE_RET_REG;
		ir.ret.assign.to_assign.reg = 0;
		ir.ret.assign.to_assign.reg_sz = 8;

		switch (rhs_ast->type)
		{
		case AST_CAST:
		case AST_DEREF:
		case AST_CALL:
		{
			GenStackThenIR(lang_stat, rhs_ast, out, &ir.ret.assign.to_assign, &ir.ret.assign.lhs);
			ir.ret.assign.only_lhs = true;
		}break;
		case AST_BINOP:
		{
			ir.ret.assign.only_lhs = true;
			ir.ret.assign.lhs.type = IR_TYPE_REG;
			ir.ret.assign.lhs.reg = AllocReg(lang_stat);
			ir.ret.assign.lhs.reg_sz = 8;

			GenStackThenIR(lang_stat, rhs_ast, out, &ir.ret.assign.lhs);
		}break;
		case AST_IDENT:
		case AST_INT:
		{
			ir.ret.assign.only_lhs = true;
			GetIRVal(lang_stat, rhs_ast, &ir.ret.assign.lhs);
		}break;
		{
			ir.ret.assign.only_lhs = true;
			GetIRVal(lang_stat, rhs_ast, &ir.ret.assign.lhs);
		}break;
		default:
			ASSERT(0)
		}
		//if()
		if (ast->ret.tp.type == TYPE_STRUCT && ast->ret.tp.ptr == 0)
			ir.ret.assign.lhs.deref = 0;
		else
			ir.ret.assign.lhs.deref += 1;

		ir.ret.assign.to_assign.is_float = ir.ret.assign.lhs.is_float;
		out->emplace_back(ir);
	}break;
    case AST_STATS:
	{
		FOR_VEC(st, ast->stats)
		{
			FreeAllRegs(lang_stat);
			ast_rep* s = *st;
			if (s->type == AST_EMPTY)
				continue;



			if (s->type == AST_WHILE || s->type == AST_IF)
			{
				GetIRFromAst(lang_stat, s, out);
			}
			else
			{
				ASSERT(!lang_stat->ir_in_stmnt)
				lang_stat->ir_in_stmnt = true;

				bool can_emplace_stmnt = s->type != AST_IDENT;
				int stmnt_idx = 0;
				if(can_emplace_stmnt)
					stmnt_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_STMNT, (void *)(long long)s->line_number);

				GetIRFromAst(lang_stat, s, out);

				if(can_emplace_stmnt)
					IRCreateEndBlock(lang_stat, stmnt_idx, out, IR_END_STMNT);

				lang_stat->ir_in_stmnt = false;
			}



			int cur_sz = lang_stat->cur_func->strct_ret_size_per_statement;
			lang_stat->cur_func->strct_ret_size_per_statement = max(cur_sz, lang_stat->cur_strct_ret_size_per_statement);
			lang_stat->cur_strct_ret_size_per_statement = 0;

			int i = 0;
		}
	}break;
    case AST_MINUS_MINUS:
    case AST_PLUS_PLUS:
	{
		FreeAllRegs(lang_stat);
		GenLhsEqual(lang_stat, ast->unop_assign.ast, &ast->unop_assign.tp, out, &ir.assign.to_assign);
		
		ir.type = IR_ASSIGNMENT;
		ir.assign.only_lhs = false;
		ir.assign.op = ast->type == AST_PLUS_PLUS ? T_PLUS : T_MINUS;
		ir.assign.lhs = ir.assign.to_assign;
		ir.assign.lhs.deref ++;
		ir.assign.lhs.is_unsigned = ir.assign.rhs.is_unsigned;
		ir.assign.rhs.type = IR_TYPE_INT;
		ir.assign.rhs.i = 1;

		if (IsIrValFloat(&ir.assign.lhs))
			ir.assign.to_assign.is_float = true;

		out->emplace_back(ir);
		//ir.assign.rhs.ptr++////;

	}break;
    case AST_BINOP:
    {
        switch(ast->op)
        {
        case T_PLUS_EQUAL:
        case T_MINUS_EQUAL:
        case T_EQUAL:
        {
			FreeAllRegs(lang_stat);
            ir_val lhs;
            ir_val rhs;

            ast_rep *rhs_ast = ast->expr[1];
            ast_rep *lhs_ast = ast->expr[0];

			ir.type = IR_ASSIGNMENT;
            

			ir_val rhs_top = {};
			switch (rhs_ast->type)
			{
			case AST_IF:
			{
				GetIRFromAst(lang_stat, rhs_ast, out);
				ir.assign.only_lhs = true;
				ir.assign.lhs.type = IR_TYPE_RET_REG;
				ir.assign.lhs.reg = 0;
				ir.assign.lhs.is_float = ast->cond.expr_type.IsFloat();

			}break;
			case AST_CALL:
			case AST_BINOP:
			case AST_CAST:
			case AST_INDEX:
			case AST_DEREF: 
			case AST_NEGATIVE: 
			case AST_ARRAY_COSTRUCTION:
			case AST_ADDRESS_OF: 
			{
				ir.assign.lhs.type = IR_TYPE_REG;
				ir.assign.lhs.reg = AllocReg(lang_stat);
				ir.assign.lhs.reg_sz = GetAstTypeSize(lang_stat, rhs_ast);
				ir.assign.only_lhs = true;
				GenStackThenIR(lang_stat, rhs_ast, out, &ir.assign.lhs, &rhs_top);
				ir.assign.lhs.deref = 0;

			}break;
			case AST_INT: 
			case AST_FLOAT: 
			case AST_IDENT:
			case AST_STR_LIT:
			case AST_CHAR:
			{
				ir.assign.only_lhs = true;
				GetIRVal(lang_stat, rhs_ast, &ir.assign.lhs);
				ir.assign.lhs.ptr = 1;

			}break;
			default:
				ASSERT(0)
			}
			ir.assign.lhs.deref = 1;


			GenLhsEqual(lang_stat, lhs_ast, &ast->lhs_tp, out, &ir.assign.to_assign);

			//if (ir.assign.to_assign.type == IR_TYPE_REG && ir.assign.to_assign.ptr > 0)
				//ir.assign.to_assign.deref = 1;
			//GetIRVal(lang_stat, ast->expr[0], &ir.assign.to_assign);
			if (ast->op == T_PLUS_EQUAL || ast->op == T_MINUS_EQUAL)
			{
				ir.assign.only_lhs = false;
				ir.assign.op = ast->op == T_PLUS_EQUAL ? T_PLUS : T_MINUS;
				ir.assign.rhs = ir.assign.lhs;
				ir.assign.lhs = ir.assign.to_assign;
				ir.assign.lhs.deref ++;
				ir.assign.lhs.is_unsigned = ir.assign.rhs.is_unsigned;

				if (IsIrValFloat(&ir.assign.lhs))
					ir.assign.to_assign.is_float = true;
				//ir.assign.rhs.ptr++////;
			}
			if (ir.assign.to_assign.type == IR_TYPE_DECL && ir.assign.to_assign.decl->type.type == TYPE_FUNC_PTR)
				ir.assign.lhs.deref = 0;
			ir.assign.to_assign.is_float = false;

			/*
			if (ir.assign.to_assign.type == IR_TYPE_REG && ir.assign.to_assign.is_float && ir.assign.to_assign.deref > 0)
			{
				ir.assign.to_assign.is_float = false;
				ir.assign.to_assign.type = IR_TYPE_REG;
			}
			*/
			ASSERT(ir.assign.lhs.reg_sz <= 8);
			ASSERT(ir.assign.rhs.reg_sz <= 8);
			ASSERT(ir.assign.to_assign.reg_sz <= 8);
			out->emplace_back(ir);
			FreeAllRegs(lang_stat);
        }break;
		case T_COND_AND:
		{
            FOR_VEC(expr, ast->expr)
            {
                ast_rep *e = *expr;
                if(e->type == AST_BINOP && e->op == T_COND_OR)
                {
                    int block_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_AND_BLOCK);

                    GetIRFromAst(lang_stat, e, out);

                    IRCreateBeginBlock(lang_stat, out, IR_END_AND_BLOCK);

                }
				else if (e->type == AST_BINOP && e->op != T_POINT)
				{
					tkn_type2 op;
					tkn_type2 opposite = OppositeCondCmp(e->op);
					e->op = opposite;
					// false is for the it_is_jmp_if_true var in ir_rep
					GetIRBin(lang_stat, e, out, FromTokenOpToIRType(opposite), (void *)false);

				}
                else
                {
					ir_rep ir = {};
					ir.bin.lhs.type = IR_TYPE_REG;
					ir.bin.lhs.reg_sz = 8;
					ir.bin.lhs.reg = AllocReg(lang_stat);
					GenStackThenIR(lang_stat, e, out, &ir.bin.lhs, nullptr);
					ir.bin.lhs.deref++;
					ir.bin.rhs.type = IR_TYPE_INT;
					ir.bin.rhs.i = 1;
					ir.type = IR_CMP_NE;
					ir.bin.it_is_jmp_if_true = false;
					ir.bin.op = T_COND_NE;
					out->emplace_back(ir);

                    //GetIRFromAst(lang_stat, e, out);
                }
				FreeAllRegs(lang_stat);
            }
		}break;
        case T_COND_OR:
        {
			int idx = 0;
            FOR_VEC(expr, ast->expr)
            {
                ast_rep *e = *expr;
                if((idx == (ast->expr.size() - 1)) && e->type == AST_BINOP && e->op == T_COND_AND)
                {
                    GetIRFromAst(lang_stat, e, out);
                }
                else if(e->type == AST_BINOP && e->op == T_COND_AND)
                {
                    int block_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_OR_BLOCK);

                    GetIRFromAst(lang_stat, e, out);

                    IRCreateBeginBlock(lang_stat, out, IR_END_OR_BLOCK);

                }
				else if (e->type == AST_BINOP)
				{
					//ASSERT(e->op == T_COND_NE || e->op == T_COND_EQ || e-);
					tkn_type2 op = e->op;
					if (idx == (ast->expr.size() - 1))
					{
						tkn_type2 opposite = OppositeCondCmp(e->op);
						e->op = opposite;
						GetIRBin(lang_stat, e, out, FromTokenOpToIRType(e->op), (void *)false);
					}
					else
						GetIRBin(lang_stat, e, out, FromTokenOpToIRType(e->op), (void *)true);

				}
                else
                {
					ir_rep ir = {};
					ir.bin.lhs.type = IR_TYPE_REG;
					ir.bin.lhs.reg_sz = 8;
					ir.bin.lhs.reg = AllocReg(lang_stat);
					GenStackThenIR(lang_stat, e, out, &ir.bin.lhs, nullptr);
					ir.bin.rhs.type = IR_TYPE_INT;
					ir.bin.rhs.i = 1;
					ir.type = IR_CMP_EQ;
					ir.bin.it_is_jmp_if_true = true;
					ir.bin.op = T_COND_EQ;
					out->emplace_back(ir);
                }
				idx++;
				FreeAllRegs(lang_stat);
            }
        }break;
		case T_COND_NE:
        {
			GetIRBin(lang_stat, ast, out, IR_CMP_NE);
        }break;
        case T_COND_EQ:
        {
			GetIRBin(lang_stat, ast, out, IR_CMP_EQ);
        }break;
		default:
			ASSERT(0)

        }
    }break;
	case AST_IDENT: break;
	case AST_WHILE:
    {
		bool prev_is_in_stmnt = lang_stat->ir_in_stmnt;
		lang_stat->ir_in_stmnt = false;
		int block_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_BLOCK);
		int loop_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_LOOP_BLOCK);
		int stmnt_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_STMNT, (void *)(long long)ast->line_number);
		//int cond_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_BLOCK);
		GetIRCond(lang_stat, ast->loop.cond, out);
		IRCreateEndBlock(lang_stat, stmnt_idx, out, IR_END_STMNT);

		if (ast->loop.scope)
		{
			GetIRFromAst(lang_stat, ast->loop.scope, out);
		}

		//IRCreateEndBlock(lang_stat, cond_idx, out, IR_END_IF_BLOCK);
		IRCreateEndBlock(lang_stat, loop_idx, out, IR_END_LOOP_BLOCK);
		IRCreateEndBlock(lang_stat, block_idx, out, IR_END_BLOCK);

		lang_stat->ir_in_stmnt = prev_is_in_stmnt;
    }break;
	case AST_DBG_BREAK:
	{
		if (lang_stat->release)
			ir.type = IR_NOP;
		else
			ir.type = IR_DBG_BREAK;
		out->emplace_back(ir);
	}break;
	case AST_BREAK:
	{
		ir.type = IR_BREAK;
		out->emplace_back(ir);
	}break;
	case AST_FOR:
	{
		bool prev_is_in_stmnt = lang_stat->ir_in_stmnt;
		lang_stat->ir_in_stmnt = false;
		GetIRFromAst(lang_stat, ast->for_info.start_stat, out);
		int block_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_BLOCK);
		int loop_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_LOOP_BLOCK);
		int stmnt_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_STMNT, (void *)(long long)ast->line_number);
		//int cond_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_BLOCK);
		GetIRCond(lang_stat, ast->for_info.cond_stat, out);
		IRCreateEndBlock(lang_stat, stmnt_idx, out, IR_END_STMNT);

		if (ast->for_info.scope)
		{
			GetIRFromAst(lang_stat, ast->for_info.scope, out);
		}

		GetIRFromAst(lang_stat, ast->for_info.at_loop_end_stat, out);
		//IRCreateEndBlock(lang_stat, cond_idx, out, IR_END_IF_BLOCK);
		IRCreateEndBlock(lang_stat, loop_idx, out, IR_END_LOOP_BLOCK);
		IRCreateEndBlock(lang_stat, block_idx, out, IR_END_BLOCK);

		lang_stat->ir_in_stmnt = prev_is_in_stmnt;

	}break;
	case AST_IF:
    {
		FreeAllRegs(lang_stat);

		bool is_stmnt_without_semicolon = ast->cond.scope->stats.back()->stmnt_without_semicolon;
		bool was_in_stmnt = lang_stat->ir_in_stmnt;
		if (is_stmnt_without_semicolon)
			lang_stat->ir_in_stmnt = false;
		/*

		int stmnt_without_semicolon_idx = 0;

		if(is_stmnt_without_semicolon)
			stmnt_without_semicolon_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_EXPR_BLOCK, (void *)(long long)ast->line_number);
		*/

		int stmnt_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_STMNT, (void *)(long long)ast->line_number);
        int if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_IF_BLOCK);

        bool has_elses = ast->cond.elses.size();

        int sub_if_idx = 0;
        if(has_elses)
            sub_if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_SUB_IF_BLOCK);

		int cond_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_COND_BLOCK);
		GetIRCond(lang_stat, ast->cond.cond, out);
		IRCreateEndBlock(lang_stat, stmnt_idx, out, IR_END_STMNT);
        //GetIRFromAst(lang_stat, ast->cond.cond, out);
		IRCreateEndBlock(lang_stat,cond_idx, out, IR_END_COND_BLOCK);

        if(ast->cond.scope)
        {
            GetIRFromAst(lang_stat, ast->cond.scope, out);
			if (is_stmnt_without_semicolon)
				GenIfExpr(lang_stat, ast->cond.scope->stats.back(), out);
            if(has_elses)
            {
                ir.type = IR_BREAK_OUT_IF_BLOCK;
                out->emplace_back(ir);
            }

        }

        if(has_elses)
            IRCreateEndBlock(lang_stat, sub_if_idx, out, IR_END_SUB_IF_BLOCK);

        int i = 0;
        FOR_VEC(el, ast->cond.elses)
        {
            ast_rep *e = *el;

            bool is_last = (i + 1) >= ast->cond.elses.size();

            if(!is_last)
                sub_if_idx = IRCreateBeginBlock(lang_stat, out, IR_BEGIN_SUB_IF_BLOCK);

			if (e->type == AST_ELSE_IF)
				GetIRCond(lang_stat, e->cond.cond, out);

            GetIRFromAst(lang_stat, e->cond.scope, out);

			if (is_stmnt_without_semicolon)
				GenIfExpr(lang_stat, e->cond.scope->stats.back(), out);

            if(!is_last)
            {
                ir.type = IR_BREAK_OUT_IF_BLOCK;
                out->emplace_back(ir);
                IRCreateEndBlock(lang_stat, sub_if_idx, out, IR_END_SUB_IF_BLOCK);
            }
        }

        IRCreateEndBlock(lang_stat, if_idx, out, IR_END_IF_BLOCK);
		if (is_stmnt_without_semicolon)
			lang_stat->ir_in_stmnt = was_in_stmnt;

    }break;
    default:
        ASSERT(0)
    }
}

