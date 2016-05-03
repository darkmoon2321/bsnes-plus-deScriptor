#ifndef BRANCH_CPP
#define BRANCH_CPP

#include "BRANCH.hpp"

BRANCH::BRANCH()
{
    for(int i=0;i<0x20000;i++)
    {
         RAM_sources[i]=-1;
    }
    Ah_source=-1;
    Al_source=-1;
    Xh_source=-1;
    Xl_source=-1;
    Yh_source=-1;
    Yl_source=-1;
}
void BRANCH::Move(int32_t source,int32_t destination)
{
    destination=isRAM(destination);
    if(destination<0) return;
    
    RAM_sources[destination]=convertPosition(source);
}
int32_t BRANCH::convertPosition(int32_t pos)
{
    if(pos<0) return pos;
    if(pos<0x7E0000 || pos>=0x800000)
    {
         SNES::Bus::Page &page = SNES::bus.page[pos >> 8];
         if (page.access == cartrom || page.access == vsprom)
         {
             return page.offset + pos;
         }
         else if(page.access == wram)
         {
              pos&=0x1FFF;
              return RAM_sources[pos];
         }
         else
         {
             return -1;
         }
    }
    pos&=0x01FFFF;
    return RAM_sources[pos];
}
int32_t BRANCH::isRAM(int32_t pos)
{
    if(pos<0) return pos;
    if(pos<0x7E0000 || pos>=0x800000)
    {
         SNES::Bus::Page &page = SNES::bus.page[pos >> 8];
         if (page.access == wram)
         {
              pos&=0x01FFF;
              return pos;
         }
         else
         {
             return -1;
         }
    }
    pos&=0x1FFFF;
    return pos;
}
bool BRANCH::isROM(int32_t pos)
{
    if(pos<0) return pos;
    SNES::Bus::Page &page = SNES::bus.page[pos >> 8];
    if (page.access == cartrom || page.access == vsprom)
    {
         return true;
    }
    return false;
}
void BRANCH::STA(int32_t address,bool MSET)
{
    address=isRAM(address);
    if(address<0) return;
    
    RAM_sources[address]=Al_source;
    if(!MSET) RAM_sources[address+1]=Ah_source;
}
void BRANCH::STX(int32_t address,bool XSET)
{
    address=isRAM(address);
    if(address<0) return;
    
    RAM_sources[address]=Xl_source;
    if(!XSET) RAM_sources[address+1]=Xh_source;
}
void BRANCH::STY(int32_t address,bool XSET)
{
    address=isRAM(address);
    if(address<0) return;
    
    RAM_sources[address]=Yl_source;
    if(!XSET) RAM_sources[address+1]=Yh_source;
}
void BRANCH::setAddressInvalid(int32_t address,bool SET)
{
    address=isRAM(address);
    if(address<0) return;
    
    RAM_sources[address]=-1;
    if(!SET) RAM_sources[address+1]=-1;
}
void BRANCH::LDA(int32_t address,bool MSET)
{
    Al_source=convertPosition(address);
    if(!MSET) Ah_source=convertPosition(address+1);
}
void BRANCH::LDX(int32_t address,bool XSET)
{
    Xl_source=convertPosition(address);
    if(!XSET) Xh_source=convertPosition(address+1);
}
void BRANCH::LDY(int32_t address,bool XSET)
{
    Yl_source=convertPosition(address);
    if(!XSET) Yh_source=convertPosition(address+1);
}
void BRANCH::XBA()
{
    uint32_t temp_source;
    
    temp_source=Ah_source;
    Ah_source=Al_source;
    Al_source=temp_source;
}
void BRANCH::TAX(bool XSET)
{
    Xl_source=Al_source;
    if(!XSET) Xh_source=Ah_source;
}
void BRANCH::TAY(bool XSET)
{
    Yl_source=Al_source;
    if(!XSET) Yh_source=Ah_source;
}
void BRANCH::TXY(bool XSET)
{
    Yl_source=Xl_source;
    if(!XSET) Yh_source=Xh_source;
}
void BRANCH::TYX(bool XSET)
{
    Xl_source=Yl_source;
    if(!XSET) Xh_source=Yh_source;
}
void BRANCH::TXA(bool MSET)
{
    Al_source=Xl_source;
    if(!MSET) Ah_source=Xh_source;
}
void BRANCH::TYA(bool MSET)
{
    Al_source=Yl_source;
    if(!MSET) Ah_source=Yh_source;
}
#endif
