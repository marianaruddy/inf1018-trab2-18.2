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
    unsigned char* codigo = (unsigned char*)code;
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
	static unsigned char imul_cnst[] = {0x48,0x69,0xc0}; // multiplicação com constante
	/* imulq -idx(%rbp), %rax */
	static unsigned char imul_Vx[] = {0x48,0x0f,0xaf,0x45}; // multiplicação com variaveis
	/* imulq (%rdi, %rsi ou %rdx), %rax */
	static unsigned char imul_param[] = {0x48,0x0f,0xaf};// multiplicação com parametros
    /* leave - ret */
	static unsigned char ret[] = {0xc9, 0xc3}; // retorno
    /* call func */
    static unsigned char call[] = {0xe8}; // call de função
	/* movq %rax, -idx0(%rbp) */
	static unsigned char atribut[] = {0x48,0x89,0x45};// atribuicao de valor de retorno para pilha
	/* je */
    static unsigned char je[] = {0x74};// verificar se condicional eh verdadeira
    /* jne */
    static unsigned char jne[] = {0x75};// verificar se condicional eh falsa
    //memcpy(&codigo[i], prologo, sizeof(prologo));
    //i+=sizeof(prologo);
    
    /* Aqui eu to fazendo com que o ponteiro codigo tenha o codigo de maquina dos comandos para executar essa funcao:
     *
     * function
     * ret $10
     * end 
     *************************/
    
    memcpy(&codigo[i], mov_cnst, sizeof(mov_cnst));
    i+=sizeof(mov_cnst);
    int idx = 4;
    *((int*) &codigo[i]) = idx;		//insere valor em little endian
    i += sizeof(int);
    memcpy(&codigo[i], ret, sizeof(ret));
    i+=sizeof(ret);
    //code = malloc(NUM_MAX_LINHAS*100);
    //entry = malloc(NUM_MAX_LINHAS*100);
    
    
    
    /* Comentei essa parte do código para ver se consigo "marretar" o código de máquina para executar o comando que eu quero */
    
//     while ((c = fgetc(f)) != EOF) {
//         switch (c) {
//         case 'f': { /* function */
//             char c0;
//             if (fscanf(f, "unction%c", &c0) != 1){
//                 error("comando invalido", line);
//             }
//             printf("function\n");
//             break;
//         }
//         case 'e': { /* end */
//             char c0;
//             if (fscanf(f, "nd%c", &c0) != 1){
//                 error("comando invalido", line);
//             }
//             printf("end\n");
//             break;
//         }
//         case 'r': {  /* retorno incondicional */
//             int idx0, idx1;
//             char var0, var1;
//             
//             if (fscanf(f, "et %c%d", &var0, &idx0) != 2){
//                 error("comando invalido", line);
//             }
//             if(var0 == '$') {
//                 memcpy(&code[i], mov_cnst, sizeof(mov_cnst));
//                 i += sizeof(mov_cnst);
//                 memcpy(&code[i], &idx0, sizeof(idx0));
//                 i += sizeof(idx0);
//             } else if(var0 == 'v' ) {
//                 memcpy(&code[i], mov_Vx, sizeof(mov_Vx));
//                 i += sizeof(mov_Vx);
//             }
//             memcpy(&code[i],ret,sizeof(ret));
//             i += sizeof(ret);
//             printf("ret %c%d\n", var0, idx0);
//             break;
//         }
//         case 'z': {  /* retorno condicional */
//             int idx0, idx1;
//             char var0, var1;
//             if (fscanf(f, "ret %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4) {
//                 error("comando invalido", line);
//             }
//             printf("zret %c%d %c%d\n", var0, idx0, var1, idx1);
//             break;
//         }
//         case 'v': {  /* atribuicao */
//             int idx0;
//             char var0 = c, c0;
//             if (fscanf(f, "%d = %c",&idx0, &c0) != 2){
//                 error("comando invalido", line);
//             }
//             if (c0 == 'c') { /* call */
//                 int idx1, idx2;
//                 char var1;
//                 if (fscanf(f, "all %d %c%d\n", &idx1, &var1, &idx2) != 3){
//                     error("comando invalido", line);
//                 }
//                 memcpy(&code[i], call, sizeof(call));
//                 i += sizeof(call);
//                 memcpy(&code[i], &idx1, sizeof(idx1));
//                 i += sizeof(idx1);
//                 memcpy(&code[i], &idx2, sizeof(idx2));
//                 i += sizeof(idx2);
//                 printf("%c%d = call %d %c%d\n",var0, idx0, idx1, var1, idx2);
//             }
//             else { /* operacão aritmetica */
//                 int idx1, idx2;
//                 char var1 = c0, var2, op;
//                 if (fscanf(f, "%d %c %c%d", &idx1, &op, &var2, &idx2) != 4){
//                     error("comando invalido", line);
//                 }
//                 if( op == '+' ) {
//                     if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + p0 
//                         memcpy(&code[i], add_Vx, sizeof(add_Vx));
//                         i += sizeof(add_Vx);
//                     } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 + p0 
//                         memcpy(&code[i], add_cnst, sizeof(add_cnst));
//                         i += sizeof(add_cnst);
//                     } else { // v0 = p0 + x
//                         memcpy(&code[i], add_param, sizeof(add_param));  
//                         i += sizeof(add_param);
//                     }
//                     
//                 } else if( op == '-' ) {
//                     if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + p0 
//                         memcpy(&code[i], sub_Vx, sizeof(sub_Vx));
//                         i += sizeof(sub_Vx);
//                     } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 + p0 
//                         memcpy(&code[i], sub_cnst, sizeof(sub_cnst));
//                         i += sizeof(sub_cnst);
//                     } else { // v0 = p0 + x
//                         memcpy(&code[i], sub_param, sizeof(sub_param));  
//                         i += sizeof(sub_param);
//                     }
//                     
//                 } else if( op == '*' ) {
//                     if( c0 == 'v' || var2 == 'v' ) { // v0 = v0 + x
//                         memcpy(&code[i], imul_Vx, sizeof(imul_Vx));
//                         i += sizeof(imul_Vx);
//                     } else if( c0 == '$' || var2 == '$' ) { // v0 = $10 + x 
//                         memcpy(&code[i], imul_cnst, sizeof(imul_cnst));
//                         i += sizeof(imul_cnst);
//                     } else { // v0 = p0 + x
//                         memcpy(&code[i], imul_param, sizeof(imul_param));  
//                         i += sizeof(imul_param);
//                     }
//                 } else {
//                     /*operação inexistente*/
//                 }
//                 printf("%c%d = %c%d %c %c%d\n",
//                         var0, idx0, var1, idx1, op, var2, idx2);
//             }
//             break;
//         }
//         default: error("comando desconhecido", line);
//         }
//         line ++;
//         fscanf(f, " ");
//         
//     }
    // nessa parte eu to printando code, e vendo que ta tudo certinho msm
    printf("%p \n",code[0]);
    entry = (funcp*)code;
}
void libera_codigo (void *p){
    free(p);
}
int main (void){
    FILE *fp;
    void *code = (unsigned char *) malloc(sizeof(char)*NUM_MAX_LINHAS);;
    funcp funcSBF;
    int res;
    if ((fp = fopen ("teste.txt", "r")) == NULL) {
        perror ("nao conseguiu abrir arquivo!");
        exit(1);
    }
    gera_codigo (fp, &code, &funcSBF);
    res = (*funcSBF)();
    printf("resultado = %d \n",res);
    if ((code == NULL) || (funcSBF == NULL)) {
       printf("Erro na geracao\n");
    }
    //libera_codigo(code);
    //libera_codigo(funcSBF);
    //fclose(fp);
    return 0;
}
