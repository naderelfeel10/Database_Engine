#ifndef S_HASH_H
#define S_HASH_H

#include"Storage/Table/RID.h"
#include"Storage/Page/Field.h"
#include"Buffer/BufferPoolManager.h"
#include<vector>
#include <optional>
using namespace std;

#define STATIC_HASH_INDEX_DEFAULT_CAPACITY 100


//to do: change T into FieldType struct (done)

//template<typename T>
struct hashEntry{
    Field key;
    RID rid;
    RID next_rid;
    hashEntry* next;
    hashEntry(Field k,RID r );
 
    void serializeOneEntry(char* data);
    void deSerializeOneEntry(char* data);
    int getEntrySize();
    int getLinkedListSize();

    hashEntry& operator=(const hashEntry& other);


    void serialize(char* data, int&size);

    void deserialize(char* data, int &size);
};

class hashIndex {

    private:
    
        vector<optional<hashEntry>> hashTable;
        FieldType field_type;
        int col_index;
        size_t hashFunction(Field key);
        BufferPoolManager* BPM;
        int number_of_entries{0};
        int first_page_id{-1};
        int last_page_id{-1};

    public:
        size_t capacity{STATIC_HASH_INDEX_DEFAULT_CAPACITY}; 

        hashIndex(BufferPoolManager* BPM,FieldType field_type,int col_index, int tablesize=-1, int existing_first_page_id=-1,int existing_last_page_id=-1);
        size_t getCapacity();
        
        int get_number_ofentries();
        void insertIndex(Field key, RID value);
        void updateIndex(Field key, RID value);
        void deleteIndex(Field key);

        hashEntry* getHashEntryPtr(Field key);
        // i need to get the first entry of any given key, so i can search through it's next pointer
        hashEntry* getFirstValue(Field key);
        // let this function to return vector of RIDs(done)
        vector<RID> getValue(Field key);
        vector<optional<hashEntry>>& getHashTable() ;

        void saveIndexMeta();
        void loadIndexMeta();
        void load_hash_table(int page_id);

        void serializeHashIndex(char* data);
        void deserializeHashIndex(char* data);

        int get_first_pageid();
        int get_last_pageid();
        void set_last_page_id(int last_page_id);
        void set_first_page_id(int first_page_id);
        int getColindex();

        ~hashIndex();
};

#endif