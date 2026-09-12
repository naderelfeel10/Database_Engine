#ifndef WAL_RECORD_H
#define WAL_RECORD_H

#include"Storage/Table/RID.h"
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

//wal ensures durability of transactions
//recovers from crashes and redos commited txns

enum class LogType{BEGIN, INSERT, DELETE, UPDATE, COMMIT, ABORT};

//basic record will be : type, txn_id, rid of edited tuple, old and new value 
class WALRecord {
public:
    LogType type;
    int transaction_id;
    int table_id;
    RID rid;
    Tuple old_tuple;
    Tuple new_tuple;

    WALRecord():type(LogType::ABORT),transaction_id(-1),rid(RID(-1,-1)), old_tuple(Tuple({})), new_tuple(Tuple({})){}

    WALRecord(LogType type,int transaction_id, int table_id, RID rid,Tuple old_tuple,Tuple new_tuple)
        :type(type), transaction_id(transaction_id),table_id(table_id) ,rid(rid), old_tuple((old_tuple)), new_tuple((new_tuple)) {}   

    void serialize_WAL_record(char* buffer);
    void deSerialize_WAL_record(char* buffer);

    int get_record_size();
};
#endif