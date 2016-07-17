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
BRANCH * tracker_address;

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

extern "C" void setPluginGlobals(ROM_DATA * data,void * p, unsigned long f_size,uint8_t * ram,uint8_t * rom,BRANCH & source_tracker)
{
     //busRead=readFunction;
     bus_page=(Page *)p;
     ROM_data=data;  
     file_size=f_size;
     wram=ram;
     cartrom=rom;
     tracker_address=&source_tracker;
}

extern "C" const char * unloadPlugin()
{
       return "";
}

extern "C" const char * scriptPluginFunction(const OPCODE_STEP &step,BRANCH & source_tracker)
{
     //need to convert data from opcode step to position and converted_position
     //need a busRead function defined
     //need access to functions to convert integers to hex strings.
     //
     
     uint32_t temp_int;
     uint32_t temp_int2;
     if(step.converted_counter!=0x001AA8) return "";
     
     d_string error_message="";
     
     uint32_t converted_position=step.converted_address;
     uint32_t position;
     if(ROM_data[converted_position].step.indirect_address < 0)
     {
          position=ROM_data[converted_position].step.accessed_address;
     }
     else
     {
          position=ROM_data[converted_position].step.indirect_address;
     }
     
     if(converted_position<0) return "";
     
     ROM_data[converted_position].frequency++;
     
     if(ROM_data[converted_position].flags) return ""; //don't record the same position twice.
     
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
                  temp_int2=getSourceByte(position+1);
                  if(temp_int2<0x80)
                  {
                       ROM_data[converted_position].description+="+0x";
                       ROM_data[converted_position].description+=convertByteToHexString(temp_int2 + 4);
                       temp_int+=temp_int2 + 4;
                  }
                  else
                  {
                       ROM_data[converted_position].description+="-0x";
                       ROM_data[converted_position].description+=convertByteToHexString(~(temp_int2));
                       temp_int-=(uint8_t)~(temp_int2);
                  }
                  ROM_data[converted_position].description+=")  [";
                  temp_int2=busRead16(temp_int);
                  ROM_data[converted_position].description+=convertWordToHexString(temp_int2);
                  ROM_data[converted_position].description+="]";
                  break;
             }
        case 0x82:
             {
                  //ROM_data[converted_position].description="Load next two bytes, add $04 (+4 if positive) to them, and get value from address formed to store into $0A.";
                  ROM_data[converted_position].description="$0A= WORD at RAM address($04";
                  temp_int=getSourceWord(position+1);
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
                       temp_int2-=(uint16_t)(~temp_int);
                  }
                  ROM_data[converted_position].description+=") [";
                  temp_int=busRead16(temp_int2);
                  ROM_data[converted_position].description+=convertWordToHexString(temp_int);
                  ROM_data[converted_position].description+="]";
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
                       temp_int2-=(uint16_t)(~temp_int);
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
                       temp_int2-=(uint16_t)(~temp_int);
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
                       temp_int2-=(uint16_t)(~temp_int);
                  }
                  ROM_data[converted_position].description+=")  [";
                  temp_int=busRead8(temp_int2);
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
                       temp_int2-=(uint16_t)(~temp_int);
                  }
                  ROM_data[converted_position].description+=")  [";
                  temp_int=busRead8(temp_int2);
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
                  temp_int2=busRead8(0x7E000A);
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
                       temp_int2-=(uint16_t)(~temp_int);
                  }
                  ROM_data[converted_position].description+=")  [";
                  temp_int=busRead8(temp_int2);
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
                  temp_int2=busRead8(0x7E0000+temp_int);
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
                  temp_int2=busRead8(0x7E0000+temp_int);
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
                  temp_int2=busRead8(0x7E000A);
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
                  temp_int2=busRead8(0x7E0000+temp_int);
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
                  temp_int=busRead16(0x7E0001+step.S);
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
                  
                  if(temp_int2>file_size) break;
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
                       position-=(uint8_t)((~temp_int)+2);
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
                       position-=(uint8_t)((~temp_int)+2);
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
                       position-=(uint8_t)((~temp_int)+2);
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
     if(p.access==tracker_address->cartrom) return cartrom[p.offset+location];
     if(p.access==tracker_address->wram) return wram[p.offset+location];
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
     uint32_t source_address= tracker_address->convertPosition(pos);
     if(source_address<0 || source_address>=file_size) return 0;
     ROM_data[source_address].flags|=OPERAND;
     return ROM_data[source_address].ROM_bytes;
}

uint16_t getSourceWord(uint32_t pos)
{
     uint32_t source_address= tracker_address->convertPosition(pos);
     if(source_address<0 || source_address>=file_size) return 0;
     ROM_data[source_address].flags|=OPERAND;
     uint16_t result = ROM_data[source_address].ROM_bytes;
     
     source_address= tracker_address->convertPosition(pos+1);
     if(source_address<0 || source_address>=file_size) return 0;
     ROM_data[source_address].flags|=OPERAND;
     result+= ROM_data[source_address].ROM_bytes << 8;
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
