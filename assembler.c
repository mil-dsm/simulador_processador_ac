/* Instructions */
#define OP_JMP   (0x0)
#define OP_JCOND (0x1)
#define OP_LDR   (0x2)
#define OP_STR   (0x3)
#define OP_MOV   (0x4)
#define OP_ADD   (0x5)
#define OP_ADDI  (0x6)
#define OP_SUB   (0x7)
#define OP_SUBI  (0x8)
#define OP_AND   (0x9)
#define OP_OR    (0xA)
#define OP_SHR   (0xB)
#define OP_SHL   (0xC)
#define OP_CMP   (0xD)
#define OP_PUSH  (0xE)
#define OP_POP   (0xF)
#define INST_HALT (0xFFFF)

/* PC and SP */
#define SP (14)
#define PC (15)

/* INPUT AND OUTPUT */
#define CHAR_IN (0xF000)
#define CHAR_OUT (0xF001)
#define INT_IN (0xF002)
#define INT_OUT (0xF003)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct label {
    char nome[32];
    int address;

} Label;

/* Função de busca de labels */
// Caso a label exista, a função retorná o endereço da label
// Caso contrário, a função retornará -1
int search_label(char *nome, Label *labels, int label_count) {
    for(int i = 0; i < label_count; i++) {
        if(strcmp(labels[i].nome, nome) == 0) {
            return labels[i].address;
        }
    }
    // Caso não encontre, retorne -1
    return -1;
}

// Entrada: [arquivo em Assembly] [arquivo em hexadeciaml]
int main(int argc, char *argv[]) {
    /* Verificação dos argumentos */
    if(argc < 3) {
        printf("Uso: %s code.asm code.hex\n", argv[0]);
        return 1;
    }

    /* Cria arquivos: in(input, em assembly), out(output, em hexadecimal)*/
    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen(argv[2], "w");

    if(!in || !out) {
        printf("Erro ao abrir arquivos.\n");
        return 1;
    }

    char line[128];
    
    /* Identificação das labels */
    Label labels[100];
    int label_count = 0;
    // Cada instrução ou dado ocupa um endereço, então cria-se um Program Counter (PC)
    int pc = 0;

    // Se a linha tem ":" é label
    // O endereço da label é o pc atual
    while(fgets(line, sizeof(line), in) != NULL) {  // Lê uma linha e guarda o valor
        
        // Ignorar linhas vazias ou com comentários
        if(line[0] == '\n' || line[0] == '\r' || line[0] == ';') {
            continue;
        }

        // Verifica label e marca a posição dela ao ver se 
        // em que instrução ela está imediatamente anterior
        char label_nome[32]; // Guarda o nome das labels
        char resto[128]; // Caso após a label tenha algo, guarda na string

        // Lê do inicio da linha aponta até encontrar ":", e, caso encontre,
        // continua lendo o resto da linha para saber se tem algo mais
        if(sscanf(line, " %31[^:]:%127[^\n]", label_nome, resto) >= 1) {

            // Salva qual label está em determinado pc
            // ex: labels[0] = {"start", pc};
            strcpy(labels[label_count].nome, label_nome);
            labels[label_count].address = pc;
            label_count++;

            // Verificar se tem alguma instrução ou dado depois da label
            char *p = resto;
            while(*p == ' ') {
                p++;
            }

            if(*p != '\0') { // Se tiver um código depois da label, registra que tem
                pc++;
            }
        }
        // Linha sem label -> instrução ou dado
        else {
            pc++;
        }
    }

    // O ponteiro in sai do EOF e volta ao inicio do 
    // arquivo de input para começar a tradução
    rewind(in);

    /* Tradução das instruções */
    while(fgets(line, sizeof(line), in)) {

        // JLT #Im

        // JGE #Im

        // LDR Rd, [Rm, #Im]

        // STR Rn, [Rm, #Im]

        // MOV Rd, #Im

        // ADD Rd, Rm, Rn

        // ADDI Rd, Rm, #Im

        // SUB Rd, Rm, Rn

        // SUBI Rd, Rm, #Im

        // AND Rd, Rm, Rn

        // OR Rd, Rm, Rn

        // SHR Rd, Rm, #Im

        // SHL Rd, Rm, #Im

        // CMP Rm, Rn

        // PUSH Rn

        // POP Rd
    }

    fclose(in);
    fclose(out);

    return 0;
}