#ifndef PROJECTION_H
#define PROJECTION_H


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
using namespace std;


class Projection: public AbstractExecuter{
    private:
        AbstractExecuter* child_operator;
        vector<string> projection_cols;
        vector<Column> output_schema;
        vector<size_t> projection_col_indecies;

    public:
    Projection(AbstractExecuter* child_operator, vector<string> projection_cols):child_operator(child_operator),
                                                                                 projection_cols(projection_cols){
        open();
        }

    void open();
    void close() { this->child_operator->close(); }
    bool getNext(Tuple* tuple);
    TableHeap* getTableHeap(){return this->child_operator->getTableHeap();}
    vector<Column> get_output_schema(){return this->output_schema;};

    bool has_column(string col_name);

};

#endif