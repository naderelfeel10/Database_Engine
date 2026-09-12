#include<iostream>
#include"Storage/Indexing/static_hash_index.h"
using namespace std;


hashIndex::hashIndex(BufferPoolManager* BPM ,FieldType field_type,int col_index, int tablesize,int existing_first_page_id,int existing_last_page_id )
:field_type(field_type),col_index(col_index){

        this->BPM = BPM;
            
        if(existing_first_page_id == -1){
            first_page_id = BPM->newPage();
            last_page_id = BPM->newPage();

            char* page_buffer = BPM->fetchPage(first_page_id);
            PageHeader* pageHeader = reinterpret_cast<PageHeader*>(page_buffer);
            pageHeader->next_page_id = last_page_id;

            BPM->markAsDirty(first_page_id);
        
        }else {
                this->first_page_id = existing_first_page_id;
                this->last_page_id = existing_last_page_id;
                
            }
            if(tablesize == -1){
                hashTable.resize(capacity);
            }
            else{
                hashTable.resize(tablesize);
                capacity = tablesize;
        }

}


size_t hashIndex::hashFunction(Field key) {
            
            size_t raw_val = 0;

            if (key.getFieldType() == TYPE_INT ||
                key.getFieldType() == TYPE_FLOAT ||
                key.getFieldType() == TYPE_BOOL ) {
                    
                raw_val = static_cast<int>(key.getFieldValueInt());
                
            }
            else if (key.getFieldType() == TYPE_STRING) {
                // Use a standard hash for strings
                raw_val = std::hash<string>{}(key.getFieldValueStr());
            }
            //cout<<"row value is : "<<raw_val <<endl;
            //cout<<"index slot is : "<<raw_val % capacity<<endl;
            return raw_val % capacity;
        }
        

size_t hashIndex::getCapacity(){
        return this->capacity;
}


void hashIndex::insertIndex(Field key, RID value){

       //key.print();
        size_t index = hashFunction(key); 
        //cout<<"index : "<<index<<endl;
        if(hashTable[index].has_value()){

            hashEntry* curr_hash_entry = &hashTable[index].value();
            while(curr_hash_entry->next){
                curr_hash_entry = curr_hash_entry->next;
            }

            curr_hash_entry->next = new hashEntry(key, value);
            curr_hash_entry->next_rid = value;
            //cout<<"inserted "<<endl;
                
        }else{
            //cout<<"inserted into new hash entry "<<endl;
            hashTable[index] = hashEntry(key, value); 
        }
        number_of_entries++;

}



void hashIndex::updateIndex(Field key, RID value){
            hashEntry* curr_ptr = getHashEntryPtr(key);

            if(curr_ptr != nullptr){
                curr_ptr->key = key;
                curr_ptr->rid = value;
                cout<<"index updated successfuly"<<endl;
            }
            else cout<<"key not found"<<endl;
            
        }


void hashIndex::deleteIndex(Field key){

            size_t index = hashFunction(key);

            if(hashTable[index]->key == key){
                hashEntry* toDelete = hashTable[index]->next;
                
                if(toDelete != nullptr){
                    hashEntry* next_next = toDelete->next;
                    hashTable[index] = *toDelete;
                    hashTable[index]->next = next_next;
                    hashTable[index]->next_rid = next_next->rid;
                    delete toDelete;
                }
                else{
                    hashTable[index] = nullopt;
                }
                cout<<"index deleted successfuly1"<<endl;
                number_of_entries--;
                

            }
            else{
                hashEntry* curr_ptr = &hashTable[index].value();

                while(curr_ptr->next != nullptr){
                    if(curr_ptr->next->key == key){
                        curr_ptr->next = curr_ptr->next->next;
                        curr_ptr->next_rid = curr_ptr->next->next_rid;
                        cout<<"index deleted successfuly2"<<endl;
                        return;
                    }
                    curr_ptr = curr_ptr->next;
                }

                 cout<<"key not found"<<endl;
        }

        }

hashEntry* hashIndex::getHashEntryPtr(Field key){
            
            size_t index = hashFunction(key);

            if(!hashTable[index].has_value()){
                //cout<<"hashTable has no value"<<endl;
                return nullptr;
            }

            hashEntry* curr_hash_entry = &hashTable[index].value();

            while(curr_hash_entry != nullptr ){

                    if(curr_hash_entry->key == key){
                        return curr_hash_entry;
                    }
                    curr_hash_entry = curr_hash_entry->next;
            }

            return nullptr;

        }


vector<RID> hashIndex::getValue(Field key){

            //size_t index = hashFunction(key);
            hashEntry* curr_ptr = this->getHashEntryPtr(key);

            vector<RID>res;
            if(curr_ptr == nullptr){
                //cout<<"curr_ptr is null"<<endl;
                return res;
            }else{
                while(curr_ptr){
                    if(curr_ptr->key == key){
                        res.push_back(curr_ptr->rid);
                    }
                    curr_ptr= curr_ptr->next;
                }
                //return curr_ptr->rid;
                return res;
            }

            return res;
}
hashEntry* hashIndex::getFirstValue(Field key){
    size_t index = hashFunction(key);
    if(!hashTable[index].has_value()){
        return nullptr;
    }
    return &hashTable[index].value();
}


int hashIndex::get_number_ofentries(){
    return this->number_of_entries;
}
vector<optional<hashEntry>>& hashIndex::getHashTable()  {
    return this->hashTable;
}


//save index meta data

void hashIndex::saveIndexMeta(){

    int offset{sizeof(PageHeader)};
    char* buffer = BPM->fetchPage(this->first_page_id);
    
    if (buffer == nullptr) return;

    // lets save meta data
    // save last page id
    memcpy(buffer+offset,&this->last_page_id, sizeof(int));
    offset+=sizeof(int);
    cout<<"lst page id : "<<this->last_page_id;
    //save number of entries
    memcpy(buffer+offset,&this->number_of_entries, sizeof(int));
    offset+=sizeof(int);

    //save capacity
    memcpy(buffer+offset,&this->capacity, sizeof(size_t));
    offset+=sizeof(size_t);
    cout<<this->capacity<<endl;
    //save fieldType
    memcpy(buffer+offset,&this->field_type, sizeof(FieldType));
    offset+=sizeof(FieldType);

    //save col index
    memcpy(buffer+offset,&this->col_index, sizeof(int));
    offset+=sizeof(int);

    // here i need to save the main vetor of entries only
    // i from it i can access the rest of real hashEntries on disk

    /*
    for(int i=0;i<this->capacity;i++){
        bool hasValue = hashTable[i].has_value();
        memcpy(buffer+offset,&hasValue, sizeof(bool));
        offset+=sizeof(bool);

        if(hasValue){
            cout<<i<<" : "<<"hash value"<<endl;
            hashTable[i]->serializeOneEntry(buffer+offset);
            offset+=hashTable[i]->getEntrySize();

        }
    }
    */

    // save on disk
    BPM->markAsDirty(first_page_id);
    cout<<"index meta is saved"<<endl;
}

void hashIndex::loadIndexMeta(){

    int offset{sizeof(PageHeader)};
    char* buffer = BPM->fetchPage(this->first_page_id);
    if (buffer == nullptr) return;

    // lets load meta data

    //load last page id
    memcpy(&this->last_page_id,buffer+offset, sizeof(int));
    offset+=sizeof(int);
    cout<<"last page id from load index"<<this->last_page_id<<"00000000000000000000000000000000000000000000000"<<endl;
    //load number of entries
    memcpy(&this->number_of_entries,buffer+offset, sizeof(int));
    offset+=sizeof(int);

    //load capacity
    memcpy(&this->capacity,buffer+offset, sizeof(size_t));
    offset+=sizeof(size_t);
    cout<<"capacity : "<<this->capacity<<endl;

    //load fieldType
    memcpy(&this->field_type,buffer+offset, sizeof(FieldType));
    offset+=sizeof(FieldType);

    //load col index
    memcpy(&this->col_index,buffer+offset, sizeof(int));
    offset+=sizeof(int);
    /*
    this->hashTable.assign(this->capacity, nullopt);
    for(int i=0;i<this->capacity;i++){
        bool hasValue{false};
        memcpy(&hasValue, buffer+offset, sizeof(bool));
        offset+=sizeof(bool);

        if(hasValue){
            Field f(field_type);
            RID rid(-1,-1);
            hashEntry e(f,rid);
            e.deSerializeOneEntry(buffer+offset);
            offset+=e.getEntrySize();            
            hashTable[i] = e;
        }
    }
    */
   cout<<"index meta is loaded"<<endl;


}


void hashIndex::load_hash_table(int page_id){
    
    if(page_id == -1){
        return;
    }
    // fetch the first page first, starting with it i can reload the whole index
    char* page_buffer = BPM->fetchPage(page_id);
    //initiaize
    PageHeader* header = reinterpret_cast<PageHeader*>(page_buffer);
    Page* page = reinterpret_cast<Page*>(page_buffer);
    //Slot* slots = reinterpret_cast<Slot*>(page_buffer+sizeof(PageHeader));

    this->hashTable.resize(this->capacity);
    // loop through all slots to fill the hashTable with
    for(int i=0;i<header->num_tuples;i++){
        // get the entry from the page
        Tuple tuple({});
        bool isFound = page->getTuple(i,tuple);
        tuple.print();
        for(auto&f:tuple.fields){
            f.print();
        }
        if(isFound){
            // i want to find the certain field
            Field key(this->field_type);
            key = tuple.fields[this->col_index];
            this->insertIndex(key,RID(page_id,i));
            //int index = this->hashFunction(key);
            //this->hashTable[index] = hashEntry(key, RID(page_id,i));
        }
    }
    load_hash_table(header->next_page_id);
    cout<<"hash table size : "<<this->hashTable.size()<<endl;
}



void hashIndex::serializeHashIndex(char* data){
    int offset{sizeof(PageHeader)};
    //save number of entries
    memcpy(data+offset,&number_of_entries, sizeof(int));
    offset+=sizeof(int);

    //save capacity
    memcpy(data+offset,&capacity, sizeof(size_t));
    offset+=sizeof(size_t);

    //save fieldType
    memcpy(data+offset,&field_type, sizeof(field_type));
    offset+=sizeof(field_type);
    

    //save hash entries :
    //1. go through each entry in the vector, serialize it into the disk
    for(auto& e:this->hashTable){
        bool hasValue = e.has_value();
        memcpy(data+offset,&hasValue, sizeof(bool));
        offset+=sizeof(bool);

        // if it has value, then serialize it
        if(hasValue){
            int size;
            e->serialize(data+offset, size);
            offset+=size;
        }
    }


}


void hashIndex::deserializeHashIndex(char* data) {
    int offset = 0;

    // 1. Restore metadata
    memcpy(&this->number_of_entries, data + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&this->capacity, data + offset, sizeof(size_t));
    offset += sizeof(size_t);

    memcpy(&this->field_type, data + offset, sizeof(field_type));
    offset += sizeof(field_type);

    // 2. Prepare the table
    this->hashTable.clear(); // Ensure it's empty
    this->hashTable.reserve(this->capacity); // Use reserve, not resize

    // 3. Iterate over CAPACITY (the buckets), not entries
    for (size_t i = 0; i < this->capacity; i++) {
        bool hasvalue = false;
        memcpy(&hasvalue, data + offset, sizeof(bool));
        offset += sizeof(bool);

        if (hasvalue) {
            Field f(field_type);
            RID rid(-1, -1);
            hashEntry e(f, rid);
            
            int entry_bytes_read = 0;
            // You need to update your hashEntry::deserialize to return 
            // or set how many bytes it actually consumed!
            e.deserialize(data + offset, entry_bytes_read); 
            offset += entry_bytes_read;

            this->hashTable.push_back(e);
        } else {
            this->hashTable.push_back(std::nullopt);
        }
    }
}

hashIndex::~hashIndex() {
    cout<<"deleting hash index"<<endl;
    for (auto& f : this->hashTable) {
        if (f.has_value()) {
            hashEntry* curr = f.value().next; 
            
            while (curr != nullptr) {
                hashEntry* next_node = curr->next; 
                delete curr;                     
                curr = next_node;                 
            }
            f.value().next = nullptr; 
        }
    }
    cout<<"hash index deleted"<<endl;
}


int hashIndex::get_first_pageid(){
    return this->first_page_id;
}

int hashIndex::get_last_pageid(){
    return this->last_page_id;
}

void hashIndex::set_last_page_id(int last_page_id) {
    this->last_page_id = last_page_id;
}

void hashIndex::set_first_page_id(int first_page_id) {
    this->first_page_id = first_page_id;
}

int hashIndex::getColindex(){
    return this->col_index;
}

/////////////////////////


hashEntry::hashEntry(Field k,RID r ):key(k),rid(r),next(nullptr),next_rid(-1,-1){

}
 
void hashEntry::serializeOneEntry(char* data){
        int offset{0};
        // ser. field
        key.serialize(data+offset);
        offset+=key.getSerializedSize();
        // ser. rid
        rid.serialize(data+offset);
        offset += rid.getSerializedSize();
        
        // save next rid pointer
        next_rid.serialize(data+offset);
        offset += next_rid.getSerializedSize();
    }

    void hashEntry::deSerializeOneEntry(char* data){
        int offset{0};
        // deser. field
        this->key.deserialize(data+offset);
        offset+= this->key.getSerializedSize();

        // dser. rid
        this->rid.deserialize(data+offset);        
        offset += this->rid.getSerializedSize();
        
        this->next_rid.deserialize(data+offset);
        offset += this->next_rid.getSerializedSize();
        
        this->next = nullptr;
    }

    int hashEntry::getEntrySize(){
        return key.getSerializedSize()+2*rid.getSerializedSize();
    }

    int hashEntry::getLinkedListSize(){
        int res{0};
        hashEntry* dummy_next= next;
        while(dummy_next){
            res++;
            dummy_next = dummy_next->next;
        }
        
        return res;
}



hashEntry& hashEntry::operator=(const hashEntry& other){

        this->key = other.key;
        this->rid = other.rid;
        this->next = other.next;
        return *this;
}


void hashEntry::serialize(char* data, int&size){
        int offset{0};
        // ser. field
        key.serialize(data+offset);
        offset+=key.getSerializedSize();


        // ser. rid
        rid.serialize(data+offset);
        offset += rid.getSerializedSize();

        // i need to save next pointer
        // saving a pointer is not correct here , it's just a  location in memory 
        // i will save the key, and a punch of rids, and while seserializing the Entry again i will recreate them in the same order
        
        hashEntry* dummy_next = next;

        // i need to save linked list of entries size
        int linked_list_size = getLinkedListSize();

        memcpy(data+offset, &linked_list_size, sizeof(int));
        offset+=sizeof(int);

        dummy_next = next;

        while(dummy_next!=nullptr){
            
            dummy_next->key.serialize(data+offset);
            offset+= dummy_next->key.getSerializedSize();
            dummy_next->rid.serialize(data+offset);
            offset+= dummy_next->rid.getSerializedSize();

            //cout<<"ser_test : ";
            //dummy_next->rid.print();
            dummy_next = dummy_next->next;
        }
        size = offset;

}


void hashEntry::deserialize(char* data, int &size){
       int offset{0};

        // deserialize field
        this->key.deserialize(data+offset);
        offset+=this->key.getSerializedSize();

        // deserialize rid
        this->rid.deserialize(data+offset);
        offset += rid.getSerializedSize();

        // size of linked list of RIds
        int linked_list_size{0};
        memcpy(&linked_list_size, data+offset,sizeof(int));
        offset+=sizeof(int);
        
        hashEntry* curr =  this;
        while(linked_list_size--){

            // ddeserialize key
            Field dummyfield(this->key.getFieldType());
            dummyfield.deserialize(data+offset);
            offset += dummyfield.getSerializedSize();

            // deserialize each RID
            RID dummyRID(-1,-1);
            dummyRID.deserialize(data+offset);
            offset += dummyRID.getSerializedSize();
            
            // create new hashEntry on heap
            hashEntry* e = new hashEntry(dummyfield, dummyRID);
            curr->next = e;
            curr = e;
        }

    size = offset;
        
}



/*
int
main(){

    
    cout<<"nader elfeel"<<endl;

    DiskManager* DM = new DiskManager("test_indexDB");
    BufferPoolManager* BPM = new BufferPoolManager(DM);

    hashIndex* hash_index1 = new hashIndex(BPM,TYPE_INT,80);

    cout<<"hash index capacity : "<<hash_index1->getCapacity()<<endl;


    int int_key1 = 21;
    Field f1(TYPE_INT, int_key1);
    RID rid1(1,13);
    hash_index1->insertIndex(f1, rid1);


    int int_key2 = 221;
    Field f2(TYPE_INT, int_key2);
    RID rid2(4,4);
    hash_index1->insertIndex(f2, rid2);

    int int_key3 = 421;
    Field f3(TYPE_INT, int_key3);
    RID rid3(6,6);
    hash_index1->insertIndex(f3, rid3);

    //cout<<"RID 21 :  ("<<hash_index1->getValue(f1)[0].getPageId()<<", "<<hash_index1->getValue(f1)[0].getSlotNum()<<")"<<endl;
    //cout<<"RID 221 :  ("<<hash_index1->getValue(f2)[0].getPageId()<<", "<<hash_index1->getValue(f2)[0].getSlotNum()<<")"<<endl;
    //cout<<"RID 421 :  ("<<hash_index1->getValue(f3)[0].getPageId()<<", "<<hash_index1->getValue(f3)[0].getSlotNum()<<")"<<endl;

    for(auto& rid: hash_index1->getValue(f2))rid.print();

    hash_index1->updateIndex(f2, RID(3,3));
    cout<<"RID 221 :  ("<<hash_index1->getValue(f2)[0].getPageId()<<", "<<hash_index1->getValue(f3)[0].getSlotNum()<<")"<<endl;



    hash_index1->updateIndex(Field(TYPE_INT, 222) , RID(5,5));
    cout<<"RID 221 :  ("<<hash_index1->getValue(f2)[0].getPageId()<<", "<<hash_index1->getValue(f2)[0].getSlotNum()<<")"<<endl;
    

    //hash_index1->deleteIndex(f1);
    cout<<"RID 21 :  ("<<hash_index1->getValue(f1 )[0].getPageId()<<", "<<hash_index1->getValue(f1)[0].getSlotNum()<<")"<<endl;
    cout<<"RID 221 :  ("<<hash_index1->getValue(f2)[0].getPageId()<<", "<<hash_index1->getValue(f2)[0].getSlotNum()<<")"<<endl;
    cout<<"RID 421 :  ("<<hash_index1->getValue(f3)[0].getPageId()<<", "<<hash_index1->getValue(f3)[0].getSlotNum()<<")"<<endl;

    //hash_index1->deleteIndex(f2);
    cout<<"RID 21 :  ("<<hash_index1->getValue(f1) [0].getPageId()<<", "<<hash_index1->getValue(f1)[0].getSlotNum()<<")"<<endl;
    cout<<"RID 221 :  ("<<hash_index1->getValue(f2)[0].getPageId()<<", "<<hash_index1->getValue(f2)[0].getSlotNum()<<")"<<endl;
    cout<<"RID 421 :  ("<<hash_index1->getValue(f3)[0].getPageId()<<", "<<hash_index1->getValue(f3)[0].getSlotNum()<<")"<<endl;


    Field f4(TYPE_STRING, "elfeel");
    hash_index1->insertIndex(f4,RID(7,7));
    cout<<"RID 'elfeel' :  ("<<hash_index1->getValue(f4)[0].getPageId()<<", "<<hash_index1->getValue(f4)[0].getSlotNum()<<")"<<endl;

    
    Field dm_f1(TYPE_FLOAT,1.344);
    Field dm_f2(TYPE_FLOAT,1.344);
    
    cout<<dm_f1.getFieldValueFloat()<<endl;
    cout<<dm_f2.getFieldValueFloat()<<endl;
    

    if(dm_f1 == dm_f2){
        cout<<"f1 = f2";
    }else{
        cout<<"f1 != f2";
    }
    
    cout<<"=====testing serialization ======"<<endl;

    // test serialization and deserialization of hash entry
    Field f5(TYPE_STRING, "nader");
    RID rid5 (19,19);
    RID rid6 (5,5);
    RID rid7 (6,6);

    hashEntry e1(f5,rid5);
    hashEntry* p_e = new hashEntry(f5,rid6);
    e1.next = p_e;
    hashEntry* p_e1 = new hashEntry(f5,rid7);
    p_e->next = p_e1;

    
    e1.key.print();
    e1.rid.print();


    char* data= new char[100];
    int entry_size{0};
    e1.serialize(data,entry_size);

    cout<<"===deseerialization===="<<endl;
    Field dummy_f(TYPE_STRING, "");
    hashEntry e2(dummy_f,RID(-1,-1));

    int dummy_size{0};
    e2.deserialize(data,dummy_size);
    e2.key.print();
    e2.rid.print();
    
    hashEntry* dummy_e = e2.next;
    while(dummy_e){
        cout<<"dummy_e"<<endl;
        dummy_e->key.print();
        dummy_e->rid.print();
        dummy_e = dummy_e->next;
    }
    cout<<"=======serializing a whole index======="<<endl;

    char* index_data = new char[500];
    hash_index1->serializeHashIndex(index_data);

    cout<<hash_index1->getHashTable().size()<<endl;

    
    hashIndex hash_index2(BPM,TYPE_STRING,0);
    hash_index2.deserializeHashIndex(index_data);

    cout<<"index2 info"<<endl;

    cout<<hash_index2.getCapacity()<<endl;
    cout<<hash_index2.get_number_ofentries()<<endl;

    vector<optional<hashEntry>> index_table = hash_index2.getHashTable();

    cout<<index_table.size()<<endl;
    for(int i=0;i<index_table.size();i++){

        if(index_table[i].has_value()){
            cout<<"has value"<<endl;
             hashEntry* curr = &index_table[i].value();

            while(curr){
                curr->key.print();
                curr->rid.print();
                curr = curr->next;
            }
            cout<<"==="<<endl;
        }
    }
    // test to insert into this deserialized index
    hash_index2.insertIndex(f4,RID(7,8));

    vector<RID> rid_res =  hash_index2.getValue(f4);
    for(auto& rid: rid_res)rid.print();
   



    cout << "======= testing save and load an index =======" << endl;

    int hashindex_1_page_id = hash_index1->get_first_pageid();
    cout<<"h_id : "<<hashindex_1_page_id<<endl;
    hash_index1->saveIndexMeta();

    //shut down the first instance
    delete hash_index1; 
    //DM->~DiskManager();
    BPM->~BufferPoolManager();

    cout << "-----system rebooted-----" << endl;

    // 2.restart from the same file
    DiskManager* DM2 = new DiskManager("test_indexDB");
    // Assuming your BPM takes (pool_size, disk_manager)
    BufferPoolManager* BPM2 = new BufferPoolManager( DM2); 

    // 3. Create the new index object (Pass the meta_page_id!)
    // You need to know which page the meta was saved on (e.g., page 0)
    hashIndex* hash_index_loaded = new hashIndex(BPM2,TYPE_INT,300,hashindex_1_page_id); 

    // 4. Now load the data from the disk into this new object
    hash_index_loaded->loadIndexMeta();

    cout << "hash index capacity: " << hash_index_loaded->capacity << endl;
    cout << "number of entries: " << hash_index_loaded->get_number_ofentries() << endl;

    // Cleanup
    delete hash_index_loaded;
    delete BPM2;
    delete DM2;
    

    hashIndex* hash_index_test_save_store = new hashIndex(BPM,TYPE_INT,40);
    Field f_sl_1(TYPE_INT, 11);
    Field f_sl_2(TYPE_INT, 22);
    Field f_sl_3(TYPE_INT, 33);
    Field f_sl_4(TYPE_INT, 44);
    Field f_sl_5(TYPE_INT, 55);
    Field f_sl_6(TYPE_INT, 66);
    hash_index_test_save_store->insertIndex(f_sl_1,RID(0,1));
    hash_index_test_save_store->insertIndex(f_sl_2,RID(0,2));
    hash_index_test_save_store->insertIndex(f_sl_3,RID(0,3));
    hash_index_test_save_store->insertIndex(f_sl_4,RID(0,4));
    hash_index_test_save_store->insertIndex(f_sl_5,RID(0,5));
    hash_index_test_save_store->insertIndex(f_sl_6,RID(0,6));

    
    cout<< "first page : "<<hash_index_test_save_store->get_first_pageid()<<endl;
    cout<< "last page : "<<hash_index_test_save_store->get_last_pageid()<<endl;
    hash_index_test_save_store->saveIndexMeta();

    delete hash_index_test_save_store;
    BPM->~BufferPoolManager();
    DM->~DiskManager();

    DiskManager* DM3 = new DiskManager("test_indexDB");
    BufferPoolManager* BPM3 = new BufferPoolManager( DM3); 

    hashIndex* test_hash_index_loaded = new hashIndex(BPM3,TYPE_INT,300); 
    test_hash_index_loaded->loadIndexMeta();

    cout<< "first page : "<<hash_index_test_save_store->get_first_pageid()<<endl;
    cout<< "last page : "<<hash_index_test_save_store->get_last_pageid()<<endl;
    //hash_index_test_save_store->getFirstValue(f_sl_1)->rid.print();







}
*/