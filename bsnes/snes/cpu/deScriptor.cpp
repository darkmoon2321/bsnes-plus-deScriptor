#ifndef DESCRIPTOR_CPP
#define DESCRIPTOR_CPP

#include "deScriptor.hpp"
#include "asm_descriptions.cpp"

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
     initializeASMDescriptions();
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
             step.converted_indirect=source_tracker.convertPosition(temp_int);
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
             step.converted_indirect=source_tracker.convertPosition(temp_int);
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
   ROM_data[converted_position].description=asm_strings[step.opcode];
   switch(ROM_data[converted_position].ROM_bytes)
   {
        case 0x10:
        case 0x30:
        case 0x50:
        case 0x70:
        case 0x80:
        case 0x90:
        case 0xB0:
        case 0xD0:
        case 0xF0:
             {
                  temp_int= busRead8(position+1);
                  ROM_data[converted_position].description+=" (GOTO 0x";
                  position+=2;
                  
                  temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                  temp_int2=source_tracker.convertPosition(temp_int);
                  ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                  ROM_data[converted_position].description+=")";
                  
                  if(temp_int2>0) ROM_data[temp_int2].addLabel(converted_position);
                  break;
             }
        case 0x82:
             {
                  temp_int=busRead16(position+1);
                  ROM_data[converted_position].description+=" (GOTO 0x";
                  position+=3;
                  
                  temp_int= (temp_int<0x8000 ? (position+temp_int) : (position-(uint16_t)(0-temp_int)) );
                  temp_int2= source_tracker.convertPosition(temp_int);
                  ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                  ROM_data[converted_position].description+=")";
              
                  if(temp_int2>0) ROM_data[temp_int2].addLabel(converted_position);
                  break;
             }
        
        case 0x20:
             {
                  temp_int=busRead16(position+1);
                  temp_int+=(position&0xFF0000);
                  temp_int=source_tracker.convertPosition(temp_int);
                  //add branch label
                  if(temp_int>0) ROM_data[temp_int].addLabel(converted_position);
                  break;
             }
        case 0x22:
             {
                  temp_int= busRead24(position+1);
                  temp_int= source_tracker.convertPosition(temp_int);
                  //add branch label
                  if(temp_int>0) ROM_data[temp_int].addLabel(converted_position);
                  break;
             }
        case 0x4C: //JMP addr
             {
                  temp_int=busRead16(position+1);
                  temp_int+=(position&0xff0000);
                  temp_int=source_tracker.convertPosition(temp_int);
                  //add branch label
                  if(temp_int>0) ROM_data[temp_int].addLabel(converted_position);
                  break;
             }
        case 0x5C: //JMP long
             {
                  temp_int= busRead24(position+1);
                  temp_int=source_tracker.convertPosition(temp_int);
                  //add label
                  if(temp_int>0) ROM_data[temp_int].addLabel(converted_position);
                  break;
             }
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
