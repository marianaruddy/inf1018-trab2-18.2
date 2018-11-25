#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/mman.h>
#define NUM_MAX_LINHAS 100

static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

typedef int (*funcp) ();
void gera_codigo (FILE *f, void **code, funcp *entry){
    int line = 1;
    int  c;
    int  i = 0;
    // criei um um unsigned char para conseguir colocar os bytes certinhos.
    unsigned char *codigo = (unsigned char *) malloc(sizeof(char)*NUM_MAX_LINHAS);
    /***********************************  TO DO  ******************************************************* 
    *1) precisamos confirmar os endereços de memória desses caras aqui de baixo, mas acho que a ideia é essa aqui.
    *2) precisamos corrigir os comandos de comparação
    *3) falta pegar os je ou jne de assembly
    * 
    ****************************************************************************************************/
    /* pushq %rbp - movq %rsp, %rbp */
	static unsigned char prologo[] = {0x55,0x48,0x89,0xe5};
    /* movq $1, %rax */
	static unsigned char mov_cnst[] = {0x48,0xc7,0xc0}; // atribuicao de valor de constante
	/* movq -idx(%rbp), %rax */
	static unsigned char mov_Vx[] = {0x48,0x8b,0x45}; // atribuicao de valor de variavel
    /* movq (%rdi, %rsi ou %rdx), %rax */
	static unsigned char mov_Px[] = {0x48,0x89};
    /* movq %rax, -idx0(%rbp) */
	static unsigned char mov_ret_to_Vx[] = {0x48,0x89,0x45};// atribuicao de valor de retorno para pilha
	/* cmp $constante, %rax */
	static unsigned char cmp_cnst[] = {0x48,0x83, 0xf8}; // comparacao de constante
    /* cmp -idx(%rbp), %rax */
	static unsigned char cmp_Vx[] = {0x48, 0x3b, 0x45}; // comparacao de variavel
    /* cmp %rdi, %rax */
	static unsigned char cmp_param[] = {0x48, 0x39, 0xf8}; // comparacao de parametro
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
	static unsigned char sub_param[] = {0x48,0x29}; // subtração com parametros
	/* imulq $constante, %rax */
	static unsigned char imul_cnst[] = {0x48,0x6b,0xc0}; // multiplicação com constante
	/* imulq -idx(%rbp), %rax */
	static unsigned char imul_Vx[] = {0x48,0x0f,0xaf,0x45}; // multiplicação com variaveis
	/* imulq (%rdi, %rsi ou %rdx), %rax */
	static unsigned char imul_param[] = {0x48,0x0f,0xaf};// multiplicação com parametros
    /* leave - ret */
	static unsigned char ret[] = {0xc9, 0xc3}; // retorno
    /* call func */
    static unsigned char call[] = {0xe8}; // call de função
	/* je */
    static unsigned char je[] = {0x74};// verificar se condicional eh verdadeira
    /* jne */
    static unsigned char jne[] = {0x75};// verificar se condicional eh falsa
    /* espaco em codigo reservado caso exista a necessidade de pilha */
    static unsigned char pilha[] = {0x90,0x90,0x90,0x90};
    
    int num_Vx = 0;
    
    memcpy(&codigo[i], prologo, sizeof(prologo));
    i+=sizeof(prologo);
    memcpy(&codigo[i],pilha,sizeof(pilha));
    i+=sizeof(pilha);

    while ((c = fgetc(f)) != EOF) {
        if(i == 90 ) 
            printf("código muito grande para ser compilado\n");
        switch (c) {
        case 'f': { /* function */
            char c0;
            if (fscanf(f, "unction%c", &c0) != 1){
                error("comando invalido", line);
            }
            printf("function\n");
            break;
        }
        case 'e': { /* end */
            char c0;
            if (fscanf(f, "nd%c", &c0) != 1){
                error("comando invalido", line);
            }
            printf("end\n");
            break;
        }
        case 'r': {  /* retorno incondicional */
            int idx0, idx1;
            char var0, var1;
            
            if (fscanf(f, "et %c%d", &var0, &idx0) != 2){
                error("comando invalido", line);
            }
            if(var0 == '$') {
                memcpy(&codigo[i], mov_cnst, sizeof(mov_cnst));
                i += sizeof(mov_cnst);
                *((int*) &codigo[i]) = idx0;
                i+=sizeof(idx0);
            } else if(var0 == 'v' ) {
                memcpy(&codigo[i], mov_Vx, sizeof(mov_Vx));
                i += sizeof(mov_Vx);
         	 	codigo[i++]= 0xf8 -8*idx0;
            } else if(var0 == 'p') {
                memcpy(&codigo[i], mov_Vx, sizeof(mov_Vx));
                i+=sizeof(mov_Px);
                if(idx0 == 0)
                    codigo[i++]= 0xf8;
                else if(idx0 == 1)
                    codigo[i++]= 0xf0;
                else
                    codigo[i++]= 0xd0;
            }
            memcpy(&codigo[i],ret,sizeof(ret));
            i += sizeof(ret);
            printf("ret %c%d\n", var0, idx0);
            break;
        }
        case 'z': {  /* retorno condicional */
            int idx0, idx1;
            char var0, var1;
            if (fscanf(f, "ret %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4) {
                error("comando invalido", line);
            }
            if(var0 == 'p') {
                memcpy(&codigo[i], mov_Px,sizeof(mov_Px));
                i+=sizeof(mov_Px);
                if(idx0 == 0)
                    codigo[i++]= 0xf8;
                else if(idx0 == 1)
                    codigo[i++]= 0xf0;
                else
                    codigo[i++]= 0xd0;
            } else if( var0 == 'v') {
                memcpy(&codigo[i], mov_Vx,sizeof(mov_Vx));
                i+=sizeof(mov_Vx);
                codigo[i++]= 0xf8 -8*idx0;
            } else {
                if(idx0 == 0 ) {
                    memcpy(&codigo[90], ret, sizeof(ret));
                }
            }
            memcpy(&codigo[i], cmp_cnst, sizeof(cmp_cnst));
            i+=sizeof(cmp_cnst);
            *((int*) &codigo[i]) = 0;
            i+=sizeof(0);
            memcpy(&codigo[i], je, sizeof(je));
            i+=sizeof(je);
            codigo[i++] = 0x5a; // Colocando em 0x5a sequencia que indicará que condicional foi aceita
            memcpy(&codigo[90], ret, sizeof(ret));
            printf("zret %c%d %c%d\n", var0, idx0, var1, idx1);
            break;
        }
        case 'v': {  /* atribuicao */
            int idx0;
            char var0 = c, c0;
            num_Vx++;
            if (fscanf(f, "%d = %c",&idx0, &c0) != 2){
                error("comando invalido", line);
            }
            if (c0 == 'c') { /* call */
                int idx1, idx2;
                char var1;
                if (fscanf(f, "all %d %c%d\n", &idx1, &var1, &idx2) != 3){
                    error("comando invalido", line);
                }
                memcpy(&codigo[i], call, sizeof(call));
                i += sizeof(call);
                memcpy(&codigo[i], &idx1, sizeof(idx1));
                i += sizeof(idx1);
                memcpy(&codigo[i], &idx2, sizeof(idx2));
                i += sizeof(idx2);
                printf("%c%d = call %d %c%d\n",var0, idx0, idx1, var1, idx2);
            }
            else { /* operacão aritmetica */
                int idx1, idx2;
                char var1 = c0, var2, op;
                if (fscanf(f, "%d %c %c%d", &idx1, &op, &var2, &idx2) != 4){
                    error("comando invalido", line);
                }
                if( op == '+' ) {
                    if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + p0
                        memcpy(&codigo[i], mov_Vx, sizeof(mov_Vx));
                        i += sizeof(mov_Vx);
                        if(c0 == 'v') {
                            codigo[i++]= 0xf8 -8*idx1;
                            if(var2 == 'p') {
                                memcpy(&codigo[i], add_param, sizeof(add_param));
                                i+=sizeof(add_param);
                                if(idx2 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx2 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else if(var2 == 'v') {
                                memcpy(&codigo[i], add_Vx, sizeof(add_Vx));
                                i+=sizeof(add_Vx);
                                codigo[i++]= 0xf8 -8*idx2;
                            } else { // se var2 == $
                                memcpy(&codigo[i], add_cnst, sizeof(add_cnst));
                                i+=sizeof(add_cnst);
                                *((int*) &codigo[i]) = idx2;
                                i+=sizeof(idx2);
                            }
                        } else { // var2 == v
                            codigo[i++]= 0xf8 -8*idx2;
                            if(c0 == 'p') {
                                memcpy(&codigo[i], add_param, sizeof(add_param));
                                i+=sizeof(add_param);
                                if(idx1 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx1 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else if(c0 == 'v') {
                                memcpy(&codigo[i], add_Vx, sizeof(add_Vx));
                                i+=sizeof(add_Vx);
                                codigo[i++]= 0xf8 -8*idx1;
                            } else { // se c0 == $
                                memcpy(&codigo[i], add_cnst, sizeof(add_cnst));
                                i+=sizeof(add_cnst);
                                *((int*) &codigo[i]) = idx1;
                                i+=sizeof(idx1);
                            }
                        }            
                        
                    } else if( c0 == '$' || var2 == '$' ) { // v0 = p0 + $10
                        memcpy(&codigo[i], mov_cnst, sizeof(mov_cnst));
                        i+=sizeof(mov_cnst);
                        if(c0 == '$') {
                            *((int*) &codigo[i]) = idx1;
                            i+=sizeof(idx1);
                            if(var2 == 'p') {
                                memcpy(&codigo[i], add_param, sizeof(add_param));
                                i+=sizeof(add_param);
                                if(idx2 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx2 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else { //var2 == v
                                memcpy(&codigo[i], add_Vx, sizeof(add_Vx));
                                i+=sizeof(add_Vx);
                                codigo[i++]= 0xf8 -8*idx2;
                            }
                        } else { // var2 == $
                            *((int*) &codigo[i]) = idx2;
                            i+=sizeof(idx2);
                            if(c0 == 'p') {
                                memcpy(&codigo[i], add_param, sizeof(add_param));
                                i+=sizeof(add_param);
                                if(idx1 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx1 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else { //c0 == v
                                memcpy(&codigo[i], add_Vx, sizeof(add_Vx));
                                i+=sizeof(add_Vx);
                                codigo[i++]= 0xf8 -8*idx1;
                            }
                        }                        
                    } else { // v0 = p0 + x
                        memcpy(&codigo[i], mov_Px, sizeof(mov_Px));
                        i+=sizeof(mov_Px);
                        if(idx1 == 0)
                            codigo[i++]= 0xf8;
                        else if(idx1 == 1)
                            codigo[i++]= 0xf0;
                        else
                            codigo[i++]= 0xd0;
                        memcpy(&codigo[i], add_param, sizeof(add_param));
                        i+=sizeof(add_param);
                        if(idx2 == 0)
                            codigo[i++]= 0xf8;
                        else if(idx1 == 1)
                            codigo[i++]= 0xf0;
                        else
                            codigo[i++]= 0xd0;
                    }
                } else if( op == '-' ) {
                    if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 - p0 
                        memcpy(&codigo[i], mov_Vx, sizeof(mov_Vx));
                        i += sizeof(mov_Vx);
                        if(c0 == 'v') {
                            codigo[i++]= 0xf8 -8*idx1;
                            if(var2 == 'p') {
                                memcpy(&codigo[i], sub_param, sizeof(sub_param));
                                i+=sizeof(sub_param);
                                if(idx2 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx2 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else if(var2 == 'v') {
                                memcpy(&codigo[i], sub_Vx, sizeof(sub_Vx));
                                i+=sizeof(sub_Vx);
                                codigo[i++]= 0xf8 -8*idx2;
                            } else { // se var2 == $
                                memcpy(&codigo[i], sub_cnst, sizeof(sub_cnst));
                                i+=sizeof(sub_cnst);
                                *((int*) &codigo[i]) = idx2;
                                i+=sizeof(idx2);
                            }
                        } else { // var2 == v
                            codigo[i++]= 0xf8 -8*idx2;
                            if(c0 == 'p') {
                                memcpy(&codigo[i], sub_param, sizeof(sub_param));
                                i+=sizeof(sub_param);
                                if(idx1 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx1 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else if(c0 == 'v') {
                                memcpy(&codigo[i], sub_Vx, sizeof(sub_Vx));
                                i+=sizeof(sub_Vx);
                                codigo[i++]= 0xf8 -8*idx1;
                            } else { // se c0 == $
                                memcpy(&codigo[i], sub_cnst, sizeof(sub_cnst));
                                i+=sizeof(sub_cnst);
                                *((int*) &codigo[i]) = idx1;
                                i+=sizeof(idx1);
                            }
                        } 
                    } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 - p0 
                        memcpy(&codigo[i], mov_cnst, sizeof(mov_cnst));
                        i+=sizeof(mov_cnst);
                        if(c0 == '$') {
                            *((int*) &codigo[i]) = idx1;
                            i+=sizeof(idx1);
                            if(var2 == 'p') {
                                memcpy(&codigo[i], sub_param, sizeof(sub_param));
                                i+=sizeof(sub_param);
                                if(idx2 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx2 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else { //var2 == v
                                memcpy(&codigo[i], sub_Vx, sizeof(sub_Vx));
                                i+=sizeof(sub_Vx);
                                codigo[i++]= 0xf8 -8*idx2;
                            }
                        } else { // var2 == $
                            *((int*) &codigo[i]) = idx2;
                            i+=sizeof(idx2);
                            if(c0 == 'p') {
                                memcpy(&codigo[i], sub_param, sizeof(sub_param));
                                i+=sizeof(sub_param);
                                if(idx1 == 0)
                                    codigo[i++]= 0xf8;
                                else if(idx1 == 1)
                                    codigo[i++]= 0xf0;
                                else
                                    codigo[i++]= 0xd0;
                            } else { //c0 == v
                                memcpy(&codigo[i], sub_Vx, sizeof(sub_Vx));
                                i+=sizeof(sub_Vx);
                                codigo[i++]= 0xf8 -8*idx1;
                            }
                        }
                    } else { // v0 = p0 - x
                        memcpy(&codigo[i], mov_Px, sizeof(mov_Px));
                        i+=sizeof(mov_Px);
                        if(idx1 == 0)
                            codigo[i++]= 0xf8;
                        else if(idx1 == 1)
                            codigo[i++]= 0xf0;
                        else
                            codigo[i++]= 0xd0;
                        memcpy(&codigo[i], sub_param, sizeof(sub_param));
                        i+=sizeof(sub_param);
                        if(idx2 == 0)
                            codigo[i++]= 0xf8;
                        else if(idx1 == 1)
                            codigo[i++]= 0xf0;
                        else
                            codigo[i++]= 0xd0;
                    }                    
                } else if( op == '*' ) {
                    printf("multiplicação\n");
                   if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 * x
                        memcpy(&codigo[i], mov_Vx, sizeof(mov_Vx));
                        i += sizeof(mov_Vx);
                        if(c0 == 'v') {
                            codigo[i++]= 0xf8 -8*idx1;
                            if(var2 == 'p') {
                                memcpy(&codigo[i], imul_param, sizeof(imul_param));
                                i+=sizeof(imul_param);
                                if(idx2 == 0)
                                    codigo[i++]= 0xc7;
                                else if(idx2 == 1)
                                    codigo[i++]= 0xc6;
                                else
                                    codigo[i++]= 0xc2;
                            } else if(var2 == 'v') {
                                memcpy(&codigo[i], imul_Vx, sizeof(imul_Vx));
                                i+=sizeof(imul_Vx);
                                codigo[i++]= 0xf8 -8*idx2;
                            } else { // se var2 == $
                                memcpy(&codigo[i], imul_cnst, sizeof(imul_cnst));
                                i+=sizeof(imul_cnst);
                                *((int*) &codigo[i]) = idx2;
                                i+=sizeof(idx2);
                            }
                        } else { // var2 == v
                            codigo[i++]= 0xf8 -8*idx2;
                            if(c0 == 'p') {
                                memcpy(&codigo[i], imul_param, sizeof(imul_param));
                                i+=sizeof(imul_param);
                                if(idx1 == 0)
                                    codigo[i++]= 0xc7;
                                else if(idx1 == 1)
                                    codigo[i++]= 0xc6;
                                else
                                    codigo[i++]= 0xc2;
                            } else if(c0 == 'v') {
                                memcpy(&codigo[i], imul_Vx, sizeof(imul_Vx));
                                i+=sizeof(imul_Vx);
                                codigo[i++]= 0xf8 -8*idx1;
                            } else { // se c0 == $
                                memcpy(&codigo[i], imul_cnst, sizeof(imul_cnst));
                                i+=sizeof(imul_cnst);
                                *((int*) &codigo[i]) = idx1;
                                i+=sizeof(idx1);
                            }
                        } 
                    } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 * x 
                        memcpy(&codigo[i], mov_cnst, sizeof(mov_cnst));
                        i+=sizeof(mov_cnst);
                        if(c0 == '$') {
                            *((int*) &codigo[i]) = idx1;
                            i+=sizeof(idx1);
                            if(var2 == 'p') {
                                memcpy(&codigo[i], imul_param, sizeof(imul_param));
                                i+=sizeof(imul_param);
                                if(idx2 == 0)
                                    codigo[i++]= 0xc7;
                                else if(idx2 == 1)
                                    codigo[i++]= 0xc6;
                                else
                                    codigo[i++]= 0xc2;
                            } else { //var2 == v
                                memcpy(&codigo[i], imul_Vx, sizeof(imul_Vx));
                                i+=sizeof(imul_Vx);
                                codigo[i++]= 0xf8 -8*idx2;
                            }
                        } else { // var2 == $
                            *((int*) &codigo[i]) = idx2;
                            i+=sizeof(idx2);
                            if(c0 == 'p') {
                                memcpy(&codigo[i], imul_param, sizeof(imul_param));
                                i+=sizeof(imul_param);
                                if(idx1 == 0)
                                    codigo[i++]= 0xc7;
                                else if(idx1 == 1)
                                    codigo[i++]= 0xc6;
                                else
                                    codigo[i++]= 0xc2;
                            } else { //c0 == v
                                memcpy(&codigo[i], imul_Vx, sizeof(imul_Vx));
                                i+=sizeof(imul_Vx);
                                codigo[i++]= 0xf8 -8*idx1;
                            }
                        }
                    } else { // v0 = p0 * x
                        memcpy(&codigo[i], mov_Px, sizeof(mov_Px));
                        i+=sizeof(mov_Px);
                        if(idx1 == 0)
                            codigo[i++]= 0xc7;
                        else if(idx1 == 1)
                            codigo[i++]= 0xc6;
                        else
                            codigo[i++]= 0xc2;
                    }
                } else {
                    /*operação inexistente*/
                }
                memcpy(&codigo[i],mov_ret_to_Vx,sizeof(mov_ret_to_Vx));
                i += sizeof(mov_ret_to_Vx);
         	 	codigo[i++]= 0xf8 -8*idx0;
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
    int aux = 0;
    while( aux < i) {
        printf("%d - %p\n",aux,codigo[aux]);
        aux++;
    }
    if(num_Vx != 0) { // Preencheremos esse espaço de memória para fornecer memória em caso de variaveis 
		codigo[4]= 0x48;
		codigo[5]= 0x83;
		codigo[6]= 0xec;
		codigo[7]= 16*(num_Vx) - (num_Vx/2)*16;		
	}
	printf("Numero de variaveis locais: %d\n",num_Vx); 
    funcp fu = (funcp)codigo;
    *entry = (funcp*)codigo;
    code = &codigo;
    int res = (*fu)(100);
    printf("resultado = %d \n",res);
}
void libera_codigo (void *p){
    free(p);
}
int main (void) {
    FILE *fp;
    void *code = (unsigned char *) malloc(sizeof(char)*NUM_MAX_LINHAS);;
    funcp funcSBF;
    int res;
    if ((fp = fopen ("teste.txt", "r")) == NULL) {
        perror ("nao conseguiu abrir arquivo!");
        exit(1);
    }
    gera_codigo (fp, &code, &funcSBF);
    res = (*funcSBF)(100);
    printf("resultado_main = %d \n",res);
    if ((code == NULL) || (funcSBF == NULL)) {
       printf("Erro na geracao\n");
    }
    //libera_codigo(code);
    //libera_codigo(funcSBF);
    fclose(fp);
    return 0;
}
