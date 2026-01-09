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
void process_output(int16_t value, uint16_t address){
    if (address == CHAR_OUT){
        printf("OUT => ");
        printf("%c", (char)value);
    } else if (address == INT_OUT){
        printf("OUT => ");
        printf("%d", value);
    }
    printf("\n");
    fflush(stdout);
}

int16_t process_input(uint16_t address){
    fflush(stdout);
    if (address == CHAR_IN){
        printf("IN => ");
        return (int16_t)getchar();
    } else if (address == INT_IN){
        int temp;
        printf("IN => ");
        if (scanf("%d", &temp) == 1) return (int16_t)temp;
    }
    return 0;
}

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
        if (breakpoints[original_pc]){
            printf("\n--- BREAKPOINT AT 0x%04X ---\n", original_pc);
            printf("IR: 0x%04X\n", cpu.ir);
            for (int i = 0; i < 14; i++) {
                printf("R%d = 0x%04hX\n", i, cpu.regs[i]);
            }
            printf("R14 (SP) = 0x%04hX\n", cpu.regs[SP]);
            printf("R15 (PC) = 0x%04hX\n", cpu.regs[PC]);
            printf("Flags:\n Z = %d\n C = %d\n", cpu.flags.zero, cpu.flags.carry);
            for (int addr = 0; addr < 0x2000; addr++) {
                if (cpu.mem_accessed[addr]) {
                    printf("%04X %04X\n", addr, cpu.ram[addr]);
                }
            }
            if (cpu.regs[SP] != 0x2000) {
                for (int addr = 0x1FFF; addr >= cpu.regs[SP]; addr--) {
                    printf("%04X %04X\n", addr, cpu.ram[addr]);
                }
            }
            while (getchar() != '\n'); //é pra limpar o buffer;
            getchar();
        }
        
        /* Execute subcycle */
        switch(opcode) {
            /* Saltos */
            // JMP #Im: PC = PC + #Im
            case OP_JMP: {
                int16_t imediato = 0;
                imediato = (rd << 8) | (rm << 4) | rn;
                if (imediato & 0x0800) {   // se o bit 11 for 1
                    imediato |= 0xF000;    // estende o sinal para 16 bits
                }
                cpu.regs[PC] = cpu.regs[PC] + imediato;

                break;
            }
            
            // J<cond>
            case OP_JCOND: {
                int16_t imediato;
                bool deve_saltar = false;

                int cond = (cpu.ir >> 14) & 0x3;
                imediato = (cpu.ir >> 4) & 0x03FF;

                if(imediato & 0x0200) {   // bit de sinal (8 bits)
                    imediato |= 0xFC00;
                }

                if (cond == 0) {                 // JEQ
                    if (cpu.flags.zero)
                        deve_saltar = true;
                } else if (cond == 1) {          // JNE
                    if (!cpu.flags.zero)
                        deve_saltar = true;
                } else if (cond == 2) {          // JLT
                    if (cpu.flags.carry)
                        deve_saltar = true;
                } else if (cond == 3) {          // JGE
                    if (!cpu.flags.carry)
                        deve_saltar = true;
                }
                if (deve_saltar) {
                    cpu.regs[PC] = cpu.regs[PC] + imediato;
                }
                break;
            }

            // MOV: Rd, #Im: Rd = #Im
            case OP_MOV: {
                int16_t im = (rm << 4) | rn;
                if(im & 0x80) {
                    im |= 0xFF00;
                }
                cpu.regs[rd] = im;
                break;
            }

            /* Instruções básicas */
            //soma dos registradores
            case OP_ADD: {
                int32_t result = cpu.regs[rm] + cpu.regs[rn];
                cpu.regs[rd] = (int16_t) result;
                cpu.flags.zero = (cpu.regs[rd] == 0);
                cpu.flags.carry = (result > 0xFFFF);
                break;
            }

            //soma com imediato
            case OP_ADDI: {
                int8_t imm = rn;
                int32_t result = cpu.regs[rm] + imm;
                cpu.regs[rd] = (int16_t) result;
                cpu.flags.carry = (result > 0xFFFF);
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

            // mover para a direira (shift right)
            case OP_SHR: {
                int16_t imm = rn & 0xF;
                cpu.regs[rd] = cpu.regs[rm] >> imm;
                if (imm > 0)
                    cpu.flags.carry = (cpu.regs[rm] >> (imm - 1)) & 1;
                else
                    cpu.flags.carry = false;
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;
            }
            //mover para a esquerda (shift left)
            case OP_SHL: {
                int16_t imm = rn & 0xF;
                cpu.regs[rd] = cpu.regs[rm] << imm;
                if (imm > 0)
                    cpu.flags.carry = (cpu.regs[rm] >> (16 - imm)) & 1;
                else
                    cpu.flags.carry = false;
                cpu.flags.zero = (cpu.regs[rd] == 0);
                break;

            }

            //comparação entre registradores
            case OP_CMP: {
                int16_t result = (int16_t)(cpu.regs[rm] - cpu.regs[rn]);
                cpu.flags.carry = (cpu.regs[rm] < cpu.regs[rn]);
                cpu.flags.zero = (result == 0);
                break;
            }

            // Operações de memória e pilha
            //Rd = MEM[Rm + #Im] 
			case OP_LDR: {
                int8_t im = rn;
                uint16_t add = cpu.regs[rm] + im;
                //Chama a função de Entrada;
                if (add >= 0xF000){
                    cpu.regs[rd] = process_input(add);
                } else if (add < MEM_SIZE){
                    cpu.regs[rd] = cpu.ram[add];
                    cpu.mem_accessed[add] = true;
                } else {
                    isa_halt = true;
                }
                break;
            }

            //MEM[Rm + #Im] = Rn
            case OP_STR: {
                int8_t im = rd;
                uint16_t add = cpu.regs[rm] + im;
                //Chama a função de Saída;
                if (add >= 0xF000){
                    process_output(cpu.regs[rn], add);
                } else if (add < MEM_SIZE){
                    cpu.ram[add] = cpu.regs[rn];
                    cpu.mem_accessed[add] = true;
                } else {
                    isa_halt = true;
                }
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
    for(int i = 0; i < 16; i++) {
        printf("R%d = 0x%04hX\n", i, cpu.regs[i]);
    }
    /* Impressão final do estado da memória acessada em notação hexadecimal */
    for(uint16_t addr = 0; addr < 0x2000; addr++) {
        if(cpu.mem_accessed[addr]) {
            printf("%04X %04X\n", addr, cpu.ram[addr]);
        }
    }
    /* Impressão final do estado das flags */
    printf("Z = 0x%X\n", cpu.flags.zero);
    printf("C = 0x%X\n", cpu.flags.carry);
    /* Impressão final do estado da pilha em notação hexadecimal */
    if(cpu.regs[SP] != 0x2000) {
        uint16_t sp = cpu.regs[SP];
        if(sp < 0x2000) {
            for(uint16_t addr = sp; addr < 0x2000; addr++) {
                printf("[0x%04X] = 0x%04X\n", addr, cpu.ram[addr]);
            }
        }
    }
    
    return 0;
}