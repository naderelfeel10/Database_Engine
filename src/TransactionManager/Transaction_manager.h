#ifndef TXN_MANAGER_H
#define TXN_MANAGER_H

#include "Transaction.h"

//this is the manager of all transactions :
//in this case it's just for serialzable isolation level

class TransactionManager{

private:
    int next_txn_id;
    Transaction* current_txn;

public:
    TransactionManager():next_txn_id(0), current_txn(nullptr){}

    //begin, commit, abort a txn
    Transaction* begin();
    bool commit(Transaction* txn);
    bool abort(Transaction* txn);

    Transaction* get_current_transaction(){
        return this->current_txn;
    }

    void rollback_write(WriteRecord& record);


};

#endif