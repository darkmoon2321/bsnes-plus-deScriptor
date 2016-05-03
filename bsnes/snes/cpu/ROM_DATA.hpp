#ifndef ROM_DATA_H
#define ROM_DATA_H

struct ROM_DATA{
           uint8_t ROM_bytes;
           d_string description;
           uint8_t flags;
           uint32_t * labels;
           uint32_t num_labels;
           uint64_t frequency;
           OPCODE_STEP step;
           int16_t script_index;
           
           ROM_DATA();
           void addLabel(uint32_t label);
           void addMultipleLabel(uint32_t label);
};

#endif
