#include"insert_statement_executer.h"
#include<iostream>
using namespace std;


InsertTuple::InsertTuple(WALManager* wal_manager, TransactionManager* txn_manager, TableHeap* table_heap,Tuple tuple){
    this->tuple = tuple;
    this->table_heap = table_heap;
    this->txn_manager = txn_manager;
    this->wal_manager = wal_manager;

    cout << "wal_manager = " << wal_manager << endl;
    cout << "table_heap  = " << table_heap << endl;

    if(this->wal_manager)
        this->wal_manager->set_table_heap(table_heap);


    RID rid = this->table_heap->insertTuple(tuple);  
    rid.print();
    
    if(rid.getPageId() !=-1 && rid.getSlotNum() != -1)
        inserted = true;
        //then add this as record into the txn 

    
    Transaction* txn = txn_manager->get_current_transaction();
    if (txn != nullptr) {
        //prepare record to insert into curr_txn
        WriteRecord record{
            WriteType::INSERT,
            table_heap,
            rid,
            Tuple({})
        };

        txn->add_write(record);
    }
    //insert this into WAL 
    int txn_id = txn->GetTransactionId();
    string table_name = table_heap->getTableName();
    WALRecord record(LogType::INSERT, txn_id, table_name, rid, Tuple({}), tuple);
    record.new_tuple.print();
    this->wal_manager->add_record(record);
}


bool InsertTuple::is_inserted(){return this->inserted;}

bool InsertTuple::getNext(Tuple* tuple){return false;};

TableHeap* InsertTuple::getTableHeap(){return this->table_heap;}

vector<Column> InsertTuple::get_output_schema(){return {};}

bool InsertTuple::has_column(string col_name){return false;};

Tuple InsertTuple::get_tuple(){
    return this->tuple;
}


