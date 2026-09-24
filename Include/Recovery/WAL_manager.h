
#ifndef WAL_H
#define WAL_H

#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>

#include"Storage/Table/RID.h"
#include"Recovery/WAL_record.h"
#include"Recovery/WAL_recovery.h"
#include"Storage/Table/TableHeap.h"
#include"Catalog/Catalog.h"
#include"unordered_set"

//an instance of WAL manager creates a wal file on HD, then we append data into it
class WALManager{
private:
    WALRecovery* recovery_manager;
    TableHeap* table_heap;
    Catalog* catalog;
    int fd;
    ofstream log_file;
    //for full recovery : 
    //commited txns : redo
    //uncommited txns : undo
    unordered_set<int> committed_txns;
    unordered_set<int> un_committed_txns;
public:
    WALManager(Catalog* catalog, WALRecovery* recovery_manager,TableHeap* table_heap, const char* path);
    void add_record(WALRecord& record);
    void flush();

    void clear();
    void recover();
    void set_table_heap(TableHeap* table_heap){
        this->table_heap = table_heap;
        //cout<<"BPM : "<<table_heap->BPM<<endl;
        //cout<<"BPM : "<<this->table_heap->BPM<<endl;
    }
};

#endif