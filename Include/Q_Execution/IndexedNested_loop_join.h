#ifndef INDEX_NESTED_LOOP_JOIN_H
#define INDEX_NESTED_LOOP_JOIN_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractPredicate.h"
#include"AbstractExecuter.h"
#include"seq_scan_operator.h"
#include"select_operator.h"
#include"Projection_operator.h"
using namespace std;

class IndexedNestedLoopJoin: public AbstractExecuter{
    
    private:
        //outer table
        // index on inner table
        AbstractExecuter* outer_table;
        Index* inner_index;
        AbstractExecuter* inner_table;
        AbstractPredicate* join_condition; 
        BufferPoolManager* BPM;

        vector<Column>output_schema;
        vector<RID> inner_matches;
        Tuple curr_outer_tuple = Tuple({});
        int col_index{-1};

    public:
        IndexedNestedLoopJoin(BufferPoolManager* BPM, AbstractExecuter* outer_table,AbstractExecuter* inner_table, Index* inner_index, AbstractPredicate* join_condition);

        //main three methods:
        void open();
        void close();
        bool getNext(Tuple*tuple);

        TableHeap* getTableHeap();
        TableHeap* getOuterTableHeap();
        
        void getTuple(RID rid, Tuple& tuple);
        void set_output_schema();
        vector<Column> get_output_schema();

        bool has_column(string col_name);
};
#endif