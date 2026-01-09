main:   MOV R0, #char1
        ADDI R1, R15, #2
        PUSH R1
        JMP puts
        HALT
puts:   PUSH R1
start:  PUSH R0
        LDR R0, [R0, #0]
        OR R0, R0, R0
        JEQ end
        ADDI R1, R15, #2
        PUSH R1
        JMP putc
        POP R0
        ADDI R0, R0, #1
        JMP start
end:    POP R0
        POP R1
        POP R15
putc:   PUSH R1
        MOV R1, #0xF0   
        SHL R1, R1, #8
        STR R0, [R1, #1]
        POP R1
        POP R15
char1:  'H'
char2:  'e'
char3:  'l'
char4:  'l'
char5:  'o'
char6:  '\0'