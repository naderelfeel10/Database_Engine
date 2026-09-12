#ifndef TXN_H
#define TXN_H

#include"Catalog\Catalog.h"


//write types the txn should handle in order to apply commiting or rolling back
//select and other retrieving operators are not important

enum class WriteType{
    INSERT,
    UPDATE,
    DELETE
};

// the record itself the txn saves
struct WriteRecord {
public:
    WriteRecord() = default;

    WriteType type;
    TableHeap* table;
    //in order to update it back 
    RID rid;
    //old and new tuple values 
    Tuple old_tuple= Tuple({});
    //Tuple new_tuple;

    
};


//txn might be running, commited or aborted for failures
enum class TransactionState{
    RUNNING,
    COMMITTED,
    ABORTED
};

//might be read_commted, snapshot or serialiazable
//for now i support just serializable means each transaction runs after the other 
enum class IsolationLevel{SERIALIZABLE};

//each transaction has an id, and a state
class Transaction {

private:
    int txn_id;
    TransactionState state;
    IsolationLevel isolation_level;

    //txn queries or records
    vector<WriteRecord> writes_set;

public:
    
    Transaction(int txn_id);
    int GetTransactionId();

    TransactionState GetState();

    //add record to the txn means like a new query at it
    void add_write(const WriteRecord& record);

    vector<WriteRecord>& get_write_set();

    IsolationLevel GetIsolationLevel();

    void SetState(TransactionState state);

};

#endif