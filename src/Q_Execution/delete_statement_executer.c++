#include"delete_statement_executer.h"
#include<iostream>
using namespace std;


bool DeleteTuple::is_deleted(){return this->deleted;}

bool DeleteTuple::getNext(Tuple* tuple){return false;};

TableHeap* DeleteTuple::getTableHeap(){return this->table_heap;}

vector<Column> DeleteTuple::get_output_schema(){return {};}

bool DeleteTuple::has_column(string col_name){return false;};



bool DeleteTuple::delete_tuple(RID rid){

    Tuple* old_tuple = this->table_heap->getTuple(rid);

    this->deleted =  this->table_heap->deleteTupleBool(rid);

    Transaction* txn = txn_manager->get_current_transaction();
    if (txn != nullptr) {
        //prepare record to insert into curr_txn
        WriteRecord record{
            WriteType::DELETE,
            table_heap,
            rid,
            *old_tuple
        };

        txn->add_write(record);
    }

    //insert this into WAL 
    int txn_id = txn->GetTransactionId();
    int table_id = table_heap->getTableId();

    WALRecord record(LogType::UPDATE, txn_id, table_id, rid, *old_tuple, Tuple({}));
    record.old_tuple.print();
    //record.new_tuple.print();    
    this->wal_manager->add_record(record);

    return deleted;

}