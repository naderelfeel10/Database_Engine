#ifndef SELECT_OPERATOR_H
#define SELECT_OPERATOR_H

#include"Storage/Table/TableIterator.h"
#include"Storage/Table/TableHeap.h"
#include"Storage/Table/RID.h"
#include"Storage/Table/Column.h"
#include"Storage/Indexing/Index.h"
#include"Storage/Indexing/StaticHashIndexWrapper.h"
#include"Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"AbstractExecuter.h"
#include"ComplexPredicate.h"
#include"seq_scan_operator.h"
#include"AbstractPredicate.h"

using namespace std;

class Select:public AbstractExecuter{
    private:
        AbstractExecuter* child_operator;
        AbstractPredicate* predicate;

        //holds the rids of the filtered tuples
        vector<RID>select_tuples_rids;
        RID curr_rid = RID(-1,-1);
    public:
        Select(AbstractExecuter* child_operator, AbstractPredicate* predicate):child_operator(child_operator),predicate(predicate){
            open();
        };
        void open();
        void close();
        bool getNext(Tuple* tuple);
        TableHeap* getTableHeap(){return this->child_operator->getTableHeap();}
        vector<Column> get_output_schema(){return this->child_operator->get_output_schema();}

        bool has_column(string col_name);

        RID get_curr_rid(){
            return this->curr_rid;
        }
        vector<RID> get_table_rids(){
            return this->select_tuples_rids;
        }
};

#endif
