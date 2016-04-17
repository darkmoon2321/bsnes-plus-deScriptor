#ifndef DESCRIPTOR
#define DESCRIPTOR

#include <string>
#include <fstream>

typedef std::string d_string;
    
#define TEXT_WIDTH 80

enum byteFlags{
            SCRIPT      =0X01,
            SNES65C816  =0X02,
            OPERAND     =0X04,
            DATA        =0x08
};

struct MVN_TRACKER_ENTRY{
       uint32_t ROM_source;
       uint32_t RAM_destination;
       uint16_t num_bytes;
       MVN_TRACKER_ENTRY operator = (MVN_TRACKER_ENTRY m)
       {
            ROM_source=m.ROM_source;
            RAM_destination=m.RAM_destination;
            num_bytes=m.num_bytes;
       }
};

struct BRANCH{
       uint32_t RAM_sources[0x20000];
       bool invalid;
       SNES::MappedRAM * cartrom;
       void * vsprom;
       
       BRANCH()
       {
            for(int i=0;i<0x20000;i++)
            {
                 RAM_sources[i]=0x00000000;
            }
            invalid=false;
       }
       
       void updateRAM(MVN_TRACKER_ENTRY newMVN)
       {
            int i;
            newMVN.ROM_source&=0x7FFFFF;
            newMVN.RAM_destination&=0x7FFFFF;
            if(newMVN.RAM_destination<0x7E0000) return;
            for(i=0;i<newMVN.num_bytes;i++)
            {
                 if(((newMVN.RAM_destination&0x01ffff)+i)>0x1ffff) break;
                 if(((newMVN.ROM_source+i))<0x7E0000)
                 {
                     RAM_sources[(i+newMVN.RAM_destination)&0x01ffff]=newMVN.ROM_source+i;
                 }
                 else
                 {
                     if((newMVN.ROM_source+i)>0x7FFFFF) break;
                     RAM_sources[(i+newMVN.RAM_destination)&0x01ffff]=RAM_sources[(newMVN.ROM_source+i)&0x1ffff];
                 }
            }
       }
       uint32_t convertPosition(uint32_t pos)
       {
            if(pos<0x7E0000 || pos>=0x800000)
            {
                 SNES::Bus::Page &page = SNES::bus.page[pos >> 8];
                 if (page.access == cartrom || page.access == vsprom)
                 {
                     return page.offset + pos;
                 }
                 else
                 {
                     invalid=true;
                     return 0;
                 }
            }
            return RAM_sources[pos&0x01FFFF];
       }
};//end of BRANCH definition

class deScriptor{
private:
    
    struct ROM_DATA{
           uint8_t ROM_bytes;
           d_string description;
           uint8_t flags;
           uint32_t RAM_address; //This is set when assembly or script is executed from WRAM
           uint32_t * labels;
           uint32_t num_labels;
           uint64_t frequency;
           
           ROM_DATA()
           {
                description="";
                flags=0;
                RAM_address=0;
                labels=NULL;
                num_labels=0;
                frequency=0;
           }
           void addLabel(uint32_t label)
           {
                uint32_t * temp=new uint32_t[num_labels+1];
                for(uint32_t i=0;i<num_labels;i++)
                {
                     temp[i]=labels[i];
                }
                temp[num_labels]=label;
                num_labels++;
                delete[] labels;
                labels=temp;
           }
       };
    
    BRANCH source_tracker;
    ROM_DATA * ROM_data;
    d_string file_base;
    long file_size;
    uint32_t temp_int;
    uint32_t temp_int2;
    bool display_error;
    d_string error_message;
    uint16_t * regsA;
    uint16_t * regsX;
    uint16_t *  regsY;
    uint16_t *  Stack;
    bool * MSET;
    bool * XSET;
    uint8_t * regsDB;
    
public:
           
    deScriptor()
    {
         ROM_data=NULL;
         file_base="";
         file_size=0;
         display_error=true;
         error_message="";
    }
    
    bool error()
    {
         return display_error;
    }
    
    const char * getMessage()
    {
          return error_message.c_str();
    }
    bool loadData(nall::string b_name,uint8_t * data_start,const long f_size,uint16_t * pregsA,
         uint16_t * pregsX, uint16_t * pregsY,bool * pMSET,bool * pXSET,uint8_t * pregsDB,uint16_t * pregsS,
         SNES::MappedRAM * cartrom, void * vsprom)
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
        
        regsA=pregsA;
        regsX=pregsX;
        regsY=pregsY;
        MSET=pMSET;
        XSET=pXSET;
        regsDB=pregsDB;
        Stack=pregsS;
        source_tracker.cartrom=cartrom;
        source_tracker.vsprom=vsprom;
        
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
                  binary_in.read((char *)ROM_data[i].RAM_address,sizeof(uint32_t));
                  if(ROM_data[i].flags&SCRIPT || ROM_data[i].flags&SNES65C816)
                  {
                       getline(text_in,buffer);
                       ROM_data[i].description=buffer.substr(37,TEXTWIDTH-37);
                       do
                       {
                            line_start=text_in.tellg();
                            getline(text_in,buffer);
                            if(buffer[0]==' ')
                            {
                                 ROM_data[i].description+=buffer.substr(37,TEXTWIDTH-37);
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

    bool unloadData()
    {
        long i,j;
        d_string f_name=file_base;
        f_name+="-dsc.txt";
        std::ofstream outfile;
        uint8_t  pc_bank=0;
        uint16_t pc_address=0x8000;
        outfile.open(f_name.c_str());
        /*std::ofstream outfile2;
        f_name=file_base;
        f_name+="-dsc.bin";
        outfile2.open(f_name.c_str(),std::ios::binary);*/
        
        //Print results of disassembly to file    
        outfile<<"Hex     LoROM   RAM     Bytes        Description";
        for(temp_int=48;temp_int<TEXT_WIDTH;temp_int++) outfile<<' ';
        outfile<<"|Frequency       |Labels"<<std::endl;
        pc_bank=0;
        pc_address=0x8000;
        for(i=0;i<(file_size);i++)
        {
             j=i;
             //outfile2.write((char *)ROM_data[i].flags,sizeof(uint8_t));
             //outfile2.write((char *)ROM_data[i].RAM_address,sizeof(uint32_t));
             if(!(ROM_data[i].flags&SCRIPT) && !(ROM_data[i].flags&SNES65C816))
             {
                 pc_address++;
                 if(pc_address<0x8000)
                 {
                      pc_bank++;
                      pc_address+=0x8000;
                 }
                 continue;
             }
             outfile<<convert24BitToHexString(i)<<"  ";
             outfile<<convert24BitToHexString((pc_bank<<16) + pc_address)<<"  ";
             if(ROM_data[i].RAM_address)
             {
                  outfile<<convert24BitToHexString(ROM_data[i].RAM_address)<<"  ";
             }
             else
             {
                  outfile<<"        ";
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
                      pc_address++;
                      if(pc_address<0x8000)
                      {
                           pc_bank++;
                           pc_address+=0x8000;
                      }
                      if((i+2)<(file_size) && ROM_data[i+2].flags&OPERAND)
                      {
                           outfile<<convertByteToHexString(ROM_data[i+2].ROM_bytes);
                           outfile<<' ';
                           j++;
                           pc_address++;
                           if(pc_address<0x8000)
                           {
                                pc_bank++;
                                pc_address+=0x8000;
                           }
                           if((i+3)<(file_size) && ROM_data[i+3].flags&OPERAND)
                           {
                                outfile<<convertByteToHexString(ROM_data[i+3].ROM_bytes);
                                outfile<<"  ";
                                j++;
                                pc_address++;
                                if(pc_address<0x8000)
                                {
                                     pc_bank++;
                                     pc_address+=0x8000;
                                }
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
                 pc_address++;
                 if(pc_address<0x8000)
                 {
                      pc_bank++;
                      pc_address+=0x8000;
                 }
                 if((ROM_data[i].description).length() > (TEXT_WIDTH-37))
                 {
                      outfile<<(ROM_data[i].description).substr(0,TEXT_WIDTH-37);
                      temp_int=TEXT_WIDTH-37;
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
                           outfile<<(ROM_data[i].description).substr(temp_int,TEXT_WIDTH-37);
                           outfile<<std::endl;
                           temp_int+=(TEXT_WIDTH-37);
                      }
                 }
                 else
                 {
                      outfile<<ROM_data[i].description;
                      for(temp_int=ROM_data[i].description.length();temp_int<(TEXT_WIDTH-37);temp_int++)
                      {
                           outfile<<' ';
                      }
                      outfile<<"|";
                      outfile<<convert64BitToHexString(ROM_data[i].frequency);
                      outfile<<"|";
                      for(temp_int2=0;temp_int2<ROM_data[i].num_labels;temp_int2++)
                      {
                           outfile<<"  0x";
                           outfile<<convert24BitToHexString(ROM_data[i].labels[temp_int2]);
                      }
                      outfile<<std::endl;
                 }
             }
             i=j;
        }
        outfile.close();
        //outfile2.close();
        
        return true;
    }
    
    void loadState(uint8_t slot)
    {
         d_string file_name=file_base;
         file_name+=convertToASCII(slot);
         file_name+=".dst";
         std::ifstream infile(file_name.c_str());
         
         for(int i=0;i<0x20000;i++)
         {
              infile.read((char *)(&source_tracker.RAM_sources[i]),sizeof(uint32_t));
         }
    }
    
    void saveState(uint8_t slot)
    {
         d_string file_name=file_base;
         file_name+=convertToASCII(slot);
         file_name+=".dst";
         std::ofstream outfile(file_name.c_str());
         
         for(int i=0;i<0x20000;i++)
         {
              outfile.write((char *)(&source_tracker.RAM_sources[i]),sizeof(uint32_t));
         }
    }
    
   void recordScript(uint32_t position)
   { 
         //Convert program or script counter to a ROM array index.
         uint32_t converted_position=source_tracker.convertPosition(position);
         
         error_message=convert24BitToHexString(position);
         
         ROM_data[converted_position].frequency++;
         
         if(ROM_data[converted_position].flags) return; //don't record the same position twice.
         
         if((position&0x7FFFFF)>=0x7E0000) ROM_data[converted_position].RAM_address=position;
         
         ROM_data[converted_position].flags|=SCRIPT;
         switch(ROM_data[converted_position].ROM_bytes)
         {
            case 0x00:
                 {
                      //$0A=*((WORD *)(0xffe9+$04))
                      //$0A=WORD at RAM address $04-x17;
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x17  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x17;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x01:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x15  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x15;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x02:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x13  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x13;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x03:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x11  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x11;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x04:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x0F  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xf;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x05:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x0D  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xd;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x06:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x0B  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xb;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x07:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x09  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x9;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x08:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x07  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x7;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x09:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x05  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x5;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x0A:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x03  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x3;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x0B:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04-x01  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x1;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x0C:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04+x0F  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0xf;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x0D:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04+x11  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x11;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x0E:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04+x13  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x13;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x0F:
                 {
                      ROM_data[converted_position].description="$0A=WORD at RAM address $04+x15  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x15;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x10:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x17  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x17;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x11:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x15  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x15;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x12:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x13  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x13;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x13:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x11  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x11;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x14:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x0F  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xf;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x15:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x0D  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xd;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x16:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x0B  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xb;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x17:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x09  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x9;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x18:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x07  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x7;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x19:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x05  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x5;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x1A:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x03  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x3;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x1B:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04-x01  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x1;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x1C:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04+x0F  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0xf;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x1D:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04+x11  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x11;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x1E:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04+x13  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x13;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x1F:
                 {
                      ROM_data[converted_position].description="$0E=WORD at RAM address $04+x15  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x15;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x20:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x17 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x21:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x15 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x22:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x13 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x23:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x11 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x24:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x0F = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x25:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x0D = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x26:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x0B = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x27:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x09 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x28:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x07 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x29:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x05 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x2A:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x03 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x2B:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04-0x01 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x2C:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04+0x0F = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x2D:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04+0x11 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x2E:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04+0x13 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x2F:
                 {
                      ROM_data[converted_position].description="WORD at RAM address $04+0x15 = $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x30:
                 {
                      //Push Word at RAM address $04-0x17
                      //Push *($04-0x17)
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x17  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x17;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x31:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x15  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x15;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x32:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x13  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x13;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x33:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x11  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x11;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x34:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x0F  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xf;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x35:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x0D  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xd;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x36:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x0B  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0xb;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x37:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x09  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x9;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x38:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x07  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x7;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x39:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x05  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x5;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x3A:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x03  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x3;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x3B:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04-0x01  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int-=0x1;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x3C:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04+0x0F  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0xf;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x3D:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04+0x11  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x11;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x3E:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04+0x13  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x13;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x3F:
                 {
                      ROM_data[converted_position].description="Push Word at RAM address $04+0x15  [";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      temp_int+=0x15;
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x40:
                 {
                      ROM_data[converted_position].description="$0A=0";
                      break;
                 }
            case 0x41:
                 {
                      ROM_data[converted_position].description="$0A=1";
                      break;
                 }
            case 0x42:
                 {
                      ROM_data[converted_position].description="$0A=2";
                      break;
                 }
            case 0x43:
                 {
                      ROM_data[converted_position].description="$0A=3";
                      break;
                 }
            case 0x44:
                 {
                      ROM_data[converted_position].description="$0A=4";
                      break;
                 }
            case 0x45:
                 {
                      ROM_data[converted_position].description="$0A=5";
                      break;
                 }
            case 0x46:
                 {
                      ROM_data[converted_position].description="$0A=6";
                      break;
                 }
            case 0x47:
                 {
                      ROM_data[converted_position].description="$0A=7";
                      break;
                 }
            case 0x48:
                 {
                      ROM_data[converted_position].description="$0A=8";
                      break;
                 }
            case 0x49:
                 {
                      ROM_data[converted_position].description="$0A=1=9";
                      break;
                 }
            case 0x4A:
                 {
                      ROM_data[converted_position].description="$0A=0xA";
                      break;
                 }
            case 0x4B:
                 {
                      ROM_data[converted_position].description="$0A=0xB";
                      break;
                 }
            case 0x4C:
                 {
                      ROM_data[converted_position].description="$0A=0xC";
                      break;
                 }
            case 0x4D:
                 {
                      ROM_data[converted_position].description="$0A=0xD";
                      break;
                 }
            case 0x4E:
                 {
                      ROM_data[converted_position].description="$0A=0xE";
                      break;
                 }
            case 0x4F:
                 {
                      ROM_data[converted_position].description="$0A=0xF";
                      break;
                 }
            case 0x50:
                 {
                      ROM_data[converted_position].description="$0E=0";
                      break;
                 }
            case 0x51:
                 {
                      ROM_data[converted_position].description="$0E=1";
                      break;
                 }
            case 0x52:
                 {
                      ROM_data[converted_position].description="$0E=2";
                      break;
                 }
            case 0x53:
                 {
                      ROM_data[converted_position].description="$0E=3";
                      break;
                 }
            case 0x54:
                 {
                      ROM_data[converted_position].description="$0E=4";
                      break;
                 }
            case 0x55:
                 {
                      ROM_data[converted_position].description="$0E=5";
                      break;
                 }
            case 0x56:
                 {
                      ROM_data[converted_position].description="$0E=6";
                      break;
                 }
            case 0x57:
                 {
                      ROM_data[converted_position].description="$0E=7";
                      break;
                 }
            case 0x58:
                 {
                      ROM_data[converted_position].description="$0E=8";
                      break;
                 }
            case 0x59:
                 {
                      ROM_data[converted_position].description="$0E=9";
                      break;
                 }
            case 0x5A:
                 {
                      ROM_data[converted_position].description="$0E=0xA";
                      break;
                 }
            case 0x5B:
                 {
                      ROM_data[converted_position].description="$0E=0xB";
                      break;
                 }
            case 0x5C:
                 {
                      ROM_data[converted_position].description="$0E=0xC";
                      break;
                 }
            case 0x5D:
                 {
                      ROM_data[converted_position].description="$0E=0xD";
                      break;
                 }
            case 0x5E:
                 {
                      ROM_data[converted_position].description="$0E=0xE";
                      break;
                 }
            case 0x5F:
                 {
                      ROM_data[converted_position].description="$0E=0xF";
                      break;
                 }
            case 0x60:
                 {
                      ROM_data[converted_position].description="Push 0x0000";
                      break;
                 }
            case 0x61:
                 {
                      ROM_data[converted_position].description="Push 0x0001";
                      break;
                 }
            case 0x62:
                 {
                      ROM_data[converted_position].description="Push 0x0002";
                      break;
                 }
            case 0x63:
                 {
                      ROM_data[converted_position].description="Push 0x0003";
                      break;
                 }
            case 0x64:
                 {
                      ROM_data[converted_position].description="Push 0x0004";
                      break;
                 }
            case 0x65:
                 {
                      ROM_data[converted_position].description="Push 0x0005";
                      break;
                 }
            case 0x66:
                 {
                      ROM_data[converted_position].description="Push 0x0006";
                      break;
                 }
            case 0x67:
                 {
                      ROM_data[converted_position].description="Push 0x0007";
                      break;
                 }
            case 0x68:
                 {
                      ROM_data[converted_position].description="Push 0x0008";
                      break;
                 }
            case 0x69:
                 {
                      ROM_data[converted_position].description="Push 0x0009";
                      break;
                 }
            case 0x6A:
                 {
                      ROM_data[converted_position].description="Push 0x000A";
                      break;
                 }
            case 0x6B:
                 {
                      ROM_data[converted_position].description="Push 0x000B";
                      break;
                 }
            case 0x6C:
                 {
                      ROM_data[converted_position].description="Push 0x000C";
                      break;
                 }
            case 0x6D:
                 {
                      ROM_data[converted_position].description="Push 0x000D";
                      break;
                 }
            case 0x6E:
                 {
                      ROM_data[converted_position].description="Push 0x000E";
                      break;
                 }
            case 0x6F:
                 {
                      ROM_data[converted_position].description="Push 0x000F";
                      break;
                 }
            case 0x70:
                 {
                      ROM_data[converted_position].description="No Operation";
                      break;
                 }
            case 0x71:
                 {
                      ROM_data[converted_position].description="$0A+=1";
                      break;
                 }
            case 0x72:
                 {
                      ROM_data[converted_position].description="$0A+=2";
                      break;
                 }
            case 0x73:
                 {
                      ROM_data[converted_position].description="$0A+=3";
                      break;
                 }
            case 0x74:
                 {
                      ROM_data[converted_position].description="$0A+=4";
                      break;
                 }
            case 0x75:
                 {
                      ROM_data[converted_position].description="$0A+=5";
                      break;
                 }
            case 0x76:
                 {
                      ROM_data[converted_position].description="$0A+=6";
                      break;
                 }
            case 0x77:
                 {
                      ROM_data[converted_position].description="$0A+=7";
                      break;
                 }
            case 0x78:
                 {
                      ROM_data[converted_position].description="$0A+=8";
                      break;
                 }
            case 0x79:
                 {
                      ROM_data[converted_position].description="$0A+=9";
                      break;
                 }
            case 0x7A:
                 {
                      ROM_data[converted_position].description="$0A+=0xA";
                      break;
                 }
            case 0x7B:
                 {
                      ROM_data[converted_position].description="$0A+=0xB";
                      break;
                 }
            case 0x7C:
                 {
                      ROM_data[converted_position].description="$0A+=0xC";
                      break;
                 }
            case 0x7D:
                 {
                      ROM_data[converted_position].description="$0A+=0xD";
                      break;
                 }
            case 0x7E:
                 {
                      ROM_data[converted_position].description="$0A+=0xE";
                      break;
                 }
            case 0x7F:
                 {
                      ROM_data[converted_position].description="$0A+=0xF";
                      break;
                 }
            case 0x80:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x81:
                 {
                      //ROM_data[converted_position].description="Take next byte.  If minus, assume upper byte of FF.  Add $04+1, and get the value at this address to store into $0A.";
                      //Perform bitwise NOT on the single byte operand
                      //If operand>0x7F, $0A= WORD at RAM address ($04 - ~operand
                      //else             $0A= WORD at RAM address ($04 + operand +4)
                      //(operand<0x80) ? $0A=WORD at RAM address ($04 + operand +4) : $0A=WORD at RAM address ($04 - ~operand)
                      ROM_data[converted_position].description="$0A= WORD at RAM address ($04";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4);
                           temp_int+=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertByteToHexString(~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes));
                           temp_int-=(uint8_t)~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      ROM_data[converted_position].description+=")  [";
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x82:
                 {
                      //ROM_data[converted_position].description="Load next two bytes, add $04 (+4 if positive) to them, and get value from address formed to store into $0A.";
                      ROM_data[converted_position].description="$0A= WORD at RAM address($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int2=busRead16(0x0004);
                      temp_int2+=0x7E0000;
                      
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                           temp_int2+=temp_int+4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                           temp_int2-=~((uint16_t)temp_int);
                      }
                      ROM_data[converted_position].description+=") [";
                      temp_int=busRead16(temp_int2);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x83:
                 {
                      //ROM_data[converted_position].description="Take next byte.  If minus, assume upper byte of FF.  Add $04 (+4 if operand was positive), and get the value at the address formed to store into $0E.";
                      //Perform bitwise NOT on the single byte operand
                      //If operand>0x7F, $0E= WORD at RAM address ($04 - ~operand
                      //else             $0E= WORD at RAM address ($04 + operand +4)
                      //(operand<0x80) ? $0E=WORD at RAM address ($04 + operand +4) : $0E=WORD at RAM address ($04 - ~operand)
                      ROM_data[converted_position].description="$0E= WORD at RAM address ($04";
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4);
                           temp_int+=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertByteToHexString(~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes));
                           temp_int-=(uint8_t)~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      ROM_data[converted_position].description+=")  [";
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x84:
                 {
                      //ROM_data[converted_position].description="Take next two bytes.  Add $04 (+4 if positive), and use address formed to retrieve value to store into $0E";
                      ROM_data[converted_position].description="$0E= WORD at RAM address($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int2=busRead16(0x0004);
                      temp_int2+=0x7E0000;
                      
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                           temp_int2+=temp_int+4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                           temp_int2-=~((uint16_t)temp_int);
                      }
                      ROM_data[converted_position].description+=") [";
                      temp_int=busRead16(temp_int2);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x85:
                 {
                      //ROM_data[converted_position].description="Take next byte.  Add to $04, and use this as an address to store the value in $0A.";
                      //Perform bitwise NOT on the single byte operand
                      ROM_data[converted_position].description="WORD at RAM address ($04";
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertByteToHexString(~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes));
                      }
                      ROM_data[converted_position].description+=") = $0A [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x86:
                 {
                      //ROM_data[converted_position].description="Load next two bytes, add $04 to them, and use address formed as a destination for $0A value.";
                      ROM_data[converted_position].description="WORD at RAM address($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                      }
                      ROM_data[converted_position].description+=") =$0A [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x87:
                 {
                      //Take next byte. If plus, add operand to $04+4 and use address to get value to push to stack. If minus, subtract ~operand from $04 and do the same.
                      //Perform bitwise NOT on the single byte operand
                      //If operand>0x7F, Push WORD at RAM address ($04 - ~operand)
                      //else             Push WORD at RAM address ($04 + operand +4)
                      //(operand<0x80) ? Push WORD at RAM address ($04 + operand +4) : Push WORD at RAM address ($04 - ~operand)
                      ROM_data[converted_position].description="Push WORD at RAM address ($04";   
                      temp_int=busRead16(0x0004);
                      temp_int+=0x7E0000;
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4);
                           temp_int+=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes + 4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertByteToHexString(~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes));
                           temp_int-=(uint8_t)~(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      ROM_data[converted_position].description+=")  [";
                      temp_int2=busRead16(temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x88:
                 {
                      //ROM_data[converted_position].description="Load next two bytes, add $04 to them, and get value from address formed to push to the stack.";
                      ROM_data[converted_position].description="Push WORD at RAM address ($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int2=busRead16(0x0004);
                      temp_int2+=0x7E0000;
                      
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                           temp_int2+=temp_int+4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                           temp_int2-=~((uint16_t)temp_int);
                      }
                      ROM_data[converted_position].description+=")[";
                      temp_int=busRead16(temp_int2);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x89:
                 {
                      //ROM_data[converted_position].description="Takes value stored in next byte, and stores it in $0A. If negative, upper byte set to FF";
                      ROM_data[converted_position].description="$0A=";
                      temp_int2=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="0xFF";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x8A:
                 {
                      ROM_data[converted_position].description="$0A=0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x8B:
                 {
                      //ROM_data[converted_position].description="Takes value stored in next byte, and stores it in $0E. If negative, upper byte set to FF";
                      ROM_data[converted_position].description="$0E=";
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="0xFF";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                           temp_int2+=0xff00;
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x8C:
                 {
                      //"Takes value stored in next two bytes, and stores it in $0E.(two bytes)";
                      ROM_data[converted_position].description="$0E=0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x8D:
                 {
                      //"Takes value stored in next byte, and pushes it to the stack. If negative, upper byte is FF";
                      ROM_data[converted_position].description="Push ";
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="0xFF";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                           temp_int2+=0xff00;
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x8E:
                 {
                      //"Takes value stored in next two bytes, and pushes it to the stack.";
                      ROM_data[converted_position].description="Push 0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x8F:
                 {
                      //"Take next byte.  If plus, add value in byte to $0A, and store it in $0A. If minus, assume upper byte is FF and then add to $0A.";
                      ROM_data[converted_position].description="$0A+=";
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="0xFF";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                           temp_int2+=0xff00;
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0x90:
                 {
                      ROM_data[converted_position].description="$0A+=0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0x91:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x92: 
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x93:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x94:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x95:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x96:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x97:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x98:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x99:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x9A:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x9B:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x9C:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x9E:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x9D:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0x9F:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xA0:
                 {
                      //ROM_data[converted_position].description="Load next two bytes, add $04 to them, and use this as an address to get a stack value into $0A (8 bit).";
                      ROM_data[converted_position].description="$0A= BYTE at RAM address ($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int2=busRead16(0x0004);
                      temp_int2+=0x7E0000;
                      
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int + 4);
                           temp_int2+=temp_int+4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~(temp_int));
                           temp_int2-=~((uint16_t)temp_int);
                      }
                      ROM_data[converted_position].description+=")  [";
                      temp_int=SNES::bus.read(temp_int2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA1:
                 {
                      //ROM_data[converted_position].description="Load next two bytes, add $04 to them, and use this as an address to get a stack value into $0E (8 bit).";
                      ROM_data[converted_position].description="$0E= BYTE at RAM address ($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int2=busRead16(0x0004);
                      temp_int2+=0x7E0000;
                      
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int + 4);
                           temp_int2+=temp_int+4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~(temp_int));
                           temp_int2-=~((uint16_t)temp_int);
                      }
                      ROM_data[converted_position].description+=")  [";
                      temp_int=SNES::bus.read(temp_int2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA2:
                 {
                      //"Load next two bytes, add $04 to them, and use this as an address to store a stack value from $0A (8 bit). (Something happens if the two bytes are positive).";
                      ROM_data[converted_position].description="BYTE at RAM address ($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int + 4);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~(temp_int));
                      }
                      ROM_data[converted_position].description+=") = $0A [";
                      temp_int2=SNES::bus.read(0x7E000A);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA3:
                 {
                      //"(Add+4)/(Subtract ~) operand to $04 and use location as an address to find a BYTE to push to the stack.";
                      ROM_data[converted_position].description="Push BYTE at RAM address ($04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int2=busRead16(0x0004);
                      temp_int2+=0x7E0000;
                      
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int + 4);
                           temp_int2+=temp_int+4;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~(temp_int));
                           temp_int2-=~((uint16_t)temp_int);
                      }
                      ROM_data[converted_position].description+=")  [";
                      temp_int=SNES::bus.read(temp_int2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA4:
                 {
                      //"Load next two bytes and use this as an address to take a value from and store it into $0A (two bytes).";
                      ROM_data[converted_position].description="$0A= WORD at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=busRead16(0x7E0000+temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA5:
                 {
                      //"Load next two bytes and use this as an address to take a value from and store it into $0A.(one byte)";
                      ROM_data[converted_position].description="$0A= BYTE at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=SNES::bus.read(0x7E0000+temp_int);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA6:
                 {
                      //"Load next two bytes and use this as an address to take a value from and store it into $0E.";
                      ROM_data[converted_position].description="$0E= WORD at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=busRead16(0x7E0000+temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA7:
                 {
                      //"Load next two bytes and use value from this address to store single byte in $0E (upper byte stored as zero).";
                      ROM_data[converted_position].description="$0E= BYTE at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=SNES::bus.read(0x7E0000+temp_int);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA8:
                 {
                      //"Load next two bytes and use this as an address to store two bytes from $0A.";
                      ROM_data[converted_position].description="WORD at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="=$0A";
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xA9:
                 {
                      //*(7E:operand)=(BYTE)$0A
                      ROM_data[converted_position].description="BYTE at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=")=$0A";
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=SNES::bus.read(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xAA:
                 {
                      //"Load next two bytes and use the address formed by this to get a value to push to the stack";                      
                      ROM_data[converted_position].description="Push WORD at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=busRead16(0x7E0000+temp_int);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xAB:
                 {
                      //"Load next two bytes and use them to form an address.  Take value from that address and push to stack (single byte, with upper byte zero)";
                      ROM_data[converted_position].description="Push BYTE at RAM address 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      ROM_data[converted_position].description+="  [";
                      temp_int2=SNES::bus.read(0x7E0000+temp_int);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xAC:
                 {
                      ROM_data[converted_position].description="ASM Function7E";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int+=0x7E0000;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xAD:
                 {
                      //For(operand;operand>0;operand--) Byte at RAM address $0E=BYTE at RAM address $0A 
                      ROM_data[converted_position].description="For(X=";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=";X>0;X--) BYTE at RAM address $0E=BYTE at RAM address $0A";
                      //more like a do while loop.  Even if op is zero, we always execute at least once. (if op is zero, we loop, xffff times...)
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xAE:
                 {
                      //"Load value in the next byte, set upper byte FF, and use this to adjust the stack pointer";
                      ROM_data[converted_position].description="Stack Pointer";
                      if(ROM_data[converted_position].ROM_bytes<0x80)
                      {
                           ROM_data[converted_position].description+="+=0x";
                           ROM_data[converted_position].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-=0x";
                           ROM_data[converted_position].description+=convertByteToHexString((~ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes)+1);
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      break;
                 }
            case 0xAF:
                 {
                      //"Load value in the next two bytes, then adjust the stack pointer by this amount";
                      //SP+=operand
                      ROM_data[converted_position].description="$Stack Pointer+=0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xB0:
                 {
                      //"Load value from address formed by $0A, store this value back in $0A.";
                      ROM_data[converted_position].description="$0A=WORD at RAM address $0A";
                      break;
                 }
            case 0xB1:
                 {
                      //"Pull value off stack into $0E, use address formed by $0E as destination for value currently in $0A.";
                      ROM_data[converted_position].description="WORD at RAM address (Pull $0E)=$0A";
                      break;
                 }
            case 0xB2:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xB3:
                 {
                      ROM_data[converted_position].description="Push $0A  [";
                      temp_int=busRead16(0x7E000A);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0xB4:
                 {
                      ROM_data[converted_position].description="Pull $0E  [";
                      temp_int=busRead16(0x7E0001+*Stack);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0xB5:
                 {
                      ROM_data[converted_position].description="$0A*=$0E";
                      break;
                 }
            case 0xB6:
                 {
                      ROM_data[converted_position].description="$0A/=$0E, signed variables";
                      break;
                 }
            case 0xB7:
                 {
                      ROM_data[converted_position].description="Extended opcode";
                      position++;
                      converted_position=source_tracker.convertPosition(position);
                      ROM_data[converted_position].flags|=SCRIPT;
                      switch(ROM_data[converted_position].ROM_bytes)
                      {
                           case 0x00:
                                {
                                     ROM_data[converted_position].description="BRK #$00";
                                     break;
                                }
                           case 0x01:
                                {
                                     ROM_data[converted_position].description="$0A*=$0E, 32-bit";
                                     break;
                                }
                           case 0x02:
                                {
                                     ROM_data[converted_position].description="$0A/=$0E, 32-bit signed";
                                     break;
                                }
                           case 0x03:
                                {
                                    ROM_data[converted_position].description="$0A+=$0E, 32-bit";
                                    break;
                                }
                           case 0x04:
                                {
                                     ROM_data[converted_position].description="$0A-=$0E, 32-bit";
                                     break;
                                }
                           case 0x05:
                                {
                                     ROM_data[converted_position].description="$0A=0-$0A, 32-bit";
                                     break;
                                }
                           case 0x06:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A==$0E), 32-bit";
                                     break;
                                }
                           case 0x07:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A!=$0E), 32-bit";
                                     break;
                                }
                           case 0x08:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A-$0E)<0, 32-bit signed";
                                     break;
                                }
                           case 0x09:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0E-$0A)>=0, 32-bit signed";
                                     break;
                                }
                           case 0x0A:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0E-$0A)<0, 32-bit signed";
                                     break;
                                }
                           case 0x0B:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A-$0E)>=0, 32-bit signed";
                                     break;
                                }
                           case 0x0C:
                                {
                                     ROM_data[converted_position].description="$0A=DWORD at RAM address ($04";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     if(temp_int<0x8000)
                                     {
                                          ROM_data[converted_position].description+="+0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                                          ROM_data[converted_position].description+=")";
                                     }
                                     else
                                     {
                                          ROM_data[converted_position].description+="-0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                                          ROM_data[converted_position].description+=")";
                                     }
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x0D:
                                {
                                     ROM_data[converted_position].description="$0E=DWORD at RAM address ($04";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     if(temp_int<0x8000)
                                     {
                                          ROM_data[converted_position].description+="+0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                                          ROM_data[converted_position].description+=")";
                                     }
                                     else
                                     {
                                          ROM_data[converted_position].description+="-0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                                          ROM_data[converted_position].description+=")";
                                     }
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x0E:
                                {
                                     ROM_data[converted_position].description="DWORD at RAM address ($04";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     if(temp_int<0x8000)
                                     {
                                          ROM_data[converted_position].description+="+0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                                          ROM_data[converted_position].description+=")=$0A";
                                     }
                                     else
                                     {
                                          ROM_data[converted_position].description+="-0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                                          ROM_data[converted_position].description+=")=$0A";
                                     }
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x0F:
                                {
                                     ROM_data[converted_position].description="Push DWORD at RAM address ($04";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     if(temp_int<0x8000)
                                     {
                                          ROM_data[converted_position].description+="+0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                                          ROM_data[converted_position].description+=")";
                                     }
                                     else
                                     {
                                          ROM_data[converted_position].description+="-0x";
                                          ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                                          ROM_data[converted_position].description+=")";
                                     }
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x10:
                                {
                                     ROM_data[converted_position].description="$0A=DWORD at RAM address 7E:";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x11:
                                {
                                     ROM_data[converted_position].description="$0E=DWORD at RAM address 7E:";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x12:
                                {
                                     ROM_data[converted_position].description="DWORD at RAM address 7E:";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                                     ROM_data[converted_position].description+="=$0A";
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x13:
                                {
                                     ROM_data[converted_position].description="Push DWORD at RAM address 7E:";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     break;
                                }
                           case 0x14:
                                {
                                     ROM_data[converted_position].description="Push $0A, 32-bit";
                                     break;
                                }
                           case 0x15:
                                {
                                     ROM_data[converted_position].description="Pull $0E, 32-bit";
                                     break;
                                }
                           case 0x16:
                                {
                                     ROM_data[converted_position].description="$0A=DWORD at RAM address $0A";
                                     break;
                                }
                           case 0x17:
                                {
                                     ROM_data[converted_position].description="DWORD at RAM address (Pull $0E)=$0A";
                                     break;
                                }
                           case 0x18:
                                {
                                     ROM_data[converted_position].description="$0A=0x";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes)<<16;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+4)].ROM_bytes)<<24;
                                     ROM_data[converted_position].description+=convert32BitToHexString(temp_int);
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+3)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+4)].flags|=OPERAND;
                                     break;
                                }
                           case 0x19:
                                {
                                     ROM_data[converted_position].description="$0E=0x";
                                     temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes)<<16;
                                     temp_int+=(ROM_data[source_tracker.convertPosition(position+4)].ROM_bytes)<<24;
                                     ROM_data[converted_position].description+=convert32BitToHexString(temp_int);
                                     ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+3)].flags|=OPERAND;
                                     ROM_data[source_tracker.convertPosition(position+4)].flags|=OPERAND;
                                     break;
                                }
                           case 0x1A:
                                {
                                     ROM_data[converted_position].description="Swap $0A and $0E, 32-bit";
                                     break;
                                }
                           case 0x1B:
                                {
                                     ROM_data[converted_position].description="$0A++, 32-bit";
                                     break;
                                }
                           case 0x1C:
                                {
                                     ROM_data[converted_position].description="$0A--, 32-bit";
                                     break;
                                }
                           case 0x1D:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A!=0), 32-bit";
                                     break;
                                }
                           case 0x1E:
                                {
                                     ROM_data[converted_position].description="$0A%=$0E, 32-bit signed";
                                     break;
                                }
                           case 0x1F:
                                {
                                     ROM_data[converted_position].description="$0A=$0A<<$0E, 32-bit";
                                     break;
                                }
                           case 0x20:
                                {
                                     ROM_data[converted_position].description="$0A=$0A>>$0E, 32-bit signed";
                                     break;
                                }
                           case 0x21:
                                {
                                     ROM_data[converted_position].description="$0A=$0A XOR 0xFFFFFFFF, 32 bit";
                                     break;
                                }
                           case 0x22:
                                {
                                     ROM_data[converted_position].description="$0A&=$0E, 32-bit";
                                     break;
                                }
                           case 0x23:
                                {
                                     ROM_data[converted_position].description="$0A|=$0E, 32-bit";
                                     break;
                                }
                           case 0x24:
                                {
                                     ROM_data[converted_position].description="$0A=$0A XOR $0E, 32 bit";
                                     break;
                                }
                           case 0x25:
                                {
                                     ROM_data[converted_position].description="((WORD)$0A)>=0 ? $0C=0 : $0C=0xFFFF";
                                     break;
                                }
                           case 0x26:
                                {
                                     ROM_data[converted_position].description="$0C=0x0000";
                                     break;
                                }
                           case 0x27:
                                {
                                     ROM_data[converted_position].description="No Operation";
                                     break;
                                }
                           case 0x28:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A!=0), 32-bit";
                                     //EOR 0x0001 at the end is not saved. result is same as a previous opcode.
                                     //This is probably a bug.
                                     break;
                                }
                           case 0x29:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A<$0E), 32-bit";
                                     break;
                                }
                           case 0x2A:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A<=$0E), 32-bit";
                                     break;
                                }
                           case 0x2B:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A>$0E), 32-bit";
                                     break;
                                }
                           case 0x2C:
                                {
                                     ROM_data[converted_position].description="(WORD)$0A=($0A>=$0E), 32-bit";
                                     break;
                                }
                           case 0x2D:
                                {
                                     ROM_data[converted_position].description="$0A=$0A>>$0E, 32-bit unsigned";
                                     break;
                                }
                           case 0x2E:
                                {
                                     ROM_data[converted_position].description="$0A/=$0E, 32-bit unsigned";
                                     break;
                                }
                           case 0x2F:
                                {
                                     ROM_data[converted_position].description="$0A%=$0E, 32-bit unsigned";
                                     break;
                                }
                           default:
                                {
                                     ROM_data[converted_position].description="Invalid Ext.Opcode";
                                     break;
                                }
                      }
                      break;
                 }
            case 0xB8:
                 {
                      ROM_data[converted_position].description="$0A/=$0E, unsigned variables";
                      break;
                 }
            case 0xB9:
                 {
                      ROM_data[converted_position].description="$0A%=$0E, signed variables";
                      break;
                 }
            case 0xBA:
                 {
                      ROM_data[converted_position].description="$0A%=$0E, unsigned variables";
                      break;
                 }
            case 0xBB:
                 {
                      ROM_data[converted_position].description="$0A+=$0E";
                      break;
                 }
            case 0xBC:
                 {
                      ROM_data[converted_position].description="$0A-=$0E";
                      break;
                 }
            case 0xBD:
                 {
                      ROM_data[converted_position].description="$0A=$0A<<$0E";
                      break;
                 }
            case 0xBE:
                 {
                      ROM_data[converted_position].description="$0A=$0A>>>$0E unsigned";
                      break;
                 }
            case 0xBF:
                 {
                      ROM_data[converted_position].description="$0A=$0A>>$0E signed";
                      break;
                 }
            case 0xC0:
                 {
                      //"Compare $0A with $0E, store 1 in $0A if they are equal, otherwise store 0 in $0A.";
                      ROM_data[converted_position].description="$0A=($0A==$0E)";
                      break;
                 }
            case 0xC1:
                 {
                      //"Compare $0A with $0E, store 1 in $0A if not equal, otherwise store 0 in $0A.";
                      ROM_data[converted_position].description="$0A=($0A!=$0E)";
                      break;
                 }
            case 0xC2:
                 {
                      //"If $0A is less than $0E, store 1 in $0A, otherwise store 0 in $0A";
                      //$0A=($0A<$0E)     ????
                      //While this looks correct, it is possible for $0A-$0E to be large enough
                      //to switch the sign, making $0A<$0E inaccurate.
                      ROM_data[converted_position].description="$0A=(($0A-$0E)<0) signed";
                      break;
                 }
            case 0xC3:
                 {
                      //"If $0A is less than or equal to $0E, store 1 in $0A, otherwise store 0 in $0A";
                      //$0A=($0A<=$0E)     ????
                      ROM_data[converted_position].description="$0A=(($0E-$0A)>=0) signed";
                      break;
                 }
            case 0xC4:
                 {
                      ROM_data[converted_position].description="$0A=(($0E-$0A)<0) signed";
                      break;
                 }
            case 0xC5:
                 {
                      ROM_data[converted_position].description="$0A=(($0A-$0E)>=0) signed";
                      break;
                 }
            case 0xC6:
                 {
                      ROM_data[converted_position].description="$0A=($0A<$0E) unsigned";
                      break;
                 }
            case 0xC7:
                 {
                      ROM_data[converted_position].description="0A=($0A<=$0E) unsigned";
                      break;
                 }
            case 0xC8:
                 {
                      ROM_data[converted_position].description="$0A=($0A>$0E) unsigned";
                      break;
                 }
            case 0xC9:
                 {
                      ROM_data[converted_position].description="$0A=($0A>=$0E) unsigned";
                      break;
                 }
            case 0xCA:
                 {
                      //$0A=($0A==0) if $0A is nonzero, $0A=0, if $0A is zero, $0A=1. Same as !$0A
                      ROM_data[converted_position].description="$0A=!$0A";
                      break;
                 }
            case 0xCB:
                 {
                      ROM_data[converted_position].description="$0A=0-$0A";
                      break;
                 }
            case 0xCC:
                 {
                      ROM_data[converted_position].description="$0A=$0A XOR FFFF";
                      break;
                 }
            case 0xCD:
                 {
                      ROM_data[converted_position].description="Swap $0A and $0E";
                      break;
                 }
            case 0xCE:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xCF:
                 {
                      ROM_data[converted_position].description="Return";
                      break;
                 }
            case 0xD0:
                 {
                      ROM_data[converted_position].description="$0A++";
                      break;
                 }
            case 0xD1:
                 {
                      ROM_data[converted_position].description="$0A--";
                      break;
                 }
            case 0xD2:
                 {
                      ROM_data[converted_position].description="$0A<<1";
                      break;
                 }
            case 0xD3:
                 {
                      //$0A=*((BYTE *)($0A))
                      ROM_data[converted_position].description="$0A=BYTE at RAM address $0A";
                      break;
                 }
            case 0xD4:
                 {
                      //"Pull value from stack into $0E, use address formed by new $0E as a destination for the value currently in $0A.";
                      ROM_data[converted_position].description="BYTE at RAM address (Pull $0E)=$0A";
                      break;
                 }
            case 0xD5:
                 {
                      //Variable jump.  Using a table of RAM addresses, we use $0A as an index to determine the
                      //branch location.  All operands are WORDs.  Op1 defines lowest table index. Op2 defines the
                      //maximum table index. Op3 defines the branch location if max table index is exceeded.
                      ROM_data[converted_position].description="BranchTable[$0A]";
                      for(int i=ROM_data[converted_position].description.length()+37;i<TEXT_WIDTH;i++)
                      {
                           ROM_data[converted_position].description+=" ";
                      }
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      uint16_t low_index=temp_int;
                      temp_int=ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+4)].ROM_bytes)<<8;
                      uint16_t high_index=temp_int;
                      temp_int=ROM_data[source_tracker.convertPosition(position+5)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+6)].ROM_bytes)<<8;
                      uint32_t default_branch=temp_int;
                      
                      ROM_data[converted_position].description+="Table Start: 7E:";
                      if(low_index>high_index)
                      {
                          //table starts at position+6.  Minimum acceptable value of $0A to avoid default branch is 0-low_index
                          temp_int=position+6;
                      }
                      else
                      {
                          //table starts at 2*low_index + position + 6.
                          //I don't know if this opcode is ever officially used in this way.
                          temp_int=(low_index<<1)+position+6;
                          
                      }
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      while(ROM_data[converted_position].description.length()%(TEXT_WIDTH-37))
                      {
                           ROM_data[converted_position].description+=" ";
                      }
                      
                      ROM_data[converted_position].description+="Table End: 7E:";
                      temp_int=(high_index<<1)+position+6;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="/0x";
                      temp_int+=0x7E0000;
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      while(ROM_data[converted_position].description.length()%(TEXT_WIDTH-37))
                      {
                           ROM_data[converted_position].description+=" ";
                      }
                      
                      ROM_data[converted_position].description+="If $0A>=0x";
                      ROM_data[converted_position].description+=convertWordToHexString(high_index-low_index);
                      ROM_data[converted_position].description+=", GOTO 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+5)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+6)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="/0x";
                      temp_int+=0x7E0000;
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+3)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+4)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+5)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+6)].flags|=OPERAND;
                      
                      break;
                 }
            case 0xD6:
                 {
                      ROM_data[converted_position].description="GOTO 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int+=0x7E0000;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      
                      position=temp_int;
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xD7:
                 {
                      ROM_data[converted_position].description="if($0A), GOTO 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int+=0x7E0000;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xD8:
                 {
                      //ROM_data[converted_position].description="BEQ";
                      ROM_data[converted_position].description="if(!$0A), GOTO 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      temp_int+=0x7E0000;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xD9:
                 {
                      ROM_data[converted_position].description="Switch($0A)";
                      while(ROM_data[converted_position].description.length()%(TEXT_WIDTH-37))
                      {
                           ROM_data[converted_position].description+=" ";
                      }
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      int16_t num_cases=temp_int;
                      
                      position+=3;
                      for(int i=0;i<num_cases;i++)
                      {
                           ROM_data[converted_position].description+=" case 0x";
                           temp_int=ROM_data[source_tracker.convertPosition(position)].ROM_bytes;
                           temp_int+=(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes)<<8;
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                           ROM_data[converted_position].description+=": GOTO 7E:";
                           temp_int=ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes;
                           temp_int+=(ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes)<<8;
                           temp_int+=0x7E0000;
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                           ROM_data[converted_position].description+="/0x";
                           temp_int2=source_tracker.convertPosition(temp_int);
                           ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                           ROM_data[converted_position].description+="/";
                           ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                           ROM_data[source_tracker.convertPosition(position)].flags|=OPERAND;
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                           ROM_data[source_tracker.convertPosition(position+3)].flags|=OPERAND;
                           
                           ROM_data[temp_int2].addLabel(converted_position);
                           
                           while(ROM_data[converted_position].description.length()%(TEXT_WIDTH-37))
                           {
                                ROM_data[converted_position].description+=" ";
                           }
                           position+=4;
                      }
                      
                      ROM_data[converted_position].description+=" default: GOTO 7E:";
                      temp_int=ROM_data[source_tracker.convertPosition(position)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      temp_int+=0x7E0000;
                      ROM_data[source_tracker.convertPosition(position)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xDA:
                 {
                      //"Compares $0A and $0E with bitwise AND, returns result in $0A";
                      ROM_data[converted_position].description="$0A&=$0E";
                      break;
                 }
            case 0xDB:
                 {
                      //"Compares $0A and $0E with bitwise OR, and returns result in $0A";
                      ROM_data[converted_position].description="$0A|=$0E";
                      break;
                 }
            case 0xDC:
                 {
                      ROM_data[converted_position].description="$0A=$0A XOR $0E";
                      break;
                 }
            case 0xDD:
                 {
                      ROM_data[converted_position].description="ASM Function($0A)";
                      temp_int=busRead16(0x00000A);
                      temp_int+=0x7E0000;
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xDE:
                 {
                      //"Take value from next two bytes, add $04(+1 if negative), and store in $0A.";
                      ROM_data[converted_position].description="$0A=$04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xDF:
                 {
                      //"Take value from next two bytes, add $04(+1 if negative), and store in $0E.";
                      ROM_data[converted_position].description="$0E=$04";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      if(temp_int<0x8000)
                      {
                           ROM_data[converted_position].description+="+0x";
                           ROM_data[converted_position].description+=convertWordToHexString(temp_int+4);
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-0x";
                           ROM_data[converted_position].description+=convertWordToHexString(~((uint16_t)temp_int));
                      }
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      break;
                 }
            case 0xE0:
                 {
                      ROM_data[converted_position].description="$0A=0;$0C=0xFFFF<<0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      ROM_data[converted_position].description=convertByteToHexString(temp_int);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;

                      //cout<<"Inefficient opcode E0 used."<<'\n';
                      break;
                 }
            case 0xE1:
                 {
                      ROM_data[converted_position].description="$0A=0;$0C=0xFFFF>>(0x10-0x";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      ROM_data[converted_position].description=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")   ($0C=0xFFFF if op1>=0x10)";
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      //cout<<"Inefficient opcode E1 used."<<'\n';
                      break;
                 }
            case 0xE2:
                 {
                      ROM_data[converted_position].description="$0C=(0xFFFF>>(0x10-(BYTE)$0E))<<0xE";
                      while(ROM_data[converted_position].description.length()%(TEXT_WIDTH-37))
                      {
                           ROM_data[converted_position].description+=" ";
                      }
                      ROM_data[converted_position].description+="$0A=($0A<<0xE)&$0C";
                      while(ROM_data[converted_position].description.length()%(TEXT_WIDTH-37))
                      {
                           ROM_data[converted_position].description+=" ";
                      }
                      ROM_data[converted_position].description+="WORD at RAM address $0E=(($0C XOR FFFF)&(Word at RAM Address $0E))|$0A";
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      //The operand is never even properly used.
                      //cout<<"Inefficient opcode E2 used."<<'\n';
                      break;
                 }
            case 0xE3:
            case 0xE6:
                 {
                      ROM_data[converted_position].description="GOTO $06";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      
                      
                      if(temp_int<0x80)
                      {
                           ROM_data[converted_position].description+="+=0x";
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           position+=temp_int+1;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-=0x";
                           ROM_data[converted_position].description+=convertByteToHexString((~temp_int)+1);
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           position-=((~temp_int)+2);
                      }
                      ROM_data[converted_position].description+="(7E:";
                      ROM_data[converted_position].description+=convertWordToHexString(position);
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(position);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0xE4:
            case 0xE7:
                 {
                      ROM_data[converted_position].description="if($0A) GOTO ($06";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      
                      if(temp_int<0x80)
                      {
                           ROM_data[converted_position].description+="+=0x";
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           //temp_int=position+temp_int+1;
                           position+=temp_int+1;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-=0x";
                           ROM_data[converted_position].description+=convertByteToHexString((~temp_int)+1);
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           //temp_int=position-(~((uint8_t)temp_int)) + 2;
                           position-=((~temp_int)+2);
                      }
                      ROM_data[converted_position].description+=")  (7E:";
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0xE5:
            case 0xE8:
                 {
                      ROM_data[converted_position].description="if(!$0A) GOTO ($06";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      
                      if(temp_int<0x80)
                      {
                           ROM_data[converted_position].description+="+=0x";
                           ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           //temp_int=position+temp_int+1;
                           position+=temp_int+1;
                      }
                      else
                      {
                           ROM_data[converted_position].description+="-=0x";
                           ROM_data[converted_position].description+=convertByteToHexString((~temp_int)+1);
                           ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                           //temp_int=position-(~((uint8_t)temp_int)) + 2;
                           position-=((~temp_int)+2);
                      }
                      ROM_data[converted_position].description+=")  (7E:";
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      break;
                 }
            case 0xE9:
                 {
                      ROM_data[converted_position].description="ASM Function7E";
                      temp_int=ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes;
                      temp_int+=(ROM_data[source_tracker.convertPosition(position+2)].ROM_bytes)<<8;
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      temp_int+=0x7E0000;
                      ROM_data[converted_position].description+="/0x";
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+="/";
                      ROM_data[converted_position].description+=convertIntToLoROMString(temp_int2);
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+2)].flags|=OPERAND;
                      ROM_data[source_tracker.convertPosition(position+3)].flags|=SCRIPT;
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      ROM_data[source_tracker.convertPosition(position+3)].description="Stack Pointer";
                      if(ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes<0x80)
                      {
                           ROM_data[source_tracker.convertPosition(position+3)].description+="+=0x";
                           ROM_data[source_tracker.convertPosition(position+3)].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[source_tracker.convertPosition(position+3)].description+="-=0x";
                           ROM_data[source_tracker.convertPosition(position+3)].description+=convertByteToHexString((~ROM_data[source_tracker.convertPosition(position+3)].ROM_bytes)+1);
                      }
                      break;
                 }
            case 0xEA:
                 {
                      ROM_data[converted_position].description="ASM Function($0A)";
                      
                      temp_int=busRead16(0x00000A);
                      temp_int+=0x7E0000;
                      temp_int2=source_tracker.convertPosition(temp_int);
                      if(temp_int2>file_size)
                      {
                           error_message="BAD SCRIPT JUMP:";
                           error_message+=convert24BitToHexString(converted_position);
                           break;
                      }
                      ROM_data[temp_int2].addLabel(converted_position);
                      
                      ROM_data[source_tracker.convertPosition(position+1)].flags|=SCRIPT;
                      
                      ROM_data[source_tracker.convertPosition(position+1)].description="Stack Pointer";
                      if(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes<0x80)
                      {
                           ROM_data[source_tracker.convertPosition(position+1)].description+="+=0x";
                           ROM_data[source_tracker.convertPosition(position+1)].description+=convertByteToHexString(ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes);
                      }
                      else
                      {
                           ROM_data[source_tracker.convertPosition(position+1)].description+="-=0x";
                           ROM_data[source_tracker.convertPosition(position+1)].description+=convertByteToHexString((~ROM_data[source_tracker.convertPosition(position+1)].ROM_bytes)+1);
                      }
                      break;
                 }
            case 0xEB:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xEC:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xED:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xEE:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xEF:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF0:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF1:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF2:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF3:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF4:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF5:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF6:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF7:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF8:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xF9:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xFA:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xFB:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xFC:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xFD:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xFE:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            case 0xFF:
                 {
                      ROM_data[converted_position].description="BRK #$00";
                      break;
                 }
            default:
                 {
                      ROM_data[converted_position].description="Impossible Value";
                      break;
                 }
         } //switch(ROM_bytes[position])
    } //end of recordScript function    
    
    void recordASM(uint32_t position)
    {
       if(position==0x00006C || position==0x00006F) return;
       
         
       uint32_t converted_position=source_tracker.convertPosition(position);
       
       ROM_data[converted_position].frequency++;
       
       if(converted_position>=file_size) return;
       
       //error_message=convert24BitToHexString(converted_position);
       
       if(converted_position==0x001A90) //00/9A90, start of script loop.
       {
            ROM_data[converted_position].description="Jump to Script";
            ROM_data[converted_position].flags=SNES65C816;
            uint32_t script_addr=busRead16(0x0006);
            script_addr+=SNES::bus.read(0x0008)<<16;
            recordScript(script_addr);
       }
       
       if(converted_position==0x00168A)
       {
            //Currently can't handle JSR to a RAM location via absolute addressing using non 7E data bank register
            ROM_data[converted_position].description="Reusable MVN";
            ROM_data[converted_position].flags=SNES65C816;
            
            uint32_t dataSource=SNES::bus.read(0x006E)<<16;
            uint32_t dataDest=SNES::bus.read(0x006D)<<16;
              
        
            //dataSource+=source_tracker.regs.X;
            dataSource+=*regsX;
            //dataDest=source_tracker.regs.Y;
            dataDest+=*regsY;
            
            dataSource=source_tracker.convertPosition(dataSource);
            
            MVN_TRACKER_ENTRY newMVN;
            newMVN.ROM_source=dataSource;
            newMVN.RAM_destination=dataDest&0x7FFFFF;
            newMVN.num_bytes=*regsA+1;
            
            if((newMVN.RAM_destination)<0x7E0000) return;
            source_tracker.updateRAM(newMVN);
            return;
       }
       
       //change this if we enable more complex source-destination tracking.
       if(ROM_data[converted_position].flags) return; //only record for positions that haven't been recorded yet.
       
       if((position&0x7FFFFF)>=0x7E0000) ROM_data[converted_position].RAM_address=position;
       
       ROM_data[converted_position].flags|=SNES65C816;
       switch(ROM_data[converted_position].ROM_bytes)
       {
            case 0x00:
                 {
                      ROM_data[converted_position].description="BRK $";
                      //we must be able to convert position to converted_position for each operand...
                      temp_int=getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x01:
                 {
                      ROM_data[converted_position].description="ORA ($";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0x02:
                 {
                      ROM_data[converted_position].description="COP $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
                 }
            case 0x03:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+=",S";
                      break;
                 }
            case 0x04:
                 {
                      ROM_data[converted_position].description="TSB $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
                 }
            case 0x05:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
                 }
            case 0x06:
                 {
                      ROM_data[converted_position].description="ASL $";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      break;
                 }
            case 0x07:
                 {
                      ROM_data[converted_position].description="ORA [$";
                      temp_int2= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x08:
                 {
                      ROM_data[converted_position].description="PHP";
                      break;
                 }
            case 0x09:
                 {
                      ROM_data[converted_position].description="ORA #$";
                      if(*MSET)
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
                 }
            case 0x0A:
                 {
                      ROM_data[converted_position].description="ASL A";
                      break;
                 }
            case 0x0B:
                 {
                      ROM_data[converted_position].description="PHD";
                      break;
                 }
            case 0x0C:
                 {
                      ROM_data[converted_position].description="TSB $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x0D:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x0E:
                 {
                      ROM_data[converted_position].description="ASL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x0F:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x10:
                 {
                      ROM_data[converted_position].description="BPL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x11:
                 {
                      ROM_data[converted_position].description="ORA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0x12:
                 {
                      ROM_data[converted_position].description="ORA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0x13:
                 {
                      ROM_data[converted_position].description="ORA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",S),y";
                      break;
                 }
            case 0x14:
                 {
                      ROM_data[converted_position].description="TRB $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x15:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x16:
                 {
                      ROM_data[converted_position].description="ASL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x17:
                 {
                      ROM_data[converted_position].description="ORA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0x18:
                 {
                      ROM_data[converted_position].description="CLC";
                      break;
                 }
            case 0x19:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x1A:
                 {
                      ROM_data[converted_position].description="INC A";
                      break;
                 }
            case 0x1B:
                 {
                      ROM_data[converted_position].description="TCS";
                      break;
                 }
            case 0x1C:
                 {
                      ROM_data[converted_position].description="TRB $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x1D:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x1E:
                 {
                      ROM_data[converted_position].description="ASL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x1F:
                 {
                      ROM_data[converted_position].description="ORA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x20:
                 {
                      ROM_data[converted_position].description="JSR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int+=(position&0xFF0000);
                      temp_int=source_tracker.convertPosition(temp_int);
                      
                      //add branch label
                      ROM_data[temp_int].addLabel(converted_position);
                      break;
                 }
            case 0x21:
                 {
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0x22:
                 {
                      ROM_data[converted_position].description="JSL $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      temp_int<<=16;
                      ROM_data[converted_position].description+=":";
                      temp_int+=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      
                      temp_int= source_tracker.convertPosition(temp_int);
                      //add branch label
                      ROM_data[temp_int].addLabel(converted_position);
                      break;
                 }
            case 0x23:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",S";
                      break;
                 }
            case 0x24:
                 {
                      ROM_data[converted_position].description="BIT $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x25:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x26:
                 {
                      ROM_data[converted_position].description="ROL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x27:
                 {
                      ROM_data[converted_position].description="AND [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x28:
                 {
                      ROM_data[converted_position].description="PLP";
                      break;
                 }
            case 0x29:
                 {
                      ROM_data[converted_position].description="AND #$";
                      if(*MSET)
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
                 }
            case 0x2A:
                 {
                      ROM_data[converted_position].description="ROL A";
                      break;
                 }
            case 0x2B:
                 {
                      ROM_data[converted_position].description="PLD";
                      break;
                 }
            case 0x2C:
                 {
                      ROM_data[converted_position].description="BIT $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x2D:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x2E:
                 {
                      ROM_data[converted_position].description="ROL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x2F:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x30:
                 {
                      ROM_data[converted_position].description="BMI $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      ROM_data[temp_int2].addLabel(converted_position);
                      //put label at address temp_int2
                      break;
                 }
            case 0x31:
                 {
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0x32:
                 {
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0x33:
                 {
                      ROM_data[converted_position].description="AND ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0x34:
                 {
                      ROM_data[converted_position].description="BIT $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x35:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x36:
                 {
                      ROM_data[converted_position].description="ROL $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x37:
                 {
                      ROM_data[converted_position].description="AND [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0x38:
                 {
                      ROM_data[converted_position].description="SEC";
                      break;
                 }
            case 0x39:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x3A:
                 {
                      ROM_data[converted_position].description="DEC A";
                      break;
                 }
            case 0x3B:
                 {
                      ROM_data[converted_position].description="TSC";
                      break;
                 }
            case 0x3C:
                 {
                      ROM_data[converted_position].description="BIT $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x3D:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x3E:
                 {
                      ROM_data[converted_position].description="ROL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x3F:
                 {
                      ROM_data[converted_position].description="AND $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x40:
                 {
                      ROM_data[converted_position].description="RTI";
                      break;
                 }
            case 0x41:
                 {
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0x42:
                 {
                      ROM_data[converted_position].description="WDM";
                      break;
                 }
            case 0x43:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
                 }
            case 0x44:
                 {
                      ROM_data[converted_position].description="MVP ";
                      temp_int= getSourceByte(position+2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" ";
                      uint32_t dataSource = temp_int<<16;
                      
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      uint32_t dataDest = temp_int<<16;
                    
                      //dataSource+=source_tracker.regs.X;
                      dataSource+=*regsX;
                      //dataDest=source_tracker.regs.Y;
                      dataDest+=*regsY;
                        
                      dataSource=source_tracker.convertPosition(dataSource);
                      
                      if(((dataDest-*regsA)&0x7FFFFF)<0x7E0000) break;
                      
                      MVN_TRACKER_ENTRY newMVN;
                      newMVN.ROM_source=dataSource-(*regsA);
                      newMVN.RAM_destination=dataDest-(*regsA);
                      newMVN.num_bytes=1;
                      
                      source_tracker.updateRAM(newMVN);
                      break;
                 }
            case 0x45:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x46:
                 {
                      ROM_data[converted_position].description="LSR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x47:
                 {
                      ROM_data[converted_position].description="EOR [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x48:
                 {
                      ROM_data[converted_position].description="PHA";
                      break;
                 }
            case 0x49:
                 {
                      ROM_data[converted_position].description="EOR #$";
                      if(*MSET)
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
                 }
            case 0x4A:
                 {
                      ROM_data[converted_position].description="LSR A";
                      break;
                 }
            case 0x4B:
                 {
                      ROM_data[converted_position].description="PHK";
                      break;
                 }
            case 0x4C:
                 {
                      ROM_data[converted_position].description="JMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int+=(position&0xff0000);
                      temp_int=source_tracker.convertPosition(temp_int);
                      //add branch label
                      ROM_data[temp_int].addLabel(converted_position);
                      
                      break;
                 }
            case 0x4D:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x4E:
                 {
                      ROM_data[converted_position].description="LSR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x4F:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x50:
                 {
                      ROM_data[converted_position].description="BVC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      ROM_data[temp_int2].addLabel(converted_position);
                      //Set labels at temp_int and position+2
                      break;
                 }
            case 0x51:
                 {
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0x52:
                 {
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0x53:
                 {
                      ROM_data[converted_position].description="EOR ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0x54:
                 {
                      ROM_data[converted_position].description="MVN ";
                      
                      temp_int= getSourceByte(position+2);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" ";
                      uint32_t dataSource = temp_int<<16;
                      
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      uint32_t dataDest= temp_int<<16;
                      
                      //dataSource+=source_tracker.regs.X;
                      dataSource+=*regsX;
                      //dataDest=source_tracker.regs.Y;
                      dataDest+=*regsY;
                        
                      dataSource=source_tracker.convertPosition(dataSource);
                      
                      if((dataDest&0x7FFFFF)<0x7E0000) break;
                      
                      MVN_TRACKER_ENTRY newMVN;
                      newMVN.ROM_source=dataSource;
                      newMVN.RAM_destination=dataDest;
                      newMVN.num_bytes=1;
            
                      source_tracker.updateRAM(newMVN);
                      break;
                 }
            case 0x55:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x56:
                 {
                      ROM_data[converted_position].description="LSR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x57:
                 {
                      ROM_data[converted_position].description="EOR [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0x58:
                 {
                      ROM_data[converted_position].description="CLI";
                      break;
                 }
            case 0x59:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x5A:
                 {
                      ROM_data[converted_position].description="PHY";
                      break;
                 }
            case 0x5B:
                 {
                      ROM_data[converted_position].description="TCD";
                      break;
                 }
            case 0x5C:
                 {
                      ROM_data[converted_position].description="JML $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      temp_int<<=16;
                      ROM_data[converted_position].description+=":";
                      temp_int+=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      
                      temp_int=source_tracker.convertPosition(temp_int);
                      //add label
                      ROM_data[temp_int].addLabel(converted_position);
                      
                      break;
                 }
            case 0x5D:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x5E:
                 {
                      ROM_data[converted_position].description="LSR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x5F:
                 {
                      ROM_data[converted_position].description="EOR $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x60:
                 {
                      ROM_data[converted_position].description="RTS";
                      //no label for returns?
                      break;
                 }
            case 0x61:
                 {
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0x62:
                 {
                      ROM_data[converted_position].description="PER $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x63:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
                 }
            case 0x64:
                 {
                      ROM_data[converted_position].description="STZ $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x65:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x66:
                 {
                      ROM_data[converted_position].description="ROR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x67:
                 {
                      ROM_data[converted_position].description="ADC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x68:
                 {
                      ROM_data[converted_position].description="PLA";
                      break;
                 }
            case 0x69:
                 {
                      ROM_data[converted_position].description="ADC #$";
                      if(*MSET)
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
                 }
            case 0x6A:
                 {
                      ROM_data[converted_position].description="ROR A";
                      break;
                 }
            case 0x6B:
                 {
                      ROM_data[converted_position].description="RTL";
                      
                      //no label for now
                      break;
                 }
            case 0x6C:
                 {
                      ROM_data[converted_position].description="JMP ($";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      
                      temp_int+=*regsDB<<16;
                      temp_int2=busRead16(temp_int);
                      
                      position=(position&0xff0000) + temp_int2;
                      //set label at position
                      temp_int2=source_tracker.convertPosition(position);
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x6D:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x6E:
                 {
                      ROM_data[converted_position].description="ROR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x6F:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x70:
                 {
                      ROM_data[converted_position].description="BVS $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      //set label at position and temp_int2
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x71:
                 {
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0x72:
                 {
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0x73:
                 {
                      ROM_data[converted_position].description="ADC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0x74:
                 {
                      ROM_data[converted_position].description="STZ $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x75:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x76:
                 {
                      ROM_data[converted_position].description="ROR $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x77:
                 {
                      ROM_data[converted_position].description="ADC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0x78:
                 {
                      ROM_data[converted_position].description="SEI";
                      break;
                 }
            case 0x79:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x7A:
                 {
                      ROM_data[converted_position].description="PLY";
                      break;
                 }
            case 0x7B:
                 {
                      ROM_data[converted_position].description="TDC";
                      break;
                 }
            case 0x7C:
                 {
                      ROM_data[converted_position].description="JMP ($";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      
                      temp_int+=*regsX;
                      temp_int+=*regsDB<<16;
                      temp_int2=busRead16(temp_int);
                      
                      position=(position&0xff0000) + temp_int2;
                      
                      //label at position
                      temp_int2=source_tracker.convertPosition(position);
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x7D:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x7E:
                 {
                      ROM_data[converted_position].description="ROR $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x7F:
                 {
                      ROM_data[converted_position].description="ADC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x80:
                 {
                      ROM_data[converted_position].description="BRA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2=source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                  
                      //label at position
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x81:
                 {
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0x82:
                 {
                      ROM_data[converted_position].description="BRL $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=3;
                      
                      temp_int= (temp_int<0x8000 ? (position+temp_int) : (position-(uint16_t)(0-temp_int)) );
                      temp_int2= source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                  
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x83:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
                 }
            case 0x84:
                 {
                      ROM_data[converted_position].description="STY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x85:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x86:
                 {
                      ROM_data[converted_position].description="STX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0x87:
                 {
                      ROM_data[converted_position].description="STA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0x88:
                 {
                      ROM_data[converted_position].description="DEY";
                      break;
                 }
            case 0x89:
                 {
                      ROM_data[converted_position].description="BIT #$";
                      if(*MSET)
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
                 }
            case 0x8A:
                 {
                      ROM_data[converted_position].description="TXA";
                      break;
                 }
            case 0x8B:
                 {
                      ROM_data[converted_position].description="PHB";
                      break;
                 }
            case 0x8C:
                 {
                      ROM_data[converted_position].description="STY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x8D:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x8E:
                 {
                      ROM_data[converted_position].description="STX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x8F:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x90:
                 {
                      ROM_data[converted_position].description="BCC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2= source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      //label at temp_int and position+2
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0x91:
                 {
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0x92:
                 {
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0x93:
                 {
                      ROM_data[converted_position].description="STA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0x94:
                 {
                      ROM_data[converted_position].description="STY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x95:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x96:
                 {
                      ROM_data[converted_position].description="STX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x97:
                 {
                      ROM_data[converted_position].description="STA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0x98:
                 {
                      ROM_data[converted_position].description="TYA";
                      break;
                 }
            case 0x99:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0x9A:
                 {
                      ROM_data[converted_position].description="TXS";
                      break;
                 }
            case 0x9B:
                 {
                      ROM_data[converted_position].description="TXY";
                      break;
                 }
            case 0x9C:
                 {
                      ROM_data[converted_position].description="STZ $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0x9D:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x9E:
                 {
                      ROM_data[converted_position].description="STZ $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0x9F:
                 {
                      ROM_data[converted_position].description="STA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xA0:
                 {
                      ROM_data[converted_position].description="LDY #$";
                      if(*XSET)
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
                 }
            case 0xA1:
                 {
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0xA2:
                 {
                      ROM_data[converted_position].description="LDX #$";
                      if(*XSET)
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
                 }
            case 0xA3:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
                 }
            case 0xA4:
                 {
                      ROM_data[converted_position].description="LDY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xA5:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xA6:
                 {
                      ROM_data[converted_position].description="LDX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xA7:
                 {
                      ROM_data[converted_position].description="LDA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0xA8:
                 {
                      ROM_data[converted_position].description="TAY";
                      break;
                 }
            case 0xA9:
                 {
                      ROM_data[converted_position].description="LDA #$";
                      if(*MSET)
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
                 }
            case 0xAA:
                 {
                      ROM_data[converted_position].description="TAX";
                      break;
                 }
            case 0xAB:
                 {
                      ROM_data[converted_position].description="PLB";
                      break;
                 }
            case 0xAC:
                 {
                      ROM_data[converted_position].description="LDY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xAD:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xAE:
                 {
                      ROM_data[converted_position].description="LDX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xAF:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xB0:
                 {
                      ROM_data[converted_position].description="BCS $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2= source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      //add label at position+2 and temp_int
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0xB1:
                 {
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0xB2:
                 {
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0xB3:
                 {
                      ROM_data[converted_position].description="LDA ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0xB4:
                 {
                      ROM_data[converted_position].description="LDY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xB5:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xB6:
                 {
                      ROM_data[converted_position].description="LDX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0xB7:
                 {
                      ROM_data[converted_position].description="LDA [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0xB8:
                 {
                      ROM_data[converted_position].description="CLV";
                      break;
                 }
            case 0xB9:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0xBA:
                 {
                      ROM_data[converted_position].description="TSX";
                      break;
                 }
            case 0xBB:
                 {
                      ROM_data[converted_position].description="TYX";
                      break;
                 }
            case 0xBC:
                 {
                      ROM_data[converted_position].description="LDY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xBD:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xBE:
                 {
                      ROM_data[converted_position].description="LDX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0xBF:
                 {
                      ROM_data[converted_position].description="LDA $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xC0:
                 {
                      ROM_data[converted_position].description="CPY #$";
                      if(*XSET)
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
                 }
            case 0xC1:
                 {
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0xC2:
                 {
                      ROM_data[converted_position].description="REP #";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xC3:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
                 }
            case 0xC4:
                 {
                      ROM_data[converted_position].description="CPY $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xC5:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xC6:
                 {
                      ROM_data[converted_position].description="DEC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xC7:
                 {
                      ROM_data[converted_position].description="CMP [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0xC8:
                 {
                      ROM_data[converted_position].description="INY";
                      break;
                 }
            case 0xC9:
                 {
                      ROM_data[converted_position].description="CMP #$";
                      if(*MSET)
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
                 }
            case 0xCA:
                 {
                      ROM_data[converted_position].description="DEX";
                      break;
                 }
            case 0xCB:
                 {
                      ROM_data[converted_position].description="WAI";
                      break;
                 }
            case 0xCC:
                 {
                      ROM_data[converted_position].description="CPY $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xCD:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xCE:
                 {
                      ROM_data[converted_position].description="DEC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xCF:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xD0:
                 {
                      ROM_data[converted_position].description="BNE $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2= source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      //label at position+2 and temp_int
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0xD1:
                 {
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0xD2:
                 {
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0xD3:
                 {
                      ROM_data[converted_position].description="CMP ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0xD4:
                 {
                      ROM_data[converted_position].description="PEI ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0xD5:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xD6:
                 {
                      ROM_data[converted_position].description="DEC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xD7:
                 {
                      ROM_data[converted_position].description="CMP [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0xD8:
                 {
                      ROM_data[converted_position].description="CLD";
                      break;
                 }
            case 0xD9:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0xDA:
                 {
                      ROM_data[converted_position].description="PHX";
                      break;
                 }
            case 0xDB:
                 {
                      ROM_data[converted_position].description="STP";
                      break;
                 }
            case 0xDC:
                 {
                      ROM_data[converted_position].description="JMP [$";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      
                      temp_int+=*regsDB<<16;
                      temp_int2=busRead16(temp_int);
                      temp_int2+=SNES::bus.read(temp_int+2)<<16;
                      
                      //label at position
                      temp_int2=source_tracker.convertPosition(temp_int2);
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0xDD:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xDE:
                 {
                      ROM_data[converted_position].description="DEC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xDF:
                 {
                      ROM_data[converted_position].description="CMP $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xE0:
                 {
                      ROM_data[converted_position].description="CPX #$";
                      if(*XSET)
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
                 }
            case 0xE1:
                 {
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      break;
                 }
            case 0xE2:
                 {
                      ROM_data[converted_position].description="SEP #";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xE3:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s";
                      break;
                 }
            case 0xE4:
                 {
                      ROM_data[converted_position].description="CPX $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xE5:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xE6:
                 {
                      ROM_data[converted_position].description="INC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      break;
                 }
            case 0xE7:
                 {
                      ROM_data[converted_position].description="SBC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="]";
                      break;
                 }
            case 0xE8:
                 {
                      ROM_data[converted_position].description="INX";
                      break;
                 }
            case 0xE9:
                 {
                      ROM_data[converted_position].description="SBC #$";
                      if(*MSET)
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
                 }
            case 0xEA:
                 {
                      ROM_data[converted_position].description="NOP";
                      break;
                 }
            case 0xEB:
                 {
                      ROM_data[converted_position].description="XBA";
                      break;
                 }
            case 0xEC:
                 {
                      ROM_data[converted_position].description="CPX $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xED:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xEE:
                 {
                      ROM_data[converted_position].description="INC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xEF:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xF0:
                 {
                      ROM_data[converted_position].description="BEQ $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=" (GOTO 0x";
                      position+=2;
                      
                      temp_int= (temp_int<0x80 ? (position+temp_int) : (position-(uint8_t)(0-temp_int)) );
                      temp_int2= source_tracker.convertPosition(temp_int);
                      ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
                      ROM_data[converted_position].description+=")";
                      
                      //label at position+2 and temp_int
                      ROM_data[temp_int2].addLabel(converted_position);
                      break;
                 }
            case 0xF1:
                 {
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="),y";
                      break;
                 }
            case 0xF2:
                 {
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=")";
                      break;
                 }
            case 0xF3:
                 {
                      ROM_data[converted_position].description="SBC ($";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",s),y";
                      break;
                 }
            case 0xF4:
                 {
                      ROM_data[converted_position].description="PEA #$";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      break;
                 }
            case 0xF5:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xF6:
                 {
                      ROM_data[converted_position].description="INC $";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xF7:
                 {
                      ROM_data[converted_position].description="SBC [$";
                      temp_int= getSourceByte(position+1);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+="],y";
                      break;
                 }
            case 0xF8:
                 {
                      ROM_data[converted_position].description="SED";
                      break;
                 }
            case 0xF9:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",y";
                      break;
                 }
            case 0xFA:
                 {
                      ROM_data[converted_position].description="PLX";
                      break;
                 }
            case 0xFB:
                 {
                      ROM_data[converted_position].description="XCE";
                      break;
                 }
            case 0xFC:
                 {
                      ROM_data[converted_position].description="JSR ($";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x)";
                      
                      temp_int+=*regsX;
                      temp_int+=*regsDB<<16;
                      temp_int2=busRead16(temp_int);
                      
                      temp_int2+=(position&0xff0000);
                      
                      temp_int= source_tracker.convertPosition(temp_int2);
                      //label at test_position
                      ROM_data[temp_int].addLabel(converted_position);
                      break;
                 }
            case 0xFD:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xFE:
                 {
                      ROM_data[converted_position].description="INC $";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            case 0xFF:
                 {
                      ROM_data[converted_position].description="SBC $";
                      temp_int= getSourceByte(position+3);
                      ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                      ROM_data[converted_position].description+=":";
                      temp_int=getSourceWord(position+1);
                      ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                      ROM_data[converted_position].description+=",x";
                      break;
                 }
            }//end of switch
    }//end of recordASM function

    char convertToASCII(char c)
    {
         if(c<10)
         {
              c+=48;
         }
         else
         {
              c+=55;
         }
         return c;
    }
    
    d_string convertByteToHexString(uint8_t toConvert)
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
    
    d_string convertWordToHexString(uint16_t toConvert)
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
    
    
    d_string convert24BitToHexString(uint32_t toConvert)
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
    
    d_string convertIntToLoROMString(uint32_t toConvert)
    {
         int i;
         d_string s;
         uint32_t address=toConvert&0xFFFF;
         uint32_t bank=toConvert&0xFF0000;
         
         bank<<=1;
         if(address&0x8000)
         {
              bank+=0x010000;
         }
         else
         {
              address+=0x8000;
         }
         
         bank=bank>>16;
         i=bank>>4;
         i=i&0xF;
         s+=convertToASCII(i);
         i=bank&0xF;
         s+=convertToASCII(i);
         s+=':';
         i=address>>12;
         i=i&0xF;
         s+=convertToASCII(i);
         i=address>>8;
         i=i&0xF;
         s+=convertToASCII(i);
         i=address>>4;
         i=i&0xF;
         s+=convertToASCII(i);
         i=address&0xF;
         s+=convertToASCII(i);
         
         return s;
    }         
    
    d_string convert32BitToHexString(uint32_t toConvert)
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
    
    
    uint16_t busRead16(uint32_t location)
    {
         uint16_t result=SNES::bus.read(location);
         result+=(SNES::bus.read(location+1)<<8);
         return result;
    }
    
    d_string convert64BitToHexString(uint64_t toConvert)
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
    
    uint8_t getSourceByte(uint32_t pos)
    {
         uint32_t source_address= source_tracker.convertPosition(pos);
         ROM_data[source_address].flags|=OPERAND;
         return ROM_data[source_address].ROM_bytes;
    }
    
    uint16_t getSourceWord(uint32_t pos)
    {
         uint32_t source_address= source_tracker.convertPosition(pos);
         ROM_data[source_address].flags|=OPERAND;
         uint16_t result = ROM_data[source_address].ROM_bytes;
         
         source_address= source_tracker.convertPosition(pos+1);
         ROM_data[source_address].flags|=OPERAND;
         result+= ROM_data[source_address].ROM_bytes << 8;
         return result;
    }

}; //end of deScriptor class definition

//}//end of scriptor namespace
    
#endif
