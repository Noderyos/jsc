#include <stdlib.h>
#include <stdio.h>
#include <mujs.h>
#include <jsi.h>

#define TODO(x) do {fprintf(stderr, "TODO: " x);exit(1);}while(0);

#define READSTRING() \
    memcpy(&str, pc, sizeof(str)); \
    pc += sizeof(str) / sizeof(*pc)

char *read_file(const char *filename);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [JS file]\n", argv[0]);
        exit(1);
    }

    js_State *J = js_newstate(NULL, NULL, 0);
    
    const char *filename = argv[1];
    const char *source = read_file(filename);

    if (source == NULL) {
        fprintf(stderr, "ERROR: Cannot open file '%s'\n", filename);
        exit(1);
    }

    js_Ast *P;
    js_Function *F;
    
    if (js_try(J)) {
        jsP_freeparse(J);
    }

    P = jsP_parse(J, filename, source);
    F = jsC_compilescript(J, P, J->default_strict);
    jsP_freeparse(J);

    js_endtry(J);

    js_Instruction *pc = F->code;
    js_Instruction *pc_end = F->code + F->codelen;

    const char *str;    

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    printf("main:\n"
           "    push rbp\n"
           "    mov rbp, rsp\n"
           "    sub rsp, %d\n", F->varlen*8);
    while (pc < pc_end) {
        int line = *pc++;
        (void)line;

        enum js_OpCode opcode = *pc++;
        switch (opcode) {
            case OP_POP: {
                printf("// OP_POP\n");
                printf("    pop rax\n");
            };break;
            case OP_SUB: {
                printf("// OP_SUB\n");
                printf("    pop rax\n");
                printf("    sub [rsp], rax\n");
            }; break;
            case OP_DUP: TODO("OP_DUP");break;
            case OP_DUP2: TODO("OP_DUP2");break;
            case OP_ROT2: TODO("OP_ROT2");break;
            case OP_ROT3: TODO("OP_ROT3");break;
            case OP_ROT4: TODO("OP_ROT4");break;
            case OP_INTEGER: {
                int integer = *pc++ - 32768;
                printf("// OP_INTEGER(%d)\n", integer);
                printf("    push %d\n", integer);
            };break;
            case OP_NUMBER: TODO("OP_NUMBER");break;
            case OP_STRING: TODO("OP_STRING");break;
            case OP_CLOSURE: TODO("OP_CLOSURE");break;
            case OP_NEWARRAY: TODO("OP_NEWARRAY");break;
            case OP_NEWOBJECT: TODO("OP_NEWOBJECT");break;
            case OP_NEWREGEXP: TODO("OP_NEWREGEXP");break;
            case OP_UNDEF: {
                printf("// OP_UNDEF\n");
                printf("    push 0\n");
            };break;
            case OP_NULL: TODO("OP_NULL");break;
            case OP_TRUE: TODO("OP_TRUE");break;
            case OP_FALSE: TODO("OP_FALSE");break;
            case OP_THIS: TODO("OP_THIS");break;
            case OP_CURRENT: TODO("OP_CURRENT");break;
            case OP_GETLOCAL: {
                int index = *pc++;
                printf("// OP_GETLOCAL(%d)\n", index);
                printf("    mov rax, [rbp-%d]\n", 8*index);
                printf("    push rax\n");
            };break;
            case OP_SETLOCAL: {
                int index = *pc++;
                printf("// OP_SETLOCAL(%d)\n", index);
                printf("    mov rax, [rsp]\n");
                printf("    mov [rbp-%d], rax\n", 8*index);
            };break;
            case OP_DELLOCAL: TODO("OP_DELLOCAL");break;
            case OP_HASVAR: TODO("OP_HASVAR");break;
            case OP_GETVAR: {
                READSTRING();
                printf("// OP_GETVAR(%s)\n", str);
                printf("    push offset %s\n", str);
            };break;
            case OP_SETVAR: {
                READSTRING();
                printf("// OP_SETVAR(%s)\n", str);
                printf("    pop %s\n", str);
            };break;
            case OP_DELVAR: TODO("OP_DELVAR");break;
            case OP_IN: TODO("OP_IN");break;
            case OP_SKIPARRAY: TODO("OP_SKIPARRAY");break;
            case OP_INITARRAY: TODO("OP_INITARRAY");break;
            case OP_INITPROP: TODO("OP_INITPROP");break;
            case OP_INITGETTER: TODO("OP_INITGETTER");break;
            case OP_INITSETTER: TODO("OP_INITSETTER");break;
            case OP_GETPROP: TODO("OP_GETPROP");break;
            case OP_GETPROP_S: TODO("OP_GETPROP_S");break;
            case OP_SETPROP: TODO("OP_SETPROP");break;
            case OP_SETPROP_S: TODO("OP_SETPROP_S");break;
            case OP_DELPROP: TODO("OP_DELPROP");break;
            case OP_DELPROP_S: TODO("OP_DELPROP_S");break;
            case OP_ITERATOR: TODO("OP_ITERATOR");break;
            case OP_NEXTITER: TODO("OP_NEXTITER");break;
            case OP_EVAL: TODO("OP_EVAL");break;
            case OP_CALL: {
                int arity = *pc++;
                printf("// OP_CALL(%d)\n", arity);
                char regs[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                for (int i = 0; i < arity; i++)
                    printf("    pop %s\n", regs[i]);
                printf("    pop rax\n"); // Discarding 'this'
                printf("    pop rax\n");
                printf("    call rax\n");
                printf("    push rax\n");
            };break;
            case OP_NEW: TODO("OP_NEW");break;
            case OP_TYPEOF: TODO("OP_TYPEOF");break;
            case OP_POS: TODO("OP_POS");break;
            case OP_NEG: TODO("OP_NEG");break;
            case OP_BITNOT: TODO("OP_BITNOT");break;
            case OP_LOGNOT: TODO("OP_LOGNOT");break;
            case OP_INC: TODO("OP_INC");break;
            case OP_DEC: TODO("OP_DEC");break;
            case OP_POSTINC: TODO("OP_POSTINC");break;
            case OP_POSTDEC: TODO("OP_POSTDEC");break;
            case OP_MUL: {
                printf("// OP_MUL\n");
                printf("    pop rax\n");
                printf("    pop rbx\n");
                printf("    mul rbx\n");
                printf("    push rax\n");
            }; break;
            case OP_DIV: TODO("OP_DIV");break;
            case OP_MOD: TODO("OP_MOD");break;
            case OP_ADD: {
                printf("// OP_ADD\n");
                printf("    pop rax\n");
                printf("    add [rsp], rax\n");
            }; break;
            case OP_SHL: TODO("OP_SHL");break;
            case OP_SHR: TODO("OP_SHR");break;
            case OP_USHR: TODO("OP_USHR");break;
            case OP_LT: TODO("OP_LT");break;
            case OP_GT: TODO("OP_GT");break;
            case OP_LE: TODO("OP_LE");break;
            case OP_GE: TODO("OP_GE");break;
            case OP_EQ: TODO("OP_EQ");break;
            case OP_NE: TODO("OP_NE");break;
            case OP_STRICTEQ: TODO("OP_STRICTEQ");break;
            case OP_STRICTNE: TODO("OP_STRICTNE");break;
            case OP_JCASE: TODO("OP_JCASE");break;
            case OP_BITAND: TODO("OP_BITAND");break;
            case OP_BITXOR: TODO("OP_BITXOR");break;
            case OP_BITOR: TODO("OP_BITOR");break;
            case OP_INSTANCEOF: TODO("OP_INSTANCEOF");break;
            case OP_THROW: TODO("OP_THROW");break;
            case OP_TRY: TODO("OP_TRY");break;
            case OP_ENDTRY: TODO("OP_ENDTRY");break;
            case OP_CATCH: TODO("OP_CATCH");break;
            case OP_ENDCATCH: TODO("OP_ENDCATCH");break;
            case OP_WITH: TODO("OP_WITH");break;
            case OP_ENDWITH: TODO("OP_ENDWITH");break;
            case OP_DEBUGGER: TODO("OP_DEBUGGER");break;
            case OP_JUMP: TODO("OP_JUMP");break;
            case OP_JTRUE: TODO("OP_JTRUE");break;
            case OP_JFALSE: TODO("OP_JFALSE");break;
            case OP_RETURN: {
                printf("// OP_RETURN\n");
                printf("    pop rax\n");
                printf("    add rsp, %d\n", F->varlen*8);
                printf("    pop rbp\n");
                printf("    ret\n");
            };break;
            default:
                fprintf(stderr, "Unexpected opcode %u\n", opcode);
        }
    }
}

char *read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *string = malloc(fsize + 1);
    int a = fread(string, fsize, 1, f);
    (void)a;
    fclose(f);

    string[fsize] = 0;

    return string;
}
