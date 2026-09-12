#include<iostream>
#include<iostream>
#include"Storage/Indexing/StaticHashIndexWrapper.h"
using namespace std;



StaticHashIndexWrapper::StaticHashIndexWrapper(BufferPoolManager* BPM, hashIndex* index, string col_name, FieldType field_type):
        staticHashIndex(index),col_name(col_name),field_type(field_type){
    
     this->BPM = BPM;
    // first_page_id = BPM->newPage();
    // last_page_id = BPM->newPage();

    // char* page_buffer = BPM->fetchPage(first_page_id);
    // PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
    // pageHeader->next_page_id = last_page_id;
    
    // BPM->markAsDirty(first_page_id);
    
}

void  StaticHashIndexWrapper::Insert(Field&field,string col_name, vector<Column> tuple_cols, RID rid){
    

    if(field.getFieldType() != this->field_type){
        cerr<<"field types don't match"<<endl;
        return;
    }
    int col_index{0};
    for(auto&col:tuple_cols){
        if(col.getColName() == col_name){
            break;
        }
        col_index++;
    }

    char* page_buffer = BPM->fetchPage(this->staticHashIndex->get_last_pageid());
    PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
    Page* page = reinterpret_cast<Page*>(page_buffer);

    //cout<<"ratio: "<<this->staticHashIndex->get_number_ofentries()*1.0 / this->staticHashIndex->getCapacity()<<endl;

    if(this->staticHashIndex->get_number_ofentries()*1.0 / this->staticHashIndex->getCapacity() >=0.75){
        
        //cout<<"==============hash table is almost full================================="<<endl;

        // double the capacity
        int old_table_capacity = this->staticHashIndex->getCapacity();

        hashIndex* new_staticHashIndex = new hashIndex(this->BPM, field_type, col_index, 10*old_table_capacity,
            this->staticHashIndex->get_first_pageid(),this->staticHashIndex->get_last_pageid());

        // copy the old one into the new hash index
        vector<optional<hashEntry>> hashindex_entries = this->staticHashIndex->getHashTable();

        for(auto& bucket:hashindex_entries){
            if(bucket.has_value()){
                hashEntry* curr = &bucket.value();

                while(curr != nullptr){
                    new_staticHashIndex->insertIndex(curr->key,curr->rid);
                    curr = curr->next;
                }
            }
        }
        
        // delete the old one and reassgin
        delete this->staticHashIndex;
        this->staticHashIndex = new_staticHashIndex;

    }
    
    hashEntry tmp_hash(field, rid);
    
    int entry_size = tmp_hash.getEntrySize();
    char* entry_data =new char[entry_size];
    tmp_hash.serializeOneEntry(entry_data);

    int slot_num = page->insertData(entry_data,entry_size);
    // if slot num is -1, means the page is full, so :
    // allocate new Page, 
    // reassign pointers
    //insert the value
    if(slot_num == -1){
        //cout<<"allocating new page"<<endl;
        //create new page
        int new_page_id = BPM->newPage();
        pageHeader->next_page_id = new_page_id;
        //BPM->markAsDirty(new_page_id);
        BPM->markAsDirty(pageHeader->page_id);

        
        //char* new_page_buffer = BPM->fetchPage(this->staticHashIndex->get_last_pageid());
        char* new_page_buffer = BPM->fetchPage(new_page_id);
        
        PageHeader* new_pageHeader = reinterpret_cast<PageHeader*>(new_page_buffer);
        Page* newPage = reinterpret_cast<Page*>(new_page_buffer);

        //last_page_id = new_page_id;
        this->staticHashIndex->set_last_page_id(new_page_id);
        BPM->markAsDirty(new_page_id);
        //cout<<"reinserting into index:  "<<this->staticHashIndex->get_first_pageid()<<" === "<<this->staticHashIndex->get_last_pageid()<<endl;

        // lets try to reinsert
        int new_slot_num = newPage->insertData(entry_data,entry_size);
     
        //cout<<"hashEntry is inserted successfuly, "<< new_page_id<<" "<<new_slot_num<<endl;
        
    }
    // i need to find the prev entry and update it's next


    BPM->markAsDirty(this->staticHashIndex->get_last_pageid());
    //cout<<"hashEntry is inserted successfuly, "<< this->staticHashIndex->get_last_pageid()<<" "<<slot_num<<endl;


    // insert also to the hash index
    this->staticHashIndex->insertIndex(field,rid);
    
}

// delete a hash index wrapper over the original one
void  StaticHashIndexWrapper::Delete(Field&field){

    this->staticHashIndex->deleteIndex(field);
    cout<<"field is deleted."<<endl;
}

// searching over the hash index
vector<RID>  StaticHashIndexWrapper::Search(Field&field)const{
    return this->staticHashIndex->getValue(field);
}


void StaticHashIndexWrapper::displayIndexPages() {
    int next = this->staticHashIndex->get_first_pageid();
    std::cout << "\n--- hash Index Structure ---\n";
    
    while (next != -1) {
        char* page_buffer = BPM->fetchPage(next);
        PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
        
        // Print detailed info for each link in the chain
        std::cout << "[Page " << next << " | index Tuples: " << header->num_tuples 
                  << " | Next: " << header->next_page_id << "]" << std::endl;
        
        next = header->next_page_id;
        //break;
    }
    std::cout << "--- End of Table ---\n";
}
 StaticHashIndexWrapper::~StaticHashIndexWrapper(){
    //cout<<"deleting  hash wrapper"<<endl;
    //this->staticHashIndex->saveIndexMeta();
    if (this->staticHashIndex != nullptr) {
        this->staticHashIndex->saveIndexMeta();
        delete this->staticHashIndex;
    }
    //cout<<"hash wrapper is deleted"<<endl;
}
/*
int 
main(){
    DiskManager* DM = new DiskManager("test_index_wrapperDB");
    BufferPoolManager* BPM = new BufferPoolManager(DM);
    hashIndex* hash_index1 = new hashIndex(BPM,TYPE_INT,80,-1);
    StaticHashIndexWrapper* s_hash_wrapper = new  StaticHashIndexWrapper(BPM,hash_index1, "id", TYPE_INT);
}
*/