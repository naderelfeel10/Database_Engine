#include"TransactionManager/Transaction_manager.h"
#include"iostream"

using namespace std;

Transaction* TransactionManager::begin(){

    if(current_txn != nullptr){
        throw runtime_error("another txn is running, wait till it ends");
    }

    //if not then create new txn with next id and return this new id
    current_txn = new Transaction(next_txn_id++);

    return current_txn;
}

//commit means your data is preserved no matter what happens, so if a crash happens after commiting, the engine should handle it
//i will handle it later haha :)
bool TransactionManager::commit(Transaction* txn){
    cout<<"commiting"<<endl;
    //check if it is the same curr_txn
    if(txn == nullptr || txn != current_txn)
        return false;

    //update the it's state then delete it
    txn->SetState(TransactionState::COMMITTED);

    delete current_txn;
    //ready to begin new one
    current_txn= nullptr;

    return true;
}

//abort has the same intuation of commit, if you aborted the txn, so all changed data should be unchanged
bool TransactionManager::abort(Transaction* txn){

    cout<<"abortinggg"<<endl;
    //check if it is the same curr_txn
    if(txn == nullptr || txn != current_txn)
        return false;

    //update the it's state then delete it
    txn->SetState(TransactionState::ABORTED);

    //TODO : rollback mechanisim
    //the system should roll back all changes, i will handle this in recovery stage using WAL 
    auto& writes = txn->get_write_set();
    cout<<writes.size();

    //loop in reverse order
    for(auto it = writes.rbegin();it != writes.rend(); ++it){
        rollback_write(*it);
    }

    delete current_txn;
    current_txn = nullptr;

    return true;
}

void TransactionManager::rollback_write(WriteRecord& record){

    cout<<"roll back record 1"<<endl;
    //for each record, do the inverse of the operation :
    //insert -> delete
    //update -> update to old version
    //delete ->insert 
    switch(record.type){

        case WriteType::UPDATE:
            record.table->updateTuple(record.rid, record.old_tuple);
            break;

        case WriteType::INSERT:
            record.table->deleteTuple(record.rid);
            break;

        case WriteType::DELETE:
            record.table->insertTuple(record.old_tuple);
            break;

    }

}

/*
int
main(){

}
*/