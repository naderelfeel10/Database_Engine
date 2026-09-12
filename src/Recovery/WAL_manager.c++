#include"Recovery/WAL_manager.h"


//new instance creates a new log file
WALManager::WALManager(Catalog* catalog, WALRecovery* recovery_manager,TableHeap* table_heap, const char* path){

    this->recovery_manager = recovery_manager;
    this->table_heap = table_heap;
    this->catalog = catalog;

    cout<<path<<endl;
    fd = _sopen(
        path,
        _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY,
        _SH_DENYNO,
        _S_IREAD | _S_IWRITE
    );

    if (fd == -1) {
        cout << "WAL _open failed\n";
        cout << "path = [" << path << "]\n";
        cout << "errno = " << errno << "\n";
        cout << "error = " << strerror(errno) << endl;

        throw std::runtime_error("Failed to open WAL file");
    }
}

void WALManager::flush(){
    log_file.flush();

    //it forces the os buffer to write the data into HD
    
    //fsync(fd); // works for linux
    _commit(fd); // same for windos
}

//serialize the record into a stream of bytes then append it into the end of the log
void WALManager::add_record(WALRecord& record){

    int record_size = record.get_record_size();
    char* buffer = new char[record_size];

    //serialize 
    record.serialize_WAL_record(buffer);

    int written = _write(fd, buffer, record_size);

    if(written != record_size){
        //delete[] buffer;
        throw runtime_error("failed to write");
    }

    //record commit, then clear
    if(record.type == LogType::COMMIT){
        this->flush();  
        //this->clear();  
    }
    this->flush();  
    delete[] buffer;
}


//it clears the file after successful commits, so the rest is just data to recover
void WALManager::clear(){
    //clear then flush
    if(_chsize(fd, 0) != 0){
        throw runtime_error("failed to clear WAL file");
    }

    if(_commit(fd) != 0){
        throw runtime_error("failed to flush WAL clear");
    }
}


//full recovery :
//fetch all records
//for each record :
    //check it's type
    //redo or undo based on commited or not
void WALManager::recover(){

    vector<WALRecord> records = recovery_manager->read_all_records();

    for(const WALRecord& record:records){
        
        if(record.type == LogType::BEGIN){
            cout<<"txn_id"<<record.transaction_id<<endl;
            un_committed_txns.insert(record.transaction_id);
        }
        else if(record.type == LogType::COMMIT){
            cout<<"txn_id"<<record.transaction_id<<endl;
            committed_txns.insert(record.transaction_id);
            un_committed_txns.erase(record.transaction_id);
        }

        else if(record.type == LogType::ABORT){
            cout<<"txn_id"<<record.transaction_id<<endl;
            un_committed_txns.erase(record.transaction_id);
        }
    } 
    cout<<"///"<<endl;

    for(auto&txn_id:committed_txns)cout<<txn_id;
    cout<<"///"<<endl;
    for(auto&txn_id:un_committed_txns)cout<<txn_id;
    cout<<"///"<<endl;

    for(WALRecord& record : records){

        cout<<record.transaction_id<<endl;

        if(record.type == LogType::BEGIN || record.type == LogType::COMMIT || record.type == LogType::ABORT){
            continue;
        }
        //first load table heap from record table_id
        int table_id = record.table_id;
        cout<<table_id<<endl;
        TableInfo* table_info =  this->catalog->GetTable(table_id);
        if(table_info == nullptr){
            throw runtime_error("table not found");
        }

        TableHeap* curr_table_heap = table_info->get_table_heap();
        if(curr_table_heap == nullptr){
            throw runtime_error("table heap is nullp");
        }
        //if commited then redo it, else undo changes
        if(committed_txns.find(record.transaction_id) != committed_txns.end()){

            switch(record.type){
                case LogType::UPDATE:
                    curr_table_heap->updateTuple(record.rid, record.new_tuple);
                    break;

                case LogType::INSERT:
                    curr_table_heap->insertTuple(record.new_tuple);
                    //curr_table_heap->updateTuple(record.rid, record.new_tuple);
                    break;

                case LogType::DELETE:
                    curr_table_heap->deleteTuple(record.rid);
                    break;
            }
        }
        //undo or roll back
            //insert -> delete
            //update -> update to old version
            //delete ->insert 
        else{
            record.rid.print();
            switch(record.type){
                case LogType::UPDATE:
                    curr_table_heap->updateTuple(record.rid, record.old_tuple);
                    break;

                case LogType::INSERT:{
                    curr_table_heap->deleteTuple(record.rid);
                    break;
                }

                case LogType::DELETE:
                    curr_table_heap->insertTuple(record.old_tuple);
                    //curr_table_heap->updateTuple(record.rid, record.old_tuple);
                    break;
            }
        }
    }

    //clear after recovery
    this->clear();  
    
}

vector<Column> CreateUserSchemaa(){
    return{
        Column(TYPE_INT, "user_id", sizeof(int)),
        Column(TYPE_STRING, "firstName", 30),
        Column(TYPE_STRING, "lastName", 30),
        Column(TYPE_INT, "age", sizeof(int))
    };
}

/*
int
main(){

    const char* file_name = "wal.bin";

    DiskManager* dm = new DiskManager("catalog.db");
    BufferPoolManager* BPM = new BufferPoolManager(dm);

    Catalog* catalog = new Catalog(BPM, true);
    catalog->CreateTable("test_wal.db", CreateUserSchemaa());

    WALRecovery* recovery = new WALRecovery(file_name);

    WALManager* wal = new WALManager(recovery, catalog->GetTable("test_wal.db")->table_heap, file_name);

    WALRecord record1(LogType::BEGIN, 1, RID(2, 2), Tuple({Field(TYPE_INT, 12)}), Tuple({Field(TYPE_INT, 18)}));
    WALRecord record2(LogType::INSERT, 1, RID(2, 2), Tuple({}), Tuple({Field(TYPE_INT, 18)}));
    WALRecord record3(LogType::COMMIT, 1, RID(2, 2), Tuple({Field(TYPE_INT, 12)}), Tuple({Field(TYPE_INT, 18)}));

    wal->add_record(record1);
    wal->add_record(record2);
    wal->add_record(record3);

    wal->flush();
    wal->recover();

    
    vector<WALRecord> records = recovery.read_all_records();

    for(auto&record:records) {
        cout<<"txn_id: "<<record.transaction_id<<endl;
        record.rid.print();
        record.old_tuple.print();
        record.new_tuple.print();
    }
    
}
*/