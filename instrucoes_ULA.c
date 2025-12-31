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
#define OP_ADD   (0x5) // to do
#define OP_ADDI  (0x6) // to do
#define OP_SUB   (0x7) // to do
#define OP_SUBI  (0x8) // to do
#define OP_AND   (0x9) // to do
#define OP_OR    (0xA) // to do
#define OP_SHR   (0xB) // to do
#define OP_SHL   (0xC) // to do
#define OP_CMP   (0xD) // to do
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
// Falta implementar

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
    int16_t imm;
    /*Variáveis para instruções de ULA*/
    uint16_t opcode;
    uint8_t rd, rs; // registradores destino e fonte
    uint32_t result; // resultado das operações
	/* Processor running */
    bool isa_halt = false;
    do {
        /* PC before modifications */
		uint16_t original_pc = cpu.regs[PC];

		/* Fetch subcycle */
        // Falta implementar
        
        /* Decode subcycle */
        // Falta implementar

        /* Breakpoint subcycle */
        // Falta implementar

		/* Execute subcycle
            Instruções ULA */
         switch(opcode) { 
            //soma dos registradores
            case OP_ADD: {
                 result = cpu.regs[rd] + cpu.regs[rs];
                cpu.carry = (result > INT16_MAX || result < INT16_MIN);
                cpu.regs[rd] = (int16_t) result;
                cpu.zero = (cpu.regs[rd] == 0);
                break;

            }
            //soma com imediato
            case OP_ADDI: {
                result = cpu.regs[rd] + imm;
                cpu.carry = (result > INT16_MAX || result < INT16_MIN);
                cpu.regs[rd] = (int16_t) result;
                cpu.zero = (cpu.regs[rd] == 0);
                break;

            }
            //subtração dos registradores
            case OP_SUB: {
                result = cpu.regs[rd] - cpu.regs[rs];
                cpu.carry = (result > INT16_MAX || result < INT16_MIN);
                cpu.regs[rd] = (int16_t) result;
                cpu.zero = (cpu.regs[rd] == 0);
                break;

            }
            //subtração com imediato
            case OP_SUBI: {
                result = cpu.regs[rd] - imm;
                cpu.carry = (result > INT16_MAX || result < INT16_MIN);
                cpu.regs[rd] = (int16_t) result;
                cpu.zero = (cpu.regs[rd] == 0);
                break;
            }
            //AND entre registradores
            case OP_AND: {
                cpu.regs[rd] = cpu.regs[rd] & cpu.regs[rs];
                cpu.carry = false;
                cpu.zero = (cpu.regs[rd] == 0);
                break;
            }
            //OR entre registradores
            case OP_OR: {
                cpu.regs[rd] = cpu.regs[rd] | cpu.regs[rs];
                cpu.carry = false;
                cpu.zero = (cpu.regs[rd] == 0);
                break;
            }
            // mover para a direira (shift right)
            case OP_SHR: {
                cpu.regs[rd] >>= 1;
                cpu.carry = cpu.regs[rd] & 0x1;
                cpu.zero = (cpu.regs[rd] == 0);
                break;
            }
            //mover para a esquerda (shift left)
            case OP_SHL: {
                cpu.regs[rd] <<= 1;
                cpu.carry = (cpu.regs[rd] & 0x8000) != 0;
                cpu.zero = (cpu.regs[rd] == 0);
                break;
            }
            //comparação
            //altera flags
            case OP_CMP: {
                int32_t result = cpu.regs[rd] - cpu.regs[rs];
                cpu.zero = (result == 0);
                cpu.carry = (result < 0);
                break;
            }

          }
        
    } while(!isa_halt);
    
    return 0;
}