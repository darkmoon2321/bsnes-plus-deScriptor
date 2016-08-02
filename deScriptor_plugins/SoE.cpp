#include <windows.h>
#include <string>
#include <stdint.h>

#include "../bsnes/snes/cpu/BRANCH.hpp"
#include "../bsnes/snes/cpu/OPCODE_STEP.hpp"
#include "../bsnes/snes/cpu/ROM_DATA.hpp"

#define TEXT_WIDTH 80

#ifdef BUILD_DLL
    #define DLLAPI __declspec(dllexport)
#else
    #define DLLAPI
#endif // BUILD_DLL

//include BRANCH.hpp, OPCODE_STEP.hpp, and ROM_DATA.hpp

enum byteFlags{
     SCRIPT      =0X01,
     SNES65C816  =0X02,
     OPERAND     =0X04,
     NOTJUMP     =0x08
};

typedef std::string d_string;

Page * bus_page;
ROM_DATA * ROM_data;
unsigned long file_size;
uint8_t * wram;
uint8_t * cartrom;
BRANCH * sources;
d_string error_message;

char convertToASCII(char c);
d_string convertByteToHexString(uint8_t toConvert);
d_string convertWordToHexString(uint16_t toConvert);
d_string convert24BitToHexString(uint32_t toConvert);
d_string convertIntToLoROMString(uint32_t toConvert);
d_string convert32BitToHexString(uint32_t toConvert);
d_string convert64BitToHexString(uint64_t toConvert);
uint8_t busRead8(uint32_t location);
uint16_t busRead16(uint32_t location);
uint8_t getSourceByte(uint32_t);
uint16_t getSourceWord(uint32_t);
uint32_t getSourceAddress(uint32_t);
uint32_t script0CEA5D(uint32_t);
uint32_t script0CD0AF(uint32_t);

extern "C" void setPluginGlobals(ROM_DATA * data,void * p, unsigned long f_size,uint8_t * ram,uint8_t * rom,BRANCH & source_tracker)
{
     //busRead=readFunction;
     bus_page=(Page *)p;
     ROM_data=data;  
     file_size=f_size;
     wram=ram;
     cartrom=rom;
     sources=&source_tracker;
}

extern "C" const char * unloadPlugin()
{
       return "";
}

extern "C" const char * scriptPluginFunction(const OPCODE_STEP &step,BRANCH & source_tracker)
{    
     //error_message="";
     /*uint32_t converted_position=step.converted_address;
     
     if(converted_position<0 || converted_position>file_size) return "";
     ROM_data[converted_position].frequency++;
     if(ROM_data[converted_position].flags) return ""; //don't record the same position twice.
     ROM_data[converted_position].flags|=SCRIPT;
     return "";*/
     switch(step.converted_counter)
     {
          case 0x0CD0AF:
               script0CD0AF(sources->Xl_source);
               break;
          case 0x0CEA5D:
               if(!ROM_data[sources->Xl_source].flags)
               {
                    script0CEA5D(sources->Xl_source);
                    //error_message=convert24BitToHexString(sources->Xl_source);
                    //return error_message.c_str();
               }
               break;
          case 0x0CCA33:
               ROM_data[sources->Xl_source].flags|=SCRIPT;
               ROM_data[sources->Xl_source].description="Graphics Decompression Instructions";
               break;
          /*case 0x0CA616:
               ROM_data[sources->Xl_source].flags|=SCRIPT;
               ROM_data[sources->Xl_source].description='\'';
               ROM_data[sources->Xl_source].description+=(char)ROM_data[sources->Xl_source].ROM_bytes;
               ROM_data[sources->Xl_source].description+='\'';
               break;*/
          case 0x1080EC:
          case 0x1080FC:
               ROM_data[sources->Xl_source].flags|=SCRIPT;
               ROM_data[sources->Xl_source].description="ANIMATION: Command ";
               ROM_data[sources->Xl_source].description+=convertByteToHexString(ROM_data[sources->Xl_source].ROM_bytes);
               break;
          case 0x0C9BD8:
               ROM_data[sources->Xl_source].flags|=SCRIPT;
               ROM_data[sources->Xl_source].description="Compressed Map Data";
               break;
          default:
               return "";
     }
     return error_message.c_str();
}

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

uint8_t busRead8(uint32_t location)
{
     Page &p = bus_page[location >> 8];
     //page member "access" was redefined as void.  Need to cast it and redefine "read" function
     //I am only interested in cartrom and wram, both of which return data_[addr], where addr=p.offset+addr below
     //I need access to data_ of WRAM and cartrom, 
     if(p.access==sources->cartrom) return cartrom[p.offset+location];
     if(p.access==sources->wram) return wram[p.offset+location];
     return 0;
     
}

uint16_t busRead16(uint32_t location)
{
     uint16_t result=busRead8(location);
     result+=(busRead8(location+1)<<8);
     return result;
}

uint8_t getSourceByte(uint32_t pos)
{
     ROM_data[pos].flags|=OPERAND;
     return ROM_data[pos].ROM_bytes;
}

uint16_t getSourceWord(uint32_t pos)
{
     ROM_data[pos].flags|=OPERAND;
     uint16_t result = ROM_data[pos].ROM_bytes;
     pos++;
     
     ROM_data[pos].flags|=OPERAND;
     result+= ROM_data[pos].ROM_bytes << 8;
     return result;
}

uint32_t getSourceAddress(uint32_t pos)
{
     ROM_data[pos].flags|=OPERAND;
     uint32_t result = ROM_data[pos].ROM_bytes;
     pos++;
     ROM_data[pos].flags|=OPERAND;
     result+= ROM_data[pos].ROM_bytes << 8;
     pos++;
     ROM_data[pos].flags|=OPERAND;
     result+= ROM_data[pos].ROM_bytes << 16;
     return result;
}

extern "C" DLLAPI BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}


uint32_t script0CEA5D(uint32_t converted_position)
{
     uint32_t temp_int, temp_int2;
     uint32_t position=converted_position;
     bool final_command=false;
     
     while(!final_command)
     {
          final_command=ROM_data[converted_position].ROM_bytes&0x80;
          if(final_command)
          {
               ROM_data[converted_position].description="  SUB: Return ";
          }
          else
          {
               ROM_data[converted_position].description="  SUB: ";
          }
          ROM_data[converted_position].flags|=SCRIPT;
          position++;
          switch(ROM_data[converted_position].ROM_bytes&0x7F)
          {
            case 0x00:
                 ROM_data[converted_position].description+= (final_command)? "$02" : "Return $02";
                 break;
            case 0x01:
                 ROM_data[converted_position].description+="$02=0xFFFFFF";
                 temp_int=getSourceByte(position);
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                 position++;
                 break;
            case 0x02:
                 ROM_data[converted_position].description+="$02=0x000000";
                 temp_int=getSourceByte(position);
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                 position++;
                 break;
            case 0x03:
                 ROM_data[converted_position].description+="$02=0xFFFF";
                 temp_int=getSourceWord(position);
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                 position+=2;
                 break;
            case 0x04:
                 ROM_data[converted_position].description+="$02=0x0000";
                 temp_int=getSourceWord(position);
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                 position+=2;
                 break;
            case 0x05:
                 ROM_data[converted_position].description+="$02=7E:";
                 temp_int2=getSourceWord(position);
                 position+=2;
                 temp_int=(temp_int2>>3) + 0x2258;
                 temp_int2&=0x7;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                 ROM_data[converted_position].description+=" Bit ";
                 ROM_data[converted_position].description+=convertToASCII(temp_int2);
                 break;
            case 0x06:
                 ROM_data[converted_position].description+="$02=(BYTE)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2258;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 ROM_data[converted_position].description+=", signed. Upper word negative";
                 break;
            case 0x07:
                 ROM_data[converted_position].description+="$02=(BYTE)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2258;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 break;
            case 0x08:
                 ROM_data[converted_position].description+="$02=0xFFFF0000 + (BYTE)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2258;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 break;
            case 0x09:
                 ROM_data[converted_position].description+="$02=(WORD)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2258;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 break;
            case 0x0a:
                 ROM_data[converted_position].description+="$02=7E:";
                 temp_int2=getSourceWord(position);
                 position+=2;
                 temp_int=(temp_int2>>3) + 0x2834;
                 temp_int2&=0x7;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                 ROM_data[converted_position].description+=" Bit ";
                 ROM_data[converted_position].description+=convertToASCII(temp_int2);
                 break;
            case 0x0b:
                 ROM_data[converted_position].description+="$02=(BYTE)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2834;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 ROM_data[converted_position].description+=", signed. Upper word negative";
                 break;
            case 0x0c:
                 ROM_data[converted_position].description+="$02=(BYTE)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2834;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 break;
            case 0x0d:
                 ROM_data[converted_position].description+="$02=0xFFFF0000 + (WORD)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2834;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 break;
            case 0x0e:
                 ROM_data[converted_position].description+="$02=(WORD)7E:";
                 temp_int2=getSourceWord(position);
                 temp_int2+=0x2834;
                 position+=2;
                 ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                 break;
            case 0x0f:
                 ROM_data[converted_position].description+="$02=Current Event Byte 0x";
                 temp_int2=getSourceByte(position);
                 position++;
                 temp_int=(temp_int2>>3) + 0xf;
                 temp_int2&=0x7;
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int);
                 ROM_data[converted_position].description+=" Bit ";
                 ROM_data[converted_position].description+=convertToASCII(temp_int2);
                 break;
            case 0x10:
                 ROM_data[converted_position].description+="$02=Current Event Byte 0x";
                 temp_int2=getSourceByte(position) + 0xf;
                 position++;
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                 ROM_data[converted_position].description+=",signed. Upper word negative";
                 break;
            case 0x11:
                 ROM_data[converted_position].description+="$02=Current Event Byte 0x";
                 temp_int2=getSourceByte(position) + 0xf;
                 position++;
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                 break;
            case 0x12:
                 ROM_data[converted_position].description+="$02=Current Event Word 0x";
                 temp_int2=getSourceByte(position) + 0xf;
                 position++;
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                 ROM_data[converted_position].description+=". Upper word negative";
                 break;
            case 0x13:
                 ROM_data[converted_position].description+="$02=Current Event Word 0x";
                 temp_int2=getSourceByte(position) + 0xf;
                 position++;
                 ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
                 break;
            case 0x14:
                 ROM_data[converted_position].description+="$02=!$02";
                 break;
            case 0x15:
                 ROM_data[converted_position].description+="$02= $02 XOR FFFF";
                 break;
            case 0x16:
                 ROM_data[converted_position].description+="$02= 0 - $02, $04=0xFFFF";
                 break;
            case 0x17:
                 ROM_data[converted_position].description+="???? $02*=Pulled DWORD";
                 break;
            case 0x18:
                 ROM_data[converted_position].description+="$02= Pulled DWORD / $02";
                 break;
            case 0x19:
            case 0x2f:
            case 0x5d:
            case 0x5e:
            case 0x5f:
                 ROM_data[converted_position].description="INVALID SUB-SCRIPT";
                 final_command=true;
                 break;
            case 0x1a:
                 ROM_data[converted_position].description+="$02+= Pulled DWORD";
                 break;
            case 0x1b:
                 ROM_data[converted_position].description+="$02= Pulled DWORD - $02";
                 break;
            case 0x1c:
                 ROM_data[converted_position].description+="$02= Pulled DWORD << $02";
                 break;
            case 0x1d:
                 ROM_data[converted_position].description+="$02= Pulled DWORD >> $02, signed";
                 break;
            case 0x1e:
                 break;
            case 0x1f:
                 break;
            case 0x20:
                 break;
            case 0x21:
                 break;
            case 0x22:
                 ROM_data[converted_position].description+="$02= ($02==Pulled DWORD)";
                 break;
            case 0x23:
                 ROM_data[converted_position].description+="$02= ($02!=Pulled DWORD)";
                 break;
            case 0x24:
                 ROM_data[converted_position].description+="$02&=Pulled DWORD";
                 break;
            case 0x25:
                 ROM_data[converted_position].description+="$02|=Pulled DWORD";
                 break;
            case 0x26:
                 ROM_data[converted_position].description+="$02= $02 XOR Pulled DWORD";
                 break;
            case 0x27:
                 ROM_data[converted_position].description+="$02= ($02 || Pulled DWORD)";
                 break;
            case 0x28:
                 ROM_data[converted_position].description+="$02= ($02 && Pulled DWORD)";
                 break;
            case 0x29:
                 ROM_data[converted_position].description+="Push $02 (32-bit)";
                 break;
            case 0x2a:
                 ROM_data[converted_position].description+="Random WORD";
                 break;
            case 0x2b:
                 break;
            case 0x2c:
                 break;
            case 0x2d:
                 ROM_data[converted_position].description+="$02=$0341, ($0341,0x3E)=itself or true????";
                 break;
            case 0x2e:
                 ROM_data[converted_position].description+="$02= Current Event Word 0xD";
                 break;
            case 0x30:
                 ROM_data[converted_position].description+="$02=0";
                 break;
            case 0x31:
                 ROM_data[converted_position].description+="$02=1";
                 break;
            case 0x32:
                 ROM_data[converted_position].description+="$02=2";
                 break;
            case 0x33:
                 ROM_data[converted_position].description+="$02=3";
                 break;
            case 0x34:
                 ROM_data[converted_position].description+="$02=4";
                 break;
            case 0x35:
                 ROM_data[converted_position].description+="$02=5";
                 break;
            case 0x36:
                 ROM_data[converted_position].description+="$02=6";
                 break;
            case 0x37:
                 ROM_data[converted_position].description+="$02=7";
                 break;
            case 0x38:
                 ROM_data[converted_position].description+="$02=8";
                 break;
            case 0x39:
                 ROM_data[converted_position].description+="$02=9";
                 break;
            case 0x3a:
                 ROM_data[converted_position].description+="$02=0xA";
                 break;
            case 0x3b:
                 ROM_data[converted_position].description+="$02=0xB";
                 break;
            case 0x3c:
                 ROM_data[converted_position].description+="$02=0xC";
                 break;
            case 0x3d:
                 ROM_data[converted_position].description+="$02=0xD";
                 break;
            case 0x3e:
                 ROM_data[converted_position].description+="$02=0xE";
                 break;
            case 0x3f:
                 ROM_data[converted_position].description+="$02=0xF";
                 break;
            case 0x40:
                 ROM_data[converted_position].description+="$02=0xFFF0";
                 break;
            case 0x41:
                 ROM_data[converted_position].description+="$02=0xFFF1";
                 break;
            case 0x42:
                 ROM_data[converted_position].description+="$02=0xFFF2";
                 break;
            case 0x43:
                 ROM_data[converted_position].description+="$02=0xFFF3";
                 break;
            case 0x44:
                 ROM_data[converted_position].description+="$02=0xFFF4";
                 break;
            case 0x45:
                 ROM_data[converted_position].description+="$02=0xFFF5";
                 break;
            case 0x46:
                 ROM_data[converted_position].description+="$02=0xFFF6";
                 break;
            case 0x47:
                 ROM_data[converted_position].description+="$02=0xFFF7";
                 break;
            case 0x48:
                 ROM_data[converted_position].description+="$02=0xFFF8";
                 break;
            case 0x49:
                 ROM_data[converted_position].description+="$02=0xFFF9";
                 break;
            case 0x4a:
                 ROM_data[converted_position].description+="$02=0xFFFA";
                 break;
            case 0x4b:
                 ROM_data[converted_position].description+="$02=0xFFFB";
                 break;
            case 0x4c:
                 ROM_data[converted_position].description+="$02=0xFFFC";
                 break;
            case 0x4d:
                 ROM_data[converted_position].description+="$02=0xFFFD";
                 break;
            case 0x4e:
                 ROM_data[converted_position].description+="$02=0xFFFE";
                 break;
            case 0x4f:
                 ROM_data[converted_position].description+="$02=0xFFFF";
                 break;
            case 0x50:
                 ROM_data[converted_position].description+="$02= Pointer for Boy";//$0F3E, pointer to Boy's RAM struct";
                 break;
            case 0x51:
                 ROM_data[converted_position].description+="$02= Pointer for Dog";//$0F40, pointer to Dog's RAM struct";
                 break;
            case 0x52:
                 ROM_data[converted_position].description+="$02= Pointer to Active Player";//$0F42, pointer to Active Player's Ram struct";
                 break;
            case 0x53:
                 ROM_data[converted_position].description+="$02= Pointer to Inactive Player (Boy or Dog)";
                 break;
            case 0x54:
                 ROM_data[converted_position].description+="$02= Current Event Word x09";
                 break;
            case 0x55:
                 ROM_data[converted_position].description+="$02= WORD at RAM address $02";
                 break;
            case 0x56:
                 ROM_data[converted_position].description+="$02= BYTE at RAM address $02";
                 break;
            case 0x57:
                 ROM_data[converted_position].description+="$02= (Player==Dog)";
                 break;
            case 0x58:
                 ROM_data[converted_position].description+="$02= $0B19";
                 break;
            case 0x59:
                 ROM_data[converted_position].description+="$02= $0B1B";
                 break;
            case 0x5a:
                 break;
            case 0x5b:
                 break;
            case 0x5c:
                 ROM_data[converted_position].description+="$02= (Word @ ($02+0x2A)) >= (Word @ ($02+0x76))";
                 //0x2A is word for current Hit Points for a Monster/NPC/Player, Not sure what 0x76 is.
                 break;
            case 0x60:
                 ROM_data[converted_position].description+="$02=0x10";
                 break;
            case 0x61:
                 ROM_data[converted_position].description+="$02=0x11";
                 break;
            case 0x62:
                 ROM_data[converted_position].description+="$02=0x12";
                 break;
            case 0x63:
                 ROM_data[converted_position].description+="$02=0x13";
                 break;
            case 0x64:
                 ROM_data[converted_position].description+="$02=0x14";
                 break;
            case 0x65:
                 ROM_data[converted_position].description+="$02=0x15";
                 break;
            case 0x66:
                 ROM_data[converted_position].description+="$02=0x16";
                 break;
            case 0x67:
                 ROM_data[converted_position].description+="$02=0x17";
                 break;
            case 0x68:
                 ROM_data[converted_position].description+="$02=0x18";
                 break;
            case 0x69:
                 ROM_data[converted_position].description+="$02=0x19";
                 break;
            case 0x6a:
                 ROM_data[converted_position].description+="$02=0x1A";
                 break;
            case 0x6b:
                 ROM_data[converted_position].description+="$02=0x1B";
                 break;
            case 0x6c:
                 ROM_data[converted_position].description+="$02=0x1C";
                 break;
            case 0x6d:
                 ROM_data[converted_position].description+="$02=0x1D";
                 break;
            case 0x6e:
                 ROM_data[converted_position].description+="$02=0x1E";
                 break;
            case 0x6f:
                 ROM_data[converted_position].description+="$02=0x1F";
                 break;
            default:
                 ROM_data[converted_position].description="  Invalid 0CEA5D Script Command";
                 break;
          } //switch(ROM_bytes[position])
          converted_position=position;
     }
     return position;
}
uint32_t script0CD0AF(uint32_t converted_position)
{
     uint32_t position=converted_position+1;
     ROM_data[converted_position].frequency++;
     if(ROM_data[converted_position].flags) return converted_position;
     
     uint32_t temp_int;
     uint32_t temp_int2;
     ROM_data[converted_position].flags|=SCRIPT;
     switch(ROM_data[converted_position].ROM_bytes)
     {
        case 0x00:
             ROM_data[converted_position].description="EVENT: End Current Event. Return to Parent Event.";
             break;
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x06:
        case 0x0f:
        case 0x13:
        case 0x1f:
        case 0x24:
        case 0x25:
        case 0x28:
        case 0x2f:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x3e:
        case 0x40:
        case 0x41:
        case 0x4f:
        case 0x53:
        case 0x56:
        case 0x57:
        case 0x67:
        case 0x6b:
        case 0x72:
        case 0x8b:
        case 0xb8:
             ROM_data[converted_position].description="STP. Bad Event Command";
             break;
        case 0x04:
             ROM_data[converted_position].description="EVENT: GOTO 0x";
             temp_int=getSourceWord(converted_position+1);
             temp_int2=(converted_position&0xff0000)+((temp_int+3+converted_position)&0xffff);
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x05:
             ROM_data[converted_position].description="EVENT: GO BACK TO ";
             temp_int=getSourceByte(converted_position+1);
             temp_int|=0xff00;
             temp_int2=(converted_position&0xff0000) + ((converted_position+temp_int)&0xffff);
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x07:
             ROM_data[converted_position].description="EVENT: Create New Event at 0x";
             temp_int2=getSourceByte(converted_position+3);
             temp_int=getSourceWord(converted_position+1);
             temp_int2<<=1;
             temp_int&0x8000 ? temp_int2+=0x93 : temp_int2+=0x92;
             temp_int|=0x8000;
             temp_int2=(temp_int2<<16)+temp_int;
             temp_int=sources->convertPosition(temp_int2);
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int);
             if(temp_int>file_size) break;
             ROM_data[temp_int].addLabel(converted_position);
             break;
        case 0x08:
             ROM_data[converted_position].description="EVENT: If !(Sub-Result), GOTO 0x";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position);
             position+=2;
             temp_int2=position+temp_int;
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x09:
             ROM_data[converted_position].description="EVENT: If (Sub-Result), GOTO 0x";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position);
             position+=2;
             temp_int2=position+temp_int;
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x0a:
             ROM_data[converted_position].description="EVENT: Subtract ";
             temp_int=getSourceAddress(position);
             position+=3;
             temp_int2=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=temp_int;
             ROM_data[converted_position].description+=" From Currency(Sub-Result). If positive, GOTO 0x";
             temp_int2=position+temp_int2;
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x0b:
             ROM_data[converted_position].description="EVENT: Subtract ";
             temp_int=getSourceAddress(position);
             position+=3;
             temp_int2=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=temp_int;
             ROM_data[converted_position].description+=" From Currency(Sub-Result). If negative, GOTO 0x";
             temp_int2=position+temp_int2;
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x0c:
             ROM_data[converted_position].description="EVENT: Set bit ";
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int=temp_int2>>3;
             temp_int2&=0x7;
             ROM_data[converted_position].description+=convertToASCII(temp_int2);
             ROM_data[converted_position].description+=" of 7E:";
             temp_int+=0x2258;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+=" if (Sub-Result), else Clear it.";
             position=script0CEA5D(position);
             break;
        case 0x0d:
             ROM_data[converted_position].description="EVENT: Set bit ";
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int=temp_int2>>3;
             temp_int2&=0x7;
             ROM_data[converted_position].description+=convertToASCII(temp_int2);
             ROM_data[converted_position].description+=" of 7E:";
             temp_int+=0x2834;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+=" if (Sub-Result), else Clear it.";
             position=script0CEA5D(position);
             break;
        case 0x0e:
             ROM_data[converted_position].description="EVENT: Set bit ";
             temp_int2=getSourceByte(position);
             position++;
             temp_int=temp_int2>>3;
             temp_int2&=0x7;
             ROM_data[converted_position].description+=convertToASCII(temp_int2);
             ROM_data[converted_position].description+=" of Current Event byte 0x";
             temp_int+=0xf;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+=" if (Sub-Result), else Clear it.";
             position=script0CEA5D(position);
             break;
        case 0x10:
        case 0x14:
             ROM_data[converted_position].description="EVENT: Store (BYTE)(Sub-Result) at 7E:";
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int2+=0x2258;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             position=script0CEA5D(position);
             break;
        case 0x11:
        case 0x15:
             ROM_data[converted_position].description="EVENT: Store (BYTE)(Sub-Result) at 7E:";
             temp_int2=getSourceWord(converted_position+1);
             position+=2;
             temp_int2+=0x2834;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             position=script0CEA5D(position);
             break;
        case 0x12:
        case 0x16:
             ROM_data[converted_position].description="EVENT: Store (BYTE)(Sub-Result) at Current Event byte 0x";
             temp_int2=getSourceByte(position);
             position+=2;
             temp_int2+=0xf;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
             position=script0CEA5D(position);
             break;
        case 0x17:
             ROM_data[converted_position].description="EVENT: 7E:";
             temp_int=getSourceWord(position);
             position+=2;
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int+=0x2258;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="= 0x";
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             break;
        case 0x18:
        case 0x1c:
             ROM_data[converted_position].description="EVENT: 7E:";
             temp_int=getSourceWord(position);
             position+=2;
             temp_int+=0x2258;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="= (Sub-Result)";
             position=script0CEA5D(position);
             break;
        case 0x19:
        case 0x1D:
             ROM_data[converted_position].description="EVENT: 7E:";
             temp_int=getSourceWord(position);
             position+=2;
             temp_int+=0x2834;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="= (Sub-Result)";
             position=script0CEA5D(position);
             break;
        case 0x1a:
        case 0x1E:
             ROM_data[converted_position].description="EVENT: Current Event byte 0x";
             temp_int=getSourceByte(position);
             position++;
             temp_int+=0xf;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+="= (Sub-Result)";
             position=script0CEA5D(position);
             break;
        case 0x1b:
             ROM_data[converted_position].description="EVENT: 7E:";
             temp_int=getSourceWord(converted_position+1)+0x2258;
             temp_int2=getSourceByte(converted_position+5);
             temp_int2<<=3;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="=0x";
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             ROM_data[converted_position].description+=", 7E:";
             temp_int=getSourceWord(converted_position+3)+0x2258;
             temp_int2=getSourceByte(converted_position+6);
             temp_int2<<=3;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="=0x";
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             position+=6;
             break;
        case 0x20:
             ROM_data[converted_position].description="EVENT: Load Default Boy and Dog Sprites";
             break;
        case 0x21:
             ROM_data[converted_position].description="EVENT: GOTO SAME MAP";
             break;
        case 0x22:
             ROM_data[converted_position].description="EVENT: GOTO MAP ";
             temp_int=getSourceWord(converted_position+3);
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             break;
        case 0x23:
             ROM_data[converted_position].description="EVENT: $0E6E=";
             temp_int=getSourceWord(converted_position+1);
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             error_message="Command 23";
             break;
        case 0x26:
             ROM_data[converted_position].description="EVENT: Load Tile Map to VRAM, Fade-In Screen";
             //command has no operands.
             break;
        case 0x27:
             ROM_data[converted_position].description="EVENT: Fade Screen to Black";
             break;
        case 0x29:
             ROM_data[converted_position].description="EVENT: Execute New Event at 0x";
             temp_int2=getSourceByte(converted_position+3);
             temp_int=getSourceWord(converted_position+1);
             temp_int2<<=1;
             temp_int&0x8000 ? temp_int2+=0x93 : temp_int2+=0x92;
             temp_int|=0x8000;
             temp_int2=(temp_int2<<16) + temp_int;
             temp_int=sources->convertPosition(temp_int2);
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int);
             if(temp_int>file_size) break;
             ROM_data[temp_int].addLabel(converted_position);
             break;
        case 0x2a:
             ROM_data[converted_position].description="EVENT: Freeze (Sub-Script) Player/NPC/Monster";
             position=script0CEA5D(position);
             break;
        case 0x2b:
             ROM_data[converted_position].description="EVENT: Unfreeze (Sub-Script) Player/NPC/Monster";
             position=script0CEA5D(position);
             break;
        case 0x2c:
             ROM_data[converted_position].description="EVENT: Freeze Current Event Target Player/NPC/Monster";
             break;
        case 0x2d:
             ROM_data[converted_position].description="EVENT: Unfreeze Current Event Target Player/NPC/Monster";
             break;
        case 0x2e:
             ROM_data[converted_position].description="EVENT: Perform next event if monster/npc/player has ????";
             position=script0CEA5D(position);
             /*
             Get monster/npc/player struct from 0cea5d. (actually, compares struct position with 0x2000, which indicates it doesn't have to be normal monster/player/npc
       	     struct.  Check struct word x6C (if minus, etc), then check bitfield x10 with 0x0100 and 0x0040. If either of them are clear, perform next scheduled event.
       	     Uses 0040 and 0100 in bitfield x10, and uses word x6C.
       	     Perform next event if monster/npc/player has not completed move(????)
             */
             break;
        case 0x30:
        case 0x31:
        case 0x32:
             ROM_data[converted_position].description="EVENT: Play Sound Effect 0x";
             temp_int=getSourceByte(position);
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0x33:
             ROM_data[converted_position].description="EVENT: Play Music 0x";
             temp_int=getSourceByte(position);
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0x38:
        case 0x3A:
             ROM_data[converted_position].description="EVENT: Switch to next Scheduled Event";
             break;
        case 0x39:
        case 0x3B:
             ROM_data[converted_position].description="EVENT: Delay Current Event by SUB";
             break;
        case 0x3c:
             ROM_data[converted_position].description="EVENT: Spawn Monster/NPC with Flags";
             temp_int=getSourceWord(position); //NPC/Monster type
             position+=2;
             temp_int2=getSourceWord(position); //Flags for NPC bitfield x10
             position+=2;
             temp_int=getSourceByte(position);  //X position
             position++;
             temp_int=getSourceByte(position); //Y position
             position++;
             break;
        case 0x3d:
             ROM_data[converted_position].description="EVENT: Set Monster/NPC Talk Event";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position); //This specifies the talk event chosen.
             position+=2;
             break;
        case 0x3f:
             ROM_data[converted_position].description="EVENT: $0EA2(Sub1)=0x";
             //Changes value in $0EA2 and $0EAC arrays.  It looks like this
             //information is only ever used in a single animation script command?
             position=script0CEA5D(position);
             temp_int=getSourceWord(position);
             position+=2;
             temp_int2=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+=", $0EAC(Sub1)=0x";
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             break;
        case 0x42:
             ROM_data[converted_position].description="EVENT: Set (SUB) Monster/NPC/Player to X,Y";
             position=script0CEA5D(position);
             temp_int=getSourceByte(position);
             position++;
             temp_int2=getSourceByte(position);
             position++;
             break;
        case 0x43:
             ROM_data[converted_position].description="EVENT: Set (SUB1) Monster/NPC/Player to X(SUB2),Y(SUB3)";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
             ROM_data[converted_position].description="EVENT: Display Message Box in Slot at X,Y with Width,Height";
             //Has 5 bytes for operands
             temp_int=getSourceByte(position); //Slot???? usually 00. Setting too high corrupts the tilemap.  Maybe there
             //are multiple message box slots that can be occupied at a time, and this
             //selects which slot to fill?
             position++;
             temp_int=getSourceByte(position); //text box x position
             position++;
             temp_int=getSourceByte(position); //text box y position
             position++;
             temp_int=getSourceByte(position); //text box width
             position++;
             temp_int=getSourceByte(position); //text box height
             position++;
             break;
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
             ROM_data[converted_position].description="EVENT: Display Default Message Box";
             //no operands
             break;
        case 0x4c:
             ROM_data[converted_position].description="EVENT: Skip Next Byte";
             position++;
             if(position>file_size) break;
             ROM_data[position].addLabel(converted_position);
             break;
        case 0x4d:
             ROM_data[converted_position].description="EVENT: No Operation (Goto next byte)";
             break;
        case 0x4e:
             ROM_data[converted_position].description="EVENT: Tie Monster/NPC/Player (SUB) to Current Event";
             position=script0CEA5D(position);
             break;
        case 0x50:
             ROM_data[converted_position].description="EVENT: Display Message Box Dialog in Slot 0x";
             temp_int=getSourceByte(position); //Like command x44, this is the message "slot"
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             temp_int2=getSourceWord(position); //Added to 91:D000 to get a pointer to an index to a pointer to the message start
             position+=2;
             break;
        case 0x51:
             ROM_data[converted_position].description="EVENT: Display Message Box Dialog in Default Slot";
             temp_int=getSourceWord(position); //Added to 91:D000 to get a pointer to an index to a pointer to the message start
             position+=2;
             break;
        case 0x52:
             ROM_data[converted_position].description="EVENT: Set Non-Windowed Message";
             temp_int=getSourceWord(position); //Added to 91:D000 to get a pointer to an index to a pointer to the message start.
             position+=2;
             break;
        case 0x54:
             ROM_data[converted_position].description="EVENT: Close Message Box in Slot 0x";
             temp_int=getSourceByte(position);
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0x55:
             ROM_data[converted_position].description="EVENT: Close Message Box in Default Slot";
             break;
        case 0x58:
             ROM_data[converted_position].description="EVENT: Fade-In Music Volume";
             //increase volume by 0x10 out of 0xff
             break;
        case 0x59:
             ROM_data[converted_position].description="EVENT: Fade-Out Music Volume";
             //decrease volume by 0x10 out of 0xff???? Seems to turn it off though when I use it.
             break;
        case 0x5a:
             ROM_data[converted_position].description="EVENT: Disable/Clear Non-windowed Messages";
             break;
        case 0x5b:
             ROM_data[converted_position].description="EVENT: Enable New Non-windowed Messages";
             break;
        case 0x5c:
             ROM_data[converted_position].description="EVENT: Mark Room Treasure (SUB1) as Found (SUB2) times";
             //Doesn't award the ingredient/treasure itself, but keeps the dog from sniffing for it, and
             //changes array values in 107E,10CE, and 111E such that a "chest" sprite will show open.
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x5d:
             ROM_data[converted_position].description="EVENT: Set Room Treasure Event (SUB)";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position);
             position+=2;
             break;
        case 0x5e:
             ROM_data[converted_position].description="EVENT: Set/Clear Treasure (SUB) if 7E:";
             position=script0CEA5D(position);
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int=temp_int2&0x7;
             temp_int2>>=3;
             temp_int2+=0x2258;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
             ROM_data[converted_position].description+=" Bit ";
             ROM_data[converted_position].description+=convertToASCII(temp_int);
             break;
        case 0x5f:
             ROM_data[converted_position].description="EVENT: Add 0x";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             //This might be a minor glitch in Evermore.  They use a word operand but cut off the upper byte.
             ROM_data[converted_position].description+=" to the amount of times Treasure (SUB) has been found";
             break;
        case 0x60:
             ROM_data[converted_position].description="EVENT: Add 1 to the amount of times Treasure (SUB) has been found";
             position=script0CEA5D(position);
             break;
        case 0x61:
             ROM_data[converted_position].description="EVENT: Subtract 1 from the amount of times Treasure (SUB) has been found";
             position=script0CEA5D(position);
             break;
        case 0x62:
             ROM_data[converted_position].description="";
             //searches map arrangement data for something.
             temp_int=getSourceByte(position);
             position++;
             temp_int=getSourceByte(position);
             position+=2;
             temp_int2=getSourceWord(position);
             position+=2;
             error_message="Command 62";
             break;
        case 0x63:
             ROM_data[converted_position].description="EVENT: Black-Out Screen Momentarily????";
             //Updates a lot of graphics settings, uses data at 7F:C800
             //Reload room graphics?
             error_message="Command 63";
             break;
        case 0x64:
             ROM_data[converted_position].description="";
             //Causes some background graphics to appear and rise up on screen.  It's as if
             //you are rising up on the windwalker/escape pod
             error_message="Command 64";
             break;
        case 0x65:
             ROM_data[converted_position].description="";
             error_message="Command 65";
             break;
        case 0x66:
             ROM_data[converted_position].description="";
             error_message="Command 66";
             break;
        case 0x68:
             ROM_data[converted_position].description="";
             error_message="Command 68";
             break;
        case 0x69:
             ROM_data[converted_position].description="";
             error_message="Command 69";
             break;
        case 0x6a:
             ROM_data[converted_position].description="";
             error_message="Command 6A";
             break;
        case 0x6c:
             ROM_data[converted_position].description="EVENT: Move Player/Monster/NPC to Absolute Position";
             position=script0CEA5D(position);
             temp_int=getSourceByte(position);
             position++;
             temp_int2=getSourceByte(position);
             position++;
             break;
        case 0x6d:
             ROM_data[converted_position].description="EVENT: Move Player/Monster/NPC to Relative Position";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x6e:
             ROM_data[converted_position].description="EVENT: Move Player/Monster/NPC to Absolute Position. Ignore barriers.";
             position=script0CEA5D(position);
             temp_int=getSourceByte(position);
             position++;
             temp_int2=getSourceByte(position);
             position++;
             break;
        case 0x6f:
             ROM_data[converted_position].description="EVENT: Move Player/Monster/NPC to Relative Position. Ignore barriers.";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x70:
             ROM_data[converted_position].description="EVENT: Turn (Sub-Script) Monster/NPC/Player to face (Sub-Script) Monster/NPC/Player";
             break;
        case 0x71:
             ROM_data[converted_position].description="EVENT: Turn Two (Sub-Script) Monsters/NPCs/Players to Face Each Other";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x73:
             ROM_data[converted_position].description="EVENT: Move Player/Monster/NPC (SUB1) to Absolute X(SUB2),Y(SUB3). Ignore barriers.";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x74:
             ROM_data[converted_position].description="EVENT: Turn (Sub-Script) Monster/NPC/Player Up";
             position=script0CEA5D(position);
             break;
        case 0x75:
             ROM_data[converted_position].description="EVENT: Turn (Sub-Script) Monster/NPC/Player Down";
             position=script0CEA5D(position);
             break;
        case 0x76:
             ROM_data[converted_position].description="EVENT: Turn (Sub-Script) Monster/NPC/Player Left";
             position=script0CEA5D(position);
             break;
        case 0x77:
             ROM_data[converted_position].description="EVENT: Turn (Sub-Script) Monster/NPC/Player Right";
             position=script0CEA5D(position);
             break;
        case 0x78:
             ROM_data[converted_position].description="EVENT: Set (SUB1) Monster/NPC/Player Animation Script";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position); //This is the index to the index to the animation.
             position+=2;
             position=script0CEA5D(position); //This determines how you set/clear monster/npc/player bitfield(s)
             break;
        case 0x79:
             ROM_data[converted_position].description="EVENT: Set (SUB1) Monster/NPC/Player Animation and Switch to Next Event";
             position=script0CEA5D(position);
             temp_int=getSourceWord(position); //This is the index to the index to the animation.
             position+=2;
             position=script0CEA5D(position); //This determines how you set/clear monster/npc/player bitfield(s)
             break;
        case 0x7a:
             ROM_data[converted_position].description="EVENT: Store WORD (SUB2) at address (SUB1)";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x7b:
             ROM_data[converted_position].description="EVENT: Store BYTE (SUB2) at address (SUB1)";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x7c:
             ROM_data[converted_position].description="EVENT: Add "; //24-bit to currency
             position=script0CEA5D(position);
             temp_int=getSourceAddress(position);
             position+=3;
             ROM_data[converted_position].description+=temp_int;
             ROM_data[converted_position].description+=" from Currency (Sub-Script)";
             break;
        case 0x7d:
             ROM_data[converted_position].description="EVENT: Subtract "; //24-bit from currency
             position=script0CEA5D(position);
             temp_int=getSourceAddress(position);
             position+=3;
             ROM_data[converted_position].description+=temp_int;
             ROM_data[converted_position].description+=" from Currency (Sub-Script)";
             break;
        case 0x7e:
             ROM_data[converted_position].description="EVENT: Exchange Currency";
             position=script0CEA5D(position); //currency multiplier?
             position=script0CEA5D(position); //Old Currency Type
             position=script0CEA5D(position); //currency divisor?
             position=script0CEA5D(position); //New Currency Type
             break;
        case 0x7f:
             ROM_data[converted_position].description="EVENT: Enter Dog's Name";
             temp_int=getSourceWord(position); //????
             position+=2;
             break;
        case 0x80:
             ROM_data[converted_position].description="EVENT: Display Stored non-windowed message";
             break;
        case 0x81:
             ROM_data[converted_position].description="EVENT: Hide and Store non-windowed message";
             break;
        case 0x82:
             ROM_data[converted_position].description="EVENT: Set Color Add/Subtract Settings to 7E:235B-235E";
             break;
        case 0x83:
             ROM_data[converted_position].description="EVENT: Set Color Add/Subtract Settings to $0F80-$0F83";
             break;
        case 0x84:
             ROM_data[converted_position].description="EVENT: Add (Sub-Script 2) to Currency (Sub-Script 1)";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x85:
             ROM_data[converted_position].description="EVENT: Subtract (Sub-Script 2) from Currency (Sub-Script 1)";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x86:
             ROM_data[converted_position].description="EVENT: Set Music Volume";
             //Higher is louder
             position=script0CEA5D(position);
             break;
        case 0x87:
             ROM_data[converted_position].description="EVENT: Set Music Speed";
             //Higher is slower, lower is faster.
             position=script0CEA5D(position);
             break;
        case 0x88:
             ROM_data[converted_position].description="EVENT: Clear Ring Menu Item List and Exit";
             break;
        case 0x89:
             ROM_data[converted_position].description="EVENT: Set Item in Shopping Menu";
             position=script0CEA5D(position); //get Item/Ingredient
             position=script0CEA5D(position); //get Price
             break;
        case 0x8a:
             ROM_data[converted_position].description="EVENT: Switch Current Shopping Menu to Monster/NPC/Player (SUB)";
             position=script0CEA5D(position);
             break;
        case 0x8c:
             ROM_data[converted_position].description="EVENT: File Save Menu";
             break;
        case 0x8d:
             ROM_data[converted_position].description="EVENT: Enable/Disable Earthquake";
             temp_int=getSourceByte(position); //boolean to enable/disable
             position++;
             break;
        case 0x8e:
             ROM_data[converted_position].description="EVENT: Subtract SUB2 From Currency SUB1. If negative, GOTO 0x";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int2+=position;
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x8f:
             ROM_data[converted_position].description="EVENT: Subtract SUB2 From Currency SUB1. If positive, GOTO 0x";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             temp_int2=getSourceWord(position);
             position+=2;
             temp_int2+=position;
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0x90:
             ROM_data[converted_position].description="EVENT: Fade to Screen Brightness SUB";
             //This command disables ring menus.
             position=script0CEA5D(position);
             break;
        case 0x91:
             ROM_data[converted_position].description="EVENT: Set Screen Brightness to SUB";
             //Unlike command 90, the effect is immediate.
             position=script0CEA5D(position);
             break;
        case 0x92:
             ROM_data[converted_position].description="EVENT: Monster/Player/NPC SUB1 Takes SUB2 Damage, with animation";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x93:
             ROM_data[converted_position].description="EVENT: Monster/Player/NPC SUB1 Takes SUB2 Damage, no animation";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x94:
             ROM_data[converted_position].description="EVENT: Monster/Player/NPC SUB1 Heals by SUB2, with animation";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x95:
             ROM_data[converted_position].description="EVENT: MONSTER/Player/NPC SUB1 Heals by SUB2, no animation";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x96:
             ROM_data[converted_position].description="EVENT: Teleport Player X-SUB,Y-SUB relative screens, and scroll";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x97:
             ROM_data[converted_position].description="EVENT: Change Palettes/Time of Day";
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x98:
             ROM_data[converted_position].description="EVENT: If SUB is not Active Player, Switch Active Player";
             position=script0CEA5D(position); //monster/npc/player
             break;
        case 0x99:
             ROM_data[converted_position].description="EVENT: Enter Mode 7 Worldmap on Windwalker";
             //one of these may control whether you fly the windwalker or escape pod
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x9a:
             ROM_data[converted_position].description="EVENT: Change Font";
             position=script0CEA5D(position);
             //a valid SUB return value is 2. The others mostly ruin the font.
             //This command seems very glitchy.
             break;
        case 0x9b:
             ROM_data[converted_position].description="EVENT: Deallocate/Destroy (Sub-Script) Monster/NPC";
             position=script0CEA5D(position);
             break;
        case 0x9c:
             ROM_data[converted_position].description="EVENT: Decrement Events Tied to NPC/Monster/Player SUB";
             //Decrements monster/npc/player byte x3E
             //Doesn't delete or modify the actual events in any way...
             position=script0CEA5D(position);
             break;
        case 0x9d:
             ROM_data[converted_position].description="EVENT: Move Player/Monster/NPC (SUB1) to Absolute X(SUB2),Y(SUB3)";
             //Tries to avoid obstacles, unlike command 73
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             position=script0CEA5D(position);
             break;
        case 0x9e:
             ROM_data[converted_position].description="EVENT: Alchemy Attack";
             //Continues to call SUB scripts until one returns zero, or possibly another condition...
             //Stores the value of the SUB returns in an array at 7E:3364+0x2E. Increments
             //the offset by 2 after every SUB result.
             //Struct at 7E3364 is x40 bytes long. After 7E3564, struct length increases to x76.
             position=script0CEA5D(position); //Get npc/monster/player target
             position=script0CEA5D(position); //type of alchemy
             position=script0CEA5D(position); //???? attack power?
             while(ROM_data[converted_position-1].ROM_bytes!=0xB0) //This is a big question mark until I see this command used.
             {
                  position=script0CEA5D(position);
             }
             break;
        case 0x9f:
             ROM_data[converted_position].description="EVENT: Prepare Currency Display";
             //Is it always talons? Doesn't appear so.
             break;
        case 0xa0:
             ROM_data[converted_position].description="EVENT: Show Currency Amount";
             //Uses value in 7E:2348 to determine which currency to display
             break;
        case 0xa1:
             ROM_data[converted_position].description="EVENT: Hide Currency Display";
             break;
        case 0xa2:
             ROM_data[converted_position].description="EVENT: Spawn NPC/Monster with Flags";
             //Just like command 3C but uses SUB's for X and Y position
             temp_int=getSourceWord(position); //This is an NPC/Monster identifier, same as in command 3C
             position+=2;
             temp_int2=getSourceWord(position);
             position+=2;
             position=script0CEA5D(position); //X position
             position=script0CEA5D(position); //Y position
             break;
        case 0xa3:
             ROM_data[converted_position].description="EVENT: Execute Common Event ";
             temp_int=getSourceByte(position);
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0xa4:
             ROM_data[converted_position].description="EVENT: Execute Less Common Event ";
             temp_int=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             break;
        case 0xa5:
             ROM_data[converted_position].description="EVENT: Execute New Event at 0x";
             temp_int=getSourceByte(converted_position+1);
             temp_int|=0xff00;
             temp_int2=(converted_position&0xff0000) + ((converted_position+temp_int)&0xffff);
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0xa6:
             ROM_data[converted_position].description="EVENT: Execute New Event at 0x";
             temp_int=getSourceWord(converted_position+1);
             temp_int2=(converted_position&0xff0000)+((temp_int+converted_position)&0xffff);
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0xa7:
             ROM_data[converted_position].description="EVENT: Delay Current Event by 0x";
             temp_int=getSourceByte(position);
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0xa8:
             ROM_data[converted_position].description="EVENT: Delay Current Event by 0x";
             temp_int=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             break;
        case 0xa9:
             ROM_data[converted_position].description="EVENT: Modify Monster/Player/NPC Bitfields or Direction";
             position=script0CEA5D(position); //NPC pointer
             position=script0CEA5D(position); //Indicates how to modify direction or a bitfield.  Many options.
             break;
        case 0xaa:
             ROM_data[converted_position].description="EVENT: Clear Boy+Dog Statuses";
             break;
        case 0xab:
             ROM_data[converted_position].description="EVENT: Reset Game";
             break;
        case 0xac:
             ROM_data[converted_position].description="EVENT: Alchemy Attack";
             //This command is VERY similar to command 9E.  The entries in 7E3564 array are alchemy attacks
             //just like those in the 7E3364 array.  They get more bytes though.  Possibly more scripting???
             //Continues to call SUB scripts until one returns zero, or possibly another condition...
             //Stores the value of the SUB returns in an array at 7E:3564+0x2E. Increments
             //the offset by 2 after every SUB result.
             //Struct at 7E3364 is x40 bytes long. After 7E3564, struct length increases to x76. This command targets the 3564 ones.
             position=script0CEA5D(position); //Get npc/monster/player target
             position=script0CEA5D(position); //type of alchemy
             position=script0CEA5D(position); //???? attack power?
             while(ROM_data[converted_position-1].ROM_bytes!=0xB0) //This is a big question mark until I see this command used.
             {
                  position=script0CEA5D(position);
             }
             break;
        case 0xad:
             ROM_data[converted_position].description="7E:";
             temp_int=getSourceWord(converted_position+1)+0x2834;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="=0x";
             temp_int=getSourceByte(converted_position+5);
             temp_int<<=3;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+=", 7E:";
             temp_int=getSourceByte(converted_position+3)+0x2834;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+="=0x";
             temp_int=getSourceByte(converted_position+6);
             temp_int<<=3;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0xae:
             ROM_data[converted_position].description="EVENT: Event Word 0x";
             temp_int=getSourceByte(converted_position+1)+0xf;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+="=0x";
             temp_int=getSourceByte(converted_position+3);
             temp_int<<=3;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+=", Event Word 0x";
             temp_int=getSourceByte(converted_position+2)+0xf;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+="=0x";
             temp_int=getSourceByte(converted_position+4);
             temp_int<<=3;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             break;
        case 0xaf:
             ROM_data[converted_position].description="EVENT: Execute new Event at 0x";
             temp_int=getSourceByte(position);
             position++;
             for(temp_int2=0;temp_int2<temp_int;temp_int2++)
             {
                  position=script0CEA5D(position);
             }
             temp_int=getSourceWord(position);
             position+=2;
             temp_int2=getSourceByte(position);
             position++;
             temp_int2<<=1;
             temp_int&0x8000 ? temp_int2+=0x93 : temp_int2+=0x92;
             temp_int|=0x8000;
             temp_int2=(temp_int2<<16) + temp_int;
             temp_int=sources->convertPosition(temp_int2);
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int);
             ROM_data[converted_position].description+=" and Fully Initialize";
             if(temp_int>file_size) break;
             ROM_data[temp_int].addLabel(converted_position);
             break;
        case 0xb0:
             ROM_data[converted_position].description="EVENT: Execute Common Event ";
             temp_int=getSourceByte(position);
             position++;
             for(temp_int2=0;temp_int2<temp_int;temp_int2++)
             {
                  position=script0CEA5D(position);
             }
             temp_int=getSourceByte(position);
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+=" and Fully Initialize";
             break;
        case 0xb1:
             ROM_data[converted_position].description="EVENT: Execute Less Common Event ";
             temp_int=getSourceByte(position);
             position++;
             for(temp_int2=0;temp_int2<temp_int;temp_int2++)
             {
                  position=script0CEA5D(position);
             }
             temp_int=getSourceWord(position);
             position+=2;
             ROM_data[converted_position].description+=convertWordToHexString(temp_int);
             ROM_data[converted_position].description+=" and Fully Initialize";
             break;
        case 0xb2:
             ROM_data[converted_position].description="EVENT: Execute Event at 0x";
             temp_int=getSourceByte(position);
             position++;
             for(temp_int2=0;temp_int2<temp_int;temp_int2++)
             {
                  position=script0CEA5D(position);
             }
             temp_int=getSourceByte(converted_position+1);
             temp_int|=0xff00;
             temp_int2=(converted_position&0xff0000) + ((converted_position+temp_int)&0xffff);
             temp_int2|=0x008000;
             ROM_data[converted_position].description+=convert24BitToHexString(temp_int2);
             ROM_data[converted_position].description+=" and Fully Initialize";
             if(temp_int2>file_size) break;
             ROM_data[temp_int2].addLabel(converted_position);
             break;
        case 0xb3:
             ROM_data[converted_position].description="";
             error_message="Command B3";
             break;
        case 0xb4:
             ROM_data[converted_position].description="";
             error_message="Command B4";
             break;
        case 0xb5:
             ROM_data[converted_position].description="EVENT: Draw Lightning/Reveal Hidden NPC????";
             //Used to display "Secret of Evermore" at game start.
             position=script0CEA5D(position); //sprite/animation displayed at source of lightning
             position=script0CEA5D(position); // get npc/monster struct pointer
             position=script0CEA5D(position); // x position... (from current event byte x25
             position=script0CEA5D(position); // y position... (from current event byte x27
             position=script0CEA5D(position); //boolean?
             position=script0CEA5D(position); // another potential x position from current event byte x1D
             position=script0CEA5D(position); // another potential y position from current event byte x1F
             position=script0CEA5D(position); //boolean?
             break;
        case 0xb6:
             ROM_data[converted_position].description="";
             error_message="Command B6";
             break;
        case 0xb7:
             ROM_data[converted_position].description="";
             error_message="Command B7";
             break;
        case 0xb9:
             ROM_data[converted_position].description="";
             error_message="Command B9";
             break;
        case 0xba:
             ROM_data[converted_position].description="EVENT: Spawn Monster/NPC";
             break;
        case 0xbb:
             ROM_data[converted_position].description="";
             error_message="Command BB";
             break;
        case 0xbc:
             ROM_data[converted_position].description="EVENT: Freeze Boy";
             break;
        case 0xbd:
             ROM_data[converted_position].description="EVENT: Unfreeze Boy";
             break;
        case 0xbe:
             ROM_data[converted_position].description="EVENT: Freeze Dog";
             break;
        case 0xbf:
             ROM_data[converted_position].description="EVENT: Unfreeze Dog";
             break;
        case 0xc0:
             ROM_data[converted_position].description="EVENT: Freeze Boy+Dog";
             break;
        case 0xc1:
             ROM_data[converted_position].description="EVENT: Unfreeze Boy+Dog";
             break;
        case 0xc2:
             ROM_data[converted_position].description="EVENT: Conditional Spawn NPC 0x";
             temp_int=getSourceByte(position); //This is monster/npc to spawn
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             ROM_data[converted_position].description+=" at X=";
             temp_int=getSourceByte(position); //This is an X position
             position++;
             ROM_data[converted_position].description+=convertByteToHexString(temp_int);
             temp_int2=getSourceByte(position); //This is a Y position
             position++;
             ROM_data[converted_position].description+=",Y=";
             ROM_data[converted_position].description+=convertByteToHexString(temp_int2);
             break;
        default:
             ROM_data[converted_position].description="Script 0CD0AF, Command ";
             ROM_data[converted_position].description+=convertByteToHexString(ROM_data[converted_position].ROM_bytes);
             break;
     } //switch(ROM_bytes[position])
     return converted_position;
}
