#include"Recovery/WAL_recovery.h"


WALRecovery::WALRecovery(const char* path){
    cout<<path<<endl;
    //file should exist
    fd = open(
        path,
        O_RDONLY | O_CREAT
    );

    if(fd == -1){
        cout << "WAL open failed\n";
        cout << "path = [" << path << "]\n";
        cout << "errno = " << errno << "\n";
        cout << "error = " << strerror(errno) << endl;

        throw runtime_error("file not found");
    }
}

//this is the core function in recovery process 
//it reads wal file from the end till the first commit 
//read every record, deserialize it into a record class
vector<WALRecord> WALRecovery::read_all_records() {

    vector<WALRecord> records;
    //compute file size
    struct stat file_info{};

    if(fstat(fd, &file_info) == -1){
        throw runtime_error("can't get wal size");
    }

    long long file_size = file_info.st_size;
    cout<<file_size<<endl;
    //empty file
    if(file_size == 0){
        return records;
    }
    
    //read the all data in the file, then keep deserializing records till it ends
    vector<char> buffer(file_size);

    int bytes_read = read(fd,buffer.data(),static_cast<unsigned int>(file_size));

    cout<<bytes_read<<endl;

    for(auto&ch:buffer){
        cout<<ch;
    }

    if(bytes_read !=file_size){
        throw runtime_error("can't open walfile");
    }



    size_t offset = 0;

    //till the file ends, deserialize every record
    //then push to the vector 
    while(offset < file_size){
        WALRecord record;

        record.deSerialize_WAL_record(buffer.data()+offset);
        
        record.rid.print();
        record.old_tuple.print();
        record.new_tuple.print();

        records.push_back(record);

        offset += record.get_record_size();
    }


    return records;
}


WALRecovery::~WALRecovery(){
    if(fd != -1){
        close(fd);
        fd = -1;
    }
}
/*int
main(){

}*/