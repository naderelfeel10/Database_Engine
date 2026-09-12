#include"TransactionManager/Transaction.h"


Transaction::Transaction(int txn_id):txn_id(txn_id), state(TransactionState::RUNNING){

    this->isolation_level = IsolationLevel::SERIALIZABLE;

}

int Transaction::GetTransactionId(){
    return this->txn_id;
}

TransactionState Transaction::GetState(){
    return this->state;
}

void Transaction::add_write(const WriteRecord& record){
    cout<<"pushing record to txn"<<endl;
    writes_set.push_back(record);

}

vector<WriteRecord>& Transaction::get_write_set(){
    return writes_set;
}

IsolationLevel Transaction::GetIsolationLevel(){
    return this->isolation_level;
}

void Transaction::SetState(TransactionState state){
    this->state = state;
}