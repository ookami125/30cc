#include "../../libc.h"
#include "../../lexer.h"
#include "../parser.h"
#include "index.h"
#include "expr.h"

void index_debug(int depth, parser_node *node)
{
    node_index *idx = (node_index *)node->data;
    printtabs(depth);
    printf("Access:\n");
    printtabs(depth + 1);
    printf("In:\n");
    idx->arr->debug(depth + 2, idx->arr);
    printtabs(depth + 1);
    printf("Index:\n");
    idx->ind->debug(depth + 2, idx->ind);
}

apply_result *index_apply(parser_node *node, context *ctx)
{
    node_index *idx = (node_index *)node->data;

    apply_result *arr = idx->arr->apply(idx->arr, ctx);
    general_type *base_type = ((pointer_type *)arr->type->data)->of;

    int elem_size = general_type_size(base_type, ctx);
    apply_result *ind = idx->ind->apply(idx->ind, ctx);
    add_text(ctx, "mov rax, %s", ind->code);
    add_text(ctx, "mov rbx, %u", elem_size);
    add_text(ctx, "mul rbx");
    add_text(ctx, "mov rbx, %s", arr->code);
    add_text(ctx, "add rbx, rax");

    symbol *sym_addr = new_temp_symbol(ctx, arr->type);
    add_text(ctx, "mov %s, rbx", sym_addr->repl);

    char *rega = reg_a(base_type, ctx);
    add_text(ctx, "mov %s, [rbx]", rega);
    symbol *sym_val = new_temp_symbol(ctx, base_type);
    add_text(ctx, "mov %s, %s", sym_val->repl, rega);

    apply_result *ret = new_result(sym_val->repl, base_type);
    ret->code = sym_val->repl;
    ret->addr_code = sym_addr->repl;

    return ret;
}

parser_node *parse_index(typed_token **tkns_ptr, parser_node *arr)
{
    typed_token *tkn = *tkns_ptr;

    if (tkn->type_id == TKN_L_BRACK)
    {
        tkn = tkn->next;
        parser_node *ind = parse_expr(&tkn);
        if (ind)
        {
            if (tkn->type_id == TKN_R_BRACK)
            {
                tkn = tkn->next;
                *tkns_ptr = tkn;
                parser_node *node = new_node(index_debug, index_apply, sizeof(node_index));
                node_index *idx = (node_index *)node->data;
                idx->arr = arr;
                idx->ind = ind;

                return node;
            }
        }
    }

    return NULL;
}
