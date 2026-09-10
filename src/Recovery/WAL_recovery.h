#ifndef WAL_RECOVERY_H
#define WAL_RECOVERY_H

#include "D:\SWE\DB\CMU\MY_DB_ENGINE\Minimal_DB_ENGINE\src\Recovery\WAL_record.h"

#include <fcntl.h>
#include <io.h>
#include <vector>
#include <memory>
#include <stdexcept>
using namespace std;

//from row logs, recover
class WALRecovery{

private:
    int fd;

public:

    WALRecovery(const char* path);
    ~WALRecovery();

    vector<WALRecord> read_all_records();
};

#endif