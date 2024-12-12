#include "libc.h"
#include "lexer.h"
#include "parser/program.h"
#include "linked_list.h"
#include "preprocess/preprocess.h"
#include "preprocess/macro_define.h"

typed_token *process(char *filename, int log_lex, int log_prep)
{
    prep_ctx *ctx = (prep_ctx *)malloc(sizeof(prep_ctx));
    ctx->curr_path = NULL;
    ctx->defs = new_linked_list();

    seg_define *_30cc_define = (seg_define *)malloc(sizeof(seg_define));
    _30cc_define->arg_names = new_linked_list();
    _30cc_define->id = "_30CC";
    _30cc_define->replace = new_linked_list();
    add_to_list(ctx->defs, _30cc_define);

    typed_token *lexed = tokenize_file(filename);
    if (log_lex)
    {
        typed_token *t = lexed;
        while (t)
        {
            t->debug(t);
            t = t->next;
        }
    }

    typed_token *prep = preprocess(ctx, filename);
    if (log_prep)
    {
        typed_token *t = prep;
        while (t)
        {
            t->debug(t);
            t = t->next;
        }
    }

    return prep;
}

int version(int argc, char **argv) {
#ifdef _30CC
    char *compiler = "30cc";
#endif
#ifndef _30CC
    char *compiler = "gcc";
#endif
    printf("30cc compiler version 0.1.0 (Compiled with %s)\n", compiler);
    exit(0);
    return 0;
}

int usage(int argc, char **argv) {
    fprintf(stderr, "Usage: %s <filename> <mode> (<mode>: --lex, --prep, --asm or --tree)\n", argv[0]);
    exit(1);
    return 1;
}

#define OPT_MODE_BIN 0
#define OPT_MODE_LEX 1
#define OPT_MODE_PREP 2
#define OPT_MODE_TREE 3
#define OPT_MODE_ASM 4

typedef struct Options_ {
   int mode;
} Options;

Options argparse(int argc, char** argv) {
    Options ret;
    ret.mode = OPT_MODE_BIN;

    if(argc <= 1) {
        usage(argc, argv);
    }

    for(int argi=1; argi<argc; argi += 1)
    {
        char* option = 0;
        char* argument = 0;
        int seperateArgFlag = 0;

        if(argv[argi][0] != '-') {
            //ret.positionalArgs.push_back(argv[argi]);
            continue;
        }

        if(argv[argi][1] == '-') {
            option = &argv[argi][2];
            char* tok = strpbrk(option, "=");
            if(tok != 0) {
                int arg_len = strlen(tok);
                argument = (char*)malloc(arg_len+1);
                strncpy(argument, tok, arg_len);
                argument[arg_len] = '\0';
                
                // HACK: Stupid way to do this, but 64bit ints aren't supported yet
                int opt_len = strlen(option)-arg_len-1;
                char* temp = (char*)malloc(opt_len+1);
                strncpy(temp, option, opt_len);
                temp[arg_len] = '\0';

                option = temp;
            }
        }
        else
        {
            option = (char*)malloc(2);
            option[0] = argv[argi][1];
            option[1] = '\0';
            if(argv[argi][2] != '\0')
            {
                argument = &argv[argi][2];
            }
        }

        if((argument == 0 || argument[0] == '\0') && argi < argc-1) {
           argument = argv[argi + 1];
           seperateArgFlag = 1;
        }

        if(strcmp(option, "v") == 0) {
            version(argc, argv);
        } else if(strcmp(option, "h") == 0 || strcmp(option, "help") == 0) {
            usage(argc, argv);
        } else if(strcmp(option, "lex") == 0) {
            ret.mode = OPT_MODE_LEX;
        } else if(strcmp(option, "prep") == 0) {
            ret.mode = OPT_MODE_PREP;
        } else if(strcmp(option, "tree") == 0) {
            ret.mode = OPT_MODE_TREE;
        } else if(strcmp(option, "asm") == 0) {
            ret.mode = OPT_MODE_ASM;
        } else {
            fprintf(stderr, "Unknown argument %s", option);
            exit(1);
        }
    }

    return ret;
}

int main(int argc, char **argv)
{
    Options opt = argparse(argc, argv);

    typed_token *tkn = process(argv[1], opt.mode == OPT_MODE_LEX, opt.mode == OPT_MODE_PREP);
    if (tkn == NULL)
    {
        return 1;
    }

    parser_node *prog = parse_program(&tkn);
    if (prog)
    {
        if (opt.mode == OPT_MODE_TREE)
        {
            prog->debug(0, prog);
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "Parse failed!\n");
        return 1;
    }

    if (opt.mode == OPT_MODE_ASM || opt.mode == OPT_MODE_BIN)
    {
        context *ctx = new_context();
        prog->apply(prog, ctx);
        list_node *curr = ctx->data->first;
        printf("section .data\n");
        while (curr)
        {
            printf("%s\n", (char *)curr->value);
            curr = curr->next;
        }
        printf("section .text\n");
        curr = ctx->text->first;
        while (curr)
        {
            printf("%s\n", (char *)curr->value);
            curr = curr->next;
        }
    }

    if (opt.mode == OPT_MODE_BIN)
    {
        fprintf(stderr, "Failed to invoke linker!");
        return 1;
    }

    return 0;
}
