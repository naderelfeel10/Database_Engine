#ifndef page_table_H
#define page_table_H

#include<iostream>
#include"Storage/Page/page.h"
#include"Storage/Page/Tuple.h"
#include"Buffer/BufferPoolManager.h"
#include"RID.h"
#include"Column.h"

#include<map>
#include"Storage/Indexing/static_hash_index.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"

#include"Storage/Indexing/BPlusTreeIndex.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"

#include<memory>



// todo : 
// 1.serialzation and deserialization of indexes (hash done)
// 2.create table schema
// 3.

typedef enum{
    STATIC_HASH_INDEX,
    BPLUS_TREE_INDEX
}indexes_t;

struct Index_pages_struct{
    int index_first_page_id;
    int index_last_page_id;
    Index_pages_struct():index_first_page_id(-1),index_last_page_id(-1){}
    Index_pages_struct(int f, int l):index_first_page_id(f),index_last_page_id(l){}

};

class TableHeap{

    private:
        string table_name;
        int table_id;
        int first_page_id = -1;
        int last_page_id = -1; 
        vector<Column> cols;
        int num_of_tuples{0};
        RID starting_rid = RID(-1,-1);
        RID stopping_rid = RID(-1,-1);

        vector<RID>table_rids;

    public:
        // a map between col_name -> vector of indexes on this col ex : (HASH_Index, B+, ...)
        map<string, vector<Index*> > indexes_map;
        // to keep track of indexes pages.
        // for each pair of (col_name, index_type) we have a struct of first_page_id , last_page_id of this specific index 
        map< pair<string,indexes_t>, Index_pages_struct*> indexes_pages_ids;
        
        BufferPoolManager* BPM;
        TableHeap(BufferPoolManager* BPM,int first_page_id, int last_page_id);
        
        //crud :
        RID insertTuple(Tuple tuple);
        RID updateTuple(RID rid, Tuple tuple);
        void deleteTuple(RID rid);
        bool deleteTupleBool(RID rid);
        Tuple* getTuple(RID rid);

        //overloading to update a list of rids
        vector<RID> updateTuple(vector<RID> rid, Tuple tuple);

        
        void saveMetaData();
        void loadMetaData(); // to be impelemented

        vector<Column> getCols();
        void setCols(vector<Column> cols);

        string getTableName();
        int getTableId();
        void setTableName(string table_name);
        void setTableId(int table_id);
        
        int get_first_page_id(){
            return this->first_page_id;
        }
        int get_last_page_id(){
            return this->last_page_id;
        }

        void set_first_page_id(int p_id){
            this->first_page_id = p_id;
        }
        void set_last_page_id(int p_id){
            this->last_page_id = p_id;
        }

        RID getStartingRID(){
            return this->starting_rid;
        }  

        RID getStoppigRID(){
            return this->stopping_rid;
        }

        void displayTablePages();
        void printColumns();

        void createIndex(indexes_t index_type, string string,int index_size);
        Index* getIndex(string col_name, indexes_t index_type);
        Tuple getTupleFromRID(RID rid);

        int getColIndex(string col_name);
        int get_tuples_num(){return this->num_of_tuples;}
        vector<Column>get_output_schema(){return this->cols;}
    
        void deleteTableHeap();
        vector<RID> setAllRIDs();

        vector<RID> getTableRIDS(){
            return this->table_rids;
        }
        
        ~TableHeap();

};

#endif
