#ifndef DESCRIPTOR_CPP
#define DESCRIPTOR_CPP

#include "deScriptor.hpp"

#define busRead8 SNES::bus.read
#define TEXT_WIDTH 80
#define REG16BIT   00
#define REG8BIT    01
#define STEPS_MAX 100

SCRIPT_COMMAND::SCRIPT_COMMAND()
{
     value=0;
     description="";
     code=NULL;
}

SCRIPT_JUMP::SCRIPT_JUMP()
{
     commands=NULL;
     description="";
     address=-1;
     bit16=false;
}

deScriptor::deScriptor(uint16_t &_regsA,uint16_t &_regsX,uint16_t &_regsY,uint16_t &_Stack,uint16_t &_regsDP,uint8_t &_regsDB,bool &_MSET,bool &_XSET) :
                    regsA(_regsA),regsX(_regsX),regsY(_regsY),Stack(_Stack),regsDP(_regsDP),regsDB(_regsDB),MSET(_MSET),XSET(_XSET)
{
     ROM_data=NULL;
     file_base="";
     file_size=0;
     display_error=false;
     error_message="";
     scripts=NULL;
     num_scripts=0;
}
    
bool deScriptor::error()
{
     return display_error;
}

const char * deScriptor::getMessage()
{
      return error_message.c_str();
}
bool deScriptor::loadData(nall::string b_name,uint8_t * data_start,const long f_size,SNES::MappedRAM * cartrom, void * vsprom,
                           SNES::StaticRAM * wram)
{
    long i;
    
    if(!data_start) return false;
    file_base=b_name();
    file_size=f_size;
    ROM_data=new ROM_DATA[f_size];
    if(!ROM_data) return false;
    for(i=0;i<file_size;i++)
    {
         ROM_data[i].ROM_bytes=data_start[i];
    }
    source_tracker.cartrom=cartrom;
    source_tracker.vsprom=vsprom;
    source_tracker.wram=wram;
    
    /*d_string f_name=file_base+"dsc.bin";
    d_string f_name2=file_base+"dsc.txt";
    d_string buffer;
    streampos line_start;
    std::ifstream binary_in(f_name.c_str(),std::ios::binary);
    std::ifstream text_in(f_name2.c_str());
    if(binary_in && text_in)
    {
         for(i=0;i<file_size;i++)
         {
              binary_in.read((char *)ROM_data[i].flags,sizeof(uint8_t));
              binary_in.read((char *)ROM_data[i].SNES_address,sizeof(uint32_t));
              if(ROM_data[i].flags&SCRIPT || ROM_data[i].flags&SNES65C816)
              {
                   getline(text_in,buffer);
                   ROM_data[i].description=buffer.substr(29,TEXTWIDTH-29);
                   do
                   {
                        line_start=text_in.tellg();
                        getline(text_in,buffer);
                        if(buffer[0]==' ')
                        {
                             ROM_data[i].description+=buffer.substr(29,TEXTWIDTH-29);
                        }
                        else
                        {
                             text_in.seekg(line_start);
                        }
                   }while(buffer[0]==' ');
              }
         }
    }
    binary_in.close();
    text_in.close();
    
    */
    return true;
}

bool deScriptor::unloadData()
{
    long i,j;
    d_string f_name=file_base;
    f_name+="-dsc.txt";
    std::ofstream outfile;
    outfile.open(f_name.c_str());
    /*std::ofstream outfile2;
    f_name=file_base;
    f_name+="-dsc.bin";
    outfile2.open(f_name.c_str(),std::ios::binary);*/
    
    //Print results of disassembly to file    
    outfile<<"Hex     SNES    Bytes        Description";
    for(temp_int=40;temp_int<TEXT_WIDTH;temp_int++) outfile<<' ';
    outfile<<"|Frequency       |Labels"<<std::endl;
    for(i=0;i<(file_size);i++)
    {
         j=i;
         //outfile2.write((char *)ROM_data[i].flags,sizeof(uint8_t));
         //outfile2.write((char *)ROM_data[i].SNES_address,sizeof(uint32_t));
         if(!(ROM_data[i].flags&SCRIPT) && !(ROM_data[i].flags&SNES65C816))
         {
             continue;
         }
         outfile<<convert24BitToHexString(i)<<"  ";
         if(ROM_data[i].flags&SCRIPT)
         {
              if(ROM_data[i].step.indirect_address>=0)
              {
                   outfile<<convert24BitToHexString(ROM_data[i].step.indirect_address)<<"  ";
              }
              else
              {
                   outfile<<convert24BitToHexString(ROM_data[i].step.accessed_address)<<"  ";
              }
         }
         if(ROM_data[i].flags&SNES65C816)
         {
              outfile<<convert24BitToHexString(ROM_data[i].step.program_counter)<<"  ";
         }
         if(ROM_data[i].flags&SCRIPT || ROM_data[i].flags&SNES65C816)
         {
             outfile<<convertByteToHexString(ROM_data[i].ROM_bytes);
             outfile<<' ';
             if((i+1)<(file_size) && ROM_data[i+1].flags&OPERAND)
             {
                  outfile<<convertByteToHexString(ROM_data[i+1].ROM_bytes);
                  outfile<<' ';
                  j++;
                  if((i+2)<(file_size) && ROM_data[i+2].flags&OPERAND)
                  {
                       outfile<<convertByteToHexString(ROM_data[i+2].ROM_bytes);
                       outfile<<' ';
                       j++;
                       if((i+3)<(file_size) && ROM_data[i+3].flags&OPERAND)
                       {
                            outfile<<convertByteToHexString(ROM_data[i+3].ROM_bytes);
                            outfile<<"  ";
                            j++;
                       }
                       else
                       {
                            outfile<<"    ";
                       }
                  }
                  else
                  {
                       outfile<<"       ";
                  }
             }
             else
             {
                  outfile<<"          ";
             }
             if((ROM_data[i].description).length() > (TEXT_WIDTH-29))
             {
                  outfile<<(ROM_data[i].description).substr(0,TEXT_WIDTH-29);
                  temp_int=TEXT_WIDTH-29;
                  outfile<<"|";
                  outfile<<convert64BitToHexString(ROM_data[i].frequency);
                  outfile<<"|";
                  for(temp_int2=0;temp_int2<ROM_data[i].num_labels;temp_int2++)
                  {
                       outfile<<"  0x";
                       outfile<<convert24BitToHexString(ROM_data[i].labels[temp_int2]);
                  }
                  outfile<<std::endl;
                  while(temp_int<ROM_data[i].description.length())
                  {
                       outfile<<"                                     ";
                       outfile<<(ROM_data[i].description).substr(temp_int,TEXT_WIDTH-29);
                       outfile<<std::endl;
                       temp_int+=(TEXT_WIDTH-29);
                  }
             }
             else
             {
                  outfile<<ROM_data[i].description;
                  for(temp_int=ROM_data[i].description.length();temp_int<(TEXT_WIDTH-29);temp_int++)
                  {
                       outfile<<' ';
                  }
                  outfile<<"|";
                  outfile<<convert64BitToHexString(ROM_data[i].frequency);
                  outfile<<"|";
                  for(temp_int2=0;temp_int2<ROM_data[i].num_labels;temp_int2++)
                  {
                       outfile<<"0x";
                       outfile<<convert24BitToHexString(ROM_data[i].labels[temp_int2]);
                       outfile<<"  ";
                  }
                  outfile<<std::endl;
             }
         }
         i=j;
    }
    
    return true;
}

void deScriptor::loadState(uint8_t slot)
{
     d_string file_name=file_base;
     file_name+=convertToASCII(slot);
     file_name+=".dst";
     std::ifstream infile(file_name.c_str());
     
     for(int i=0;i<0x20000;i++)
     {
          infile.read((char *)(&source_tracker.RAM_sources[i]),sizeof(int32_t));
     }
}

void deScriptor::saveState(uint8_t slot)
{
     d_string file_name=file_base;
     file_name+=convertToASCII(slot);
     file_name+=".dst";
     std::ofstream outfile(file_name.c_str());
     
     for(int i=0;i<0x20000;i++)
     {
          outfile.write((char *)(&source_tracker.RAM_sources[i]),sizeof(int32_t));
     }
}

void deScriptor::recordScript()
{ 
     if(step.converted_counter<0) return;
     
     int16_t index=ROM_data[step.converted_counter].script_index;
     bool Bit16Ops=false;
     
     if(index<0)
     {
          if(step.opcode==0x6C || step.opcode==0xDC)
          {
               if(source_tracker.isRAM(step.accessed_address)<0) ROM_data[step.converted_counter].flags|=NOTJUMP;
          }
          //Add the label! Even if not truly variable jump, need to add the only label.
          if(ROM_data[step.converted_counter].flags&NOTJUMP)
          {
               if(step.converted_indirect>=0) ROM_data[step.converted_indirect].addLabel(step.converted_counter);
          }
          else
          {
               if(step.opcode==0x6C || step.opcode==0xDC)
               {
                    temp_int=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address)];
                    temp_int2=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address+1)];
                    Bit16Ops=(temp_int2>=0 && temp_int2!=temp_int);
                    if(temp_int<0)
                    {
                         addEmptyScriptJump(step.converted_counter);
                         index=num_scripts-1;
                         ROM_data[step.converted_counter].script_index=index;
                    }
               }
               else if(step.opcode==0x60 || step.opcode==0x6B)
               {
                    temp_int=step.converted_address;
                    temp_int2=source_tracker.convertPosition(Stack+2);
                    Bit16Ops=(temp_int2>=0 && temp_int2!=temp_int);//is this right?
                    if(temp_int<0)
                    {
                         addEmptyScriptJump(step.converted_counter);
                         index=num_scripts-1;
                         ROM_data[step.converted_counter].script_index=index;
                    }
               }
               else
               {
                    if(source_tracker.Xl_source<0)
                    {
                         temp_int=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address)];
                         temp_int2=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address+1)];
                         Bit16Ops=(temp_int2>=0 && temp_int2!=temp_int);
                         if(temp_int<0)
                         {
                              addEmptyScriptJump(step.converted_counter);
                              index=num_scripts-1;
                              ROM_data[step.converted_counter].script_index=index;
                         }
                    }
                    else
                    {
                         Bit16Ops=((source_tracker.Xh_source>=0) && (source_tracker.Xh_source!=source_tracker.Xl_source));
                    }
               }
               if(index<0)
               {
                    addScriptJump(step.converted_counter,Bit16Ops);
                    index=num_scripts-1;
                    ROM_data[step.converted_counter].script_index=index;
               }
          }
          if(index<0) return;
     }
     
     int32_t converted_position=-1;
     switch(step.opcode)
     {
          case 0x7C:
          case 0xFC:
               converted_position=source_tracker.Xl_source;
               if(converted_position<0)
               {
                    temp_int=source_tracker.isRAM(step.accessed_address);
                    if(temp_int<0) return;
                    converted_position=source_tracker.RAM_sources[temp_int];
                    if(converted_position<0) return;
                    temp_int2=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address+1)];
                    Bit16Ops=(temp_int2>=0 && temp_int2!=temp_int);
                    break;
               }
               Bit16Ops=((source_tracker.Xh_source>=0) && (source_tracker.Xh_source!=source_tracker.Xl_source));
               break;
          case 0x6C:
          case 0xDC:
               converted_position=step.converted_address;
               if(converted_position<0) return;
               temp_int=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address)];
               temp_int2=source_tracker.RAM_sources[source_tracker.isRAM(step.accessed_address+1)];
               Bit16Ops=(temp_int2>=0 && temp_int2!=temp_int);
               break;
          case 0x60:
          case 0x6B:
               temp_int=source_tracker.isRAM(step.accessed_address);
               if(temp_int<0) return;
               converted_position=source_tracker.RAM_sources[temp_int];
               if(converted_position<0) return;
               temp_int2=source_tracker.convertPosition(Stack+2);
               Bit16Ops=(temp_int2>=0 && temp_int2!=temp_int);//is this right?
               break;
     }
     if(converted_position<0) return;
     ROM_data[converted_position].frequency++;
     if(ROM_data[converted_position].flags) return; //don't record the same position twice.
     ROM_data[converted_position].flags|=SCRIPT;
     
     if(scripts[index].commands==NULL) increaseScriptSize(index,Bit16Ops);
     if(Bit16Ops && !scripts[index].bit16) increaseScriptSize(index,Bit16Ops);
     
     uint16_t command_index=ROM_data[converted_position].ROM_bytes;
     if(Bit16Ops) command_index+=ROM_data[converted_position+1].ROM_bytes<<8;
     if(scripts[index].commands[command_index].code==NULL)
     {
          //define a new command
          scripts[index].commands[command_index].code=new int32_t[1];
          scripts[index].commands[command_index].code[0]=step.converted_indirect;
          scripts[index].commands[command_index].value=command_index;
          //****Load key file information for the command, if available
          //scripts[index].commands[command_index].description= ****keyfile description
          
          
          //If not available:
          scripts[index].commands[command_index].description="Script ";
          scripts[index].commands[command_index].description+=convert24BitToHexString(scripts[index].address);
          scripts[index].commands[command_index].description+=", Command ";
          scripts[index].commands[command_index].description+= (Bit16Ops) ? convertWordToHexString(command_index) :
               convertByteToHexString(command_index);
     }
     
     ROM_data[converted_position].description=scripts[index].commands[command_index].description;
} //end of recordScript function    

void deScriptor::recordASM(uint32_t position)
{  
   int32_t converted_position=source_tracker.convertPosition(position);
   step.initialize(position,converted_position,regsA,regsX,regsY,Stack,regsDP,XSET,MSET,SNES::bus.read(position));
   step.operand=busRead24(position+1);
   step.opcode=busRead8(position);
   
   switch(step.opcode)
   {
   //Move and Transfer opcodes
        case 0x44:
             {
                  temp_int= busRead8(position+2);
                  int32_t dataSource = temp_int<<16;
                  temp_int= busRead8(position+1);
                  int32_t dataDest = temp_int<<16;
                  
                  dataSource+=(regsX-regsA)&0xffff;
                  dataDest+=(regsY-regsA)&0xffff;
                  
                  source_tracker.Move(dataSource,dataDest);
                  step.accessed_address=dataSource;
                  step.converted_address=source_tracker.convertPosition(dataSource);
                  step.indirect_address=dataDest;
                  step.converted_indirect=step.converted_address;
                  if(source_tracker.isROM(dataSource) && step.converted_address>0) ROM_data[step.converted_address].step=step;
                  break;     
             }
        case 0x54:
             {
                  temp_int= busRead8(position+2);
                  int32_t dataSource = temp_int<<16;
                  temp_int= busRead8(position+1);
                  int32_t dataDest = temp_int<<16;
                  
                  dataSource+=regsX;
                  dataDest+=regsY;
                  
                  source_tracker.Move(dataSource,dataDest);
                  step.accessed_address=dataSource;
                  step.converted_address=source_tracker.convertPosition(dataSource);
                  step.indirect_address=dataDest;
                  step.converted_indirect=step.converted_address;
                  if(source_tracker.isROM(dataSource) && step.converted_address>0) ROM_data[step.converted_address].step=step;
                  break;     
             }
        case 0xAA: //TAX
             source_tracker.TAX(XSET);
             break;
        case 0xA8: //TAY
             source_tracker.TAY(XSET);
             break;
        case 0x8A: //TXA
             source_tracker.TXA(MSET);
             break;
        case 0x98: //TYA
             source_tracker.TYA(MSET);
             break;
        case 0x9B: //TXY
             source_tracker.TXY(XSET);
             break;
        case 0xBB: //TYX
             source_tracker.TYX(XSET);
             break;
        case 0x3B: //TSC
        case 0x7B: //TDC
             source_tracker.Al_source=-1;
             source_tracker.Ah_source=-1;
             break;
        case 0xBA: //TSX
             source_tracker.Xl_source=-1;
             if(!XSET) source_tracker.Xh_source=-1;
             break;
   //Bitwise AND constant
        case 0x29:
             temp_int=step.operand&0xff;
             if(!temp_int) source_tracker.Al_source=-1;
             temp_int=(step.operand>>8)&0xff;
             if(!temp_int) source_tracker.Ah_source=-1;
   //Push opcodes
        case 0xF4: //PEA
        case 0x62: //PER
        case 0x0D: //PHD
             source_tracker.setAddressInvalid(Stack-1,REG16BIT);
             break;
        case 0x4B: //PHK
        case 0x8B: //PHB
        case 0x08: //PHP
             source_tracker.setAddressInvalid(Stack,REG8BIT);
             break;
        case 0xD4: //PEI
             temp_int=dpIndirectPEI(position+1);
             source_tracker.RAM_sources[Stack]=source_tracker.RAM_sources[temp_int+1];
             source_tracker.RAM_sources[Stack-1]=source_tracker.RAM_sources[temp_int];
             break;
        case 0x48: //PHA
             if(MSET)
             {
                  source_tracker.RAM_sources[Stack]=source_tracker.Al_source;
             }
             else
             {
                  source_tracker.RAM_sources[Stack]=source_tracker.Ah_source;
                  source_tracker.RAM_sources[Stack-1]=source_tracker.Al_source;
             }
             break;
        case 0xDA: //PHX
             if(XSET)
             {
                  source_tracker.RAM_sources[Stack]=source_tracker.Xl_source;
             }
             else
             {
                  source_tracker.RAM_sources[Stack]=source_tracker.Xh_source;
                  source_tracker.RAM_sources[Stack-1]=source_tracker.Xl_source;
             }
             break;
        case 0x5A: //PHY
             if(XSET)
             {
                  source_tracker.RAM_sources[Stack]=source_tracker.Yl_source;
             }
             else
             {
                  source_tracker.RAM_sources[Stack]=source_tracker.Yh_source;
                  source_tracker.RAM_sources[Stack-1]=source_tracker.Yl_source;
             }
             break;
   //Non-Variable JSR/JSL
        case 0x20:
             source_tracker.RAM_sources[Stack]=-1;
             source_tracker.RAM_sources[Stack-1]=-1;
             break;
        case 0x22:
             source_tracker.RAM_sources[Stack]=-1;
             source_tracker.RAM_sources[Stack-1]=-1;
             source_tracker.RAM_sources[Stack-2]=-1;
             break;
   //Pull opcodes
        case 0x68: //PLA
             if(MSET)
             {
                  source_tracker.Al_source=source_tracker.RAM_sources[Stack+1];
             }
             else
             {
                  source_tracker.Al_source=source_tracker.RAM_sources[Stack+1];
                  source_tracker.Ah_source=source_tracker.RAM_sources[Stack+2];
             }
             break;
        case 0xFA: //PLX
             if(XSET)
             {
                  source_tracker.Xl_source=source_tracker.RAM_sources[Stack+1];
             }
             else
             {
                  source_tracker.Xl_source=source_tracker.RAM_sources[Stack+1];
                  source_tracker.Xh_source=source_tracker.RAM_sources[Stack+2];
             }
             break;
        case 0x7A: //PLY
             if(XSET)
             {
                  source_tracker.Yl_source=source_tracker.RAM_sources[Stack+1];
             }
             else
             {
                  source_tracker.Yl_source=source_tracker.RAM_sources[Stack+1];
                  source_tracker.Yh_source=source_tracker.RAM_sources[Stack+2];
             }
             break;
   //STZ opcodes
        case 0x64:
             temp_int=dp(position+1,MSET);
             source_tracker.setAddressInvalid(temp_int,MSET);
             break;
        case 0x74:
             temp_int=dpIndexedX(position+1,MSET);
             source_tracker.setAddressInvalid(temp_int,MSET);
             break;
        case 0x9C:
             temp_int=absolute(position+1,MSET);
             source_tracker.setAddressInvalid(temp_int,MSET);
             break;
        case 0x9E:
             temp_int=absoluteIndexedX(position+1,MSET);
             source_tracker.setAddressInvalid(temp_int,MSET);
             break;
   //STA opcodes
        case 0x81:
             temp_int=dpIndexedIndirectX(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x83:
             temp_int=stackRelative(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x85:
             temp_int=dp(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x87:
             temp_int=dpIndirectLong(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x8D:
             temp_int=absolute(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x8F:
             temp_int=absoluteLong(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x91:
             temp_int=dpIndirectIndexedY(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x92:
             temp_int=dpIndirect(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x93:
             temp_int=srIndirectIndexedY(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x95:
             temp_int=dpIndexedX(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x97:
             temp_int=dpIndirectLongIndexedY(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x99:
             temp_int=absoluteIndexedY(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x9D:
             temp_int=absoluteIndexedX(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
        case 0x9F:
             temp_int=absoluteLongIndexedX(position+1,MSET);
             source_tracker.STA(temp_int,MSET);
             break;
   //STX opcodes
        case 0x86:
             temp_int=dp(position+1,XSET);
             source_tracker.STX(temp_int,XSET);
             break;
        case 0x8E:
             temp_int=absolute(position+1,XSET);
             source_tracker.STX(temp_int,XSET);
             break;
        case 0x96:
             temp_int=dpIndexedY(position+1,XSET);
             source_tracker.STX(temp_int,XSET);
             break;
   //STY opcodes
        case 0x84:
             temp_int=dp(position+1,XSET);
             source_tracker.STY(temp_int,XSET);
             break;
        case 0x8C:
             temp_int=absolute(position+1,XSET);
             source_tracker.STY(temp_int,XSET);
             break;
        case 0x94:
             temp_int=dpIndexedY(position+1,XSET);
             source_tracker.STY(temp_int,XSET);
             break;
   //LDA opcodes
        case 0xA1:
             temp_int=dpIndexedIndirectX(position+1,MSET);
             if(source_tracker.Xl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Xl_source;
                  source_tracker.Ah_source=(source_tracker.Xh_source<0) ? source_tracker.Xl_source : source_tracker.Xh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_indirect>0) ROM_data[step.converted_indirect].step=step;
             break;
        case 0xA3:
             temp_int=stackRelative(position+1,MSET);
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xA5:
             temp_int=dp(position+1,MSET);
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xA7:
             temp_int=dpIndirectLong(position+1,MSET);
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_indirect>0) ROM_data[step.converted_indirect].step=step;
             break;
        case 0xA9:
             source_tracker.Al_source=-1;
             if(!MSET) source_tracker.Ah_source=-1;
             break;
        case 0xAD:
             temp_int=absolute(position+1,MSET);
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xAF:
             temp_int=absoluteLong(position+1,MSET);
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xB1:
             temp_int=dpIndirectIndexedY(position+1,MSET);
             if(source_tracker.Yl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Yl_source;
                  source_tracker.Ah_source=(source_tracker.Yh_source<0) ? source_tracker.Yl_source : source_tracker.Yh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_indirect>0) ROM_data[step.converted_indirect].step=step;
             break;
        case 0xB2:
             temp_int=dpIndirect(position+1,MSET);
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_indirect>0) ROM_data[step.converted_indirect].step=step;
             break;
        case 0xB3:
             temp_int=srIndirectIndexedY(position+1,MSET);
             if(source_tracker.Yl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Yl_source;
                  source_tracker.Ah_source=(source_tracker.Yh_source<0) ? source_tracker.Yl_source : source_tracker.Yh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_indirect>0) ROM_data[step.converted_indirect].step=step;
             break;
        case 0xB5:
             temp_int=dpIndexedX(position+1,MSET);
             if(source_tracker.Xl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Xl_source;
                  source_tracker.Ah_source=(source_tracker.Xh_source<0) ? source_tracker.Xl_source : source_tracker.Xh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xB7:
             temp_int=dpIndirectLongIndexedY(position+1,MSET);
             if(source_tracker.Yl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Yl_source;
                  source_tracker.Ah_source=(source_tracker.Yh_source<0) ? source_tracker.Yl_source : source_tracker.Yh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_indirect>0) ROM_data[step.converted_indirect].step=step;
             break;
        case 0xB9:
             temp_int=absoluteIndexedY(position+1,MSET);
             if(source_tracker.Yl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Yl_source;
                  source_tracker.Ah_source=(source_tracker.Yh_source<0) ? source_tracker.Yl_source : source_tracker.Yh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xBD:
             temp_int=absoluteIndexedX(position+1,MSET);
             if(source_tracker.Xl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Xl_source;
                  source_tracker.Ah_source=(source_tracker.Xh_source<0) ? source_tracker.Xl_source : source_tracker.Xh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xBF:
             temp_int=absoluteLongIndexedX(position+1,MSET);
             if(source_tracker.Xl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Xl_source;
                  source_tracker.Ah_source=(source_tracker.Xh_source<0) ? source_tracker.Xl_source : source_tracker.Xh_source;
                  break;
             }
             source_tracker.LDA(temp_int,MSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        //LDX opcodes
        case 0xA2:
             source_tracker.Xl_source=-1;
             if(!XSET) source_tracker.Xh_source=-1;
             break;
        case 0xA6:
             temp_int=dp(position+1,XSET);
             source_tracker.LDX(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xAE:
             temp_int=absolute(position+1,XSET);
             source_tracker.LDX(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xB6:
             temp_int=dpIndexedY(position+1,XSET);
             if(source_tracker.Yl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Yl_source;
                  source_tracker.Ah_source=(source_tracker.Yh_source<0) ? source_tracker.Yl_source : source_tracker.Yh_source;
                  break;
             }
             source_tracker.LDX(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xBE:
             temp_int=absoluteIndexedY(position+1,XSET);
             if(source_tracker.Yl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Yl_source;
                  source_tracker.Ah_source=(source_tracker.Yh_source<0) ? source_tracker.Yl_source : source_tracker.Yh_source;
                  break;
             }
             source_tracker.LDX(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        //LDY opcodes
        case 0xA0:
             source_tracker.Yl_source=-1;
             if(!XSET) source_tracker.Yh_source=-1;
             break;
        case 0xA4:
             temp_int=dp(position+1,XSET);
             source_tracker.LDY(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xAC:
             temp_int=absolute(position+1,XSET);
             source_tracker.LDY(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xB4:
             temp_int=dpIndexedX(position+1,XSET);
             if(source_tracker.Xl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Xl_source;
                  source_tracker.Ah_source=(source_tracker.Xh_source<0) ? source_tracker.Xl_source : source_tracker.Xh_source;
                  break;
             }
             source_tracker.LDY(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        case 0xBC:
             temp_int=absoluteIndexedX(position+1,XSET);
             if(source_tracker.Xl_source>=0)
             {
                  source_tracker.Al_source=source_tracker.Xl_source;
                  source_tracker.Ah_source=(source_tracker.Xh_source<0) ? source_tracker.Xl_source : source_tracker.Xh_source;
                  break;
             }
             source_tracker.LDY(temp_int,XSET);
             if(source_tracker.isROM(temp_int) && step.converted_address>0) ROM_data[step.converted_address].step=step;
             break;
        //XBA
        case 0xEB:
             source_tracker.XBA();
             break;
        //Indirect JSR/JMP opcodes
        case 0x6C:
             temp_int=absoluteIndirect(position+1);
             if(ROM_data[converted_position].flags&NOTJUMP) break;
             if(step.converted_indirect>=0)
             {
                  recordScript(); //step should already have the arguments
             }
             if(step.converted_indirect>0 && step.converted_counter>0)
               ROM_data[step.converted_indirect].addMultipleLabel(step.converted_counter);
             break;
        case 0x7C:
             temp_int=absoluteIndexedIndirect(position+1);
             if(step.converted_indirect>=0 || source_tracker.Xl_source>=0)
             {
                  recordScript(); //step should already have the arguments
             }
             if(step.converted_indirect>0 && step.converted_counter>0)
               ROM_data[step.converted_indirect].addMultipleLabel(step.converted_counter);
             break;
        case 0xDC:
             temp_int=absoluteIndirectLong(position+1);
             if(ROM_data[converted_position].flags&NOTJUMP) break;
             if(step.converted_indirect>=0)
             {
                  recordScript(); //step should already have the arguments
             }
             if(step.converted_indirect>0 && step.converted_counter>0)
               ROM_data[step.converted_indirect].addMultipleLabel(step.converted_counter);
             break;
        case 0xFC:
             source_tracker.RAM_sources[Stack]=-1;
             source_tracker.RAM_sources[Stack-1]=-1;
             temp_int=absoluteIndexedIndirect(position+1);
             if(step.converted_indirect>=0 || source_tracker.Xl_source>=0)
             {
                  recordScript(); //step should already have the arguments
             }
             if(step.converted_indirect>0 && step.converted_counter>0)
               ROM_data[step.converted_indirect].addMultipleLabel(step.converted_counter);
             break;
        //RTS and RTL opcodes
        case 0x60: //RTS
             step.accessed_address=Stack+1;
             step.converted_address=source_tracker.convertPosition(Stack+1);
             temp_int=busRead16(Stack+1);
             temp_int+=position&0xff0000;
             step.indirect_address=temp_int+1;
             step.converted_indirect=source_tracker.convertPosition(temp_int+1);
             if(step.converted_indirect>=0 && step.converted_address>=0)
             {
                  recordScript(); //step should already have the arguments
             }
             if(step.converted_indirect>=0 && step.converted_counter>=0) //error_message=convert24BitToHexString(step.indirect_address);
               ROM_data[step.converted_indirect].addMultipleLabel(step.converted_counter);
             break;
        case 0x6B: //RTL
             step.accessed_address=Stack+1;
             step.converted_address=source_tracker.convertPosition(Stack+1);
             temp_int=busRead24(Stack+1);
             step.indirect_address=temp_int+1;
             step.converted_indirect=source_tracker.convertPosition(temp_int+1);
             if(step.converted_indirect>=0 && step.converted_address>=0)
             {
                  recordScript(); //step should already have the arguments
             }
             if(step.converted_indirect>=0 && step.converted_counter>=0) //error_message=convert24BitToHexString(step.indirect_address);
               ROM_data[step.converted_indirect].addMultipleLabel(step.converted_counter);
             break;
   }
   if(converted_position<0) return;
   ROM_data[converted_position].frequency++;
   if(ROM_data[converted_position].flags) return; //only record for positions that haven't been recorded yet.
   ROM_data[converted_position].step=step;
   ROM_data[converted_position].flags|=SNES65C816;
   switch(ROM_data[converted_position].ROM_bytes)
       {
            case 0x00:
                      ROM_data[converted_position].description="BRK $";
                      //we must be able to convert position to converted_position for each operand...
                      temp_int=getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x01:
                      ROM_data[converted_position].description="ORA ($";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0x02:
                      ROM_data[converted_position].description="COP $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
            case 0x03:
                      ROM_data[converted_position].description="ORA $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+=",S";
                      break;
            case 0x04:
                      ROM_data[converted_position].description="TSB $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
            case 0x05:
                      ROM_data[converted_position].description="ORA $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
            case 0x06:
                      ROM_data[converted_position].description="ASL $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
            case 0x07:
                      ROM_data[converted_position].description="ORA [$";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0x08:
                      ROM_data[converted_position].description="PHP";
                      break;
            case 0x09:
                      ROM_data[converted_position].description="ORA #$";
                      if(MSET)
                      {
                           temp_int2= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      
                      }
                      else
                      {
                          temp_int= getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0x0A:
                      ROM_data[converted_position].description="ASL A";
                      break;
            case 0x0B:
                      ROM_data[converted_position].description="PHD";
                      break;
            case 0x0C:
                      ROM_data[converted_position].description="TSB $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x0D:
                      ROM_data[converted_position].description="ORA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x0E:
                      ROM_data[converted_position].description="ASL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x0F:
                      ROM_data[converted_position].description="ORA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x10:
                      ROM_data[converted_position].description="BPL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x11:
                      ROM_data[converted_position].description="ORA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0x12:
                      ROM_data[converted_position].description="ORA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0x13:
                      ROM_data[converted_position].description="ORA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",S),y";
                      break;
            case 0x14:
                      ROM_data[converted_position].description="TRB $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x15:
                      ROM_data[converted_position].description="ORA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x16:
                      ROM_data[converted_position].description="ASL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x17:
                      ROM_data[converted_position].description="ORA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0x18:
                      ROM_data[converted_position].description="CLC";
                      break;
            case 0x19:
                      ROM_data[converted_position].description="ORA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x1A:
                      ROM_data[converted_position].description="INC A";
                      break;
            case 0x1B:
                      ROM_data[converted_position].description="TCS";
                      break;
            case 0x1C:
                      ROM_data[converted_position].description="TRB $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x1D:
                      ROM_data[converted_position].description="ORA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x1E:
                      ROM_data[converted_position].description="ASL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x1F:
                      ROM_data[converted_position].description="ORA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x20:
                      ROM_data[converted_position].description="JSR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int+=(position&0xFF0000);
                      temp_int=source_tracker.convertPosition(temp_int);
                      if(temp_int<0) break;
                      //add branch label
                      ROM_data[temp_int].addLabel(converted_position);
                      break;
            case 0x21:
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0x22:
                      ROM_data[converted_position].description="JSL $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      temp_int<<=16;
                      ROM_data[converted_position].description+=":";
                      temp_int+=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int= source_tracker.convertPosition(temp_int);
                      if(temp_int<0) break;
                      //add branch label
                      ROM_data[temp_int].addLabel(converted_position);
                      break;
            case 0x23:
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",S";
                      break;
            case 0x24:
                      ROM_data[converted_position].description="BIT $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x25:
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x26:
                      ROM_data[converted_position].description="ROL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x27:
                      ROM_data[converted_position].description="AND [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0x28:
                      ROM_data[converted_position].description="PLP";
                      break;
            case 0x29:
                      ROM_data[converted_position].description="AND #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0x2A:
                      ROM_data[converted_position].description="ROL A";
                      break;
            case 0x2B:
                      ROM_data[converted_position].description="PLD";
                      break;
            case 0x2C:
                      ROM_data[converted_position].description="BIT $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x2D:
                      ROM_data[converted_position].description="AND $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x2E:
                      ROM_data[converted_position].description="ROL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x2F:
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x30:
                      ROM_data[converted_position].description="BMI $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x31:
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0x32:
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0x33:
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0x34:
                      ROM_data[converted_position].description="BIT $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x35:
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x36:
                      ROM_data[converted_position].description="ROL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x37:
                      ROM_data[converted_position].description="AND [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0x38:
                      ROM_data[converted_position].description="SEC";
                      break;
            case 0x39:
                      ROM_data[converted_position].description="AND $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x3A:
                      ROM_data[converted_position].description="DEC A";
                      break;
            case 0x3B:
                      ROM_data[converted_position].description="TSC";
                      break;
            case 0x3C:
                      ROM_data[converted_position].description="BIT $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x3D:
                      ROM_data[converted_position].description="AND $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x3E:
                      ROM_data[converted_position].description="ROL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x3F:
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x40:
                      ROM_data[converted_position].description="RTI";
                      break;
            case 0x41:
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0x42:
                      ROM_data[converted_position].description="WDM";
                      break;
            case 0x43:
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
            case 0x44:
                      ROM_data[converted_position].description="MVP ";
                      temp_int= getSourceByte(position+2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" ";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
            case 0x45:
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x46:
                      ROM_data[converted_position].description="LSR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x47:
                      ROM_data[converted_position].description="EOR [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0x48:
                      ROM_data[converted_position].description="PHA";
                      break;
            case 0x49:
                      ROM_data[converted_position].description="EOR #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0x4A:
                      ROM_data[converted_position].description="LSR A";
                      break;
            case 0x4B:
                      ROM_data[converted_position].description="PHK";
                      break;
            case 0x4C:
                      ROM_data[converted_position].description="JMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int+=(position&0xff0000);
                      temp_int=source_tracker.convertPosition(temp_int);
                      //add branch label
                      if(temp_int<0) break;
                      ROM_data[temp_int].addLabel(converted_position);
                      
                      break;
            case 0x4D:
                      ROM_data[converted_position].description="EOR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x4E:
                      ROM_data[converted_position].description="LSR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x4F:
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x50:
                      ROM_data[converted_position].description="BVC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x51:
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0x52:
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0x53:
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0x54:
                      ROM_data[converted_position].description="MVN ";
                      temp_int= getSourceByte(position+2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" ";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x55:
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x56:
                      ROM_data[converted_position].description="LSR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x57:
                      ROM_data[converted_position].description="EOR [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0x58:
                      ROM_data[converted_position].description="CLI";
                      break;
            case 0x59:
                      ROM_data[converted_position].description="EOR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x5A:
                      ROM_data[converted_position].description="PHY";
                      break;
            case 0x5B:
                      ROM_data[converted_position].description="TCD";
                      break;
            case 0x5C:
                      ROM_data[converted_position].description="JML $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      temp_int<<=16;
                      ROM_data[converted_position].description+=":";
                      temp_int+=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int=source_tracker.convertPosition(temp_int);
                      //add label
                      if(temp_int<0) break;
                      ROM_data[temp_int].addLabel(converted_position);
                      break;
            case 0x5D:
                      ROM_data[converted_position].description="EOR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x5E:
                      ROM_data[converted_position].description="LSR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x5F:
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x60:
                      ROM_data[converted_position].description="RTS";
                      //no label for returns?
                      break;
            case 0x61:
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0x62:
                      ROM_data[converted_position].description="PER $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x63:
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
            case 0x64:
                      ROM_data[converted_position].description="STZ $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x65:
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x66:
                      ROM_data[converted_position].description="ROR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x67:
                      ROM_data[converted_position].description="ADC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0x68:
                      ROM_data[converted_position].description="PLA";
                      break;
            case 0x69:
                      ROM_data[converted_position].description="ADC #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0x6A:
                      ROM_data[converted_position].description="ROR A";
                      break;
            case 0x6B:
                      ROM_data[converted_position].description="RTL";
                      break;
            case 0x6C:
                      ROM_data[converted_position].description="JMP ($";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0x6D:
                      ROM_data[converted_position].description="ADC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x6E:
                      ROM_data[converted_position].description="ROR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x6F:
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x70:
                      ROM_data[converted_position].description="BVS $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x71:
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0x72:
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0x73:
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0x74:
                      ROM_data[converted_position].description="STZ $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x75:
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x76:
                      ROM_data[converted_position].description="ROR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x77:
                      ROM_data[converted_position].description="ADC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0x78:
                      ROM_data[converted_position].description="SEI";
                      break;
            case 0x79:
                      ROM_data[converted_position].description="ADC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x7A:
                      ROM_data[converted_position].description="PLY";
                      break;
            case 0x7B:
                      ROM_data[converted_position].description="TDC";
                      break;
            case 0x7C:
                      ROM_data[converted_position].description="JMP ($";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0x7D:
                      ROM_data[converted_position].description="ADC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x7E:
                      ROM_data[converted_position].description="ROR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x7F:
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x80:
                      ROM_data[converted_position].description="BRA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x81:
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0x82:
                      ROM_data[converted_position].description="BRL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      position+=3;
                      temp_int= (temp_int<0x8000 ? (position+temp_int) : (position-(uint16_t)(0-temp_int)) );
                      temp_int2= source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                  
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x83:
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
            case 0x84:
                      ROM_data[converted_position].description="STY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x85:
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x86:
                      ROM_data[converted_position].description="STX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0x87:
                      ROM_data[converted_position].description="STA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0x88:
                      ROM_data[converted_position].description="DEY";
                      break;
            case 0x89:
                      ROM_data[converted_position].description="BIT #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0x8A:
                      ROM_data[converted_position].description="TXA";
                      break;
            case 0x8B:
                      ROM_data[converted_position].description="PHB";
                      break;
            case 0x8C:
                      ROM_data[converted_position].description="STY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x8D:
                      ROM_data[converted_position].description="STA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x8E:
                      ROM_data[converted_position].description="STX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x8F:
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x90:
                      ROM_data[converted_position].description="BCC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0x91:
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0x92:
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0x93:
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0x94:
                      ROM_data[converted_position].description="STY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x95:
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x96:
                      ROM_data[converted_position].description="STX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x97:
                      ROM_data[converted_position].description="STA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0x98:
                      ROM_data[converted_position].description="TYA";
                      break;
            case 0x99:
                      ROM_data[converted_position].description="STA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0x9A:
                      ROM_data[converted_position].description="TXS";
                      break;
            case 0x9B:
                      ROM_data[converted_position].description="TXY";
                      break;
            case 0x9C:
                      ROM_data[converted_position].description="STZ $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0x9D:
                      ROM_data[converted_position].description="STA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x9E:
                      ROM_data[converted_position].description="STZ $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0x9F:
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xA0:
                      ROM_data[converted_position].description="LDY #$";
                      if(XSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xA1:
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0xA2:
                      ROM_data[converted_position].description="LDX #$";
                      if(XSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xA3:
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
            case 0xA4:
                      ROM_data[converted_position].description="LDY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xA5:
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xA6:
                      ROM_data[converted_position].description="LDX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xA7:
                      ROM_data[converted_position].description="LDA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0xA8:
                      ROM_data[converted_position].description="TAY";
                      break;
            case 0xA9:
                      ROM_data[converted_position].description="LDA #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xAA:
                      ROM_data[converted_position].description="TAX";
                      break;
            case 0xAB:
                      ROM_data[converted_position].description="PLB";
                      break;
            case 0xAC:
                      ROM_data[converted_position].description="LDY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xAD:
                      ROM_data[converted_position].description="LDA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xAE:
                      ROM_data[converted_position].description="LDX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xAF:
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xB0:
                      ROM_data[converted_position].description="BCS $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0xB1:
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0xB2:
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0xB3:
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0xB4:
                      ROM_data[converted_position].description="LDY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xB5:
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xB6:
                      ROM_data[converted_position].description="LDX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0xB7:
                      ROM_data[converted_position].description="LDA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0xB8:
                      ROM_data[converted_position].description="CLV";
                      break;
            case 0xB9:
                      ROM_data[converted_position].description="LDA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0xBA:
                      ROM_data[converted_position].description="TSX";
                      break;
            case 0xBB:
                      ROM_data[converted_position].description="TYX";
                      break;
            case 0xBC:
                      ROM_data[converted_position].description="LDY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xBD:
                      ROM_data[converted_position].description="LDA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xBE:
                      ROM_data[converted_position].description="LDX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0xBF:
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xC0:
                      ROM_data[converted_position].description="CPY #$";
                      if(XSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xC1:
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0xC2:
                      ROM_data[converted_position].description="REP #";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xC3:
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
            case 0xC4:
                      ROM_data[converted_position].description="CPY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xC5:
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xC6:
                      ROM_data[converted_position].description="DEC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xC7:
                      ROM_data[converted_position].description="CMP [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0xC8:
                      ROM_data[converted_position].description="INY";
                      break;
            case 0xC9:
                      ROM_data[converted_position].description="CMP #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xCA:
                      ROM_data[converted_position].description="DEX";
                      break;
            case 0xCB:
                      ROM_data[converted_position].description="WAI";
                      break;
            case 0xCC:
                      ROM_data[converted_position].description="CPY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xCD:
                      ROM_data[converted_position].description="CMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xCE:
                      ROM_data[converted_position].description="DEC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xCF:
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xD0:
                      ROM_data[converted_position].description="BNE $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0xD1:
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0xD2:
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0xD3:
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0xD4:
                      ROM_data[converted_position].description="PEI ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0xD5:
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xD6:
                      ROM_data[converted_position].description="DEC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xD7:
                      ROM_data[converted_position].description="CMP [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0xD8:
                      ROM_data[converted_position].description="CLD";
                      break;
            case 0xD9:
                      ROM_data[converted_position].description="CMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0xDA:
                      ROM_data[converted_position].description="PHX";
                      break;
            case 0xDB:
                      ROM_data[converted_position].description="STP";
                      break;
            case 0xDC:
                      ROM_data[converted_position].description="JMP [$";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0xDD:
                      ROM_data[converted_position].description="CMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xDE:
                      ROM_data[converted_position].description="DEC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xDF:
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xE0:
                      ROM_data[converted_position].description="CPX #$";
                      if(XSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xE1:
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0xE2:
                      ROM_data[converted_position].description="SEP #";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xE3:
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
            case 0xE4:
                      ROM_data[converted_position].description="CPX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xE5:
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xE6:
                      ROM_data[converted_position].description="INC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
            case 0xE7:
                      ROM_data[converted_position].description="SBC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
            case 0xE8:
                      ROM_data[converted_position].description="INX";
                      break;
            case 0xE9:
                      ROM_data[converted_position].description="SBC #$";
                      if(MSET)
                      {
                           temp_int= getSourceByte(position+1);
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      }
                      else
                      {
                          temp_int=getSourceWord(position+1);
                          ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      }
                      break;
            case 0xEA:
                      ROM_data[converted_position].description="NOP";
                      break;
            case 0xEB:
                      ROM_data[converted_position].description="XBA";
                      break;
            case 0xEC:
                      ROM_data[converted_position].description="CPX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xED:
                      ROM_data[converted_position].description="SBC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xEE:
                      ROM_data[converted_position].description="INC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xEF:
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xF0:
                      ROM_data[converted_position].description="BEQ $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      position+=2;
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2<0) break;
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
            case 0xF1:
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
            case 0xF2:
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
            case 0xF3:
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
            case 0xF4:
                      ROM_data[converted_position].description="PEA #$";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
            case 0xF5:
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xF6:
                      ROM_data[converted_position].description="INC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xF7:
                      ROM_data[converted_position].description="SBC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
            case 0xF8:
                      ROM_data[converted_position].description="SED";
                      break;
            case 0xF9:
                      ROM_data[converted_position].description="SBC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
            case 0xFA:
                      ROM_data[converted_position].description="PLX";
                      break;
            case 0xFB:
                      ROM_data[converted_position].description="XCE";
                      break;
            case 0xFC:
                      ROM_data[converted_position].description="JSR ($";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
            case 0xFD:
                      ROM_data[converted_position].description="SBC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xFE:
                      ROM_data[converted_position].description="INC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            case 0xFF:
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
            }//end of switch
}//end of recordASM function

char deScriptor::convertToASCII(char c)
{
     return (c<10) ? c+48 : c+55;
}

d_string deScriptor::convertByteToHexString(uint8_t toConvert)
{
     int i=0;
     uint8_t letters[2];
     d_string s;
     
     for(i=0;i<2;i++)
     {
          letters[i]=toConvert&0xF;
          letters[i]=convertToASCII(letters[i]);
          toConvert=toConvert>>4;
     }
     for(i=1;i>=0;i--)
     {
          s+=letters[i];
     }
     return s;
}

d_string deScriptor::convertWordToHexString(uint16_t toConvert)
{
     int i=0;
     uint8_t letters[4];
     d_string s;
     
     for(i=0;i<4;i++)
     {
          letters[i]=toConvert&0xF;
          letters[i]=convertToASCII(letters[i]);
          toConvert=toConvert>>4;
     }
     for(i=3;i>=0;i--)
     {
          s+=letters[i];
     }
     return s;
}

d_string deScriptor::convert24BitToHexString(uint32_t toConvert)
{
     int i=0;
     uint8_t letters[6];
     d_string s;
     
     for(i=0;i<6;i++)
     {
          letters[i]=toConvert&0xF;
          letters[i]=convertToASCII(letters[i]);
          toConvert=toConvert>>4;
     }
     for(i=5;i>=0;i--)
     {
          s+=letters[i];
     }
     return s;
}         

d_string deScriptor::convert32BitToHexString(uint32_t toConvert)
{
     int i=0;
     uint8_t letters[8];
     d_string s;
     
     for(i=0;i<8;i++)
     {
          letters[i]=toConvert&0xF;
          letters[i]=convertToASCII(letters[i]);
          toConvert=toConvert>>4;
     }
     for(i=7;i>=0;i--)
     {
          s+=letters[i];
     }
     return s;
}

uint16_t deScriptor::busRead16(uint32_t location)
{
     uint16_t result=SNES::bus.read(location);
     result+=(SNES::bus.read(location+1)<<8);
     return result;
}

uint32_t deScriptor::busRead24(uint32_t location)
{
     uint32_t result=SNES::bus.read(location);
     result+=(SNES::bus.read(location+1)<<8);
     result+=(SNES::bus.read(location+2)<<16);
     return result;
}

d_string deScriptor::convert64BitToHexString(uint64_t toConvert)
{
     int i=0;
     uint8_t letters[16];
     d_string s;
     bool upper_zero=true;
     
     for(i=0;i<16;i++)
     {
          letters[i]=toConvert&0xF;
          letters[i]=convertToASCII(letters[i]);
          toConvert=toConvert>>4;
     }
     for(i=15;i>=0;i--)
     {
          if(letters[i]!='0') upper_zero=false;
          if(upper_zero)
          {
               s+=' ';
          }
          else
          {
               s+=letters[i];
          }
     }
     return s;
}

uint32_t deScriptor::dpIndexedIndirectX(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     temp_int+=regsX; //Always add entire X register, even when XSET is true (that's just how snes works)
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead16(temp_int);
     temp_int2+=regsDB<<16;
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::stackRelative(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=Stack;            //temp_int now holds RAM address to read. Source of this is new A_source.
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
             
}
uint32_t deScriptor::dp(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;        
}
uint32_t deScriptor::dpIndirectLong(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead24(temp_int);
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;        
}
uint32_t deScriptor::absolute(uint32_t position,bool SET)
{
     temp_int=busRead16(position);
     temp_int+=regsDB<<16;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::absoluteLong(uint32_t position,bool SET)
{
     temp_int=busRead24(position);
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
             
}
uint32_t deScriptor::dpIndirectIndexedY(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead16(temp_int);
     temp_int2+=regsY;
     temp_int2+=regsDB<<16;
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::dpIndirect(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead16(temp_int);
     temp_int2+=regsDB<<16;
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::dpIndirectPEI(uint32_t position)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::srIndirectIndexedY(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=Stack;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead16(temp_int);
     temp_int+=regsY;
     temp_int2+=regsDB<<16;
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::dpIndexedX(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     temp_int+=regsX; //Always add entire X register, even when XSET is true (that's just how snes works)
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::dpIndirectLongIndexedY(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead24(temp_int);
     temp_int2+=regsY;
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::absoluteIndexedY(uint32_t position,bool SET)
{
     temp_int=busRead16(position);
     temp_int+=regsY;
     temp_int+=regsDB<<16;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::absoluteIndexedX(uint32_t position,bool SET)
{
     temp_int=busRead16(position);
     temp_int+=regsX;
     temp_int+=regsDB<<16;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::absoluteLongIndexedX(uint32_t position,bool SET)
{
     temp_int=busRead24(position);
     temp_int+=regsX;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::dpIndexedY(uint32_t position,bool SET)
{
     temp_int=busRead8(position);
     temp_int+=regsDP;
     temp_int+=regsY;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     return temp_int;
}
uint32_t deScriptor::absoluteIndexedIndirect(uint32_t position)
{
     temp_int=busRead16(position);
     temp_int+=regsX;
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead16(temp_int);
     temp_int2+=position&0xff0000;
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::absoluteIndirect(uint32_t position)
{
     temp_int=busRead16(position);
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead16(temp_int);
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
uint32_t deScriptor::absoluteIndirectLong(uint32_t position)
{
     temp_int=busRead16(position);
     step.accessed_address=temp_int;
     step.converted_address=source_tracker.convertPosition(temp_int);
     temp_int2=busRead24(temp_int);
     step.indirect_address=temp_int2;
     step.converted_indirect=source_tracker.convertPosition(temp_int2);
     return temp_int2;
}
void deScriptor::addScriptJump(uint32_t position,bool Bit16Ops)
{
     int i;
     SCRIPT_JUMP * temp=new SCRIPT_JUMP[num_scripts+1];
     for(i=0;i<num_scripts;i++)
     {
          temp[i]=scripts[i];
     }
     temp[num_scripts].address=position;
     temp[num_scripts].bit16=Bit16Ops;
     if(Bit16Ops)
     {
          temp[num_scripts].commands=new SCRIPT_COMMAND[0x10000];
          for(i=0;i<0x10000;i++) temp[num_scripts].commands[i].code=NULL;
     }
     else
     {
          temp[num_scripts].commands=new SCRIPT_COMMAND[0x100];
          for(i=0;i<0x100;i++) temp[num_scripts].commands[i].code=NULL;
     }
     delete[] scripts;
     scripts=temp;
     num_scripts++;
}
void deScriptor::addEmptyScriptJump(uint32_t position)
{
     int i;
     SCRIPT_JUMP * temp=new SCRIPT_JUMP[num_scripts+1];
     for(i=0;i<num_scripts;i++)
     {
          temp[i]=scripts[i];
     }
     temp[num_scripts].address=position;
     delete[] scripts;
     scripts=temp;
     num_scripts++;
}
void deScriptor::increaseScriptSize(uint16_t script_index,bool bit16)
{
     if(scripts[script_index].commands==NULL)
     {
          scripts[script_index].commands=(bit16)? new SCRIPT_COMMAND[0x10000] : new SCRIPT_COMMAND[0x100];
          return;
     }
     
     SCRIPT_COMMAND * temp=new SCRIPT_COMMAND[0x10000];
     for(int32_t i=0;i<0x100;i++)
     {
          temp[i]=scripts[script_index].commands[i];
     }
     delete[] scripts[script_index].commands;
     scripts[script_index].commands=temp;
     scripts[script_index].bit16=true;
}

uint8_t deScriptor::getSourceByte(uint32_t pos)
{
     int32_t source_address= source_tracker.convertPosition(pos);
     if(source_address<0)
     {
          return busRead8(pos);
     }
     ROM_data[source_address].flags|=OPERAND;
     return ROM_data[source_address].ROM_bytes;
}

uint16_t deScriptor::getSourceWord(uint32_t pos)
{
     int32_t source_address= source_tracker.convertPosition(pos);
     if(source_address<0)
     {
          return busRead16(pos);
     }
     ROM_data[source_address].flags|=OPERAND;
     uint16_t result = ROM_data[source_address].ROM_bytes;
     
     source_address= source_tracker.convertPosition(pos+1);
     ROM_data[source_address].flags|=OPERAND;
     result+= ROM_data[source_address].ROM_bytes << 8;
     return result;
}
/*void deScriptor::deleteScriptJump(uint32_t position)
{
     int i;
     for(i=0;i<num_scripts;i++)
     {
          if(scripts[i].address==position) break;
     }
     if(i==num_scripts) return;
     for(i=0;i<(num_scripts-1);i++)
     {
          scripts[i]=scripts[i+1];
     }
     num_scripts--;
}*/
#endif
