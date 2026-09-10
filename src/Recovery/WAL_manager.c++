#include"WAL_manager.h"


//new instance creates a new log file
WALManager::WALManager(const char* path){
    cout<<path<<endl;
    fd = _open(path, _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _S_IREAD | _S_IWRITE);

    if (fd == -1) {
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
    if(record.type == LogType::COMMIT){
        this->clear();
    }
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
        this->clear();  
    }
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

int
main(){

    const char* file_name = "wal.bin";

    /*
    WALManager wal(file_name);

    WALRecord record(LogType::UPDATE, 1, RID(2, 2), Tuple({Field(TYPE_INT, 12)}), Tuple({Field(TYPE_INT, 18)}));

    wal.add_record(record);

    wal.flush();
    */
    WALRecovery recovery(file_name);
    vector<WALRecord> records = recovery.read_all_records();

    for(auto&record:records) {
        cout<<"txn_id: "<<record.transaction_id<<endl;
        record.rid.print();
        record.old_tuple.print();
        record.new_tuple.print();
    }
}
