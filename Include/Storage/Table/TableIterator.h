#ifndef iterator_H
#define iterator_H

#include"RID.h"
#include"TableHeap.h"

class TableIterator{

    private:
        RID  curr_rid_pointer = RID(-1,-1);
        RID stopping_rid = RID(-1,-1);
        TableHeap* Table_heap;

    public:
        TableIterator(TableHeap* table_heap, RID starting_rid, RID stopping_rid);
        
        TableIterator& operator++();
        bool end();
        Tuple operator*();

        RID getCurrRIDPointer();
        
        void setCurrRIDPointer(RID rid){
            this->curr_rid_pointer=rid;
        }

        

};

#endif