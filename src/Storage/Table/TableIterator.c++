#include<iostream>
#include<cassert>
#include"Storage/Table/TableIterator.h"


TableIterator::TableIterator(TableHeap* table_heap, RID starting_rid, RID stopping_rid){
    
    // initialization
    this->Table_heap = table_heap;
    this->curr_rid_pointer = starting_rid;
    this->stopping_rid = stopping_rid;

    // if slotnum is greater than number of tuples in the page itself  
    if(curr_rid_pointer.getActualPair().getPageId() != -1){

        char* page_buffer = this->Table_heap->BPM->fetchPage(curr_rid_pointer.getActualPair().getPageId());
        PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
        
        if(pageHeader->num_tuples <= curr_rid_pointer.getActualPair().getSlotNum()){
            this->curr_rid_pointer = RID(-1,0);
        }

    }

}

TableIterator& TableIterator::operator++(){
    // 1.increament the current rid pointer by one
    
    // 2. fetch the page 
    char* page_buffer = this->Table_heap->BPM->fetchPage(curr_rid_pointer.getActualPair().getPageId());
    PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    int next_page_id_to_fetch = pageHeader->next_page_id;
    int next_slot_num_to_fetch = curr_rid_pointer.getActualPair().getSlotNum()+1;
    
    // stopping condition 
    if(stopping_rid.getActualPair().getPageId() != -1){
        this->getCurrRIDPointer().setRID(-1,-1);      
    }

    // check if it's the last tuple in the page , so we fetch the upcomming one
    if(next_slot_num_to_fetch >= pageHeader->num_tuples){
        this->curr_rid_pointer = RID(next_page_id_to_fetch, 0);//the first id in the next table_page
    }else{
        this->curr_rid_pointer = RID(curr_rid_pointer.getActualPair().getPageId(), next_slot_num_to_fetch);
    }

    return *this;
}


// get the curr tuple 
Tuple TableIterator::operator*(){
    Tuple* tuple = this->Table_heap->getTuple(curr_rid_pointer);
    Tuple t = *tuple;
    delete tuple;
    return t;
}

// curr_rid is -1 when it reaches the end of the table 
bool TableIterator::end(){
    return (this->curr_rid_pointer.getActualPair().getPageId() == -1);
}


RID TableIterator::getCurrRIDPointer(){
    return this->curr_rid_pointer;
}


/*
int
main(){

    DiskManager* dm = new DiskManager("tableIteratorDB");
    BufferPoolManager* BPM = new BufferPoolManager(dm);
    
    TableHeap* table_heap = new TableHeap(BPM);

    Field f1(TYPE_STRING,"nadermohamedelfeelisthebestevernadermohamedelfeelisthebestever");
    Tuple t1({f1,f1});
    
    for (int i = 1; i <= 100; ++i) {
        RID rid = table_heap->insertTuple(t1).getActualPair();
        if (rid.getPageId() != -1) {
            std::cout << "rid" << i << " : " 
                  << rid.getPageId() << " , " 
                  << rid.getSlotNum() << std::endl;
        } else {
        std::cerr << "Failed to insert tuple " << i << std::endl;
        }
    }
    
    table_heap->displayTablePages();

    TableIterator table_iterator(table_heap,RID(1,0),RID(4,18));

    for(; !table_iterator.end(); ++table_iterator){
        Tuple t =  *table_iterator;
        t.print();
    }


    dm->~DiskManager();
    BPM->~BufferPoolManager();

}
*/