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
        *ALUresult = ((int)A - (int)B) ? 1 : 0;
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
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // sectioning off specific ranges of bits

    // instruction[31-26]

    
    *op = (instruction & 0xfc000000) >> 26

    // instruction[25-21]

    // instruction[20-16]

    // instruction[15-11]

    // instruction[5-0]

    // instruction[15-0]

    // instruction[25-0]
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{

}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

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

        int temp = *PC & 0xf0000000;

        *PC = temp | (jsec << 2);
    }
}

