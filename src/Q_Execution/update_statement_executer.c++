#include"update_statement_executer.h"
#include<iostream>
using namespace std;


bool UpdateTuple::is_updated(){return this->updated;}

bool UpdateTuple::getNext(Tuple* tuple){return false;};

TableHeap* UpdateTuple::getTableHeap(){return this->table_heap;}

vector<Column> UpdateTuple::get_output_schema(){return {};}

bool UpdateTuple::has_column(string col_name){return false;};

Tuple UpdateTuple::get_tuple(){
    return this->tuple;
}


bool UpdateTuple::update_tuple(RID rid, Tuple tuple){

    //for txn handling, i have to insert a record contains : 
    //rid of old tuple, old tuple, and the table 
    
    //fetch running txn
    Transaction* txn = txn_manager->get_current_transaction();

    //get old tuple before modifying it 
    Tuple* old_tuple = table_heap->getTuple(rid);

    if(old_tuple == nullptr){
        return false;
    }


    RID new_rid = this->table_heap->updateTuple(rid, tuple).getActualPair();
    //new_rid.print();
    
    if(new_rid.getPageId() !=-1 && new_rid.getSlotNum() != -1)
        updated = true;
    
    //then add this as record into the txn 
    if (txn != nullptr) {
        //prepare record to insert into 
        
        WriteRecord record{
            WriteType::UPDATE,
            table_heap,
            rid,
            *old_tuple
        };

        txn->add_write(record);
    }

    //insert this into WAL 
    int txn_id = txn->GetTransactionId();
    int table_id = table_heap->getTableId();

    WALRecord record(LogType::UPDATE, txn_id, table_id, rid, *old_tuple, tuple);

    record.old_tuple.print();
    record.new_tuple.print();    
    this->wal_manager->add_record(record);

    return updated;

}