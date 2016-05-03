#ifndef BRANCH_H
#define BRANCH_H

struct BRANCH{
     int32_t RAM_sources[0x20000];
     int32_t Ah_source,Xh_source,Yh_source;
     int32_t Al_source,Xl_source,Yl_source;
     SNES::MappedRAM * cartrom;
     void * vsprom;
     SNES::StaticRAM * wram;
     
     BRANCH();
     void Move(int32_t,int32_t);
     int32_t convertPosition(int32_t);
     int32_t isRAM(int32_t);
     bool isROM(int32_t);
     void STA(int32_t,bool);
     void STX(int32_t,bool);
     void STY(int32_t,bool);
     void setAddressInvalid(int32_t,bool);
     void LDA(int32_t,bool);
     void LDX(int32_t,bool);
     void LDY(int32_t,bool);
     void XBA();
     void TAX(bool);
     void TXA(bool);
     void TAY(bool);
     void TYA(bool);
     void TXY(bool);
     void TYX(bool);
};
#endif
