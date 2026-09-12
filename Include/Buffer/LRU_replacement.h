#ifndef LRU_H
#define LRU_H

#include <iostream>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <utility>
//#include "BufferPoolManager.h"
using namespace std;


class Frame{
    public:
        int key;
        char* value;
        Frame* next;
        Frame* prev;

    Frame(int key, char* value);
    
};
class LRU{
    public:
        Frame* D_head;
        Frame* D_tail;
        unordered_map<int, Frame*>frames_map;
        int capacity;
        LRU(int capacity);
        char* get_frame(int key);
        void put_frame(int key, char* value);
        int evict_frame();
        void remove_frame(int key);

    
};

#endif