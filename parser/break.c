#include "break.h"
#include "../libc.h"
#include "parser.h"

void break_debug(int depth, parser_node *node)
{
    printtabs(depth);
    printf("Break\n");
}

apply_result *break_apply(parser_node *node, context *ctx)
{
    char* loop_end_label = (char*)malloc(128);
    get_current_loop_end_label_counter(ctx, loop_end_label);
    add_text(ctx, "jmp %s", loop_end_label);
    free(loop_end_label);
    return NULL;
}

parser_node *parse_break(typed_token **tkns_ptr)
{
    typed_token *tkn = *tkns_ptr;
    if (tkn->type_id == TKN_BREAK)
    {
        if (tkn->next->type_id != TKN_SEMICOLON) {
            return NULL;
        }
        parser_node *break_node = new_node(break_debug, break_apply, 0);
        *tkns_ptr = tkn->next->next;
        return break_node;
    }

    return NULL;
}

void continue_debug(int depth, parser_node *node)
{
    printtabs(depth);
    printf("Continue\n");
}

apply_result *continue_apply(parser_node *node, context *ctx)
{
    char* loop_start_label = (char*)malloc(128);
    get_current_loop_start_label_counter(ctx, loop_start_label);
    add_text(ctx, "jmp %s", loop_start_label);
    free(loop_start_label);
    return NULL;
}

parser_node *parse_continue(typed_token **tkns_ptr)
{
    typed_token *tkn = *tkns_ptr;
    if (tkn->type_id == TKN_CONTINUE)
    {
        if (tkn->next->type_id != TKN_SEMICOLON) {
            return NULL;
        }
        parser_node *continue_node = new_node(continue_debug, continue_apply, 0);
        *tkns_ptr = tkn->next->next;
        return continue_node;
    }

    return NULL;
}
