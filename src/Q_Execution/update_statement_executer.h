#ifndef UPDATE_OPERATOR_H
#define UPDATE_OPERATOR_H

#include"../Storage/Table/TableIterator.h"
#include"../Storage/Table/TableHeap.h"
#include"../Storage/Table/RID.h"
#include"../Storage/Table/Column.h"
#include"../Storage/Indexing/Index.h"
#include"../Storage/Indexing/StaticHashIndexWrapper.h"
#include"../Storage/Indexing/BPlusTreeIndexWrapper.h"
#include"./AbstractExecuter.h"
#include"./ComplexPredicate.h"
#include"./seq_scan_operator.h"
#include"AbstractPredicate.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\TransactionManager\Transaction_manager.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\TransactionManager\Transaction.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Recovery\WAL_manager.h"

using namespace std;

class UpdateTuple : public AbstractExecuter{
    private:
        TableHeap* table_heap;
        TransactionManager* txn_manager;
        WALManager* wal_manager;
        Tuple tuple = Tuple({});
        bool updated{false};

    public:

        UpdateTuple(WALManager* wal_manager, TransactionManager* txn_manager, TableHeap* table_heap):
        table_heap(table_heap),txn_manager(txn_manager),wal_manager(wal_manager){
            this->wal_manager->set_table_heap(this->table_heap);
        }
        bool is_updated();
        void open(){};
        void close(){};
        bool getNext(Tuple* tuple);
        
        bool update_tuple(RID rid, Tuple tuple);


        TableHeap* getTableHeap();
        vector<Column> get_output_schema();
        Tuple get_tuple();
        bool has_column(string col_name);
        
};

#endif
