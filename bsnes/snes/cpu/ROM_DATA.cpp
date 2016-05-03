#ifndef ROM_DATA_CPP
#define ROM_DATA_CPP

#include "ROM_DATA.hpp"

ROM_DATA::ROM_DATA()
{
    description="";
    flags=0;
    labels=NULL;
    num_labels=0;
    frequency=0;
    script_index=-1;
}
void ROM_DATA::addLabel(uint32_t label)
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
void ROM_DATA::addMultipleLabel(uint32_t label)
{
    uint32_t i=0;
    for(i=0;i<num_labels;i++)
    {
         if(labels[i]==label) break;
    }
    if(i<num_labels) return;
    addLabel(label);
}
#endif
