#include <stdio.h> // for stderr
#include <stdlib.h> // for exit()
#include "types.h"
#include "utils.h"
#include "riscv.h"

void execute_rtype(Instruction, Processor *);
void execute_itype_except_load(Instruction, Processor *);
void execute_branch(Instruction, Processor *);
void execute_jal(Instruction, Processor *);
void execute_load(Instruction, Processor *, Byte *);
void execute_store(Instruction, Processor *, Byte *);
void execute_ecall(Processor *, Byte *);
void execute_lui(Instruction, Processor *);

void execute_instruction(uint32_t instruction_bits, Processor *processor,Byte *memory) {    
    Instruction instruction = parse_instruction(instruction_bits);
    switch(instruction.opcode) {
        case 0x33:
            execute_rtype(instruction, processor);
            break;
        case 0x13:
            execute_itype_except_load(instruction, processor);
            break;
        case 0x73:
            execute_ecall(processor, memory);
            break;
        case 0x63:
            execute_branch(instruction, processor);
            break;
        case 0x6F:
            execute_jal(instruction, processor);
            break;
        case 0x23:
            execute_store(instruction, processor, memory);
            break;
        case 0x03:
            execute_load(instruction, processor, memory);
            break;
        case 0x37:
            execute_lui(instruction, processor);
            break;
        default: // undefined opcode
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
    processor->PC += 4;/////////////////////////
}

void execute_rtype(Instruction instruction, Processor *processor) {
    switch (instruction.rtype.funct3){
        case 0x0:
            switch (instruction.rtype.funct7) {
                case 0x0:
                  // Add
                  processor->R[instruction.rtype.rd] =
                      ((sWord)processor->R[instruction.rtype.rs1]) +
                      ((sWord)processor->R[instruction.rtype.rs2]);
                  break;
                case 0x1:
                  // Mul
                  processor->R[instruction.rtype.rd] =
                      ((sWord)processor->R[instruction.rtype.rs1]) *
                      ((sWord)processor->R[instruction.rtype.rs2]);
                  break;
                case 0x20:
                    // Sub
                    processor->R[instruction.rtype.rd] =
                      ((sWord)processor->R[instruction.rtype.rs1]) -
                      ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x1:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // SLL @@@@@@@@@ no sWord
                    processor->R[instruction.rtype.rd] =
                      (((sWord)processor->R[instruction.rtype.rs1]) <<
                      (processor->R[instruction.rtype.rs2]));

                    break;
                case 0x1:
                    // MULH     rd = (rs1 * rs2)[63:32] return upper bits /////////////
                    processor->R[instruction.rtype.rd] =
                    ((sDouble)processor->R[instruction.rtype.rs1]) *
                    ((sDouble)processor->R[instruction.rtype.rs2]);



                    break;
            }
            break;
        case 0x2:
            // SLT  /////////////
            if(((sWord)processor->R[instruction.rtype.rs1]) < ((sWord)processor->R[instruction.rtype.rs2])){
                processor->R[instruction.rtype.rd] = 1;
            }
            else{
                processor->R[instruction.rtype.rd] = 0;
            }




            break;
        case 0x4:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // XOR @@@@@@@@@ no sWord
                     processor->R[instruction.rtype.rd] =
                      (((sWord)processor->R[instruction.rtype.rs1]) ^
                      ((sWord)processor->R[instruction.rtype.rs2]));
                    break;
                case 0x1:
                    // DIV
                     processor->R[instruction.rtype.rd] =
                      (((sWord)processor->R[instruction.rtype.rs1]) /
                      ((sWord)processor->R[instruction.rtype.rs2]));
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x5:
            switch (instruction.rtype.funct7) {
                case 0x0:
                // SRL    /////////////
                    processor->R[instruction.rtype.rd] =
                    (((sWord)processor->R[instruction.rtype.rs1]) >>
                    processor->R[instruction.rtype.rs2]);   
                    break;
                case 0x20:
                    // SRA  /////////////
                    processor->R[instruction.rtype.rd] =
                    (((sWord)processor->R[instruction.rtype.rs1]) >>
                    processor->R[instruction.rtype.rs2]);  



                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                break;
            }
            break;
        case 0x6:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // OR
                    processor->R[instruction.rtype.rd] =
                      (((sWord)processor->R[instruction.rtype.rs1]) |
                      ((sWord)processor->R[instruction.rtype.rs2]));
                    break;
                case 0x1:
                    // REM
                    processor->R[instruction.rtype.rd] =
                      (((sWord)processor->R[instruction.rtype.rs1]) %
                      ((sWord)processor->R[instruction.rtype.rs2]));
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        case 0x7:
            // AND
            processor->R[instruction.rtype.rd] =
                      (((sWord)processor->R[instruction.rtype.rs1]) &
                      ((sWord)processor->R[instruction.rtype.rs2]));
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_itype_except_load(Instruction instruction, Processor *processor) {
    switch (instruction.itype.funct3) {
        case 0x0:
            // ADDI
            processor->R[instruction.itype.rd] =
            ((sWord)processor->R[instruction.itype.rs1]) +
            sign_extend_number(instruction.itype.imm, 12);
            break;
        case 0x1:
            // SLLI
            processor->R[instruction.itype.rd] = 
            (sWord)processor->R[instruction.itype.rs1] << (sign_extend_number(instruction.itype.imm, 12) & 0x0000001f);
            break;
        case 0x2:
            // SLTI 983039  2701 -1395
            //printf("%d  %d %d\n", (sWord)processor->R[instruction.itype.rs1],instruction.itype.imm,sign_extend_number(instruction.itype.imm, 12));
            if(((sWord)processor->R[instruction.itype.rs1]) < sign_extend_number(instruction.itype.imm, 12)){
                (processor->R[instruction.itype.rd]) = 1;
                
            }
            else{ //slti goes here
                (processor->R[instruction.itype.rd]) = 0;
             //   instruction.itype.rd = 0;
            }
            break;
        case 0x4:
            // XORI
            processor->R[instruction.itype.rd] =
            ((sWord)processor->R[instruction.itype.rs1]) ^
            sign_extend_number(instruction.itype.imm, 12);

            break;
        case 0x5:
            // Shift Right (You must handle both logical and arithmetic)
            processor->R[instruction.itype.rd] = 
            (sWord)processor->R[instruction.itype.rs1] >> (sign_extend_number(instruction.itype.imm, 12) & 0x0000001f);

        case 0x6:
            // ORI
            processor->R[instruction.itype.rd] =
            ((sWord)processor->R[instruction.itype.rs1]) |
            sign_extend_number(instruction.itype.imm, 12);
            break;
        case 0x7:
            // ANDI
            processor->R[instruction.itype.rd] = 
            (sWord)processor->R[instruction.itype.rs1] & (sign_extend_number(instruction.itype.imm, 12) & 0x0000001f);
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }
}

void execute_ecall(Processor *p, Byte *memory) {
    Register i;
    
    // syscall number is given by a0 (x10)
    // argument is given by a1
    switch(p->R[10]) {
        case 1: // print an integer
            printf("%d",p->R[11]);
            break;
        case 4: // print a string
            for(i=p->R[11];i<MEMORY_SPACE && load(memory,i,LENGTH_BYTE);i++) {
                printf("%c",load(memory,i,LENGTH_BYTE));
            }
            break;
        case 10: // exit
            printf("exiting the simulator\n");
            exit(0);
            break;
        case 11: // print a character
            printf("%c",p->R[11]);
            break;
        default: // undefined ecall
            printf("Illegal ecall number %d\n", p->R[10]);
            exit(-1);
            break;
    }
}

void execute_branch(Instruction instruction, Processor *processor) {
    switch (instruction.sbtype.funct3) {
        case 0x0:
            // BEQ
            break;
        case 0x1:
            // BNE
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_load(Instruction instruction, Processor *processor, Byte *memory) {
    switch (instruction.itype.funct3) {
        case 0x0:
            // LB
            processor->R[instruction.itype.rd] = 
            load(memory, (sWord)processor->R[instruction.itype.rs1] + (sWord)sign_extend_number(instruction.itype.imm,12),LENGTH_BYTE);
            break;
        case 0x1:
            // LH
            processor->R[instruction.itype.rd] = 
            load(memory, (sWord)processor->R[instruction.itype.rs1] + (sWord)sign_extend_number(instruction.itype.imm,12),LENGTH_HALF_WORD);
            break;
        case 0x2:
            // LW
            processor->R[instruction.itype.rd] = 
            load(memory, (sWord)processor->R[instruction.itype.rs1] + (sWord)sign_extend_number(instruction.itype.imm,12),LENGTH_WORD);
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }
}

void execute_store(Instruction instruction, Processor *processor, Byte *memory) {
    switch (instruction.stype.funct3) {
        case 0x0:
            // SB BYTE
            store(memory,((sWord)processor->R[instruction.stype.rs1]) + ((sWord)get_store_offset(instruction)),
            LENGTH_BYTE, processor->R[instruction.stype.rs2]);
            break;
        case 0x1:
            // SH // HALF
            store(memory,((sWord)processor->R[instruction.stype.rs1]) + ((sWord)get_store_offset(instruction)),
            LENGTH_HALF_WORD, processor->R[instruction.stype.rs2]);
            break;
        case 0x2:
            // SW FULL WORD
            store(memory,((sWord)processor->R[instruction.stype.rs1]) + ((sWord)get_store_offset(instruction)),
            LENGTH_WORD, processor->R[instruction.stype.rs2]);
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_jal(Instruction instruction, Processor *processor) {
    /* YOUR CODE HERE */
     printf("%x ",processor->R[instruction.ujtype.rd]);
    processor->R[instruction.ujtype.rd] = (processor->PC + 4);
    printf("%x \n",processor->R[instruction.ujtype.rd]);
    printf("%x ",processor->PC);
    processor->PC += get_jump_offset(instruction);
    printf("%x \n",processor->PC);
}

void execute_lui(Instruction instruction, Processor *processor) {
    processor->R[instruction.utype.rd] = ((sWord)sign_extend_number(instruction.utype.imm,21)<< 12 );
}

void store(Byte *memory, Address address, Alignment alignment, Word value) {
    /* YOUR CODE HERE */
    if(alignment == LENGTH_WORD){
        Byte b = (Byte)((value & 0x000000ff));
        memory[address] = b;
        b = (Byte)((value & 0x0000ff00) >> 8);
        memory[address + 1] = b;
        b = (Byte)((value & 0x00ff0000) >> 16);
        memory[address + 2] = b;
        b = (Byte)((value & 0xff000000) >> 24);
        memory[address + 3] = b;
    }
    else if(alignment == LENGTH_HALF_WORD){
        Byte b = (Byte)((value & 0x000000ff));
        memory[address] = b;
        b = (Byte)((value & 0x0000ff00) >> 8);
        memory[address + 1] = b;
    }
    else{
        Byte b = (Byte)((value & 0x000000ff));
        memory[address] = b;
    }
}

Word load(Byte *memory, Address address, Alignment alignment) {
    /* YOUR CODE HERE */
    Word word = 0x00000000;
    if(alignment == LENGTH_WORD){
        word = word | memory[address];
        word = word | (memory[address +1] << 8);
        word = word | (memory[address +2] << 16);
        word = word | (memory[address +3] << 24);
    }
    else if(alignment == LENGTH_HALF_WORD){
        word = word | memory[address];
        word = word | (memory[address +1] << 8);
    }
    else{
    word = word | memory[address];
    }

    return word;
}
