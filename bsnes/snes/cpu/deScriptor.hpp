#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <string>
#include <fstream>

typedef std::string d_string;

#include "BRANCH.hpp"
#include "OPCODE_STEP.hpp"
#include "ROM_DATA.hpp"


enum byteFlags{
     SCRIPT      =0X01,
     SNES65C816  =0X02,
     OPERAND     =0X04,
     NOTJUMP     =0x08
};

struct SCRIPT_COMMAND{
     uint16_t value;
     d_string description;
     int32_t * code;     //code[0] is the destination address of the script jump
     
     SCRIPT_COMMAND();
};

struct SCRIPT_JUMP{
     int32_t address;
     d_string description;
     bool bit16;
     SCRIPT_COMMAND * commands;
     
     SCRIPT_JUMP();
};


class deScriptor{
private:
    BRANCH source_tracker;
    ROM_DATA * ROM_data;
    d_string file_base;
    long file_size;
    int32_t temp_int;
    int32_t temp_int2;
    bool display_error;
    d_string error_message;
    uint16_t &regsA;
    uint16_t &regsX;
    uint16_t &regsY;
    uint16_t &Stack;
    uint16_t &regsDP;
    bool &MSET;
    bool &XSET;
    uint8_t &regsDB;
    SCRIPT_JUMP * scripts;
    uint16_t num_scripts;
    OPCODE_STEP step;
    d_string asm_strings[0x100];
    
    void initializeASMDescriptions();
    void recordScript();
    uint16_t busRead16(uint32_t);
    uint32_t busRead24(uint32_t);
    uint32_t dpIndexedIndirectX(uint32_t position,bool SET);
    uint32_t stackRelative(uint32_t position,bool SET);
    uint32_t dp(uint32_t position,bool SET);
    uint32_t dpIndirectLong(uint32_t position,bool SET);
    uint32_t absolute(uint32_t position,bool SET);
    uint32_t absoluteLong(uint32_t position,bool SET);
    uint32_t dpIndirectIndexedY(uint32_t position,bool SET);
    uint32_t dpIndirect(uint32_t position,bool SET);
    uint32_t dpIndirectPEI(uint32_t);
    uint32_t srIndirectIndexedY(uint32_t position,bool SET);
    uint32_t dpIndexedX(uint32_t position,bool SET);
    uint32_t dpIndirectLongIndexedY(uint32_t position,bool SET);
    uint32_t absoluteIndexedY(uint32_t position,bool SET);
    uint32_t absoluteIndexedX(uint32_t position,bool SET);
    uint32_t absoluteLongIndexedX(uint32_t position,bool SET);
    uint32_t dpIndexedY(uint32_t position,bool SET);
    uint32_t absoluteIndexedIndirect(uint32_t position);
    uint32_t absoluteIndirect(uint32_t position);
    uint32_t absoluteIndirectLong(uint32_t position);
    void addScriptJump(uint32_t,bool);
    void addEmptyScriptJump(uint32_t);
    void increaseScriptSize(uint16_t,bool);
    
public:
    //deScriptor();
    deScriptor(uint16_t &,uint16_t &,uint16_t &,uint16_t &,uint16_t &,uint8_t &,bool &,bool &);
    bool error();
    const char * getMessage();
    bool loadData(nall::string b_name,uint8_t * data_start,const long f_size,SNES::MappedRAM * cartrom, void * vsprom,
        SNES::StaticRAM * wram);
    bool unloadData();
    void loadState(uint8_t slot);
    void saveState(uint8_t slot);
    void recordASM(uint32_t);
    char convertToASCII(char c);
    d_string convertByteToHexString(uint8_t);
    d_string convertWordToHexString(uint16_t);
    d_string convert24BitToHexString(uint32_t);
    d_string convert32BitToHexString(uint32_t);
    d_string convert64BitToHexString(uint64_t);
    
};

#endif
