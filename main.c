#include <stdio.h>
#include <stdlib.h> // malloc, free, strtol
#include <string.h> // memset
#include <stdbool.h>
#include <stdint.h>

/* Memory size: 16KB = 16 x 1024 = 16384 / 2 = 8192 */
#define MEM_SIZE 8192

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

/* Processador ISA */
typedef struct {
    int16_t regs[16];        // R0 a R15, incluindo SP e PC
    uint16_t ram[MEM_SIZE];  // 16KB
    bool zero;
    bool carry;
} isa;

/* I/O subcicle */

int main(int argc, char *argv[]) {
	/* Check arguments */
	if(argc < 2) {
		printf("ISA expects at least 2 arguments.\n");
		return 1;
	}

	/* Open input file */
	FILE *file = fopen(argv[1], "r");
	if(!file) {
		printf("Error opening %s!\n", argv[1]);
		return 1;
	}

	/* Set breakpoints */
	bool breakpoints[MEM_SIZE] = {false};
	for(int i = 2; i < argc; i++) {
		int address = strtol(argv[i], NULL, 16);
        if(address < MEM_SIZE) {
            breakpoints[address] = true;
        }
	}

    /* Zero ISA memory and registers*/
    isa cpu = {0};

    /* Fill ISA memory */
	uint16_t address, buffer;
    while(fscanf(file, "%hX %hX%*[^\n]", &address, &buffer) == 2) {
		if(address < MEM_SIZE) {
            cpu.ram[address] = buffer;
        }
    }

    /* Fecha o arquivo */
    fclose(file);

    /* Inicia pc e sp */
    cpu.regs[SP] = 0x2000; // SP inicial
    cpu.regs[PC] = 0x0000; // PC inicial

    /* Variável imediato */
    int16_t im;
    
	/* Processor running */
    bool isa_halt = false;
    do {
        /* PC before modifications */
		uint16_t original_pc = cpu.regs[PC];

		/* Fetch subcycle */
        if(cpu.regs[PC] >= MEM_SIZE) {
            isa_halt = true;
            break;
        }
        uint16_t instruction = cpu.ram[cpu.regs[PC]];
        cpu.regs[PC]++;
        
        /* Decode subcycle */
        // O simulador utiliza uma decodificação fixa dos campos de 4 bits, 
        // reinterpretando-os na fase de execução conforme a instrução.
        int16_t rd = (instruction >> 12) & 0xF; // Extrai os 4 bits mais significativos
        int16_t rm = (instruction >> 8) & 0xF; // Extrai os 4 bits do meio alto
        int16_t rn = (instruction >> 4) & 0xF; /// Extrai os 4 bits no maio alto
        int16_t opcode = instruction & 0xF; // Extrai os 4 bits menos significativos

        /* Case INST_HALT */
        if(instruction == INST_HALT) {
            isa_halt = true;
            continue;
        }
        
        /* Breakpoint subcycle */
        
        /* Execute subcycle */
        switch(opcode) {

            
            // Operações de memória e pilha
            case OP_LDR: //Rd = MEM[Rm + #Im]

                int8_t im = rn;
                uint16_t add = cpu.regs[rm]+im;

                if (add >= MEM_SIZE) {
                isa_halt = true;   
                break;
                }
                
                cpu.regs[rd] = cpu.ram[add];;
                break;

            case OP_STR://MEM[Rm + #Im] = Rn

                int8_t im = rd;
                uint16_t add = cpu.regs[rm] + im;

                if (add >= MEM_SIZE) {
                isa_halt = true;   
                break;
                }

                cpu.ram[add] = cpu.regs[rn];
                break;

            case OP_PUSH: //SP--; MEM[SP] = Rn
                cpu.regs[SP]--;
                cpu.ram[cpu.regs[SP]] = cpu.regs[rn];
                break;

            case OP_POP: //Rd = MEM[SP]; SP++
                cpu.regs[rd] = cpu.ram[cpu.regs[SP]];
                cpu.regs[SP]++;
                break;

            default:
				printf("Invalid instruction %04X!\n", cpu->regs[PC]);
				isa_halt = true;
				break;
        }
    } while(!isa_halt);
    
    return 0;
}