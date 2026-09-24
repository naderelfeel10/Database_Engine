#ifndef CREATE_INDEX_OPERATOR_H
#define CREATE_INDEX_OPERATOR_H

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
#include"TransactionManager/Transaction_manager.h"
#include"Recovery/WAL_manager.h"

using namespace std;

class CreateIndex : public AbstractExecuter{

    private:
        TableHeap* table_heap;
        TransactionManager* txn_manager;
        WALManager* wal_manager;
        Tuple tuple = Tuple({});

    public:
        CreateIndex(TransactionManager* txn_manager, WALManager* wal_manager,
                    TableHeap* table_heap, indexes_t index_type, string col_name, int index_size);

        void open(){};
        void close(){};
        bool getNext(Tuple* tuple){return false;}
        TableHeap* getTableHeap(){return this->table_heap;}
        vector<Column> get_output_schema(){return this->table_heap->get_output_schema();}
        bool has_column(string col_name){return false;}
        
};

#endif
