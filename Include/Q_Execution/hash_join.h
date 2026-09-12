#ifndef HASH_JOIN_H

#define HASH_JOIN_H

#include<unordered_map>
#include <map>
#include <vector>
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

class HashJoin: public AbstractExecuter{
    
    private:
        AbstractExecuter* outer_table;
        AbstractExecuter* inner_table;
        AbstractPredicate* join_condition;
        string join_col_name;
        bool is_hash_built{false};
        unordered_map<Field, vector<Tuple> >hash_map;

        int tuple_index{0};
        Tuple outer_tuple;
        Tuple curr_inner_tuple;
        vector<Column> output_schema;
    
    public:
        HashJoin(AbstractExecuter* outer_table,AbstractExecuter* inner_table, AbstractPredicate* join_condition, string col_name);
        void open();
        void close();
        bool getNext(Tuple*tuple);

        TableHeap* getTableHeap();
        TableHeap* getOuterTableHeap();
        TableHeap* getInnerTableHeap();
        void set_output_schema();
        vector<Column> get_output_schema();

        bool has_column(string col_name);

};

#endif
