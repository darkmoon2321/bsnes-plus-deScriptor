#ifndef OPCODE_STEP_CPP
#define OPCODE_STEP_CPP

#include "OPCODE_STEP.hpp"

OPCODE_STEP::OPCODE_STEP()
{
     program_counter=-1;
     converted_counter=-1;
     accessed_address=-1;
     converted_address=-1;
     indirect_address=-1;
     converted_indirect=-1;
     value_held=-1;
     operand=-1;
}
void OPCODE_STEP::initialize(uint32_t pc,uint32_t cc,uint16_t a,uint16_t x,uint16_t y,
                             uint16_t s, uint16_t dp,bool xf,bool mf,uint8_t op)
{
     program_counter=pc;
     converted_counter=cc;
     A=a;
     X=x;
     Y=y;
     S=s;
     DP=dp;
     flagX=xf;
     flagM=mf;
     accessed_address=-1;
     converted_address=-1;
     indirect_address=-1;
     converted_indirect=-1;
     value_held=-1;
}
#endif
