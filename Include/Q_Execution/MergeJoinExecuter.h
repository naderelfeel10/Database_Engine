#ifndef MERGE_JOIN_EXECUTER
#define MERGE_JOIN_EXECUTER

#include<unordered_map>
#include <map>
#include <vector>
#include <queue>
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
#include"ExternalMergeSortExecuter.h"

using namespace std;


class MergeJoin : public AbstractExecuter{
    
    private:
        AbstractExecuter* outer_table;
        AbstractExecuter* inner_table;

        string join_col_name;

        BufferPoolManager* BPM;

        ExternalMergeSort* outer_table_sorted;
        ExternalMergeSort* inner_table_sorted;

        //int left_pointer{0};
        //int right_pointer{0}; 

        int outer_col_index{-1};
        int inner_col_index{-1}; 

        Tuple curr_outer_tuple = Tuple({});
        Tuple t2 = Tuple({});

        vector<Tuple>inner_matched_tuples;
        vector<Tuple>res_batch;
        
        vector<Column>table_schema;

    public:
        MergeJoin(BufferPoolManager*BPM, AbstractExecuter* outer_table, AbstractExecuter* inner_table, string join_col_name);
        void open()override;
        void close()override;
        bool getNext(Tuple*tuple)override;

        TableHeap* getTableHeap();
        TableHeap* getOuterTableHeap();
        TableHeap* getInnerTableHeap();

        //
        void set_output_schema();
        vector<Column> get_output_schema();

        bool has_column(string col_name);
};
#endif