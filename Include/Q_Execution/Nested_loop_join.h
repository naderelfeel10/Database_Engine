#ifndef NESTED_LOOP_JOIN_H

#define NESTED_LOOP_JOIN_H

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

enum join_types{INNER_JOIN, LEFT_JOIN, RIGHT_JOIN, FULL_JOIN };

class NestedLoopJoin : public AbstractExecuter{
    private:
        //outer table    
        //inner table
        AbstractExecuter* outer_table;
        AbstractExecuter* inner_table;
        AbstractPredicate* join_condition;

        Tuple outer_tuple = Tuple({});
        bool has_inner{false};
        vector<Column>output_schema;

        int inner_matches_counter{-1};
        join_types join_type;
        // private method
        void set_output_schema();
        
    public:
        NestedLoopJoin(AbstractExecuter* outer_table,AbstractExecuter* inner_table,
                       AbstractPredicate* join_condition, join_types join_type);
        
        void open()override;
        void close()override;
        bool getNext(Tuple* tuple)override;

        vector<Column>get_output_schema()override;
        TableHeap* getTableHeap()override;
        
        TableHeap* getOuterTableHeap();
        TableHeap* getInnerTableHeap();

        bool has_column(string col_name);
};

#endif
