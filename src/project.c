#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    // all supported alu operations; for cool points, we're using hex numbers
    switch (ALUControl)
    {
        // add
        case 0x0:
            *ALUresult = A + B;
        break;
        
        // subtract
        case 0x1:
            *ALUresult = A - B;
        break;

        // set less than (signed)
        case 0x2:
            // int casting to accommodate for sign; ternary operator for simplicity
            *ALUresult = ((int)A < (int)B) ? 1 : 0;
        break;

        // set less than (unsigned)
        case 0x3:
            // same, but no cast needed
            *ALUresult = (A < B) ? 1 : 0;
        break;

        // AND
        case 0x4:
            *ALUresult = A & B;
        break;

        // OR
        case 0x5:
            *ALUresult = A | B;
        break;

        // Shift left B by 16 bits
        case 0x6:
            *ALUresult = B << 16;
        break;

        // NOT
        case 0x7:
            *ALUresult = ~A;
        break;
        }

    // Zero bit; set if ALUresult is zero
    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // attempt to fetch instruction
    // apply halt if the word is unaligned
    if(PC % 4 != 0){
        return 1;

        // word is aligned
    }else{
        *instruction = Mem[PC >> 2];
        return 0;
    }
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // sectioning off specific ranges of bits for various purposes
    // instruction code is bitwise OR with the required range in hexadecimal (shifts applied when necessary)

    // instruction[31-26] or 1111 1100 0000 0000 0000 0000 0000 0000
    *op = instruction >> 26 & 0xfc000000;

    // instruction[25-21] or 0000 0011 1110 0000 0000 0000 0000 0000
    *r1 = instruction >> 21 & 0x03e00000;

    // instruction[20-16] or 0000 0000 0001 1111 0000 0000 0000 0000
    *r2 = instruction >> 16 & 0x001f0000;

    // instruction[15-11] or 0000 0000 0000 0000 1111 1000 0000 0000
    *r3 = instruction >> 11 & 0x0000f800;

    // instruction[5-0] or 0000 0000 0000 0000 0000 0000 0011 1111
    *funct = instruction & 0x0000003f;

    // instruction[15-0] or 0000 0000 0000 0000 1111 1111 1111 1111
    *offset = instruction & 0x0000ffff;

    // instruction[25-0] or 0000 0011 1111 1111 1111 1111 1111 1111
    *jsec = instruction & 0x03ffffff;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
// Initialize all controls signals to 0
    controls->RegDst   = 0;
    controls->Jump     = 0;
    controls->Branch   = 0;
    controls->MemRead  = 0;
    controls->MemtoReg = 0;
    controls->ALUOp    = 0;
    controls->MemWrite = 0;
    controls->ALUSrc   = 0;
    controls->RegWrite = 0;

/*
    Instruction values are normally in binary. Here, they'll be converted to hex
    for a) cool points and b) convience, since the extra zeroes in long bit strings
    gets tiring to type.
*/

    switch(op){

    // R-type
    case 0x0:

        // destination redgister from instruction[15-11]
        controls->RegDst = 0x1;

        // for R-Type, ALUOp is 7
        controls->ALUOp = 0x7;

        // result must be written, so asserted
        controls->RegWrite = 0x1;
    break;


    // I-Type
    // Add immediate: 0000 1000
    case 0x8:

        controls->RegWrite = 0x1;
        controls->ALUSrc = 0x1;
    break;

    // Load word (lw): 0010 0011
    case 0x23:

        controls->RegWrite = 0x1;
        controls->MemRead = 0x1;
        controls->MemtoReg = 0x1;
        controls->ALUSrc = 0x1;
    break;

    // Store word (sw): 0010 1011
    case 0x2b:

        controls->MemWrite = 0x1;
        controls->RegDst = 0x2;
        controls->MemtoReg = 0x2;
        controls->ALUSrc = 0x1;
    break;

    // Load upper immediate (lui): 0000 1111
    case 0xf:

        controls->RegWrite = 0x1;

        // Requires upper 16 bits; set ALUOp to shift
        controls->ALUOp = 0x6;

        controls->ALUSrc = 0x1;
    break;

    // Branch on equal (beq): 0000 0100
    case 0x4:

        // PC updates for branch to multiplexer path 1
        controls->Branch = 0x1;
        controls->RegDst = 0x2;
        controls->MemtoReg = 0x2;
        controls->ALUSrc = 0x1;

        // Branching requires subtraction
        controls->ALUOp = 0x1;
    break;

    // Set less than immediate (slti): 0000 1010
    case 0xa:

        // Set ALU operation for 'set less than'
        controls->ALUOp = 0x2;

        controls->RegWrite = 0x1;
        controls->ALUSrc = 0x1;
    break;

    // Set less than immediate unsigned (sltiu): 0000 1011
    case 0xb:

        controls->ALUOp = 0x3;
        controls->RegWrite = 0x1;
        controls->ALUSrc = 0x1;
    break;


    // J-Type
    // Jump (j): 0000 0010
    case 0x2:

        controls->Jump = 0x1;
        controls->RegDst = 0x2;
        controls->Branch = 0x2;
        controls->MemtoReg = 0x2;
        controls->ALUSrc = 0x2;
        controls->ALUOp = 0x2;
    break;

    // If none of the cases apply, halt condition occurs, so return 1
    default:
        return 1;
    }
    // No halt condition = successful encoding
    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    // Fetch values of registers r1 and r2 from register array
    // fill in data
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // Check if offset is negative
    // Fill upper half with 1s if constant is negative

    *extended_value = 
        (offset >> 15) == 1 ? (offset | 0xffff0000) : (offset & 0x0000ffff);     
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    // half if improper instruction is passed
    if(ALUOp < 0 || ALUOp > 7){
        return 1;
    }
    
    // convert ALUOp to unsigned int for simplicity
    unsigned ALUCtrl = ALUOp;

    // if R-Type, use funct field to determine instruction
    switch (funct)
    {
        // add
        case 0x20:
            ALUCtrl = 0;
        break;

        // subtract
        case 0x22:
            ALUCtrl = 0;
        break;

        // AND
        case 0x24:
            ALUCtrl = 4;
        break;

        // OR
        case 0x25:
            ALUCtrl = 5;
        break;

        // SLT
        case 0x2a:
            ALUCtrl = 2;
        break;
        
        // SLTu
        case 0x2b:
            ALUCtrl = 3;
        break;

        // halt condition on invalid instruction
        default:
            return 1;
    }

    // if source is asserted, so we use the extended offset and the ALUOp control signal
    unsigned data2extended = (ALUSrc ==  1) ? extended_value : data2;

    // perform operation
    ALU(data1, data2extended, ALUCtrl, ALUresult, Zero);

    // operation successful
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    // ALUresult is a valid address if MemWrite/MemRead is asserted AND
    // the address is divisible by 4

    // check for halt conditions
    if(MemRead == 1 || MemWrite == 1){

        // halt if address is word aligned
        if((ALUresult % 4) != 0) 
            return 1;
    }
    
    // write value of data2 to mem location addressed by ALUresult
    if(MemWrite == 1){
        Mem[ALUresult >> 2] = data2;
    }

    // read mem content addressed by ALUresult to memdata
    if(MemRead == 1){
        *memdata = Mem[ALUresult >> 2];
    }

    return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if(RegWrite == 1){

        // if information came from memory, I-type; RegDst = 0
        if(MemtoReg == 1)
            Reg[r2] = memdata;

        // if information came from a register, determine which
        else if(MemtoReg == 0){

            // R-type instruction; write to r3
            if(RegDst == 1)
                Reg[r3] = ALUresult;

            // I-type instruction; write to r2
            else
                Reg[r2] = ALUresult;
        }
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // increment progam counter by 4
    *PC += 4;

    // if branch was taken AND zero signal from ALU, apply offset to PC
    if(Branch == 1 && Zero == 1)
    {
        *PC += (extended_value << 2);
    }

    // if we have a jump instruction, use the top 4 bits of the old PC and shift bits for jump left by 2
    if(Jump == 1)
    {
        // PC = PC + 4[top 4 bits]
        *PC = *PC & 0xf0000000 | (jsec << 2);
    }
}

