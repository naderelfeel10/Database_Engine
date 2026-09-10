
#ifndef WAL_H
#define WAL_H

#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Storage\Table\RID.h"
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Recovery\WAL_record.h"
#include"D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Recovery\WAL_recovery.h"


//an instance of WAL manager creates a wal file on HD, then we append data into it
class WALManager{
private:
    int fd;
    ofstream log_file;

public:
    WALManager(const char* path);
    void add_record(WALRecord& record);
    void flush();

    void clear();
};
#endif