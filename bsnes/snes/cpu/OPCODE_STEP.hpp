#ifndef OPCODE_STEP_H
#define OPCODE_STEP_H

#include <stdint.h>

struct OPCODE_STEP{
     int32_t program_counter;
     int32_t converted_counter;
     int32_t accessed_address;
     int32_t converted_address;
     int32_t indirect_address;
     int32_t converted_indirect;
     int32_t value_held;
     int32_t operand;
     uint16_t A,X,Y,S,DP;
     bool flagX;
     bool flagM;
     uint8_t opcode;
     
     OPCODE_STEP();
     void initialize(uint32_t pc,uint32_t cc,uint16_t a,uint16_t x,uint16_t y,
          uint16_t s, uint16_t dp,bool xf,bool mf,uint8_t op);
};
#endif
