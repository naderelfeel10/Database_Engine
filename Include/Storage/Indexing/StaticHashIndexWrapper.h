#ifndef STATIC_H_INDEX_WRAPPER_H
#define STATIC_H_INDEX_WRAPPER_H


#include"Storage/Page/Tuple.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Index.h"
#include"static_hash_index.h"
#include"Buffer/BufferPoolManager.h"



class StaticHashIndexWrapper: public Index{
    private:
        hashIndex* staticHashIndex;
        BufferPoolManager* BPM;
        string col_name;
        FieldType field_type;
        // int first_page_id{-1};
        // int last_page_id{-1};
    
    public:
        StaticHashIndexWrapper(BufferPoolManager* BPM, hashIndex* index, string col_name, FieldType field_type);
        void Insert(Field&field,string col_name, vector<Column> tuple_cols, RID rid)override;
        void Delete(Field&field)override;
        vector<RID>  Search(Field&field)const override;
        void displayIndexPages();
        ~StaticHashIndexWrapper();
        string get_index_col_name(){return this->col_name;}
        
};

#endif 