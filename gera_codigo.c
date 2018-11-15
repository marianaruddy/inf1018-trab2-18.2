#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int (*funcp) (int x);
void gera_codigo (FILE *f, void **code, funcp *entry){
    int line = 1;
    int  c;
    int  i;
    /***********************************  TO DO  ******************************************************* 
    *1) precisamos confirmar os endereços de memória desses caras aqui de baixo, mas acho que a ideia é essa aqui.
    *2) precisamos corrigir os comandos de comparação
    *3) falta pegar os je ou jne de assembly
    * 
    ****************************************************************************************************/
    /* movq $1, %rax */
	static unsigned char mov_cnst[] = {0x48,0xc7,0xc0}; // atribuicao de valor de constante
	/* movq -idx(%rbp), %rax */
	static unsigned char mov_Vx[] = {0x48,0x8b,0x45}; // atribuicao de valor de variavel
	/* cmp $constante, %rax */
	static unsigned char cmp_cnst[] = {0x48,0x83, 0xf8}; // comparacao de constante
    /* cmp -idx(%rbp), %rax */
	static unsigned char cmp_Vx[] = {0x48, 0x3b, 0x45}; // comparacao de variavel
    /* cmp %rdi, %rax */
	static unsigned char cmp_param[] = {}; // comparacao de parametro
	/* addq $constante, %rax */
	static unsigned char add_cnst[] = {0x48,0x05}; // soma com constante
	/* addq -idx(%rbp), %rax */
	static unsigned char add_Vx[] = {0x48,0x03,0x45}; // soma com variaveis
	/* addq (%rdi, %rsi ou %rdx), %rax */
	static unsigned char add_param[] = {0x48,0x01}; // soma com param
	/* subq $constante, %rax */
	static unsigned char sub_cnst[] = {0x48,0x2d}; // subtração com constantes
	/* subq -idx(%rbp), %rax */
	static unsigned char sub_Vx[] = {0x48,0x2b,0x45}; // subtração com variaveis
	/* subq (%rdi, %rsi ou %rdx), %rax */
	static unsigned char sub_param[] = {0x48,0x29}; // subtraçãio com parametros
	/* imulq $constante, %rax */
	static unsigned char imul_cnst[] = {0x48,0x69,0xc0}; // multiplicação com constante
	/* imulq -idx(%rbp), %rax */
	static unsigned char imul_Vx[] = {0x48,0x0f,0xaf,0x45}; // multiplicação com variaveis
	/* imulq (%rdi, %rsi ou %rdx), %rax */
	static unsigned char imul_param[] = {0x48,0x0f,0xaf};// multiplicação com parametros
    /* leave - ret */
	static unsigned char ret[] = {0xc9,0xc3}; // retorno
    /* call func */
    static unsigned char call[] = {0xe8}; // call de função
	/* movq %rax, -idx0(%rbp) */
	static unsigned char atribut[] = {0x48,0x89,0x45};// atribuicao de valor de retorno para pilha
    if ((f = fopen ("programa", "r")) == NULL) {
        perror ("nao conseguiu abrir arquivo!");
        exit(1);
    }
    while ((c = fgetc(f)) != EOF) {
        switch (c) {
        case 'f': { /* function */
            char c0;
            if (fscanf(f, "unction%c", &c0) != 1)
            error("comando invalido", line);
            printf("function\n");
            break;
        }
        case 'e': { /* end */
            char c0;
            if (fscanf(f, "nd%c", &c0) != 1)
            error("comando invalido", line);
            printf("end\n");
            break;
        }
        case 'r': {  /* retorno incondicional */
            int idx0, idx1;
            char var0, var1;
            if (fscanf(f, "et %c%d", &var0, &idx0) != 2) 
            error("comando invalido", line);
            if(var0 == '$') {
                memcpy(&code[i], mov_cnst, sizeof(mov_cnst));
                i += sizeof(mov_cnst);
            } else if(var0 == 'v' ) {
                memcpy(&code[i], mov_Vx, sizeof(mov_Vx));
                i += sizeof(mov_Vx);
            }
            memcpy(&code[i],ret,sizeof(ret));
            i += sizeof(ret);
            printf("ret %c%d\n", var0, idx0);
            break;
        }
        case 'z': {  /* retorno condicional */
            int idx0, idx1;
            char var0, var1;
            if (fscanf(f, "ret %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4) 
            error("comando invalido", line);
            printf("zret %c%d %c%d\n", var0, idx0, var1, idx1);
            break;
        }
        case 'v': {  /* atribuicao */
            int idx0;
            char var0 = c, c0;
            if (fscanf(f, "%d = %c",&idx0, &c0) != 2)
            error("comando invalido",line);
            if (c0 == 'c') { /* call */
                int f, idx1;
                char var1;
                if (fscanf(f, "all %d %c%d\n", &f, &var1, &idx1) != 3)
                    error("comando invalido",line);
                memcpy(&code[i], call, sizeof(call));
                i += sizeof(call);
                memcpy(&code[i], f, sizeof(f));
                i += sizeof(f);
                memcpy(&code[i], idx1, sizeof(idx1));
                i += sizeof(idx1);
                printf("%c%d = call %d %c%d\n",var0, idx0, f, var1, idx1);
            }
            else { /* operacão aritmetica */
                int idx1, idx2;
                char var1 = c0, var2, op;
                if (fscanf(f, "%d %c %c%d", &idx1, &op, &var2, &idx2) != 4)
                    error("comando invalido", line);
                if( op == "+" ) {
                    if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + p0 
                        memcpy(&code[i], add_Vx, sizeof(add_Vx));
                        i += sizeof(add_Vx);
                    } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 + p0 
                        memcpy(&code[i], add_cnst, sizeof(add_cnst));
                        i += sizeof(add_cnst);
                    } else { // v0 = p0 + x
                        memcpy(&code[i], add_param, sizeof(add_param));  
                        i += sizeof(add_param);
                    }
                    
                } else if( op == "-" ) {
                    if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + p0 
                        memcpy(&code[i], sub_Vx, sizeof(sub_Vx));
                        i += sizeof(sub_Vx);
                    } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 + p0 
                        memcpy(&code[i], sub_cnst, sizeof(sub_cnst));
                        i += sizeof(sub_cnst);
                    } else { // v0 = p0 + x
                        memcpy(&code[i], sub_param, sizeof(sub_param));  
                        i += sizeof(sub_param);
                    }
                    
                } else if( op == "*" ) {
                    if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + x
                        memcpy(&code[i], imul_Vx, sizeof(imul_Vx));
                        i += sizeof(imul_Vx);
                    } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 + x 
                        memcpy(&code[i], imul_cnst, sizeof(imul_cnst));
                        i += sizeof(imul_cnst);
                    } else { // v0 = p0 + x
                        memcpy(&code[i], imul_param, sizeof(imul_param));  
                        i += sizeof(imul_param);
                    }
                } else {
                    /*operação inexistente*/
                }
                printf("%c%d = %c%d %c %c%d\n",
                        var0, idx0, var1, idx1, op, var2, idx2);
            }
            break;
        }
        default: error("comando desconhecido", line);
        }
        line ++;
        fscanf(f, " ");
    }

    
}
void libera_codigo (void *p){

}
