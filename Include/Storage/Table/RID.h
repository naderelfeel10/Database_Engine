#ifndef RID_H
#define RID_H

#include<iostream>
#include<utility>
#include<cassert>
#include "Storage/Page/page.h"

class RID{
    
    private:
        int page_id;
        int slot_num;
        std::pair<int, int> actual_pair;
    public:
        RID(int page_id, int slot_num);
        void setRID(int page_id, int slot_num);
        void updateActualPair(RID rid);
        RID getActualPair();
        int getPageId();
        int getSlotNum();

        void serialize(char* data);
        void deserialize(char* data);
        int getSerializedSize();
        void print();

};
#endif