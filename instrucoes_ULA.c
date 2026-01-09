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
    uint16_t ram[MEM_SIZE];
    int16_t regs[16];
    uint16_t ir;
    struct {
        bool zero;
        bool carry;
    } flags;
    bool mem_accessed[MEM_SIZE];
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

    /* Inicializar todos os registradores e memória com 0*/
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
    cpu.regs[PC] = 0x0000; // PC inicial
    cpu.regs[SP] = 0x2000; // SP inicial
    
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
        cpu.ir = cpu.ram[cpu.regs[PC]];
        cpu.regs[PC]++;
        
        /* Decode subcycle */
        // O simulador utiliza uma decodificação fixa dos campos de 4 bits, 
        // reinterpretando-os na fa se de execução conforme a instrução.
        int16_t rd = (cpu.ir >> 12) & 0xF; // Extrai os 4 bits mais significativos
        int16_t rm = (cpu.ir >> 8) & 0xF; // Extrai os 4 bits do meio alto
        int16_t rn = (cpu.ir >> 4) & 0xF; /// Extrai os 4 bits no maio alto
        int16_t opcode = cpu.ir & 0xF; // Extrai os 4 bits menos significativos

        /* Case INST_HALT */
        if(cpu.ir == INST_HALT) {
            isa_halt = true;
            continue;
        }
        
        /* Breakpoint subcycle */
        // Falta implementar
        
        /* Execute subcycle */
        switch(opcode) {
            case OP_MOV: {
                int8_t im = (int8_t)((rm << 4) | rn);
                cpu.regs[rd] = im;
                break;
            }
            //soma dos registradores
            case OP_ADD: {
                int32_t result = cpu.regs[rm] + cpu.regs[rn];
                cpu.regs[rd] = (int16_t) result;
                if((result & 0x10000) != 0) {
                    cpu.flags.carry = true;
                } else {
                    cpu.flags.carry = false;
                }
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
             
            }
            //soma com imediato
            case OP_ADDI: {
                int8_t imm = rn;
                int32_t result = cpu.regs[rm] + imm;
                cpu.regs[rd] = (int16_t) result;
                if((result & 0x10000) != 0) {
                    cpu.flags.carry = true;
                } else {
                    cpu.flags.carry = false;
                }
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
            }
            //subtração dos registradores
            case OP_SUB: {
                int32_t result = cpu.regs[rm] - cpu.regs[rn];
                cpu.regs[rd] = (int16_t) result;
                cpu.flags.carry = (cpu.regs[rn] > cpu.regs[rm]);
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
            }
            //subtração com imediato
            case OP_SUBI: {
                int8_t imm = rn;
                int32_t result = cpu.regs[rm] - imm;
                cpu.regs[rd] = (int16_t) result;
                cpu.flags.carry = (imm > cpu.regs[rm]);   
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
            }
            //AND entre registradores
            case OP_AND: {
                cpu.regs[rd] = cpu.regs[rm] & cpu.regs[rn];
                cpu.flags.carry = false;
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
            }
            //OR entre registradores
            case OP_OR: {
                cpu.regs[rd] = cpu.regs[rm] | cpu.regs[rn];
                cpu.flags.carry = false;
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
            }
            case OP_SHR: {
                cpu.regs[rd] = cpu.regs[rm] >> imm;
                cpu.flags.carry = cpu.regs[rm] >> (imm - 1) & 1;
                cpu.flags.zero = (cpu.regs[rm] == 0);
                break;
            }
            //mover para a esquerda (shift left)
            case OP_SHL: {
                cpu.regs[rd] = cpu.regs[rm] << imm;
                cpu.flags.carry = (cpu.regs[rm] >> (16 - imm)) & 1;
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;

            }
            //comparação entre registradores
            case OP_CMP: {
                int16_t result = (int16_t){cpu.regs[rm] - cpu.regs[rn]);
                cpu.flags.carry = (cpu.regs[rm] < cpu.regs[rn]);
                cpu.flags.zero = (result == 0);
                break;
            }

            // Operações de memória e pilha
            //Rd = MEM[Rm + #Im] 
            case OP_LDR: {
                int8_t im = rn;
                uint16_t add = cpu.regs[rm]+im;

                if (add >= MEM_SIZE) {
                isa_halt = true;   
                break;
                }
                
                cpu.regs[rd] = cpu.ram[add];
                break;
            }

            //MEM[Rm + #Im] = Rn
            case OP_STR: {
                int8_t im = rd;
                uint16_t add = cpu.regs[rm] + im;

                if (add >= MEM_SIZE) {
                isa_halt = true;   
                break;
                }

                cpu.ram[add] = cpu.regs[rn];
                break;
            }
            //SP--; MEM[SP] = Rn
            case OP_PUSH: {
                cpu.regs[SP]--;
                cpu.ram[cpu.regs[SP]] = cpu.regs[rn];
                break;
            }
            //Rd = MEM[SP]; SP++
            case OP_POP: {
                cpu.regs[rd] = cpu.ram[cpu.regs[SP]];
                cpu.regs[SP]++;
                break;
            }

            default: {
				printf("Invalid instruction %04X!\n", cpu.ir);
				isa_halt = true;
				break;
            }
        }
    } while(!isa_halt);

    /* Impressão final do estado dos registradores em notação hexadecimal */
    printf("\n=== REGISTRADORES ===\n");
    for(int i = 0; i < 16; i++) {
        printf("R%d=0x%04hX\n", i, cpu.regs[i]);
    }
    /* Impressão final do estado da memória acessada em notação hexadecimal */
    printf("\n=== MEMORIA DE DADOS ===\n");
    for(uint16_t addr = 0; addr < 0x2000; addr++) {
        if(cpu.mem_accessed[addr]) {
            printf("%04X %04X\n", addr, cpu.ram[addr]);
        }
    }
    /* Impressão final do estado da pilha em notação hexadecimal */
    printf("\n=== PILHA ===\n");
    uint16_t sp = cpu.regs[SP];
    if(sp < 0x2000) {
        for(uint16_t addr = sp; addr < 0x2000; addr++) {
            printf("%04X %04X\n", addr, cpu.ram[addr]);
        }
    }
    /* Impressão final do estado das flags */
    printf("\n=== FLAGS ===\n");
    printf("ZERO = 0x%X\n", cpu.flags.zero);
    printf("CARRY = 0x%X\n", cpu.flags.carry);
    
    return 0;
}
