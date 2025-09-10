#include <stdlib.h>
#include <stdio.h>
#include <mujs.h>
#include <jsi.h>

#define TODO(x) do {fprintf(stderr, "TODO: " x);exit(1);}while(0);

#define READSTRING() \
    memcpy(&str, pc, sizeof(str)); \
    pc += sizeof(str) / sizeof(*pc)

char *read_file(const char *filename);
void compile_function(js_State *J, js_Function *F);

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

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    compile_function(J, F);
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

void generate_cmp() {
    printf("    pop rax\n");
    printf("    cmp [rsp], rax\n");
    printf("    mov rcx, 0\n");
    printf("    mov rdx, 1\n");
}

void compile_function(js_State *J, js_Function *F) {
    for (int i = 0; i < F->funlen; i++)
        compile_function(J, F->funtab[i]);

    js_Instruction *pc = F->code;
    js_Instruction *pc_end = F->code + F->codelen;

    const char *str;    

    const char *name = *F->name ? F->name : "main";

    printf("%s:\n", name);
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", F->varlen*8);
    
    char regs[6][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (int i = 0; i < F->numparams; i++)
        printf("    mov %s, [rbp-%d]\n", regs[i], (i+1)*8);

    while (pc < pc_end) {
        printf("label_%s_%ld:\n", name, pc-F->code);
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
            case OP_DUP: {
                printf("// OP_DUP\n");
                printf("    mov rax, [rsp]\n");
                printf("    push rax\n");
            };break;
            case OP_DUP2: {
                printf("// OP_DUP2\n");
                printf("    mov rax, [rsp+8]\n");
                printf("    push rax\n");
                printf("    mov rax, [rsp+8]\n");
                printf("    push rax\n");
            };break;
            case OP_ROT2: {
                printf("// OP_ROT2\n");
                printf("    mov rax, [rsp]\n");
                printf("    mov rbx, [rsp+8]\n");
                printf("    mov [rsp+8], rax\n");
                printf("    mov [rsp], rbx\n");
            };break;
            case OP_ROT3: {
                printf("// OP_ROT3\n");
                printf("    mov rax, [rsp]\n");
                printf("    mov rbx, [rsp+8]\n");
                printf("    mov rcx, [rsp+16]\n");
                printf("    mov [rsp+16], rax\n");
                printf("    mov [rsp+8], rcx\n");
                printf("    mov [rsp], rbx\n");
            };break;
            case OP_ROT4: {
                printf("// OP_ROT4\n");
                printf("    mov rax, [rsp]\n");
                printf("    mov rbx, [rsp+8]\n");
                printf("    mov rcx, [rsp+16]\n");
                printf("    mov rdx, [rsp+24]\n");
                printf("    mov [rsp+24], rax\n");
                printf("    mov [rsp+16], rdx\n");
                printf("    mov [rsp+8], rcx\n");
                printf("    mov [rsp], rbx\n");
            };break;
            case OP_INTEGER: {
                int integer = *pc++ - 32768;
                printf("// OP_INTEGER(%d)\n", integer);
                printf("    push %d\n", integer);
            };break;
            case OP_NUMBER: TODO("OP_NUMBER");break;
            case OP_STRING: TODO("OP_STRING");break;
            case OP_CLOSURE: {
                int index = *pc++;
                printf("// OP_CLOSURE(%d)\n", index);
                printf("    push offset %s\n", F->funtab[index]->name);
            };break;
            case OP_NEWARRAY: TODO("OP_NEWARRAY");break;
            case OP_NEWOBJECT: TODO("OP_NEWOBJECT");break;
            case OP_NEWREGEXP: TODO("OP_NEWREGEXP");break;
            case OP_UNDEF: {
                printf("// OP_UNDEF\n");
                printf("    push 0\n");
            };break;
            case OP_NULL: TODO("OP_NULL");break;
            case OP_TRUE: {
                printf("// OP_TRUE\n");
                printf("    push 1\n");
            };break;
            case OP_FALSE: {
                printf("// OP_FALSE\n");
                printf("    push 0\n");
            };break;
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
            case OP_SETVAR: TODO("OP_SETVAR");break;
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
            case OP_POS: {
                printf("// OP_POS\n");
            };break;
            case OP_NEG: {
                printf("// OP_NEG\n");
                printf("    neg qword ptr[rsp]\n");
            };break;
            case OP_BITNOT: {
                printf("// OP_BITNOT\n");
                printf("    not qword ptr[rsp]\n");
            };break;
            case OP_LOGNOT: TODO("OP_LOGNOT");break;
            case OP_INC: {
                printf("// OP_INC\n");
                printf("    inc qword ptr[rsp]\n");
            };break;
            case OP_DEC: {
                printf("// OP_DEC\n");
                printf("    dec qword ptr[rsp]\n");
            };break;
            case OP_POSTINC: {
                printf("// OP_POSTINC\n");
                printf("    mov rax, [rsp]\n");
                printf("    inc qword ptr[rsp]\n");
                printf("    push rax\n");
            };break;
            case OP_POSTDEC: {
                printf("// OP_POSTDEC\n");
                printf("    mov rax, [rsp]\n");
                printf("    dec qword ptr[rsp]\n");
                printf("    push rax\n");
            };break;
            case OP_MUL: {
                printf("// OP_MUL\n");
                printf("    pop rax\n");
                printf("    pop rbx\n");
                printf("    mul rbx\n");
                printf("    push rax\n");
            }; break;
            case OP_DIV: {
                printf("// OP_DIV\n");
                printf("    pop rcx\n");
                printf("    pop rax\nxor rdx, rdx\n");
                printf("    idiv rcx\n");
                printf("    push rax\n");
            };break;
            case OP_MOD: {
                printf("// OP_MOD\n");
                printf("    pop rcx\n");
                printf("    pop rax\nxor rdx, rdx\n");
                printf("    idiv rcx\n");
                printf("    push rdx\n");
            };break;
            case OP_ADD: {
                printf("// OP_ADD\n");
                printf("    pop rax\n");
                printf("    add [rsp], rax\n");
            }; break;
            case OP_SHL: {
                printf("// OP_SHL\n");
                printf("    pop rcx\n");
                printf("    shl qword ptr[rsp], rcx\n");
            };break;
            case OP_SHR: {
                printf("// OP_SHR\n");
                printf("    pop rcx\n");
                printf("    sar qword ptr[rsp], rcx\n");
            };break;
            case OP_USHR: {
                printf("// OP_USHR\n");
                printf("    pop rcx\n");
                printf("    shr qword ptr[rsp], rcx\n");
            };break;
            case OP_LT: {
                printf("// OP_LT\n");
                generate_cmp();
                printf("    cmovl rcx, rdx\n");
                printf("    mov [rsp], rcx\n");
            }; break;
            case OP_GT: {
                printf("// OP_GT\n");
                generate_cmp();
                printf("    cmovg rcx, rdx\n");
                printf("    mov [rsp], rcx\n");
            }; break;
            case OP_LE: {
                printf("// OP_LE\n");
                generate_cmp();
                printf("    cmovle rcx, rdx\n");
                printf("    mov [rsp], rcx\n");
            }; break;
            case OP_GE: {
                printf("// OP_GE\n");
                generate_cmp();
                printf("    cmovge rcx, rdx\n");
                printf("    mov [rsp], rcx\n");
            }; break;
            case OP_EQ: {
                printf("// OP_EQ\n");
                generate_cmp();
                printf("    cmove rcx, rdx\n");
                printf("    mov [rsp], rcx\n");
            }; break;
            case OP_NE: {
                printf("// OP_NE\n");
                generate_cmp();
                printf("    cmovne rcx, rdx\n");
                printf("    mov [rsp], rcx\n");
            }; break;
            case OP_STRICTEQ: TODO("OP_STRICTEQ");break;
            case OP_STRICTNE: TODO("OP_STRICTNE");break;
            case OP_JCASE: TODO("OP_JCASE");break;
            case OP_BITAND: {
                printf("// OP_BITAND\n");
                printf("    pop rax\n");
                printf("    and qword ptr[rsp], rax\n");
            };break;
            case OP_BITXOR: {
                printf("// OP_BITXOR\n");
                printf("    pop rax\n");
                printf("    xor qword ptr[rsp], rax\n");
            };break;
            case OP_BITOR: {
                printf("// OP_BITOR\n");
                printf("    pop rax\n");
                printf("    or qword ptr[rsp], rax\n");
            };break;
            case OP_INSTANCEOF: TODO("OP_INSTANCEOF");break;
            case OP_THROW: TODO("OP_THROW");break;
            case OP_TRY: TODO("OP_TRY");break;
            case OP_ENDTRY: TODO("OP_ENDTRY");break;
            case OP_CATCH: TODO("OP_CATCH");break;
            case OP_ENDCATCH: TODO("OP_ENDCATCH");break;
            case OP_WITH: TODO("OP_WITH");break;
            case OP_ENDWITH: TODO("OP_ENDWITH");break;
            case OP_DEBUGGER: TODO("OP_DEBUGGER");break;
            case OP_JUMP: {
                int offset = *pc++;
                printf("// OP_JUMP(%d)\n", offset);
                printf("    jmp label_%s_%d\n", name, offset);
            };break;
            case OP_JTRUE: {
                int offset = *pc++;
                printf("// OP_JTRUE(%d)\n", offset);
                printf("    pop rax\n");
                printf("    cmp rax, 1\n");
                printf("    je label_%s_%d\n", name, offset);
            };break;
            case OP_JFALSE: {
                int offset = *pc++;
                printf("// OP_JFALSE(%d)\n", offset);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je label_%s_%d\n", name, offset);
            };break;
            case OP_RETURN: {
                printf("// OP_RETURN\n");
                printf("    pop rax\n");
                printf("    add rsp, %d\n", F->varlen*8);
                printf("    pop rbp\n");
                printf("    ret\n");
                printf("\n");
            };break;
            default:
                fprintf(stderr, "Unexpected opcode %u\n", opcode);
        }
    }

}
