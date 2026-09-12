#include<iostream>
#include<string>
#include <cstdint>
#include "Storage/Page/page.h"
using namespace std;


Page::Page(int page_id){
    memset(data,0,PAGE_SIZE);

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    header->page_id = (uint16_t)page_id;
    header->num_tuples = 0;
    header->next_page_id = -1;
    header->num_deleted_tuples = 0;
    header->free_space_pointer = PAGE_SIZE;
    //memcpy(data,header,sizeof(PageHeader));
}

// insert a tuple then return tuple id

// update reminder : check deleted slots first before inserting new tuple 
int Page::insertTuple(Tuple tuple){
    
    uint16_t tuple_size = tuple.getTupleSize();
    tuple_size = (tuple_size+7)& ~7; // padding 

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    //get free space first to check if there is enough in this page
    uint16_t free_size = header->free_space_pointer - ( sizeof(PageHeader) + (header->num_tuples)*sizeof(Slot));


    // if not return -1
    if(free_size < (tuple_size + sizeof(Slot)) ){
        return -1;
    }
    
    //else :

    header->free_space_pointer -= (uint16_t)tuple_size;
    tuple.serialize(data+header->free_space_pointer);

    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(PageHeader));
    slots[header->num_tuples].offset = (uint16_t)header->free_space_pointer;
    slots[header->num_tuples].size = (uint16_t)tuple_size; 

    header->num_tuples++;

    return (header->num_tuples -1);

}
/*
// to do : implement this function
int Page::insertData(char* buffer){

    uint16_t entry_size= strlen(buffer);
    //cout<<strlen(buffer)<<endl;
    entry_size = (entry_size+7)& ~7;

    PageHeader* header = reinterpret_cast<PageHeader*>(this->data);
    if( (header->free_space_pointer - (sizeof(header)+ (header->num_tuples + 1)*sizeof(Slot))) < entry_size ){
        return -1;
    }
    header->free_space_pointer -= (uint16_t)entry_size;
    memcpy(data+header->free_space_pointer,buffer, entry_size);
    
    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(PageHeader));
    slots[header->num_tuples].offset = (uint16_t)header->free_space_pointer;
    slots[header->num_tuples].size = (uint16_t)entry_size;

    header->num_tuples++;

    return (header->num_tuples -1);
}
*/
// Added entry_size parameter to prevent binary data truncation from strlen
int Page::insertData(char* buffer, uint16_t raw_size) {
    uint16_t aligned_entry_size = (raw_size + 7) & ~7;

    PageHeader* header = reinterpret_cast<PageHeader*>(this->data);
    
    uint16_t slot_array_end = sizeof(PageHeader) + ((header->num_tuples + 1) * sizeof(Slot));
    
    if (header->free_space_pointer < slot_array_end || 
        (header->free_space_pointer - slot_array_end) < aligned_entry_size) {
        return -1; 
    }

    header->free_space_pointer -= aligned_entry_size;
    
    memcpy(this->data + header->free_space_pointer, buffer, raw_size);
    
    if (aligned_entry_size > raw_size) {
        memset(this->data + header->free_space_pointer + raw_size, 0, aligned_entry_size - raw_size);
    }
    
    Slot* slots = reinterpret_cast<Slot*>(this->data + sizeof(PageHeader));
    slots[header->num_tuples].offset = header->free_space_pointer;
    slots[header->num_tuples].size = aligned_entry_size;

    header->num_tuples++;
    return (header->num_tuples - 1);
}


// should pass slot num and empty tuple by reference
bool Page::getTuple(int slot_num, Tuple& tuple){
    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(slot_num < 0 || slot_num >= header->num_tuples){
        return false;
    }
    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(PageHeader));
    int offset = slots[slot_num].offset;
    tuple.deserialize(data+ offset);

    if(tuple.get_is_deleted()){
        //cout<<"tuple is deleted!"<<endl;
        return false;
    }
    if(slots[slot_num].id_deleted == true){
        //cout<<"tuple deleted!"<<endl;
        return false;
    }
        
    return true;

}
bool Page::getIndexData(int slot_num, char*& buffer){
    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(slot_num < 0 || slot_num >= header->num_tuples){
        return false;
    }
    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(PageHeader));
    int offset = slots[slot_num].offset;
    buffer = data+offset;

    if(slots[slot_num].id_deleted == true){
        cout<<"index deleted!"<<endl;
        return false;
    }
        
    return true;
}
bool Page::deleteTuple(int slot_num){
    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(slot_num < 0 || slot_num >= header->num_tuples){
        return false;
    }
    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(PageHeader));
    
    if (slots[slot_num].id_deleted == true ) {
        return false; 
    }

    Tuple tuple({});
    tuple.deserialize(data+slots[slot_num].offset);

    tuple.set_is_deleted(true);
    tuple.serialize(data+slots[slot_num].offset);

    slots[slot_num].id_deleted =true;
    header->num_deleted_tuples++;

    return true;

}

int Page::updateTuple(int slot_num, Tuple new_tuple){
    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    
    if(slot_num < 0 || slot_num >= header->num_tuples){
        cout<<"slot num is invalid"<<endl;
        return -1;
    }
    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(PageHeader));
    
    int dummy2 = slot_num;
    if (new_tuple.getTupleSize() > slots[slot_num].size){
        cout<<"old and new tuple sizes are not equal"<<endl;
        bool dummy1 = this->deleteTuple(slot_num);
        if(dummy1){
        dummy2 = this->insertTuple(new_tuple);
        return dummy2;
        }
    }
    new_tuple.serialize(data+slots[slot_num].offset);
    return dummy2;

}

vector<Field> Page::get_field_from_all_tuples(int col_index){

 
    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    vector<Field> res;
    for(int i=0;i<header->num_tuples;i++){
        Tuple tuple({});
        this->getTuple(i,tuple);
        res.push_back(tuple.fields[col_index]);
    }
    return res;
}

vector<vector<Field>> Page::get_custom_fields_from_all_tuples(vector<int> col_indexes){

    PageHeader* header = reinterpret_cast<PageHeader*>(data);
    vector<vector<Field>> res;
    // i want to return custom fields from the page, not the whole tuple
    for(int i=0;i<header->num_tuples;i++){
        vector<Field> tmp_res;
        Tuple tuple({});
        this->getTuple(i,tuple);
        for(auto&col_index:col_indexes){
            tmp_res.push_back(tuple.fields[col_index]);
        }
        res.push_back(tmp_res);
    }

    return res;
}

char* Page::getData(){
    return this->data;
}

//check if tuple is deleted
bool Page::is_deleted(int slot_num){
    Tuple tuple({});
    this->getTuple(slot_num, tuple);

    return tuple.get_is_deleted();
}
/*
int main(){

    vector<Field> myFields = {
        Field(TYPE_INT, 101),
        Field(TYPE_STRING, "Nader"),
        Field(TYPE_FLOAT, 99.5)
    };
    Tuple t1(myFields);
    t1.print();

    Page page1(0);
    int t1_id = page1.insertTuple(t1);
    int t2_id = page1.insertTuple(t1);

    cout<<"----------------------------------"<<endl;
    page1.deleteTuple(0);

    Tuple t2({});
    bool is_found = page1.getTuple(1,t2);
    cout<<"is deleted : "<<t2.get_is_deleted()<<endl;
    if(is_found){
        t2.print();
    }

    
    vector<Field> myFields2 = {
        Field(TYPE_INT, 1400),
        Field(TYPE_STRING, "Elfeel"),
        Field(TYPE_FLOAT, 55.0)
    };
    Tuple t3(myFields2);
    int new_slot = page1.updateTuple(1,t3);

    Tuple t4({});
    bool is_found4 = page1.getTuple(new_slot,t4);
    if(is_found4){
        t4.print();
    }

    
}
*/